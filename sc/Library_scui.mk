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



$(eval $(call gb_Library_Library,scui))

$(eval $(call gb_Library_add_precompiled_header,scui,$(SRCDIR)/sc/inc/pch/precompiled_scui))

$(eval $(call gb_Library_set_include,scui,\
        $$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/inc/pch \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
))

$(eval $(call gb_Library_add_api,scui,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,scui,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	forui \
	i18nisolang1 \
	sal \
	sc \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

ifeq ($(ENABLE_LAYOUT),TRUE)
$(eval $(call gb_Library_add_linked_libs,scui,\
	ootk \
))
endif

$(eval $(call gb_Library_add_exception_objects,scui,\
	sc/source/ui/styleui/styledlg \
	sc/source/ui/optdlg/tpusrlst \
	sc/source/ui/optdlg/tpview \
	sc/source/ui/optdlg/tpcalc \
	sc/source/ui/optdlg/tpprint \
	sc/source/ui/optdlg/opredlin \
	sc/source/ui/attrdlg/scuiexp \
	sc/source/ui/attrdlg/scdlgfact \
	sc/source/ui/dbgui/tpsubt \
	sc/source/ui/pagedlg/tptable \
	sc/source/ui/docshell/tpstat \
	sc/source/ui/attrdlg/tabpages \
	sc/source/ui/dbgui/tpsort \
	sc/source/ui/dbgui/sortdlg \
	sc/source/ui/dbgui/validate \
	sc/source/ui/miscdlgs/textdlgs \
	sc/source/ui/dbgui/subtdlg \
	sc/source/ui/pagedlg/tphf \
	sc/source/ui/pagedlg/scuitphfedit \
	sc/source/ui/pagedlg/hfedtdlg \
	sc/source/ui/attrdlg/attrdlg \
	sc/source/ui/dbgui/scuiimoptdlg \
	sc/source/ui/miscdlgs/strindlg \
	sc/source/ui/miscdlgs/tabbgcolordlg \
	sc/source/ui/miscdlgs/shtabdlg \
	sc/source/ui/dbgui/scendlg \
	sc/source/ui/dbgui/pvfundlg \
	sc/source/ui/dbgui/pfiltdlg \
	sc/source/ui/miscdlgs/namepast \
	sc/source/ui/miscdlgs/namecrea \
	sc/source/ui/miscdlgs/mvtabdlg \
	sc/source/ui/miscdlgs/mtrindlg \
	sc/source/ui/miscdlgs/linkarea \
	sc/source/ui/miscdlgs/lbseldlg \
	sc/source/ui/miscdlgs/instbdlg \
	sc/source/ui/miscdlgs/inscodlg \
	sc/source/ui/miscdlgs/inscldlg \
	sc/source/ui/miscdlgs/groupdlg \
	sc/source/ui/miscdlgs/filldlg \
	sc/source/ui/miscdlgs/delcodlg \
	sc/source/ui/miscdlgs/delcldlg \
	sc/source/ui/dbgui/dapitype \
	sc/source/ui/dbgui/dapidata \
	sc/source/ui/miscdlgs/crdlg \
	sc/source/ui/dbgui/scuiasciiopt \
	sc/source/ui/dbgui/textimportoptions \
	sc/source/ui/miscdlgs/scuiautofmt \
	sc/source/ui/dbgui/dpgroupdlg \
	sc/source/ui/cctrl/editfield \
))

# vim: set noet sw=4 ts=4:
