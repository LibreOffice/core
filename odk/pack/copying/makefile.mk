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
    $(DESTDIRJAVAEXAMPLES)$/writer		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver	\
    $(DESTDIRBASICEXAMPLES)		 	\
    $(DESTDIRBASICEXAMPLES)$/drawing 	\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls	 	\
    $(DESTDIRBASICEXAMPLES)$/sheet	 			\
    $(DESTDIRBASICEXAMPLES)$/stock_quotes_updater	 	\
    $(DESTDIRBASICEXAMPLES)$/text	 			\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index 	\
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

EXAMPLESLIST= \
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/README           		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/makefile.mk      		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/Makefile         		\
    $(DESTDIRCPPEXAMPLES)$/officeclient$/exports.dxp      		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/SDraw.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/README	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/SCalc.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/EuroAdaption.java			\
    $(DESTDIRJAVAEXAMPLES)$/calc$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/README	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/SWriter.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleInitialization.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextDocumentStructure.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextReplace.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/BookmarkInsertion.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/HardFormatting.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleCreation.java  		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/writer$/README				\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.html		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/CalcAddins.java		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/makefile.mk			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/Manifest			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/README			\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins$/XCalcAddins.idl		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/DemoComponent.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/makefile.mk		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/manifest		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/README		\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SCalcDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SDrawDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SWriterDemo.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/DocumentConverter.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/makefile.mk			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentConverter$/README			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/DocumentLoader.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/Makefile		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentLoader$/makefile.mk		\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/DocumentSaver.java	\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/Makefile			\
    $(DESTDIRJAVAEXAMPLES)$/DocumentSaver$/makefile.mk		\
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
