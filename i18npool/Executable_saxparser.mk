# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,saxparser))

$(eval $(call gb_Executable_use_sdk_api,saxparser))

$(eval $(call gb_Executable_use_libraries,saxparser,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,saxparser,\
	i18npool/source/localedata/LocaleNode \
	i18npool/source/localedata/filewriter \
	i18npool/source/localedata/saxparser \
))

ifeq ($(OS),SOLARIS)
# the xmlparser build breaks in this header file
#
#      /opt/SUNWspro/WS6U1/include/CC/Cstd/./limits
#
# which defines a class with member functions called min() and max().
# the build breaks because in solar.h, there is something like this
#
#      #define max(x,y) ((x) < (y) ? (y) : (x))
#      #define min(x,y) ((x) < (y) ? (x) : (y))
#
# so the only choice is to prevent "CC/Cstd/./limits" from being included:
$(eval $(call gb_Executable_add_defs,saxparser,\
	-D__STD_LIMITS \
))
endif

# vim: set noet sw=4 ts=4:
