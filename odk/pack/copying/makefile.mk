#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.59 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 17:25:05 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

CONVERTTAGFLAG = $(MISC)$/converttags_html_files.flag

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f -print))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type d -print))}

DESTINCLUDELIST={$(subst,$(SOLARINCDIR),$(DESTDIRINC) $(INCLUDELIST))}
DESTINCDIRLIST={$(subst,$(INCOUT),$(DESTDIRINC) $(INCLUDEDIRLIST))}

COMPONENTLIST = \
    acceptor	\
    brdgfctr	\
    connectr	\
    corefl		\
    cpld		\
    defreg		\
    dynamicloader	\
    impreg		\
    insp		\
    inv		\
    invadp		\
    jen		\
    namingservice	\
    proxyfac	\
    rdbtdp		\
    remotebridge	\
    simreg		\
    smgr		\
    stm		\
    tcv		\
    tdmgr		\
    uuresolver

XMLLIST= \
    $(foreach,c,$(COMPONENTLIST) $(DESTDIRXML)$/$c.xml) \
    $(DESTDIRXML)$/module-description.dtd

DIRLIST = \
    $(DESTDIR) \
    $(DESTDIRBIN) \
    $(DESTDIRLIB) \
    $(DESTDIRINC) \
    $(DESTINCDIRLIST) \
    $(DESTDIRINC)$/com$/sun$/star$/uno \
    $(DESTDIRSETTINGS) \
    $(DESTDIRJAR) \
    $(DESTDIRXML) \
    $(DESTDIRDOCU) \
    $(DESTDIRCOMMONDOCU) \
    $(DESTDIRCOMMONDOCU)$/spec$/xml_format \
    $(DESTDIRDOCUIMAGES) \
    $(DESTDIRDOCU)$/DevStudioWizards$/images \
    $(DESTDIREXAMPLES)		 	\
    $(DESTDIRCLASSES)		 	\
    $(DESTDIRCPPEXAMPLES)		 	\
    $(DESTDIRCPPEXAMPLES)$/counter 	\
    $(DESTDIRCPPEXAMPLES)$/remoteclient 	\
    $(DESTDIRCPPEXAMPLES)$/officeclient 	\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader 	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter 	\
    $(DESTDIRJAVAEXAMPLES)		 	\
    $(DESTDIRJAVAEXAMPLES)$/draw		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange		\
    $(DESTDIRJAVAEXAMPLES)$/writer		\
    $(DESTDIRJAVAEXAMPLES)$/Car		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/test		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter	\
    $(DESTDIRJAVAEXAMPLES)$/Inspector	\
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess	\
    $(DESTDIRJAVAEXAMPLES)$/ToDo	\
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent   \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet   \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector   \
    $(DESTDIRBASICEXAMPLES)		 	\
    $(DESTDIRBASICEXAMPLES)$/drawing 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls	 	\
    $(DESTDIRBASICEXAMPLES)$/sheet	 			\
    $(DESTDIRBASICEXAMPLES)$/stock_quotes_updater       \
    $(DESTDIRBASICEXAMPLES)$/text	 			\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index 	\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically 	\
    $(DESTDIROLEEXAMPLES)		 	\
    $(DESTDIROLEEXAMPLES)$/vbscript		 	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables		 	\
    $(DESTDIROLEEXAMPLES)$/activex		 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}


EXELIST = \
    $(DESTDIRBIN)$/cppumaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regcomp$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regmerge$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regview$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/rdbmaker$(EXEPOSTFIX)	\
    $(DESTDIRBIN)$/idlcpp$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/idlc$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/javamaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/xml2cmp$(EXEPOSTFIX)	\
    $(DESTDIRBIN)$/uno$(EXEPOSTFIX) \
    $(DESTDIRBIN)$/autodoc$(EXEPOSTFIX)

SDKDLLLIST = \
    $(DESTDIRDLL)$/$(MY_DLLPREFIX)prot_uno_uno$(MY_DLLPOSTFIX)            		\
    $(DESTDIRDLL)$/$(MY_DLLPREFIX)juh$(MY_DLLPOSTFIX)

.IF "$(GUI)"=="WNT"
LIBLIST = \
    $(DESTDIRLIB)$/istore.lib 	\
    $(DESTDIRLIB)$/ireg.lib 	\
    $(DESTDIRLIB)$/isal.lib 	\
    $(DESTDIRLIB)$/isalhelper.lib 	\
    $(DESTDIRLIB)$/icppu.lib 	\
    $(DESTDIRLIB)$/icppuhelper.lib 	\
    $(DESTDIRLIB)$/irmcxt.lib

.IF "$(COMEX)"=="8"
    LIBLIST += $(DESTDIRLIB)$/stlport_vc7.lib
.ELSE
    LIBLIST += $(DESTDIRLIB)$/stlport_vc6.lib
.ENDIF
.ENDIF

SETTINGSLIST= \
    $(DESTDIRSETTINGS)$/settings.mk \
    $(DESTDIRSETTINGS)$/std.mk \
    $(DESTDIRSETTINGS)$/stdtarget.mk \
    $(DESTDIRSETTINGS)$/dk.mk

CPP_COUNTER= \
    $(DESTDIRCPPEXAMPLES)$/counter$/counter.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/counter$/counter.xml 		\
    $(DESTDIRCPPEXAMPLES)$/counter$/countermain.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/counter$/XCountable.idl	\
    $(DESTDIRCPPEXAMPLES)$/counter$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/counter$/exports.dxp

CPP_REMOTECLIENT= \
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/remoteclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/remoteclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/exports.dxp

CPP_OFFICECLIENT= \
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/exports.dxp

CPP_DOCUMENTLOADER= \
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/DocumentLoader.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/test.sxw 		\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/Makefile

CPP_GENERICXMLFILTER= \
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/genericfilter.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLExportFilter.cxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLExportFilter.hxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLImportFilter.cxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLImportFilter.hxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLFilter.cxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/GenericXMLFilter.hxx	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/genericxmlfilter.xml	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/README           		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/TypeDetection.xml 		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/TypeDetection.xcu 		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/exports.dxp

JAVA_DRAW= \
    $(DESTDIRJAVAEXAMPLES)$/draw$/SDraw.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/Makefile

JAVA_CALC= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/SCalc.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/EuroAdaption.java		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/Makefile

JAVA_CALC_CHARTTYPECHANGE= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/ChartTypeChange.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/Makefile	  	 		\

JAVA_WRITER= \
    $(DESTDIRJAVAEXAMPLES)$/writer$/SWriter.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleInitialization.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextDocumentStructure.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextReplace.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/BookmarkInsertion.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/HardFormatting.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleCreation.java  	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/Makefile

JAVA_CAR= \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Car.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Car.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XAccelerationControl.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XDrivingDirection.idl

JAVA_CALCADDINS= \
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.java		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/XCalcAddins.idl

JAVA_DEMOCOMP= \
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/DemoComponent.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/DemoComponent.sxw	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SCalcDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SDrawDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SWriterDemo.java

JAVA_CONVERTERSERVLET= \
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.html	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.java	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/README

JAVA_DOCUMENTCONVERTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/DocumentConverter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/test$/test1.sxw

JAVA_DOCUMENTLOADER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/DocumentLoader.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/Makefile

JAVA_DOCUMENTPRINTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/DocumentPrinter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/Makefile

JAVA_DOCUMENTSAVER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/DocumentSaver.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/Makefile

JAVA_GRAPHICSINSERTER= \
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/GraphicsInserter.java	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/Makefile	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/oo_smiley.gif

JAVA_INSPECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Inspector.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InspectorThumbnail.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InstanceInspector.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/XInstanceInspector.idl

JAVA_NOTESACCESS= \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/NotesAccess.java \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Stocks.nsf

JAVA_TODO= \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.sxc  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.xml  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/XToDo.idl  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Makefile

JAVA_WRITERSELECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/WriterSelector.java \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/Makefile

JAVA_MINIMALCOMPONENT= \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.idl \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.java\
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.xml \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/TestMinimalComponent.java\
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/Makefile

JAVA_PROPERTYSET= \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/PropTest.java \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/PropertySet.sxw \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/Makefile

BASIC_EXAMPLES= \
    $(DESTDIRBASICEXAMPLES)$/drawing$/dirtree.txt			\
    $(DESTDIRBASICEXAMPLES)$/drawing$/importexportofasciifiles.sxd 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/beef.wmf	 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/burger.wmf	 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/burger_factory.sxw	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/chicken.wmf 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/fish.wmf	 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/vegetable.wmf	\
    $(DESTDIRBASICEXAMPLES)$/sheet$/adapting_to_euroland.sxc 	\
    $(DESTDIRBASICEXAMPLES)$/stock_quotes_updater$/stock.sxc 	\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index$/index.sxw 	\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index$/indexlist.txt	\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/changing_appearance.sxw 		\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/inserting_bookmarks.sxw 		\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/replacing_text.sxw 		\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/using_regular_expressions.sxw

OLE_EXAMPLES= \
    $(DESTDIROLEEXAMPLES)$/vbscript$/readme.txt \
    $(DESTDIROLEEXAMPLES)$/vbscript$/WriterDemo.vbs

OLE_EXAMPLES_DELPHI= \
    $(DESTDIROLEEXAMPLES)$/delphi$/StarOffice_Delphi.sxw \
    $(DESTDIROLEEXAMPLES)$/delphi$/StarOffice_and_Delphi.pdf

OLE_EXAMPLES_DELPHI_INSERTTABLES= \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/Project1.dpr \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/Project1.res	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleCode.pas	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleUI.dfm	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleUI.pas

OLE_EXAMPLES_ACTIVEX= \
    $(DESTDIROLEEXAMPLES)$/activex$/example.html \
    $(DESTDIROLEEXAMPLES)$/activex$/Makefile \
    $(DESTDIROLEEXAMPLES)$/activex$/README.txt \
    $(DESTDIROLEEXAMPLES)$/activex$/resource.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.rgs \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.rgs \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.def \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.idl \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.rc \
    $(DESTDIROLEEXAMPLES)$/activex$/StdAfx2.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/StdAfx2.h

EXAMPLESLIST= \
    $(CPP_COUNTER) \
    $(CPP_REMOTECLIENT) \
    $(CPP_OFFICECLIENT) \
    $(CPP_DOCUMENTLOADER) \
    $(CPP_GENERICXMLFILTER) \
    $(JAVA_DRAW)        \
    $(JAVA_CALC)        \
    $(JAVA_CALC_CHARTTYPECHANGE)        \
    $(JAVA_CAR)         \
    $(JAVA_WRITER)      \
    $(JAVA_CALCADDINS)  \
    $(JAVA_DEMOCOMP)    \
    $(JAVA_CONVERTERSERVLET)    \
    $(JAVA_DOCUMENTCONVERTER)      \
    $(JAVA_DOCUMENTLOADER)         \
    $(JAVA_DOCUMENTPRINTER)        \
    $(JAVA_DOCUMENTSAVER)          \
    $(JAVA_GRAPHICSINSERTER)       \
    $(JAVA_INSPECTOR)              \
    $(JAVA_NOTESACCESS)            \
    $(JAVA_TODO)                   \
    $(JAVA_WRITERSELECTOR)         \
    $(JAVA_MINIMALCOMPONENT) \
    $(JAVA_PROPERTYSET)  \
    $(BASIC_EXAMPLES)              \
    $(OLE_EXAMPLES)                \
    $(OLE_EXAMPLES_DELPHI)                \
    $(OLE_EXAMPLES_DELPHI_INSERTTABLES)                \
    $(OLE_EXAMPLES_ACTIVEX)

DOCUSTUDIO4INTEGRATIONHTMLFILES= \
    $(DESTDIRDOCU)$/DevStudioWizards$/CalcAddinWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/DeployingComponents.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/IDLWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/UNOComponentWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/wizards.html

DOCUSTUDIO4INTEGRATIONGRAPHICSFILES= \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/AddInsDescription.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ChooseIDL.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/DesignIDL.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ExecuteAntScript.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/GeneratedJavaCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/InterfacesToImplementCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/InterfacesToImplementCalcAddinsExtended.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/MountWorkingDirectory.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewObjectName.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewOfficeIDL.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewOfficeObject.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/OutputWindowBuild.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ReturnCommand.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/SelectModule.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ServiceSetupPanelCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ServiceSetupPanelCalcAddinsExtended.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/mount_directory.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office01.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office02.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office03.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office04.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office05.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office06.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office07.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office08.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office09.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot10.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot11.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot12.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot13.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot14.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot15.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot16b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot17b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot18b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot20b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot21.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot22.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot23.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot24.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot25.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot26.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot27b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot3.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot32b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot33b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot34b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot35b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot42b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot43b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot8.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot9.png

DOCUHTMLFILES+= \
    $(DOCUSTUDIO4INTEGRATIONHTMLFILES) \
    $(DESTDIR)$/index.html \
    $(DESTDIRDOCU)$/tools.html \
    $(DESTDIRDOCU)$/notsupported.html \
    $(DESTDIRDOCU)$/DevelopersGuide_intro.html \
    $(DESTDIRDOCU)$/install.html \
    $(DESTDIRDOCU)$/sdk_styles.css \
    $(DESTDIREXAMPLES)$/examples.html

#	$(DESTDIREXAMPLES)$/DevelopersGuide$/examples.html

DOCUFILES+= \
    $(DOCUHTMLFILES) \
    $(DOCUSTUDIO4INTEGRATIONGRAPHICSFILES) \
    $(DESTDIRDOCUIMAGES)$/black_dot.gif \
    $(DESTDIRDOCUIMAGES)$/logo.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_logo.gif

.IF "$(GUI)"=="UNX"
INSTALLSCRIPT= \
    $(DESTDIR)$/configure \
    $(DESTDIR)$/setsdkenv_unix.in \
    $(DESTDIR)$/oo1.0.2_sdk_linkpatch
.ELSE
INSTALLSCRIPT= \
    $(DESTDIR)$/configureWindowsNT.bat \
    $(DESTDIR)$/setsdkenv_windows.bat
.ENDIF

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIRLIST) \
    $(EXELIST) \
    $(XMLLIST) \
    $(SDKDLLLIST) \
    $(LIBLIST) \
    $(SETTINGSLIST) \
    $(EXAMPLESLIST) \
    $(DOCUFILES) \
    $(INSTALLSCRIPT) \
    $(DESTIDLLIST)  \
    $(DESTINCLUDELIST) \
    $(DESTDIRCLASSES)$/oosupport.nbm \
    $(DEVGUIDE_DOCU_FLAG) \
    $(DEVGUIDE_SAMPLES_FLAG) \
    $(DESTDIRDOCU)$/DevelopersGuide$/DevelopersGuide.pdf \
    $(DESTDIRDOCU)$/common$/spec$/xml_format$/xml_specification.pdf \
    $(CONVERTTAGFLAG)

# moved into the office installation for easier use
#	$(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)  \
#	$(DESTDIRCLASSES)$/officebean.jar \

$(DIRLIST) :
     -$(MKDIRHIER) 	$@

$(DESTDIR)$/index.html : $(PRJ)$/index.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/examples.html : $(PRJ)$/docs$/examples.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/tools.html : $(PRJ)$/docs$/tools.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/notsupported.html : $(PRJ)$/docs$/notsupported.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevelopersGuide_intro.html : $(PRJ)$/docs$/DevelopersGuide_intro.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/install.html : $(PRJ)$/docs$/install.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/sdk_styles.css : $(PRJ)$/docs$/sdk_styles.css
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCUIMAGES)$/% : $(PRJ)$/docs$/images$/%
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIRDOCU)$/DevStudioWizards$/CalcAddinWizard.html : $(PRJ)$/docs$/DevStudioWizards$/CalcAddinWizard.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevStudioWizards$/DeployingComponents.html : $(PRJ)$/docs$/DevStudioWizards$/DeployingComponents.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevStudioWizards$/IDLWizard.html : $(PRJ)$/docs$/DevStudioWizards$/IDLWizard.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevStudioWizards$/UNOComponentWizard.html : $(PRJ)$/docs$/DevStudioWizards$/UNOComponentWizard.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevStudioWizards$/wizards.html : $(PRJ)$/docs$/DevStudioWizards$/wizards.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/DevStudioWizards$/images$/% : $(PRJ)$/docs$/DevStudioWizards$/images$/%
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIR)$/configure : $(PRJ)$/configure
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
.IF "$(GUI)"=="UNX"
    +-chmod 755 $@
.ENDIF

$(DESTDIR)$/setsdkenv_unix.in : $(PRJ)$/setsdkenv_unix.in
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
.IF "$(GUI)"=="UNX"
    +-chmod 755 $@
.ENDIF

$(DESTDIR)$/oo1.0.2_sdk_linkpatch : oo1.0.2_sdk_linkpatch
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
.IF "$(GUI)"=="UNX"
    +-chmod 755 $@
.ENDIF

$(DESTDIR)$/configureWindowsNT.bat : $(PRJ)$/configureWindowsNT.bat
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIR)$/setsdkenv_windows.bat : $(PRJ)$/setsdkenv_windows.bat
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.sxw : $(PRJ)$/examples$/%.sxw $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.sxc : $(PRJ)$/examples$/%.sxc $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.sxd : $(PRJ)$/examples$/%.sxd $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.xlb : $(PRJ)$/examples$/%.xlb $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.xdl : $(PRJ)$/examples$/%.xdl $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.gif : $(PRJ)$/examples$/%.gif $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.jpg : $(PRJ)$/examples$/%.jpg $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.wmf : $(PRJ)$/examples$/%.wmf $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.nsf : $(PRJ)$/examples$/%.nsf $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.pdf : $(PRJ)$/examples$/%.pdf $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIREXAMPLES)$/%.xba : $(PRJ)$/examples$/%.xba $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.cxx : $(PRJ)$/examples$/%.cxx $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.hxx : $(PRJ)$/examples$/%.hxx $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.java : $(PRJ)$/examples$/%.java $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.idl : $(PRJ)$/examples$/%.idl $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.dxp : $(PRJ)$/examples$/%.dxp $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.xml : $(PRJ)$/examples$/%.xml $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.xcu : $(PRJ)$/examples$/%.xcu $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.html : $(PRJ)$/examples$/%.html $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.txt : $(PRJ)$/examples$/%.txt $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%$/Makefile : $(PRJ)$/examples$/%$/Makefile $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%$/README : $(PRJ)$/examples$/%$/README $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.cpp : $(PRJ)$/examples$/%.cpp $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.h : $(PRJ)$/examples$/%.h $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.def : $(PRJ)$/examples$/%.def $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.rc : $(PRJ)$/examples$/%.rc $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.rgs : $(PRJ)$/examples$/%.rgs $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.dpr : $(PRJ)$/examples$/%.dpr $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.res : $(PRJ)$/examples$/%.res $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.pas : $(PRJ)$/examples$/%.pas $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.dfm : $(PRJ)$/examples$/%.dfm $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIREXAMPLES)$/%.vbs : $(PRJ)$/examples$/%.vbs $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRCLASSES)$/officebean.jar : $(OUT)$/class$/officebean.jar 
    $(GNUCOPY) -p $? $@

$(DESTDIRCLASSES)$/oosupport.nbm : $(OUT)$/class$/oosupport.nbm
    $(GNUCOPY) -p $? $@

$(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX) : $(MY_DLLOUT)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)
    $(GNUCOPY) -p $? $@

$(DESTDIRSETTINGS)$/%.mk : $(PRJ)$/settings$/%.mk $(DIRLIST)
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRSETTINGS)$/dk.mk :
    +-rm -f $@ >& $(NULLDEV)
    echo #$(PRODUCT_NAME) dependent settings > $@
    echo DKNAME=${PRODUCT_NAME} >> $@    
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    echo 'DKREGISTRYNAME=$$(subst \\,\,"$$(OFFICE_PROGRAM_PATH)$$(PS)applicat.rdb")' >> $@
.ELSE
    echo DKREGISTRYNAME=$$(subst \\,\,"$$(OFFICE_PROGRAM_PATH)$$(PS)applicat.rdb") >> $@
.ENDIF

$(DESTDIRDOCU)$/DevelopersGuide$/DevelopersGuide.pdf : DevelopersGuide.pdf
    $(GNUCOPY) -p DevelopersGuide.pdf $(DESTDIRDOCU)$/DevelopersGuide
    
$(DEVGUIDE_DOCU_FLAG) : DevelopersGuide.zip
    $(GNUCOPY) -p DevelopersGuide.zip $(DESTDIRDOCU)
    unzip -quod $(DESTDIRDOCU) $(DESTDIRDOCU)$/DevelopersGuide.zip
    +-rm -f $(DESTDIRDOCU)$/DevelopersGuide.zip >& $(NULLDEV)
    +@echo "devguide docu unzipped" > $@       

$(DEVGUIDE_SAMPLES_FLAG) : DevelopersGuideSamples.zip
    $(GNUCOPY) -p DevelopersGuideSamples.zip $(DESTDIREXAMPLES)
    unzip -quod $(DESTDIREXAMPLES) $(DESTDIREXAMPLES)$/DevelopersGuideSamples.zip
    +-rm -f $(DESTDIREXAMPLES)$/DevelopersGuideSamples.zip >& $(NULLDEV)
    +@echo "devguide samples unzipped" > $@       

$(DESTDIRCOMMONDOCU)$/spec$/xml_format$/xml_specification.pdf : xml_specification.pdf
    $(GNUCOPY) -p $? $@
        
$(DESTDIRXML)$/module-description.dtd : $(XMLOUT)$/module-description.dtd $(DIRLIST)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(XMLOUT)$/module-description.dtd $(MY_TEXTCOPY_TARGETPRE) $@  

$(DESTDIRBIN)$/%$(EXEPOSTFIX) : $(BINOUT)$/%$(EXEPOSTFIX) $(DIRLIST)
    $(GNUCOPY) -p $(BINOUT)$/$(@:f) $@

$(DESTDIRDLL)$/%$(MY_DLLPOSTFIX) : $(DLLOUT)$/%$(MY_DLLPOSTFIX) $(DIRLIST)
    $(GNUCOPY) -p $(DLLOUT)$/$(@:f) $@
    
$(DESTDIRIDL)$/% : $(IDLOUT)$/% $(DIRLIST)
    +-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRINC)$/udkversion.mk :
    +-rm -f $@ >& $(NULLDEV)
    echo #UDK version number > $@
    echo #major >> $@
    echo UDK_MAJOR=$(UDK_MAJOR) >> $@
    echo #minor >> $@
    echo UDK_MINOR=$(UDK_MINOR) >> $@
    echo #micro >> $@
    echo UDK_MICRO=$(UDK_MICRO) >> $@
    
$(DESTDIRINC)$/% : $(INCOUT)$/% $(DIRLIST)
    +-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRLIB)$/%.lib : $(LIBOUT)$/%.lib $(DIRLIST)
    $(GNUCOPY) -p $(LIBOUT)$/$(@:f) $@
    
$(DESTDIRXML)$/%.xml : $(XMLOUT)$/%.xml $(DIRLIST)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
    
$(CONVERTTAGFLAG) : $(MISC)$/deltree.txt $(DOCUHTMLFILES)
    +$(PERL) $(CONVERTTAGSCRIPT) $(CONVMODE) $(TITLE) $(OFFICEPRODUCTNAME) $(DOCUHTMLFILES) $(DESTDIREXAMPLES)$/DevelopersGuide$/examples.html
    +@echo "tags converted" > $@
