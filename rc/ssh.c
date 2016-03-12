/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

static inline int check_host_key(const char *ktype, const char *nvname, const char *hkfn)
{
	unlink(hkfn);

	if (!nvram_get_file(nvname, hkfn, 2048)) {
		eval("dropbearkey", "-t", (char *)ktype, "-f", (char *)hkfn);
		if (nvram_set_file(nvname, hkfn, 2048)) {
			return 1;
		}
	}

	return 0;
}

void start_sshd(void)
{
	int dirty = 0;
	char *argv[11];
	int argc;
	char *p;

	if (!nvram_match("sshd_enable", "1"))
		return;

	mkdir("/etc/dropbear", 0700);
	mkdir("/root/.ssh", 0700);

	f_write_string("/root/.ssh/authorized_keys", nvram_safe_get("sshd_authkeys"), 0, 0700);

	dirty |= check_host_key("rsa", "sshd_hostkey", "/etc/dropbear/dropbear_rsa_host_key");
	dirty |= check_host_key("dss", "sshd_dsskey",  "/etc/dropbear/dropbear_dss_host_key");
	if (dirty)
		nvram_commit_x();

	argv[0] = "dropbear";
	argv[1] = "-p";
	argv[2] = nvram_safe_get("sshd_port");
	argc = 3;

	if (!nvram_get_int("sshd_pass")) argv[argc++] = "-s";
	
	argv[argc++] = "-a";

	if (((p = nvram_get("sshd_rwb")) != NULL) && (*p)) {
		argv[argc++] = "-W";
		argv[argc++] = p;
	}

	argv[argc] = NULL;
	_eval(argv, NULL, 0, NULL);
}

void stop_sshd(void)
{
	killall("dropbear", SIGTERM);
}
