# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

$(eval $(call gb_Library_use_packages,inprocserv,\
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
