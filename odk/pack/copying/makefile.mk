PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

CONVERTTAGFLAG = $(MISC)$/converttags_html_files.flag

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT)$/com -type f -print))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT)$/com -type d -print))}

DIRLIST = \
    $(DESTDIRDOCU) \
    $(DESTDIRDOCUIMAGES) \
    $(DESTDIREXAMPLES)		 	\
    $(DESTDIRCLASSES)		 	\
    $(DESTDIRCPPEXAMPLES)		 	\
    $(DESTDIRCPPEXAMPLES)$/officeclient 	\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader 	\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter 	\
    $(DESTDIRJAVAEXAMPLES)		 	\
    $(DESTDIRJAVAEXAMPLES)$/draw		\
    $(DESTDIRJAVAEXAMPLES)$/calc		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange		\
    $(DESTDIRJAVAEXAMPLES)$/writer		\
    $(DESTDIRJAVAEXAMPLES)$/Car		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo	\
    $(DESTDIRJAVAEXAMPLES)$/compsamples		\
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples		\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter	\
    $(DESTDIRJAVAEXAMPLES)$/Inspector	\
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess	\
    $(DESTDIRJAVAEXAMPLES)$/ToDo	\
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean	\
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean/SimpleBean	\
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean/OfficeWriterBean	\
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
    $(DESTDIRBASICEXAMPLES)$/register_component	 			\
    $(DESTDIRBASICEXAMPLES)$/register_component$/regcomp	\
    $(DESTDIROLEEXAMPLES)		 	\
    $(DESTDIROLEEXAMPLES)$/vbscript		 	\
    $(DESTDIROLEEXAMPLES)$/delphi		 	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables		 	\
    $(DESTDIROLEEXAMPLES)$/activex		 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

CPP_OFFICECLIENT= \
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/README           		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/exports.dxp

CPP_DOCUMENTLOADER= \
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/DocumentLoader.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/README           		\
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
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/exports.dxp

JAVA_DRAW= \
    $(DESTDIRJAVAEXAMPLES)$/draw$/SDraw.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/README

JAVA_CALC= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/SCalc.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/EuroAdaption.java		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/README

JAVA_CALC_CHARTTYPECHANGE= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/ChartTypeChange.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/README

JAVA_WRITER= \
    $(DESTDIRJAVAEXAMPLES)$/writer$/SWriter.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleInitialization.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextDocumentStructure.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextReplace.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/BookmarkInsertion.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/HardFormatting.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleCreation.java  	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/writer$/README

JAVA_CAR= \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Car.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Manifest \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XAccelerationControl.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XCar.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XDrivingDirection.idl

JAVA_CALCADDINS= \
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.html		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.java		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Manifest			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/README			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/XCalcAddins.idl
    
JAVA_DEMOCOMP= \
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/DemoComponent.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/manifest		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/README		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SCalcDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SDrawDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SWriterDemo.java

JAVA_COMPSAMPLES= \
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/TestComponentA.java	\
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/TestComponentB.java	\
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/TestServiceProvider.java	\
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/$/Manifest       \
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/$/XSomethingA.idl       \
    $(DESTDIRJAVAEXAMPLES)$/compsamples$/componentsamples$/$/XSomethingB.idl

JAVA_CONVERTERSERVLET= \
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.html	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.java	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/README

JAVA_DOCUMENTCONVERTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/DocumentConverter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/README

JAVA_DOCUMENTLOADER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/DocumentLoader.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/README

JAVA_DOCUMENTPRINTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/DocumentPrinter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/README

JAVA_DOCUMENTSAVER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/DocumentSaver.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/README

JAVA_GRAPHICSINSERTER= \
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/GraphicsInserter.java	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/Makefile	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/OpenOfficeLogo.gif	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/README

JAVA_INSPECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/index.html \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/install.html \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Inspector.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InspectorThumbnail.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InstanceInspector.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Manifest \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/XInstanceInspector.idl \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/README

JAVA_NOTESACCESS= \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/NotesAccess.java \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Stocks.nsf \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/README

JAVA_TODO= \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.sxc  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.xml  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/XToDo.idl  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Makefile  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Manifest

JAVA_WRITERSELECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/WriterSelector.java \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/README

JAVA_OFFICEBEAN_SIMPLEBEAN= \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/SimpleBean$/SimpleBean.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/SimpleBean$/SimpleBeanBeanInfo.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/SimpleBean$/SimpleViewer.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/SimpleBean$/manifest.tmp \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/SimpleBean$/Makefile

JAVA_OFFICEBEAN_OFFICEWRITERBEAN= \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/OfficeWriter.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/OfficeWriterBeanInfo.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/Office.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/OfficeBeanInfo.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/DocViewer.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeWriterBean$/manifest.tmp

JAVA_OFFICEBEAN_HELPERBEANS= \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeCommand.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/BeanInfoAdapter.java \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeIconColor32.jpg \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeIconColor16.jpg \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeIconMono32.jpg \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/OfficeIconMono16.jpg \
    $(DESTDIRJAVAEXAMPLES)$/OfficeBean$/BasicOfficeBean.java

JAVA_MINIMALCOMPONENT= \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/BuildMinimalComponent.xml \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.idl \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.java\
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/MinimalComponent.xml \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/TestMinimalComponent.java\
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/README   \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/Manifest \
    $(DESTDIRJAVAEXAMPLES)$/MinimalComponent$/Makefile

JAVA_PROPERTYSET= \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/PropTest.java \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/readme.txt \
    $(DESTDIRJAVAEXAMPLES)$/PropertySet$/manifest \
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
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/using_regular_expressions.sxw \
    $(DESTDIRBASICEXAMPLES)$/register_component$/readme.txt \
    $(DESTDIRBASICEXAMPLES)$/register_component$/regcomp$/dialog.xlb \
    $(DESTDIRBASICEXAMPLES)$/register_component$/regcomp$/script.xlb \
    $(DESTDIRBASICEXAMPLES)$/register_component$/regcomp$/regcomp.xba \
    $(DESTDIRBASICEXAMPLES)$/register_component$/regcomp$/regcompdlg.xdl

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
    $(JAVA_COMPSAMPLES) \
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
    $(JAVA_OFFICEBEAN_SIMPLEBEAN)  \
    $(JAVA_OFFICEBEAN_OFFICEWRITERBEAN)  \
    $(JAVA_OFFICEBEAN_HELPERBEANS)  \
    $(JAVA_MINIMALCOMPONENT) \
    $(JAVA_PROPERTYSET)  \
    $(BASIC_EXAMPLES)              \
    $(OLE_EXAMPLES)                \
    $(OLE_EXAMPLES_DELPHI)                \
    $(OLE_EXAMPLES_DELPHI_INSERTTABLES)                \
    $(OLE_EXAMPLES_ACTIVEX)                \
    $(DESTDIRJAVAEXAMPLES)$/debugging_java.html

DOCUHTMLFILES= \
    $(DESTDIR)$/index.html \
    $(DESTDIRDOCU)$/tools.html \
    $(DESTDIRDOCU)$/notsupported.html \
    $(DESTDIRDOCU)$/devmanual_preview.html \
    $(DESTDIRDOCU)$/install.html \
    $(DESTDIREXAMPLES)$/examples.html

DOCUFILES+= \
    $(DOCUHTMLFILES) \
    $(DESTDIRDOCUIMAGES)$/black_dot.gif \
    $(DESTDIRDOCUIMAGES)$/logo.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_logo.gif \
    $(DESTDIRDOCUIMAGES)$/shadow_r.gif \
    $(DESTDIRDOCUIMAGES)$/shadow_l.gif \
    $(DESTDIRDOCUIMAGES)$/elements.jpg

.IF "$(GUI)"=="UNX"
INSTALLSCRIPT= \
    $(DESTDIR)$/configure \
    $(DESTDIR)$/setsdkenv_unix.in
.ELSE
INSTALLSCRIPT= \
    $(DESTDIR)$/configureWindowsNT.bat \
    $(DESTDIR)$/setsdkenv_windows.bat
.ENDIF

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : 	\
    remove_dk \
    $(DIRLIST) \
    $(EXAMPLESLIST) \
    $(DOCUFILES) \
    $(INSTALLSCRIPT) \
    $(DESTIDLLIST)  \
    $(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)  \
    $(DESTDIR)$/settings$/dk.mk \
    $(DESTDIRCLASSES)$/officebean.jar \
    $(DESTDIRCLASSES)$/oosupport.nbm \
    $(CONVERTTAGFLAG)
#	$(DESTDIRBIN)$/pkgchk$(PKGCHK_POSTFIX)  \
#	$(DESTDIRDLL)$/$(MY_DLLPREFIX)pkgchk$(UPD)$(DLLPOSTFIX)$(MY_DLLPOSTFIX)  \

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

$(DESTDIRDOCU)$/devmanual_preview.html : $(PRJ)$/docs$/devmanual_preview.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCU)$/install.html : $(PRJ)$/docs$/install.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCUIMAGES)$/% : $(PRJ)$/docs/images$/%
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

$(DESTDIR)$/configureWindowsNT.bat : $(PRJ)$/configureWindowsNT.bat
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIR)$/setsdkenv_windows.bat : $(PRJ)$/setsdkenv_windows.bat
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRCPPEXAMPLES)$/% : $(PRJ)$/examples$/cpp$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRJAVAEXAMPLES)$/% : $(PRJ)$/examples$/java$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRBASICEXAMPLES)$/% : $(PRJ)$/examples$/basic$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIROLEEXAMPLES)$/% : $(PRJ)$/examples$/OLE$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRCLASSES)$/unoil.jar : $(BINOUT)$/unoil.jar 
    $(GNUCOPY) -p $? $@

$(DESTDIRCLASSES)$/officebean.jar : $(OUT)$/class$/officebean.jar 
    $(GNUCOPY) -p $? $@

$(DESTDIRCLASSES)$/oosupport.nbm : $(OUT)$/class$/oosupport.nbm
    $(GNUCOPY) -p $? $@

$(DESTDIRBIN)$/applicat.rdb : $(BINOUT)$/applicat.rdb 
    $(GNUCOPY) -p $(BINOUT)$/applicat.rdb $@

$(DESTDIRBIN)$/pkgchk$(PKGCHK_POSTFIX) : $(BINOUT)$/pkgchk$(PKGCHK_POSTFIX)
    $(GNUCOPY) -p $? $@

$(DESTDIRDLL)$/$(MY_DLLPREFIX)pkgchk$(UPD)$(DLLPOSTFIX)$(MY_DLLPOSTFIX) : $(DLLOUT)$/$(MY_DLLPREFIX)pkgchk$(UPD)$(DLLPOSTFIX)$(MY_DLLPOSTFIX)
    $(GNUCOPY) -p $? $@

$(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX) : $(MY_DLLOUT)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)
    $(GNUCOPY) -p $? $@

$(DESTDIR)$/settings$/dk.mk : $(PRJ)$/util$/dk.mk
    +-rm -f $@ >& $(NULLDEV)
.IF "$(BUILD_SOSL)"==""
    echo #$(ODKNAME) dependent settings > $@
    echo DKNAME=${ODKNAME} >> $@
.ELSE
    echo #$(PRODUCT_NAME) dependent settings > $@
    echo DKNAME=${PRODUCT_NAME} >> $@    
.ENDIF
.IF "$(GUI)"=="UNX"
    echo 'DKREGISTRYNAME=$$(OFFICE_PROGRAM_PATH)$$(PS)applicat.rdb' >> $@
.ELSE
    echo DKREGISTRYNAME=$$(OFFICE_PROGRAM_PATH)$$(PS)applicat.rdb >> $@
.ENDIF

.IF "$(BUILD_SOSL)"==""
$(DESTDIR)$/odk_overview.html : $(PRJ)$/util$/odk_overview.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
.ELSE
$(DESTDIR)$/odk_overview.html : $(PRJ)$/util$/odk_overview2.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@
.ENDIF

$(DESTDIRIDL)$/% : $(IDLOUT)$/%
    +-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

remove_dk : 
    +-$(RM)  $(DESTDIRBIN)$/udkapi.rdb >& $(NULLDEV)

.IF "$(BUILD_SOSL)"!=""  
$(CONVERTTAGFLAG) : $(MISC)$/deltree.txt $(DOCUHTMLFILES)
    +$(PERL) $(CONVERTTAGSCRIPT) $(TITLE) $(OFFICEPRODUCTNAME) $(DOCUHTMLFILES)
    +@echo "tags converted" > $@
.ELSE
$(CONVERTTAGFLAG) : $(MISC)$/deltree.txt
    @echo no conversion necessary
    +@echo "no conversion necessary" > $@
.ENDIF
