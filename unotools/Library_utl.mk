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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

# declare a library
# utl is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,utl))

# declare packages that will be delivered before compilation of utl
# learn more about TYPE in the Package.mk template
$(eval $(call gb_Library_use_package,utl,unotools_inc))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,utl,unotools/util/utl))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,utl,\
    $$(INCLUDE) \
))

# add any additional definitions to be set for compilation here
# (e.g. -DLIB_DLLIMPLEMENTATION)
$(eval $(call gb_Library_add_defs,utl,\
    -DUNOTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,utl))

# add libraries to be linked to utl; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_use_libraries,utl,\
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    sal \
    salhelper \
    tl \
    ucbhelper \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,utl,\
    unotools/source/accessibility/accessiblerelationsethelper \
    unotools/source/accessibility/accessiblestatesethelper \
    unotools/source/config/accelcfg \
    unotools/source/config/bootstrap \
    unotools/source/config/cmdoptions \
    unotools/source/config/compatibility \
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
    unotools/source/config/internaloptions \
    unotools/source/config/itemholder1 \
    unotools/source/config/lingucfg \
    unotools/source/config/localisationoptions \
    unotools/source/config/misccfg \
    unotools/source/config/moduleoptions \
    unotools/source/config/options \
    unotools/source/config/optionsdlg \
    unotools/source/config/pathoptions \
    unotools/source/config/printwarningoptions \
    unotools/source/config/saveopt \
    unotools/source/config/searchopt \
    unotools/source/config/securityoptions \
    unotools/source/config/startoptions \
    unotools/source/config/syslocaleoptions \
    unotools/source/config/useroptions \
    unotools/source/config/viewoptions \
    unotools/source/config/workingsetoptions \
    unotools/source/config/xmlaccelcfg \
    unotools/source/i18n/calendarwrapper \
    unotools/source/i18n/caserotate \
    unotools/source/i18n/charclass \
    unotools/source/i18n/collatorwrapper \
    unotools/source/i18n/intlwrapper \
    unotools/source/i18n/localedatawrapper \
    unotools/source/i18n/nativenumberwrapper \
    unotools/source/i18n/numberformatcodewrapper \
    unotools/source/i18n/readwritemutexguard \
    unotools/source/i18n/textsearch \
    unotools/source/i18n/transliterationwrapper \
    unotools/source/misc/atom \
    unotools/source/misc/closeveto \
    unotools/source/misc/componentresmodule \
    unotools/source/misc/datetime \
    unotools/source/misc/desktopterminationobserver \
    unotools/source/misc/eventlisteneradapter \
    unotools/source/misc/fontcvt \
    unotools/source/misc/fontdefs \
    unotools/source/misc/sharedunocomponent \
    unotools/source/misc/syslocale \
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
