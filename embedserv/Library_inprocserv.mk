# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,inprocserv))

$(eval $(call gb_Library_set_include,inprocserv,\
	-I$(SRCDIR)/embedserv/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_system_win32_libs,inprocserv,\
	gdi32 \
	ole32 \
	uuid \
))

$(eval $(call gb_Library_use_libraries,inprocserv,\
	$(gb_UWINAPI) \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,inprocserv,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/EXPORT:DllMain \
	/EXPORT:DllRegisterServer \
	/EXPORT:DllUnregisterServer \
))
endif

$(eval $(call gb_Library_add_exception_objects,inprocserv,\
	embedserv/source/inprocserv/advisesink \
	embedserv/source/inprocserv/dllentry \
	embedserv/source/inprocserv/inprocembobj \
))

# vim: set noet sw=4 ts=4:
