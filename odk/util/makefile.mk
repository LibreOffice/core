PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: vos$/version.mk
.INCLUDE: cppuhelper/version.mk
# ------------------------------------------------------------------

PRODUCT_NAME=$(TARGET)$(UPD)$(DELIVERMINOR)

DESTDIR=$(OUT)$/bin$/$(PRODUCT_NAME)

DESTDIRIDL=$(DESTDIR)$/idl$/
DESTDIRIDL2=$(PRODUCT_NAME)$/idl$/

DESTDIREXAMPLES=$(DESTDIR)$/examples$/cpp
DESTDIRDOC=$(OUT)$/misc$/$(PRODUCT_NAME)

DESTDIRAPIDOC=$(DESTDIRDOC)$/doc$/common
PATHTOAPIWWWCOMMON=..$/..$/www$/common
UDKZIPPATH=$(SOLARBINDIR)
UDKDOC0=$(shell $(FIND) $(UDKZIPPATH) -name "udk*_doc.tar.gz" -print)
UDKDOC=$(UDKDOC0:b:b)
.IF "$(UPD)"=="614"
UDKNAME=udk208
.ELSE
UDKNAME=udk
.ENDIF
CHECKDIR=check
CHECKSRCIPT=$(SOLARENV)$/bin$/checkit.pl
 
BINOUT=$(SOLARBINDIR)
IDLOUT=$(SOLARIDLDIR)


.IF "$(GUI)"=="WNT"
DESTDIRBIN=$(DESTDIR)$/bin$/windows
#--------------------
# WNT ONLY
#--------------------
MY_DLLPREFIX=
MY_DLLPOSTFIX=.dll
EXEPOSTFIX=.exe
MY_COPY=copy /u
MY_COPY_RECURSIVE=copy /us 
DLLOUT=$(SOLARBINDIR)
MY_DELETE_RECURSIVE=del /sxy

RM_CVS_DIRS=$(FIND) $(DESTDIRDOC) -name "CVS" -type d -exec rm -rf $(0,text {)$(0,text }) ;
.ELSE
# TODO : add further platforms
DESTDIRBIN=$(DESTDIR)$/bin$/solsparc

#--------------------
# UNX ONLY
#--------------------
DLLOUT=$(SOLARLIBDIR)
MY_DLLPOSTFIX=.so
MY_DLLPREFIX=lib
EXEPOSTFIX=
MY_COPY=$(GNUCOPY) -u
MY_COPY_RECURSIVE=$(GNUCOPY) -urf
MY_DELETE_RECURSIVE=rm -rf

RM_CVS_DIRS=$(FIND) $(DESTDIRDOC) -name "CVS" -type d -exec rm -rf $(0,text {)$(0,text }) ;
.ENDIF


#--------------------------------------------------
# TARGETS
#--------------------------------------------------
.IF "$(GUI)"=="WNT"
all: docugen docu zipdocu deliver zipit checkit cleanit
.ELSE
all: deliver convertit zipit checkit cleanit
.ENDIF


deliver2 .SETDIR=$(OUT)$/bin :
    @echo Now building the binary ODK
    @echo Extracting the UDK
.IF "$(GUI)"=="WNT"
    +-$(COPY) $(UDKZIPPATH)$/$(UDKNAME).zip .
    +-unzip -q -d . $(UDKNAME)
    +-$(RM) $(UDKNAME).zip
.ELSE
    +-$(COPY) $(UDKZIPPATH)$/$(UDKNAME).tar.gz .
    gzip -df $(UDKNAME).tar.gz
    tar -xvf $(UDKNAME).tar
    +-$(RM) $(UDKNAME).tar
.ENDIF
#       ensure, that there is no older version, in case building has aborted
    +-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME)
    +-$(RENAME) $(UDKNAME) $(PRODUCT_NAME)

# ----------- Add the ODK files ----------
deliver .PHONY : deliver2
    +-$(MY_DELETE_RECURSIVE) $(DESTDIRIDL)
#------------------------------------------------------------------------------------
#       R D B  F I L E S
#------------------------------------------------------------------------------------
    +-$(RM)  $(DESTDIRBIN)$/udkapi.rdb
    +-$(MY_COPY)  $(BINOUT)$/applicat.rdb $(DESTDIRBIN)
# ------------------------------------------------------------------------------------
#       I D L files
# ------------------------------------------------------------------------------------
    +-$(MY_COPY_RECURSIVE) $(IDLOUT) $(DESTDIRIDL)
#----------------------------------------------------------------------
#       E X A M P L E S
#----------------------------------------------------------------------
    -$(MKDIRHIER) -p $(DESTDIREXAMPLES)$/officeclient >& $(NULLDEV)
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/officeclient.cxx $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/README $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/Makefile $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/makefile.mk $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/exports.dxp $(DESTDIREXAMPLES)$/officeclient
#----------------------------------------------------------------------
#       S E T T I N G S
#----------------------------------------------------------------------
    +-$(MY_COPY) $(PRJ)$/util$/dk.mk $(DESTDIR)$/settings

# ------------------------------------------------------------------------------------
#      ZIP IT
# ------------------------------------------------------------------------------------
MKFILES_CONVERT=$(shell $(FIND) . -name "*.mk" -print)
CXFILES_CONVERT=$(shell $(FIND) . -name "*.c*" -print)
HXFILES_CONVERT=$(shell $(FIND) . -name "*.h*" -print)

zipit .SETDIR=$(DESTDIR)$/.. .PHONY:
    .IF "$(OS)"=="WNT"
        +zip -r $(PRODUCT_NAME).zip $(PRODUCT_NAME)
    .ELIF "$(GUI)"=="UNX"
        $(RM) $(PRODUCT_NAME).tar.gz
        +tar cvf $(PRODUCT_NAME).tar $(PRODUCT_NAME)
        gzip $(PRODUCT_NAME).tar
    .ENDIF

zipdocu .SETDIR=$(DESTDIRDOC)$/.. .PHONY:
    +-$(RM) ..$/bin$/$(PRODUCT_NAME)_doc.zip
    +-$(RM) ..$/bin$/$(PRODUCT_NAME)_doc.tar.gz
    +zip -9 -r ..$/bin$/$(PRODUCT_NAME)_doc.zip $(PRODUCT_NAME)
    +tar cvf $(PRODUCT_NAME)_doc.tar $(PRODUCT_NAME)
    +gzip $(PRODUCT_NAME)_doc.tar
    +-$(COPY) $(PRODUCT_NAME)_doc.tar.gz ..$/bin
    +-$(RM) $(PRODUCT_NAME)_doc.tar.gz


docugen .SETDIR=$(DESTDIRDOC)$/.. .PHONY :
     +-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME)
     +-$(COPY) $(UDKZIPPATH)$/$(UDKDOC).tar.gz .
     +-gzip -df $(UDKDOC).tar.gz
     +-tar xvf $(UDKDOC).tar
     +-$(RM) $(UDKDOC).tar
     +-$(RENAME) udk* $(PRODUCT_NAME)
     +-$(MY_DELETE_RECURSIVE) "udk*"

docu .PHONY : docugen
     -$(MKDIR) $(DESTDIRAPIDOC) >& $(NULLDEV)
     +-$(MY_COPY_RECURSIVE) $(PATHTOAPIWWWCOMMON) $(DESTDIRAPIDOC)
     +$(RM_CVS_DIRS)

.IF "$(GUI)"=="UNX"
convertit: rwit dos2unx roit
rwit .SETDIR=$(DESTDIR):
    +echo rwit
    +-chmod 666 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-chmod 666 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-chmod 666 $(foreach,file,$(HXFILES_CONVERT) $(file))
dos2unx .SETDIR=$(DESTDIR):
    +echo dos2unx
    +-any2all $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-any2all $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-any2all $(foreach,file,$(HXFILES_CONVERT) $(file))
roit .SETDIR=$(DESTDIR):
    +echo roit
    +-chmod 444 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-chmod 444 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-chmod 444 $(foreach,file,$(HXFILES_CONVERT) $(file))
.ENDIF

checkit .SETDIR=$(DESTDIR)$/.. .PHONY:
    echo dumy target
# REMOVED TEMPORARILY, 
# 	-$(MKDIR) $(CHECKDIR) >& $(NULLDEV)
# 	.IF "$(OS)"=="WNT"
# 		+unzip -q -d $(CHECKDIR) $(PRODUCT_NAME).zip
# 	.ELIF "$(GUI)"=="UNX"
# 		+$(COPY) $(PRODUCT_NAME).tar.gz $(CHECKDIR)
# 		+cd $(CHECKDIR); gzip -df $(PRODUCT_NAME).tar.gz
# 		+cd $(CHECKDIR); tar xvf $(PRODUCT_NAME).tar
# 	.ENDIF
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/bin $(SOLARBINDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/bridges $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/com $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/cppu $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/cppuhelper $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/osl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/rtl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/sal $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/stl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/store $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/typelib $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/uno $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/vos $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/lib $(SOLARLIBDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt
# 	+dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/idl $(SOLARIDLDIR) > $(TMP)$/odkcheck.txt
# 	+$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
# 	+$(RM) $(TMP)$/odkcheck.txt

cleanit .PHONY:
    +-$(MY_DELETE_RECURSIVE) $(OUT)$/bin$/$(CHECKDIR) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(DESTDIR) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(DESTDIRDOC) >& $(NULLDEV)

