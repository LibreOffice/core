# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,pl))

$(eval $(call gb_Library_set_componentfile,pl,extensions/source/plugin/pl))

$(eval $(call gb_Library_use_externals,pl,\
	boost_headers \
	npapi_headers \
))

$(eval $(call gb_Library_set_include,pl,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/plugin/inc \
))

$(eval $(call gb_Library_use_sdk_api,pl))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Library_add_defs,pl,\
	-DOJI \
))

endif # ENABLE_JAVA=YES

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

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,pl,\
	advapi32 \
	ole32 \
	version \
))

$(eval $(call gb_Library_add_exception_objects,pl,\
	extensions/source/plugin/win/winmgr \
	extensions/source/plugin/win/sysplug \
))

else ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_add_objcxxobjects,pl,\
	extensions/source/plugin/aqua/macmgr \
	extensions/source/plugin/aqua/sysplug \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,pl,\
    Cocoa \
    Carbon \
    CoreFoundation \
))

else

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

endif

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

# vim:set noet sw=4 ts=4:
