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

$(eval $(call gb_Library_Library,cui))

$(eval $(call gb_Library_set_componentfile,cui,cui/util/cui))

$(eval $(call gb_Library_set_include,cui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/cui/source/inc \
))

$(eval $(call gb_Library_add_defs,cui,\
    $(if $(filter TRUE,$(ENABLE_GTK)),-DENABLE_GTK) \
    $(if $(filter TRUE,$(ENABLE_TDE)),-DENABLE_TDE) \
    $(if $(filter TRUE,$(ENABLE_KDE)),-DENABLE_KDE) \
    $(if $(filter TRUE,$(ENABLE_KDE4)),-DENABLE_KDE4) \
    $(if $(EXTRA_BUILDID),-DEXTRA_BUILDID=\""$(EXTRA_BUILDID)"\") \
))

$(eval $(call gb_Library_use_sdk_api,cui))

$(eval $(call gb_Library_use_libraries,cui,\
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nisolang1 \
    jvmfwk \
    lng \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,cui,\
    icuuc \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_use_system_win32_libs,cui,\
    advapi32 \
    shlwapi \
))
endif

$(eval $(call gb_Library_add_exception_objects,cui,\
    cui/source/customize/acccfg \
    cui/source/customize/cfg \
    cui/source/customize/cfgutil \
    cui/source/customize/eventdlg \
    cui/source/customize/macropg \
    cui/source/customize/selector \
    cui/source/dialogs/about \
    cui/source/dialogs/colorpicker \
    cui/source/dialogs/commonlingui \
    cui/source/dialogs/cuicharmap \
    cui/source/dialogs/cuifmsearch \
    cui/source/dialogs/cuigaldlg \
    cui/source/dialogs/cuigrfflt \
    cui/source/dialogs/cuihyperdlg \
    cui/source/dialogs/cuiimapwnd \
    cui/source/dialogs/cuitbxform \
    cui/source/dialogs/dlgname \
    cui/source/dialogs/hangulhanjadlg \
    cui/source/dialogs/hldocntp \
    cui/source/dialogs/hldoctp \
    cui/source/dialogs/hlinettp \
    cui/source/dialogs/hlmailtp \
    cui/source/dialogs/hlmarkwn \
    cui/source/dialogs/hltpbase \
    cui/source/dialogs/hyphen \
    cui/source/dialogs/iconcdlg \
    cui/source/dialogs/insdlg \
    cui/source/dialogs/insrc \
    cui/source/dialogs/linkdlg \
    cui/source/dialogs/multifil \
    cui/source/dialogs/multipat \
    cui/source/dialogs/newtabledlg \
    cui/source/dialogs/passwdomdlg \
    cui/source/dialogs/pastedlg \
    cui/source/dialogs/plfilter \
    cui/source/dialogs/postdlg \
    cui/source/dialogs/scriptdlg \
    cui/source/dialogs/sdrcelldlg \
    cui/source/dialogs/showcols \
    cui/source/dialogs/SpellAttrib \
    cui/source/dialogs/SpellDialog \
    cui/source/dialogs/splitcelldlg \
    cui/source/dialogs/srchxtra \
    cui/source/dialogs/thesdlg \
    cui/source/dialogs/zoom \
    cui/source/factory/cuiexp \
    cui/source/factory/cuiresmgr \
    cui/source/factory/dlgfact \
    cui/source/factory/init \
    cui/source/options/certpath \
    cui/source/options/cfgchart \
    cui/source/options/connpoolconfig \
    cui/source/options/connpooloptions \
    cui/source/options/connpoolsettings \
    cui/source/options/cuisrchdlg \
    cui/source/options/dbregister \
    cui/source/options/dbregisterednamesconfig \
    cui/source/options/dbregistersettings \
    cui/source/options/doclinkdialog \
    cui/source/options/fontsubs \
    cui/source/options/internationaloptions \
    cui/source/options/optaccessibility \
    cui/source/options/optasian \
    cui/source/options/optchart \
    cui/source/options/optcolor \
    cui/source/options/optctl \
    cui/source/options/optdict \
    cui/source/options/optfltr \
    cui/source/options/optgdlg \
    cui/source/options/optgenrl \
    cui/source/options/optHeaderTabListbox \
    cui/source/options/opthtml \
    cui/source/options/optinet2 \
    cui/source/options/optjava \
    cui/source/options/optjsearch \
    cui/source/options/optlingu \
    cui/source/options/optmemory \
    cui/source/options/optpath \
    cui/source/options/optsave \
    cui/source/options/optupdt \
    cui/source/options/personalization \
    cui/source/options/radiobtnbox \
    cui/source/options/readonlyimage \
    cui/source/options/sdbcdriverenum \
    cui/source/options/securityoptions \
    cui/source/options/treeopt \
    cui/source/options/webconninfo \
    cui/source/tabpages/align \
    cui/source/tabpages/autocdlg \
    cui/source/tabpages/backgrnd \
    cui/source/tabpages/bbdlg \
    cui/source/tabpages/borderconn \
    cui/source/tabpages/border \
    cui/source/tabpages/chardlg \
    cui/source/tabpages/connect \
    cui/source/tabpages/dstribut \
    cui/source/tabpages/grfpage \
    cui/source/tabpages/labdlg \
    cui/source/tabpages/macroass \
    cui/source/tabpages/measure \
    cui/source/tabpages/numfmt \
    cui/source/tabpages/numpages \
    cui/source/tabpages/page \
    cui/source/tabpages/paragrph \
    cui/source/tabpages/swpossizetabpage \
    cui/source/tabpages/tabarea \
    cui/source/tabpages/tabline \
    cui/source/tabpages/tabstpge \
    cui/source/tabpages/textanim \
    cui/source/tabpages/textattr \
    cui/source/tabpages/tparea \
    cui/source/tabpages/tpbitmap \
    cui/source/tabpages/tpcolor \
    cui/source/tabpages/tpgradnt \
    cui/source/tabpages/tphatch \
    cui/source/tabpages/tpline \
    cui/source/tabpages/tplnedef \
    cui/source/tabpages/tplneend \
    cui/source/tabpages/tpshadow \
    cui/source/tabpages/transfrm \
    cui/source/uno/services \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_exception_objects,cui,\
    cui/source/dialogs/winpluginlib \
))
endif

# vim: set noet sw=4 ts=4:
