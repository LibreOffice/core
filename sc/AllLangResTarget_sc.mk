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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,sc))

$(eval $(call gb_AllLangResTarget_set_imagelocations,sc,\
	sc/res/imglst/apptbx \
	sc/res/imglst/dbgui \
	sc/res/imglst/navipi \
))

$(eval $(call gb_AllLangResTarget_add_srs,sc,sc/res))

$(eval $(call gb_SrsTarget_SrsTarget,sc/res))

$(eval $(call gb_SrsTarget_set_include,sc/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,sc/res,\
	sc/source/ui/miscdlgs/instbdlg.src \
	sc/source/ui/miscdlgs/acredlin.src \
	sc/source/ui/miscdlgs/highred.src \
	sc/source/ui/miscdlgs/linkarea.src \
	sc/source/ui/miscdlgs/conflictsdlg.src \
	sc/source/ui/miscdlgs/sharedocdlg.src \
	sc/source/ui/miscdlgs/protectiondlg.src \
	sc/source/ui/miscdlgs/retypepassdlg.src \
	sc/source/ui/docshell/tpstat.src \
	sc/source/ui/src/sc.src \
	sc/source/ui/src/pseudo.src \
	sc/source/ui/src/popup.src \
	sc/source/ui/src/toolbox.src \
	sc/source/ui/src/scstring.src \
	sc/source/ui/src/attrdlg.src \
	sc/source/ui/src/sortdlg.src \
	sc/source/ui/src/filter.src \
	sc/source/ui/src/namedlg.src \
	sc/source/ui/src/dbnamdlg.src \
	sc/source/ui/src/subtdlg.src \
	sc/source/ui/src/miscdlgs.src \
	sc/source/ui/src/autofmt.src \
	sc/source/ui/src/solvrdlg.src \
	sc/source/ui/src/optsolver.src \
	sc/source/ui/src/solveroptions.src \
	sc/source/ui/src/tabopdlg.src \
	sc/source/ui/src/hdrcont.src \
	sc/source/ui/src/globstr.src \
	sc/source/ui/src/optdlg.src \
	sc/source/ui/src/scerrors.src \
	sc/source/ui/src/textdlgs.src \
	sc/source/ui/src/scfuncs.src \
	sc/source/ui/src/crnrdlg.src \
	sc/source/ui/src/condfrmt.src \
	sc/source/ui/src/opredlin.src \
	sc/source/ui/src/simpref.src \
	sc/source/ui/dbgui/textimportoptions.src \
	sc/source/ui/dbgui/pivot.src \
	sc/source/ui/dbgui/pvfundlg.src \
	sc/source/ui/dbgui/dpgroupdlg.src \
	sc/source/ui/dbgui/dapitype.src \
	sc/source/ui/dbgui/consdlg.src \
	sc/source/ui/dbgui/scendlg.src \
	sc/source/ui/dbgui/imoptdlg.src \
	sc/source/ui/dbgui/validate.src \
	sc/source/ui/dbgui/asciiopt.src \
	sc/source/ui/dbgui/outline.src \
	sc/source/ui/drawfunc/objdraw.src \
	sc/source/ui/drawfunc/drformsh.src \
	sc/source/ui/sidebar/AlignmentPropertyPanel.src \
	sc/source/ui/sidebar/CellAppearancePropertyPanel.src \
	sc/source/ui/sidebar/NumberFormatPropertyPanel.src \
	sc/source/core/src/compiler.src \
	sc/source/ui/styleui/scstyles.src \
	sc/source/ui/styleui/styledlg.src \
	sc/source/ui/formdlg/dwfunctr.src \
	sc/source/ui/pagedlg/pagedlg.src \
	sc/source/ui/pagedlg/hfedtdlg.src \
	sc/source/ui/pagedlg/tphf.src \
	sc/source/ui/navipi/navipi.src \
	sc/source/ui/cctrl/dpcontrol.src \
))

# vim: set noet sw=4 ts=4:
