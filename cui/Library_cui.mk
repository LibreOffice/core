#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,cui))

$(eval $(call gb_Library_add_precompiled_header,cui,$(SRCDIR)/cui/inc/pch/precompiled_cui))

$(eval $(call gb_Library_set_componentfile,cui,cui/util/cui))

$(eval $(call gb_Library_set_include,cui,\
        $$(INCLUDE) \
	-I$(SRCDIR)/cui/inc/pch \
	-I$(SRCDIR)/cui/source/inc \
))

ifneq ($(BUILD_VER_STRING),)
$(eval $(call gb_Library_add_defs,cui,-DBUILD_VER_STRING="$(BUILD_VER_STRING)"))
endif

$(eval $(call gb_Library_add_defs,cui,\
	-DCOPYRIGHT_YEAR=$(shell perl -e 'use POSIX qw(strftime); print strftime("%Y", gmtime($$ENV{SOURCE_DATE_EPOCH} || time));') \
))

ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_Library_add_defs,cui,-DENABLE_GTK))
endif

ifeq ($(ENABLE_KDE),TRUE)
$(eval $(call gb_Library_add_defs,cui,-DENABLE_KDE))
endif

ifeq ($(ENABLE_KDE4),TRUE)
$(eval $(call gb_Library_add_defs,cui,-DENABLE_KDE4))
endif

$(eval $(call gb_Library_add_api,cui,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,cui,\
	avmedia \
	basegfx \
	comphelper \
	cppuhelper \
	cppu \
	editeng \
	i18nisolang1 \
	jvmfwk \
	lng \
	ootk \
	sal \
	sb \
	sax \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	ucbhelper \
	utl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,cui,\
	advapi32 \
	shlwapi \
))
endif

$(call gb_Library_use_external,cui,icuuc)

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
	cui/source/factory/dlgfact \
	cui/source/factory/cuiexp \
	cui/source/factory/cuiresmgr \
	cui/source/factory/init \
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
	cui/source/options/optimprove \
	cui/source/options/optimprove2 \
	cui/source/options/optinet2 \
	cui/source/options/optjava \
	cui/source/options/optjsearch \
	cui/source/options/optlingu \
	cui/source/options/optmemory \
	cui/source/options/optpath \
	cui/source/options/optsave \
	cui/source/options/optupdt \
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
	cui/source/tabpages/border \
	cui/source/tabpages/borderconn \
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
	cui/source/uno/services/services \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_exception_objects,cui,\
	cui/source/dialogs/winpluginlib \
))
endif

# vim: set noet sw=4 ts=4:
