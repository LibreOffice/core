PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type d -print))}

DIRLIST = \
    $(DESTDIREXAMPLES)		 	\
    $(DESTDIRCPPEXAMPLES)		 	\
    $(DESTDIRCPPEXAMPLES)$/officeclient 	\
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
    $(DESTDIRJAVAEXAMPLES)$/WriterSelctor	\
    $(DESTDIRBASICEXAMPLES)		 	\
    $(DESTDIRBASICEXAMPLES)$/drawing 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls	 	\
    $(DESTDIRBASICEXAMPLES)$/sheet	 			\
    $(DESTDIRBASICEXAMPLES)$/stock_quotes_updater       \
    $(DESTDIRBASICEXAMPLES)$/text	 			\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index 	\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

CPP_OFFICECLIENT= \
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/README           		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/makefile.mk      		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/exports.dxp

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
    $(DESTDIRJAVAEXAMPLES)$/calc$/ChartTypeChange$/ChartTypeChange.sxc		\
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

BASIC_EXAMPLES= \
    $(DESTDIRBASICEXAMPLES)$/index.html				\
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

EXAMPLESLIST= \
    $(CPP_OFFICECLIENT) \
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
    $(BASIC_EXAMPLES)

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f -print))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

all : 	\
    remove_dk \
    $(DIRLIST) \
    $(EXAMPLESLIST) \
    $(DESTIDLLIST)  \
    $(DESTDIRBIN)$/applicat.rdb  \
    $(DESTDIR)$/settings$/dk.mk \
    $(DESTDIR)$/classes$/unoil.jar \
    $(DESTDIR)$/odk_overview.html \
    convert_links

.IF "$(BUILD_SOSL)"==""
convert_links : 
    +$(PERL) $(CONVERTSCRIPT) $(DESTDIREXAMPLES) odk_ examples
.ELSE
convert_links : 
    +echo no converison necessary
.ENDIF

$(DIRLIST) :
     -$(MKDIRHIER) 	$@

$(DESTDIRCPPEXAMPLES)$/% : $(PRJ)$/examples$/cpp$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRJAVAEXAMPLES)$/% : $(PRJ)$/examples$/java$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRBASICEXAMPLES)$/% : $(PRJ)$/examples$/basic$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIR)$/classes$/unoil.jar : $(BINOUT)$/unoil.jar 
    $(GNUCOPY) -p $? $@

$(DESTDIRBIN)$/applicat.rdb : $(BINOUT)$/applicat.rdb 
    $(GNUCOPY) -p $(BINOUT)$/applicat.rdb $@

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
