# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
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
#

$(eval $(call gb_Library_Library,pl))

$(eval $(call gb_Library_set_componentfile,pl,extensions/source/plugin/pl))

$(eval $(call gb_Library_use_package,pl,np_sdk_inc))

$(eval $(call gb_Library_use_external,pl,mozilla_headers))

$(eval $(call gb_Library_set_include,pl,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/plugin/inc \
))

$(eval $(call gb_Library_use_sdk_api,pl))

ifeq ($(SOLAR_JAVA),TRUE)
$(eval $(call gb_Library_add_defs,pl,\
	-DOJI \
))

endif # SOLAR_JAVA=YES

ifeq ($(SYSTEM_MOZILLA),YES)
$(eval $(call gb_Library_add_defs,pl,\
	-DSYSTEM_MOZILLA \
))
endif # SYSTEM_MOZILLA=YES

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_libs,pl,\
	-lsocket \
))
endif # OS=SOLARIS

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_use_system_win32_libs,pl,\
	advapi32 \
	ole32 \
	version \
))

$(eval $(call gb_Library_add_exception_objects,pl,\
	extensions/source/plugin/win/winmgr \
	extensions/source/plugin/win/sysplug \
))
endif # GUI=WNT

ifeq ($(GUI),UNX)

ifeq ($(GUIBASE),aqua)

$(eval $(call gb_Library_add_objcxxobjects,pl,\
	extensions/source/plugin/aqua/macmgr \
	extensions/source/plugin/aqua/sysplug \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,pl,\
    Cocoa \
    Carbon \
    CoreFoundation \
))

else # GUIBASE!=aqua

ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_Library_add_defs,pl,\
	-DENABLE_GTK \
))

$(eval $(call gb_Library_use_external,pl,gtk))
endif # ENABLE_GTK=TRUE

$(eval $(call gb_Library_add_exception_objects,pl,\
	extensions/source/plugin/unx/nppapi \
	extensions/source/plugin/unx/sysplug \
	extensions/source/plugin/unx/unxmgr \
))

$(eval $(call gb_Library_use_static_libraries,pl,\
    plugcon \
))

$(eval $(call gb_Library_add_libs,pl,\
	-lX11 \
))

endif # GUIBASE=aqua

endif # GUI=UNX

$(eval $(call gb_Library_use_libraries,pl,\
	tk \
	vcl \
	svl \
	utl \
	tl \
	comphelper \
	ucbhelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,pl,\
	extensions/source/plugin/base/context \
	extensions/source/plugin/base/evtlstnr \
	extensions/source/plugin/base/manager \
	extensions/source/plugin/base/multiplx \
	extensions/source/plugin/base/nfuncs \
	extensions/source/plugin/base/plcom \
	extensions/source/plugin/base/plctrl \
	extensions/source/plugin/base/plmodel \
	extensions/source/plugin/base/service \
	extensions/source/plugin/base/xplugin \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
