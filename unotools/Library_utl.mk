# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# declare a library
# utl is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,utl))

$(eval $(call gb_Library_use_externals,utl,\
	boost_headers \
	boost_locale \
))

$(eval $(call gb_Library_use_custom_headers,utl,\
 officecfg/registry \
))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,utl,unotools/util/utl))

$(eval $(call gb_Library_set_include,utl, \
    $$(INCLUDE) \
    -I$(SRCDIR)/unotools/inc \
))

# add any additional definitions to be set for compilation here
# (e.g. -DLIB_DLLIMPLEMENTATION)
$(eval $(call gb_Library_add_defs,utl,\
    -DUNOTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,utl,$(SRCDIR)/unotools/inc/pch/precompiled_utl))

$(eval $(call gb_Library_use_sdk_api,utl))

# add libraries to be linked to utl; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_use_libraries,utl,\
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
    tl \
    ucbhelper \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,utl,\
    unotools/source/accessibility/accessiblerelationsethelper \
    unotools/source/accessibility/accessiblestatesethelper \
    unotools/source/config/bootstrap \
    unotools/source/config/cmdoptions \
    unotools/source/config/compatibility \
    unotools/source/config/compatibilityviewoptions \
    unotools/source/config/configitem \
    unotools/source/config/configmgr \
    unotools/source/config/confignode \
    unotools/source/config/configpaths \
    unotools/source/config/configvaluecontainer \
    unotools/source/config/defaultoptions \
    unotools/source/config/docinfohelper \
    unotools/source/config/dynamicmenuoptions \
    unotools/source/config/eventcfg \
    unotools/source/config/extendedsecurityoptions \
    unotools/source/config/fltrcfg \
    unotools/source/config/fontcfg \
    unotools/source/config/fontoptions \
    unotools/source/config/historyoptions \
    unotools/source/config/itemholder1 \
    unotools/source/config/lingucfg \
    unotools/source/config/misccfg \
    unotools/source/config/moduleoptions \
    unotools/source/config/options \
    unotools/source/config/optionsdlg \
    unotools/source/config/pathoptions \
    unotools/source/config/printwarningoptions \
    unotools/source/config/saveopt \
    unotools/source/config/searchopt \
    unotools/source/config/securityoptions \
    unotools/source/config/syslocaleoptions \
    unotools/source/config/useroptions \
    unotools/source/config/viewoptions \
    unotools/source/i18n/calendarwrapper \
    unotools/source/i18n/caserotate \
    unotools/source/i18n/charclass \
    unotools/source/i18n/collatorwrapper \
    unotools/source/i18n/intlwrapper \
    unotools/source/i18n/localedatawrapper \
    unotools/source/i18n/nativenumberwrapper \
    unotools/source/i18n/readwritemutexguard \
    unotools/source/i18n/resmgr \
    unotools/source/i18n/textsearch \
    unotools/source/i18n/transliterationwrapper \
    unotools/source/misc/closeveto \
    unotools/source/misc/datetime \
    unotools/source/misc/desktopterminationobserver \
    unotools/source/misc/eventlisteneradapter \
    unotools/source/misc/fontcvt \
    unotools/source/misc/fontdefs \
    unotools/source/misc/mediadescriptor \
    unotools/source/misc/sharedunocomponent \
    unotools/source/misc/syslocale \
    unotools/source/misc/unotoolsservices \
    unotools/source/misc/wincodepage \
    unotools/source/misc/ServiceDocumenter \
    unotools/source/misc/ZipPackageHelper \
    unotools/source/streaming/streamhelper \
    unotools/source/streaming/streamwrap \
    unotools/source/ucbhelper/localfilehelper \
    unotools/source/ucbhelper/progresshandlerwrap \
    unotools/source/ucbhelper/tempfile \
    unotools/source/ucbhelper/ucbhelper \
    unotools/source/ucbhelper/ucblockbytes \
    unotools/source/ucbhelper/ucbstreamhelper \
    unotools/source/ucbhelper/xtempfile \
))

# vim: set noet sw=4 ts=4:
