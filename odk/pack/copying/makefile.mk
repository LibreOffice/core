PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type d -print))}

DIRLIST = \
    $(DESTDIREXAMPLES)$/officeclient 	\
    $(DESTDIRJAVAEXAMPLES)		 	\
    $(DESTDIRJAVAEXAMPLES)$/draw		\
    $(DESTDIRJAVAEXAMPLES)$/calc		\
    $(DESTDIRJAVAEXAMPLES)$/writer		\
    $(DESTDIRJAVAEXAMPLES)$/CalcAddins	\
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

EXAMPLESLIST= \
    $(DESTDIREXAMPLES)$/officeclient$/officeclient.cxx 		\
    $(DESTDIREXAMPLES)$/officeclient$/officeclientsample.xml	\
    $(DESTDIREXAMPLES)$/officeclient$/README           		\
    $(DESTDIREXAMPLES)$/officeclient$/makefile.mk      		\
    $(DESTDIREXAMPLES)$/officeclient$/Makefile         		\
    $(DESTDIREXAMPLES)$/officeclient$/exports.dxp      		\
    $(DESTDIRJAVAEXAMPLES)$/readme.txt		   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/SDraw.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/draw$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/SCalc.java		   	\
    $(DESTDIRJAVAEXAMPLES)$/calc$/EuroAdaption.java			\
    $(DESTDIRJAVAEXAMPLES)$/calc$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/calc$/Makefile	  	 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/SWriter.java	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleInitialization.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextDocumentStructure.java 	\
    $(DESTDIRJAVAEXAMPLES)$/writer$/TextReplace.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/BookmarkInsertion.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/HardFormatting.java 		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/StyleCreation.java  		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/makefile.mk	   		\
    $(DESTDIRJAVAEXAMPLES)$/writer$/Makefile			\
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
    $(DESTDIRJAVAEXAMPLES)$/com$/sun$/star$/comp$/demo$/SWriterDemo.java


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
    $(DESTDIR)$/odk_overview.html

$(DIRLIST) :
     -$(MKDIRHIER) 	$@

$(DESTDIREXAMPLES)$/% : $(PRJ)$/examples$/cpp$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRJAVAEXAMPLES)$/% : $(PRJ)$/examples$/java$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIR)$/classes$/unoil.jar : $(BINOUT)$/unoil.jar 
    $(GNUCOPY) -p $? $@

$(DESTDIRBIN)$/applicat.rdb : $(BINOUT)$/applicat.rdb 
    $(GNUCOPY) -p $(BINOUT)$/applicat.rdb $@

$(DESTDIR)$/settings$/dk.mk : $(PRJ)$/util$/dk.mk
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(PRJ)$/util$/dk.mk $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIR)$/odk_overview.html : $(PRJ)$/util$/odk_overview.html
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRIDL)$/% : $(IDLOUT)$/%
    +-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

remove_dk : 
    +-$(RM)  $(DESTDIRBIN)$/udkapi.rdb >& $(NULLDEV)
