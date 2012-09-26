# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

$(eval $(call gb_Library_use_external,scui,mdds_headers))

$(eval $(call gb_Library_use_libraries,scui,\
	comphelper \
	cppu \
	editeng \
	for \
	forui \
	i18nisolang1 \
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

$(eval $(call gb_Library_add_standard_system_libs,scui))

$(eval $(call gb_Library_add_exception_objects,scui,\
    sc/source/ui/attrdlg/attrdlg \
    sc/source/ui/attrdlg/scdlgfact \
    sc/source/ui/attrdlg/scuiexp \
    sc/source/ui/attrdlg/tabpages \
    sc/source/ui/cctrl/editfield \
    sc/source/ui/condformat/colorformat \
    sc/source/ui/condformat/condformatdlg \
    sc/source/ui/condformat/condformathelper \
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
$(eval $(call gb_Library_use_libraries,scui,\
	advapi32 \
	$(gb_Library_win32_OLDNAMES) \
	uwinapi \
))
endif
# vim: set noet sw=4 ts=4:
