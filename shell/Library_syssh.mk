# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,syssh))

$(eval $(call gb_Library_use_sdk_api,syssh))

$(eval $(call gb_Library_use_libraries,syssh,\
	cppu \
	cppuhelper \
	comphelper \
	sal \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,syssh,\
	ole32 \
	shell32 \
))

$(eval $(call gb_Library_set_componentfile,syssh,shell/source/win32/syssh,services))

$(eval $(call gb_Library_add_exception_objects,syssh,\
	shell/source/win32/SysShExec \
))

else # OS != WNT

$(eval $(call gb_Library_use_static_libraries,syssh,\
	shell_xmlparser \
))

$(eval $(call gb_Library_set_componentfile,syssh,shell/source/unix/exec/syssh,services))

$(eval $(call gb_Library_add_exception_objects,syssh,\
	shell/source/unix/exec/shellexec \
))

endif # OS

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
