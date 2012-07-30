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


$(eval $(call gb_AllLangResTarget_AllLangResTarget,sc))

$(eval $(call gb_AllLangResTarget_add_srs,sc,\
	sc/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sc/res))

$(eval $(call gb_SrsTarget_use_packages,sc/res,\
	editeng_inc \
	formula_inc \
	sfx2_inc \
	svl_inc \
	svx_globlmn_hrc \
	svx_inc \
	tools_inc \
))

$(eval $(call gb_SrsTarget_set_include,sc/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/source/ui/navipi \
	-I$(SRCDIR)/sc/source/ui/docshell \
	-I$(SRCDIR)/sc/source/ui/pagedlg \
	-I$(SRCDIR)/sc/source/ui/dbgui \
	-I$(SRCDIR)/sc/source/ui/formdlg \
))

$(eval $(call gb_SrsTarget_add_files,sc/res,\
    sc/source/ui/src/optdlg.src \
    sc/source/ui/src/popup.src \
    sc/source/ui/src/autofmt.src \
    sc/source/ui/src/globstr.src \
    sc/source/ui/src/optsolver.src \
    sc/source/ui/src/datafdlg.src \
    sc/source/ui/src/toolbox.src \
    sc/source/ui/src/scfuncs.src \
    sc/source/ui/src/textdlgs.src \
    sc/source/ui/src/solvrdlg.src \
    sc/source/ui/src/sc.src \
    sc/source/ui/src/pseudo.src \
    sc/source/ui/src/subtdlg.src \
    sc/source/ui/src/crnrdlg.src \
    sc/source/ui/src/hdrcont.src \
    sc/source/ui/src/scerrors.src \
    sc/source/ui/src/simpref.src \
    sc/source/ui/src/solveroptions.src \
    sc/source/ui/src/namedlg.src \
    sc/source/ui/src/namedefdlg.src \
    sc/source/ui/src/miscdlgs.src \
    sc/source/ui/src/tabopdlg.src \
    sc/source/ui/src/scstring.src \
    sc/source/ui/src/dbnamdlg.src \
    sc/source/ui/src/opredlin.src \
    sc/source/ui/src/sortdlg.src \
    sc/source/ui/src/attrdlg.src \
    sc/source/ui/src/filter.src \
    sc/source/ui/src/colorformat.src \
    sc/source/ui/src/condformatdlg.src \
    sc/source/ui/src/condformatmgr.src \
    sc/source/ui/cctrl/checklistmenu.src \
    sc/source/ui/navipi/navipi.src \
    sc/source/ui/docshell/tpstat.src \
    sc/source/ui/optdlg/calcoptionsdlg.src \
    sc/source/ui/pagedlg/pagedlg.src \
    sc/source/ui/pagedlg/tphf.src \
    sc/source/ui/pagedlg/hfedtdlg.src \
    sc/source/ui/styleui/scstyles.src \
    sc/source/ui/styleui/styledlg.src \
    sc/source/ui/dbgui/asciiopt.src \
    sc/source/ui/dbgui/pivot.src \
    sc/source/ui/dbgui/dpgroupdlg.src \
    sc/source/ui/dbgui/consdlg.src \
    sc/source/ui/dbgui/pvfundlg.src \
    sc/source/ui/dbgui/dapitype.src \
    sc/source/ui/dbgui/imoptdlg.src \
    sc/source/ui/dbgui/textimportoptions.src \
    sc/source/ui/dbgui/scendlg.src \
    sc/source/ui/dbgui/outline.src \
    sc/source/ui/dbgui/validate.src \
    sc/source/ui/drawfunc/drformsh.src \
    sc/source/ui/drawfunc/objdraw.src \
    sc/source/ui/miscdlgs/retypepassdlg.src \
    sc/source/ui/miscdlgs/instbdlg.src \
    sc/source/ui/miscdlgs/highred.src \
    sc/source/ui/miscdlgs/conflictsdlg.src \
    sc/source/ui/miscdlgs/protectiondlg.src \
    sc/source/ui/miscdlgs/acredlin.src \
    sc/source/ui/miscdlgs/sharedocdlg.src \
    sc/source/ui/miscdlgs/linkarea.src \
    sc/source/ui/formdlg/formdlgs.src \
    sc/source/ui/formdlg/dwfunctr.src \
    sc/source/core/src/compiler.src \
))

# vim: set noet sw=4 ts=4:
