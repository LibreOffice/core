# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


$(eval $(call gb_AllLangResTarget_AllLangResTarget,sc))

$(eval $(call gb_AllLangResTarget_add_srs,sc,\
	sc/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sc/res))

$(eval $(call gb_SrsTarget_use_srstargets,sc/res,\
	svx/res \
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
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
))

$(eval $(call gb_SrsTarget_add_files,sc/res,\
    sc/source/ui/src/iconsets.src \
    sc/source/ui/src/popup.src \
    sc/source/ui/src/autofmt.src \
    sc/source/ui/src/globstr.src \
    sc/source/ui/src/optsolver.src \
    sc/source/ui/src/toolbox.src \
    sc/source/ui/src/scfuncs.src \
    sc/source/ui/src/textdlgs.src \
    sc/source/ui/src/sc.src \
    sc/source/ui/src/pseudo.src \
    sc/source/ui/src/subtdlg.src \
    sc/source/ui/src/crnrdlg.src \
    sc/source/ui/src/hdrcont.src \
    sc/source/ui/src/scerrors.src \
    sc/source/ui/src/simpref.src \
    sc/source/ui/src/solveroptions.src \
    sc/source/ui/src/miscdlgs.src \
    sc/source/ui/src/tabopdlg.src \
    sc/source/ui/src/scstring.src \
    sc/source/ui/src/dbnamdlg.src \
    sc/source/ui/src/sortdlg.src \
    sc/source/ui/src/filter.src \
    sc/source/ui/src/colorformat.src \
    sc/source/ui/src/condformatdlg.src \
    sc/source/ui/src/condformatmgr.src \
    sc/source/ui/src/xmlsourcedlg.src \
    sc/source/ui/cctrl/checklistmenu.src \
    sc/source/ui/navipi/navipi.src \
    sc/source/ui/docshell/tpstat.src \
    sc/source/ui/pagedlg/pagedlg.src \
    sc/source/ui/styleui/scstyles.src \
    sc/source/ui/styleui/styledlg.src \
    sc/source/ui/dbgui/pivot.src \
    sc/source/ui/dbgui/dpgroupdlg.src \
    sc/source/ui/dbgui/pvfundlg.src \
    sc/source/ui/dbgui/dapitype.src \
    sc/source/ui/dbgui/imoptdlg.src \
    sc/source/ui/dbgui/scendlg.src \
    sc/source/ui/dbgui/outline.src \
    sc/source/ui/dbgui/validate.src \
    sc/source/ui/drawfunc/drformsh.src \
    sc/source/ui/drawfunc/objdraw.src \
    sc/source/ui/miscdlgs/retypepassdlg.src \
    sc/source/ui/miscdlgs/highred.src \
    sc/source/ui/miscdlgs/conflictsdlg.src \
    sc/source/ui/miscdlgs/acredlin.src \
    sc/source/ui/formdlg/formdlgs.src \
    sc/source/ui/formdlg/dwfunctr.src \
    sc/source/ui/sidebar/CellAppearancePropertyPanel.src \
    sc/source/ui/StatisticsDialogs/RandomNumberGeneratorDialog.src \
    sc/source/ui/StatisticsDialogs/SamplingDialog.src \
    sc/source/ui/StatisticsDialogs/DescriptiveStatisticsDialog.src \
    sc/source/core/src/compiler.src \
))

# vim: set noet sw=4 ts=4:
