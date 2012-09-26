# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,saxparser))

$(eval $(call gb_Executable_set_include,saxparser,\
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_sdk_api,saxparser))

$(eval $(call gb_Executable_use_libraries,saxparser,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
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
