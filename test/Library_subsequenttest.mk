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
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
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

$(eval $(call gb_Library_Library,subsequenttest))

$(eval $(call gb_Library_add_defs,subsequenttest,\
    -DOOO_DLLIMPLEMENTATION_TEST \
))

$(eval $(call gb_Library_use_sdk_api,subsequenttest))

$(eval $(call gb_Library_use_libraries,subsequenttest,\
    comphelper \
    cppu \
    cppuhelper \
	i18nisolang1 \
    sal \
    test \
	tl \
	utl \
	unotest \
	vcl \
    $(gb_STDLIBS) \
))

ifeq ($(GUIBASE),unx)
$(call gb_Library_get_target,subsequenttest) : \
    $(call gb_Library_get_target,desktop_detector) \
    $(call gb_Library_get_target,vclplug_svp) \

endif

$(eval $(call gb_Library_use_external,subsequenttest,cppunit))

$(eval $(call gb_Library_add_exception_objects,subsequenttest,\
    test/source/unoapi_test \
    test/source/beans/xpropertyset \
    test/source/container/xelementaccess \
    test/source/container/xindexaccess \
    test/source/container/xnamecontainer \
    test/source/container/xnamereplace \
    test/source/container/xnamed \
    test/source/sheet/cellproperties \
    test/source/sheet/datapilotfield \
    test/source/sheet/xcellrangedata \
    test/source/sheet/xcellrangereferrer \
    test/source/sheet/xcellrangesquery \
    test/source/sheet/xdatabaserange \
    test/source/sheet/xdatapilotdescriptor \
    test/source/sheet/xdatapilotfieldgrouping \
    test/source/sheet/xdatapilottable \
    test/source/sheet/xdatapilottable2 \
    test/source/sheet/xnamedrange \
    test/source/sheet/xnamedranges \
    test/source/sheet/xprintareas \
    test/source/sheet/xspreadsheetdocument \
    test/source/sheet/xspreadsheets2 \
    test/source/text/xtext \
    test/source/text/xtextfield \
    test/source/text/xtextcontent \
    test/source/util/xreplaceable \
    test/source/util/xsearchable \
))

# vim: set noet sw=4 ts=4:
