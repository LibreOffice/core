PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

TITLE="OpenOffice.org 1.0"
PRODUCTNAME="OpenOffice.org"

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type d -print))}

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
    $(DESTDIROLEEXAMPLES)$/delphi		 	\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables		 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

CPP_OFFICECLIENT= \
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/README           		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/makefile.mk      		\
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
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/GenericXMLFilter$/exports.dxp

JAVA_DRAW= \
    $(DESTDIRJAVAEXAMPLES)$/draw$/SDraw.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/README

JAVA_CALC= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/SCalc.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/EuroAdaption.java		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/README

JAVA_CALC_CHARTTYPECHANGE= \
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/ChartTypeChange.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/makefile.mk	   		\
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
    $(DESTDIRJAVAEXAMPLES)$/writer$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/writer$/README

JAVA_CAR= \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Car.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Car$/makefile.mk \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Manifest \
    $(DESTDIRJAVAEXAMPLES)$/Car$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XAccelerationControl.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XCar.idl \
    $(DESTDIRJAVAEXAMPLES)$/Car$/XDrivingDirection.idl

JAVA_CALCADDINS= \
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.html		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.java		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/makefile.mk			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Manifest			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/README			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/XCalcAddins.idl
    
JAVA_DEMOCOMP= \
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/DemoComponent.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/makefile.mk		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/manifest		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/README		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SCalcDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SDrawDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SWriterDemo.java

JAVA_CONVERTERSERVLET= \
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.html	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/ConverterServlet.java	\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/makefile.mk      \
    $(DESTDIRJAVAEXAMPLES)$/ConverterServlet$/README

JAVA_DOCUMENTCONVERTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/DocumentConverter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/makefile.mk      \
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/README

JAVA_DOCUMENTLOADER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/DocumentLoader.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/makefile.mk     \
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/README

JAVA_DOCUMENTPRINTER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/DocumentPrinter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/makefile.mk    \
    $(DESTDIRJAVAEXAMPLES)$/DocumentPrinter$/README

JAVA_DOCUMENTSAVER= \
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/DocumentSaver.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/makefile.mk      \
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/README

JAVA_GRAPHICSINSERTER= \
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/GraphicsInserter.java	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/Makefile	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/makefile.mk	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/OpenOfficeLogo.gif	\
    $(DESTDIRJAVAEXAMPLES)$/GraphicsInserter$/README

JAVA_INSPECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/index.html \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/install.html \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Inspector.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InspectorThumbnail.jpg \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/InstanceInspector.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/makefile.mk \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Manifest \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/XInstanceInspector.idl \
    $(DESTDIRJAVAEXAMPLES)$/Inspector$/README

JAVA_NOTESACCESS= \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/NotesAccess.java \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/makefile.mk \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/Stocks.nsf \
    $(DESTDIRJAVAEXAMPLES)$/NotesAccess$/README

JAVA_TODO= \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Test.java \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.sxc  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/ToDo.xml  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/XToDo.idl  \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/makefile.mk \
    $(DESTDIRJAVAEXAMPLES)$/ToDo$/Manifest

JAVA_WRITERSELECTOR= \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/WriterSelector.java \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/Makefile \
    $(DESTDIRJAVAEXAMPLES)$/WriterSelector$/makefile.mk \
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
    $(JAVA_PROPERTYSET)  \
    $(BASIC_EXAMPLES)              \
    $(OLE_EXAMPLES)                \
    $(OLE_EXAMPLES_DELPHI)                \
    $(OLE_EXAMPLES_DELPHI_INSERTTABLES)                \
    $(DESTDIRJAVAEXAMPLES)$/debugging_java.html
#	$(DESTDIREXAMPLES)$/examples.html   \

DOCUHTMLFILES= \
    $(DESTDIR)$/index.html \
    $(DESTDIRDOCU)$/tools.html \
    $(DESTDIRDOCU)$/notsupported.html \
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

INSTALLSCRIPT= \
    $(DESTDIR)$/setsdkenv_unix \
    $(DESTDIR)$/setsdkenv_windows.bat

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f -print))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

all : 	\
    remove_dk \
    $(DIRLIST) \
    $(EXAMPLESLIST) \
    $(DOCUFILES) \
    $(INSTALLSCRIPT) \
    $(DESTIDLLIST)  \
    $(DESTDIRBIN)$/applicat.rdb  \
    $(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)  \
    $(DESTDIR)$/settings$/dk.mk \
    $(DESTDIRCLASSES)$/officebean.jar \
    converttags
#	$(DESTDIRCLASSES)$/unoil.jar \
#	$(DESTDIR)$/odk_overview.html \

.IF "$(BUILD_SOSL)"!=""
converttags : 
    +$(PERL) $(CONVERTSCRIPT) $(DESTDIREXAMPLES) odk_ examples
.ELSE
converttags : 
    @echo no conversion necessary!!
.ENDIF

#.IF "$(BUILD_SOSL)"!=""
#convert_links : 
#    +$(PERL) $(CONVERTTAGSCRIPT) $(TITLE) $(PRODUCTNAME) $(DOCUHTMLFILES)
#.ELSE
#convert_links : 
#    +echo no conversion necessary!!
#.ENDIF

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

$(DESTDIRDOCU)$/install.html : $(PRJ)$/docs$/install.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRDOCUIMAGES)$/% : $(PRJ)$/docs/images$/%
    +-rm -f $@ >& $(NULLDEV)
    $(MY_COPY) $? $@

$(DESTDIR)$/setsdkenv_unix : $(PRJ)$/setsdkenv_unix
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

$(DESTDIRBIN)$/applicat.rdb : $(BINOUT)$/applicat.rdb 
    $(GNUCOPY) -p $(BINOUT)$/applicat.rdb $@

$(DESTDIRDLL)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX) : $(MY_DLLOUT)$/$(MY_DLLPREFIX)officebean$(MY_DLLPOSTFIX)
    $(GNUCOPY) -p $? $@

$(DESTDIR)$/settings$/dk.mk : $(PRJ)$/util$/dk.mk
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(PRJ)$/util$/dk.mk $(MY_TEXTCOPY_TARGETPRE) $@

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
