# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scui))

$(eval $(call gb_Library_set_include,scui,\
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/inc \
	-I$(WORKDIR)/SdiTarget/sc/sdi \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,scui))

$(eval $(call gb_Library_use_externals,scui,\
	boost_headers \
	mdds_headers \
))

$(eval $(call gb_Library_use_libraries,scui,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	forui \
	i18nlangtag \
	sal \
	sc \
	sfx \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	utl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,scui,\
    sc/source/ui/attrdlg/attrdlg \
    sc/source/ui/attrdlg/scdlgfact \
    sc/source/ui/attrdlg/scuiexp \
    sc/source/ui/attrdlg/tabpages \
    sc/source/ui/cctrl/editfield \
    sc/source/ui/condformat/condformatmgr \
    sc/source/ui/dbgui/dapidata \
    sc/source/ui/dbgui/dapitype \
    sc/source/ui/dbgui/dpgroupdlg \
    sc/source/ui/dbgui/pfiltdlg \
    sc/source/ui/dbgui/pvfundlg \
    sc/source/ui/dbgui/scendlg \
    sc/source/ui/dbgui/scuiasciiopt \
    sc/source/ui/dbgui/scuiimoptdlg \
    sc/source/ui/dbgui/sortdlg \
    sc/source/ui/dbgui/sortkeydlg \
    sc/source/ui/dbgui/subtdlg \
    sc/source/ui/dbgui/textimportoptions \
    sc/source/ui/dbgui/tpsort \
    sc/source/ui/dbgui/tpsubt \
    sc/source/ui/dbgui/validate \
    sc/source/ui/docshell/tpstat \
    sc/source/ui/miscdlgs/crdlg \
    sc/source/ui/miscdlgs/datafdlg \
    sc/source/ui/miscdlgs/delcldlg \
    sc/source/ui/miscdlgs/delcodlg \
    sc/source/ui/miscdlgs/filldlg \
    sc/source/ui/miscdlgs/groupdlg \
    sc/source/ui/miscdlgs/inscldlg \
    sc/source/ui/miscdlgs/inscodlg \
    sc/source/ui/miscdlgs/instbdlg \
    sc/source/ui/miscdlgs/lbseldlg \
    sc/source/ui/miscdlgs/linkarea \
    sc/source/ui/miscdlgs/mtrindlg \
    sc/source/ui/miscdlgs/mvtabdlg \
    sc/source/ui/miscdlgs/namecrea \
    sc/source/ui/miscdlgs/scuiautofmt \
    sc/source/ui/miscdlgs/shtabdlg \
    sc/source/ui/miscdlgs/strindlg \
    sc/source/ui/miscdlgs/tabbgcolordlg \
    sc/source/ui/miscdlgs/textdlgs \
    sc/source/ui/namedlg/namepast \
    sc/source/ui/optdlg/calcoptionsdlg \
    sc/source/ui/optdlg/opredlin \
    sc/source/ui/optdlg/tpcalc \
    sc/source/ui/optdlg/tpcompatibility \
    sc/source/ui/optdlg/tpformula \
    sc/source/ui/optdlg/tpdefaults \
    sc/source/ui/optdlg/tpprint \
    sc/source/ui/optdlg/tpusrlst \
    sc/source/ui/optdlg/tpview \
    sc/source/ui/pagedlg/hfedtdlg \
    sc/source/ui/pagedlg/scuitphfedit \
    sc/source/ui/pagedlg/tphf \
    sc/source/ui/pagedlg/tptable \
    sc/source/ui/styleui/styledlg \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,scui,\
	advapi32 \
	oldnames \
))
endif

# vim: set noet sw=4 ts=4:
