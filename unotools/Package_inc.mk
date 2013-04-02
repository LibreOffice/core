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

$(eval $(call gb_Package_Package,unotools_inc,$(SRCDIR)/unotools/inc))

$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/accessiblerelationsethelper.hxx,unotools/accessiblerelationsethelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/accessiblestatesethelper.hxx,unotools/accessiblestatesethelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/atom.hxx,unotools/atom.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/bootstrap.hxx,unotools/bootstrap.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/calendarwrapper.hxx,unotools/calendarwrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/caserotate.hxx,unotools/caserotate.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/charclass.hxx,unotools/charclass.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/closeveto.hxx,unotools/closeveto.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/cmdoptions.hxx,unotools/cmdoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/collatorwrapper.hxx,unotools/collatorwrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/compatibility.hxx,unotools/compatibility.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/componentresmodule.hxx,unotools/componentresmodule.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/configitem.hxx,unotools/configitem.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/configmgr.hxx,unotools/configmgr.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/confignode.hxx,unotools/confignode.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/configpaths.hxx,unotools/configpaths.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/configvaluecontainer.hxx,unotools/configvaluecontainer.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/datetime.hxx,unotools/datetime.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/defaultoptions.hxx,unotools/defaultoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/desktopterminationobserver.hxx,unotools/desktopterminationobserver.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/digitgroupingiterator.hxx,unotools/digitgroupingiterator.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/docinfohelper.hxx,unotools/docinfohelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/dynamicmenuoptions.hxx,unotools/dynamicmenuoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/eventcfg.hxx,unotools/eventcfg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/eventlisteneradapter.hxx,unotools/eventlisteneradapter.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/extendedsecurityoptions.hxx,unotools/extendedsecurityoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/fltrcfg.hxx,unotools/fltrcfg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/fontcfg.hxx,unotools/fontcfg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/fontcvt.hxx,unotools/fontcvt.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/fontdefs.hxx,unotools/fontdefs.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/fontoptions.hxx,unotools/fontoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/historyoptions.hxx,unotools/historyoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/idhelper.hxx,unotools/idhelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/intlwrapper.hxx,unotools/intlwrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/itemholderbase.hxx,unotools/itemholderbase.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/lingucfg.hxx,unotools/lingucfg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/linguprops.hxx,unotools/linguprops.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/localedatawrapper.hxx,unotools/localedatawrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/localfilehelper.hxx,unotools/localfilehelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/localisationoptions.hxx,unotools/localisationoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/misccfg.hxx,unotools/misccfg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/moduleoptions.hxx,unotools/moduleoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/nativenumberwrapper.hxx,unotools/nativenumberwrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/numberformatcodewrapper.hxx,unotools/numberformatcodewrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/optionsdlg.hxx,unotools/optionsdlg.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/options.hxx,unotools/options.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/pathoptions.hxx,unotools/pathoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/printwarningoptions.hxx,unotools/printwarningoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/progresshandlerwrap.hxx,unotools/progresshandlerwrap.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/readwritemutexguard.hxx,unotools/readwritemutexguard.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/saveopt.hxx,unotools/saveopt.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/searchopt.hxx,unotools/searchopt.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/securityoptions.hxx,unotools/securityoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/sharedunocomponent.hxx,unotools/sharedunocomponent.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/streamhelper.hxx,unotools/streamhelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/streamsection.hxx,unotools/streamsection.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/streamwrap.hxx,unotools/streamwrap.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/syslocale.hxx,unotools/syslocale.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/syslocaleoptions.hxx,unotools/syslocaleoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/tempfile.hxx,unotools/tempfile.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/textsearch.hxx,unotools/textsearch.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/transliterationwrapper.hxx,unotools/transliterationwrapper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/ucbhelper.hxx,unotools/ucbhelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/ucblockbytes.hxx,unotools/ucblockbytes.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/ucbstreamhelper.hxx,unotools/ucbstreamhelper.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/unotoolsdllapi.h,unotools/unotoolsdllapi.h))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/useroptions.hxx,unotools/useroptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/viewoptions.hxx,unotools/viewoptions.hxx))
$(eval $(call gb_Package_add_file,unotools_inc,inc/unotools/xmlaccelcfg.hxx,unotools/xmlaccelcfg.hxx))

# vim: set noet sw=4 ts=4:
