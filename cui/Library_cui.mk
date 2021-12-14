# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cui))

$(eval $(call gb_Library_set_plugin_for,cui,vcl))

$(eval $(call gb_Library_set_componentfile,cui,cui/util/cui,services))

$(eval $(call gb_Library_set_include,cui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/cui/inc \
    -I$(SRCDIR)/cui/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,cui,cui/inc/pch/precompiled_cui))

$(eval $(call gb_Library_add_defs,cui,\
    -DCUI_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_custom_headers,cui,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,cui))

$(eval $(call gb_Library_use_libraries,cui,\
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
    i18nutil \
    $(if $(ENABLE_JAVA), \
        jvmfwk) \
    lng \
	$(call gb_Helper_optional,OPENCL, \
		opencl) \
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
    $(if $(ENABLE_BREAKPAD), \
        crashreport \
    ) \
))

$(eval $(call gb_Library_use_externals,cui,\
	boost_headers \
	$(call gb_Helper_optional,OPENCL,\
		clew) \
    icuuc \
    icu_headers \
    libxml2 \
    orcus-parser \
    orcus \
    zxing \
))
ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,cui,\
     epoxy \
 ))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,cui,\
    advapi32 \
    shlwapi \
))

$(eval $(call gb_Library_add_exception_objects,cui,\
    cui/source/dialogs/fileextcheckdlg \
))
endif

$(eval $(call gb_Library_add_exception_objects,cui,\
    cui/source/customize/acccfg \
    cui/source/customize/cfg \
    cui/source/customize/cfgutil \
    cui/source/customize/CommandCategoryListBox \
    cui/source/customize/eventdlg \
    cui/source/customize/macropg \
    cui/source/customize/SvxConfigPageHelper \
    cui/source/customize/SvxMenuConfigPage \
    cui/source/customize/SvxToolbarConfigPage \
    cui/source/customize/SvxNotebookbarConfigPage \
    cui/source/customize/CustomNotebookbarGenerator \
    cui/source/dialogs/about \
    $(call gb_Helper_optional,EXTENSIONS, \
        cui/source/dialogs/AdditionsDialog) \
    cui/source/dialogs/colorpicker \
    cui/source/dialogs/cuicharmap \
    cui/source/dialogs/cuifmsearch \
    cui/source/dialogs/cuigaldlg \
    cui/source/dialogs/cuigrfflt \
    cui/source/dialogs/cuihyperdlg \
    cui/source/dialogs/cuiimapwnd \
    cui/source/dialogs/cuitbxform \
    cui/source/dialogs/dlgname \
    cui/source/dialogs/DiagramDialog \
    cui/source/dialogs/FontFeaturesDialog \
    cui/source/dialogs/hangulhanjadlg \
    cui/source/dialogs/hldocntp \
    cui/source/dialogs/hldoctp \
    cui/source/dialogs/hlinettp \
    cui/source/dialogs/hlmailtp \
    cui/source/dialogs/hlmarkwn \
    cui/source/dialogs/hltpbase \
    cui/source/dialogs/hyphen \
    cui/source/dialogs/iconcdlg \
    cui/source/dialogs/tipofthedaydlg \
    cui/source/dialogs/insdlg \
    cui/source/dialogs/insrc \
    cui/source/dialogs/linkdlg \
    cui/source/dialogs/multipat \
    cui/source/dialogs/newtabledlg \
    cui/source/dialogs/passwdomdlg \
    cui/source/dialogs/screenshotannotationdlg \
    cui/source/dialogs/pastedlg \
    cui/source/dialogs/postdlg \
    cui/source/dialogs/QrCodeGenDialog \
    cui/source/dialogs/GraphicTestsDialog \
    cui/source/dialogs/ImageViewerDialog \
    cui/source/dialogs/scriptdlg \
    cui/source/dialogs/SignatureLineDialogBase \
    cui/source/dialogs/SignatureLineDialog \
    cui/source/dialogs/SignSignatureLineDialog \
    cui/source/dialogs/sdrcelldlg \
    cui/source/dialogs/showcols \
    cui/source/dialogs/SpellDialog \
    cui/source/dialogs/splitcelldlg \
    cui/source/dialogs/srchxtra \
    cui/source/dialogs/thesdlg \
    cui/source/dialogs/toolbarmodedlg \
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
	cui/source/options/optaboutconfig \
    cui/source/options/optaccessibility \
    cui/source/options/optasian \
    $(call gb_Helper_optional,SCRIPTING, \
        cui/source/options/optbasic) \
    cui/source/options/optchart \
    cui/source/options/optcolor \
    cui/source/options/optctl \
    cui/source/options/optdict \
    cui/source/options/optfltr \
    cui/source/options/optgdlg \
    cui/source/options/optgenrl \
    cui/source/options/opthtml \
    cui/source/options/optinet2 \
    cui/source/options/optjava \
    cui/source/options/optjsearch \
    cui/source/options/optlingu \
	$(call gb_Helper_optional,OPENCL, \
	    cui/source/options/optopencl) \
    cui/source/options/optpath \
    cui/source/options/optsave \
    cui/source/options/optupdt \
    $(call gb_Helper_optional,DESKTOP,\
        cui/source/options/personalization) \
    cui/source/options/sdbcdriverenum \
    cui/source/options/securityoptions \
    cui/source/options/treeopt \
    cui/source/options/tsaurls \
    cui/source/options/webconninfo \
    cui/source/tabpages/align \
    cui/source/tabpages/autocdlg \
    cui/source/tabpages/backgrnd \
    cui/source/tabpages/bbdlg \
    cui/source/tabpages/border \
    cui/source/tabpages/chardlg \
    cui/source/tabpages/connect \
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
    cui/source/tabpages/TextColumnsPage \
    cui/source/tabpages/themepage \
    cui/source/tabpages/tparea \
    cui/source/tabpages/tpbitmap \
    cui/source/tabpages/tpcolor \
    cui/source/tabpages/tpgradnt \
    cui/source/tabpages/tphatch \
    cui/source/tabpages/tpline \
    cui/source/tabpages/tplnedef \
    cui/source/tabpages/tplneend \
    cui/source/tabpages/tppattern \
    cui/source/tabpages/tpshadow \
    cui/source/tabpages/tptrans \
    cui/source/tabpages/transfrm \
    cui/source/util/FontFeatures \
))

# vim: set noet sw=4 ts=4:
