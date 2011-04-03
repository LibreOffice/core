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
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
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

$(eval $(call gb_Library_Library,utl))

$(eval $(call gb_Library_add_package_headers,utl,unotools_inc))

$(eval $(call gb_Library_add_precompiled_header,utl,$(SRCDIR)/unotools/inc/pch/precompiled_unotools))

$(eval $(call gb_Library_set_include,utl,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/unotools/inc/pch) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,utl,\
    $$(DEFS) \
    -DUNOTOOLS_DLLIMPLEMENTATION \
    $(if $(filter TRUE,$(ENABLE_BROFFICE)),-DENABLE_BROFFICE) \
))

$(eval $(call gb_Library_add_linked_libs,utl,\
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    sal \
    salhelper \
    tl \
    ucbhelper \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,utl,unotools/util/utl))

$(eval $(call gb_Library_add_exception_objects,utl,\
    unotools/source/accessibility/accessiblerelationsethelper \
    unotools/source/accessibility/accessiblestatesethelper \
    unotools/source/config/accelcfg \
    unotools/source/config/bootstrap \
    unotools/source/config/cacheoptions \
    unotools/source/config/cmdoptions \
    unotools/source/config/compatibility \
    unotools/source/config/configitem \
    unotools/source/config/configmgr \
    unotools/source/config/confignode \
    unotools/source/config/configpathes \
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
    unotools/source/config/inetoptions \
    unotools/source/config/internaloptions \
    unotools/source/config/itemholder1 \
    unotools/source/config/javaoptions \
    unotools/source/config/lingucfg \
    unotools/source/config/localisationoptions \
    unotools/source/config/misccfg \
    unotools/source/config/moduleoptions \
    unotools/source/config/options \
    unotools/source/config/optionsdlg \
    unotools/source/config/pathoptions \
    unotools/source/config/printwarningoptions \
    unotools/source/config/regoptions \
    unotools/source/config/saveopt \
    unotools/source/config/searchopt \
    unotools/source/config/securityoptions \
    unotools/source/config/sourceviewconfig \
    unotools/source/config/startoptions \
    unotools/source/config/syslocaleoptions \
    unotools/source/config/undoopt \
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
    unotools/source/misc/componentresmodule \
    unotools/source/misc/datetime \
    unotools/source/misc/desktopterminationobserver \
    unotools/source/misc/eventlisteneradapter \
    unotools/source/misc/fontcvt \
    unotools/source/misc/fontdefs \
    unotools/source/misc/sharedunocomponent \
    unotools/source/misc/syslocale \
    unotools/source/processfactory/processfactory \
    unotools/source/property/propertysethelper \
    unotools/source/property/propertysetinfo \
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
