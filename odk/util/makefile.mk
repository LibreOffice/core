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
DESTDIRBIN=$(DESTDIR)$/bin$/
DESTDIRBIN2=$(PRODUCT_NAME)$/bin$/
DESTDIRLIB=$(DESTDIR)$/lib$/
DESTDIRINC=$(DESTDIR)$/include$/
DESTDIRIDL=$(DESTDIR)$/idl$/
DESTDIRIDL2=$(PRODUCT_NAME)$/idl$/
DESTDIRJAR=$(DESTDIRLIB)
DESTDIREXAMPLES=$(DESTDIR)$/examples
DESTDIRDOC=$(OUT)$/misc$/$(PRODUCT_NAME)
DESTDIRCPPAUTODOC=$(DESTDIRDOC)$/www$/cpp$/ref
DESTDIRAPIDOC=$(DESTDIRDOC)$/www$/common
TARGETDIRDOC=..$/..$/www$/common
UDKZIPPATH=$(SOLARBINDIR)
UDKDOC0=$(shell $(FIND) $(UDKZIPPATH) -name "udk*_doc.tar.gz" -print)
UDKDOC=$(UDKDOC0:b:b)
UDKZIP=udk208.zip
CHECKDIR=check
CHECKSRCIPT=$(SOLARENV)$/bin$/checkit.pl
UDK_UPD=*
 
BINOUT=$(SOLARBINDIR)
INCOUT=$(SOLARINCDIR)
IDLOUT=$(SOLARIDLDIR)
LIBOUT=$(SOLARLIBDIR)

AUTODOCPRJ= 	-P:sal;$(INCOUT)        -St:.;sal$/*.h*;rtl$/*.h*;osl$/*.h*;rtl$/ustring -p \
        -P:vos;$(INCOUT)        -St:vos;*.h* -p \
        -P:cppu;$(INCOUT)       -St:.;cppu$/*.h*;com$/*.h*;typelib$/*.h*;uno$/*.h* -p \
        -P:cppuhelper;$(INCOUT) -St:cppuhelper;*.h* -p \
        -P:store;$(INCOUT)      -St:store;*.h* -p \
        -P:registry;$(INCOUT)   -St:registry;*.h* -p \
        -P:bridges;$(INCOUT)   	-St:bridges;*.h* -p 

.IF "$(GUI)"=="WNT"

#--------------------
# WNT ONLY
#--------------------
DESTDIRDLL=$(DESTDIR)$/bin
MY_DLLPREFIX=
MY_DLLPOSTFIX=.dll
EXEPOSTFIX=.exe
MY_COPY=copy /u
MY_COPY_RECURSIVE=copy /us 
DLLOUT=$(SOLARBINDIR)
MY_AUTODOC=r:\util\nt\autodoc\autodoc_oo.exe
MY_DELETE_RECURSIVE=del /sxy

RM_CVS_DIRS=$(FIND) $(DESTDIRDOC) -name "CVS" -type d -exec rm -rf $(0,text {)$(0,text }) ;
.ELSE

#--------------------
# UNX ONLY
#--------------------
DLLOUT=$(SOLARLIBDIR)
DESTDIRDLL=$(DESTDIR)$/lib
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

.IF "$(OS)$(CPU)"=="SOLARISS"
.IF "$(COM)"=="GCC"
BINDINGDLL=gcc2_uno
.ELSE
BINDINGDLL=sunpro5_uno
.ENDIF
.ELIF "$(OS)$(CPU)"=="SOLARISI"
.IF "$(COM)"=="GCC"
BINDINGDLL=gcc2_uno
.ELSE
BINDINGDLL=sunprocc5_uno
.ENDIF
.ELIF "$(OS)"=="LINUX"
BINDINGDLL=gcc2_uno
.ELIF "$(OS)"=="WNT"
BINDINGDLL=msci_uno
.ENDIF

docugen .SETDIR=$(DESTDIRDOC)$/.. .PHONY :
    +-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME)
    +-$(COPY) $(UDKZIPPATH)$/$(UDKDOC).tar.gz .
    +-gzip -df $(UDKDOC).tar.gz
    +-tar xvf $(UDKDOC).tar
    +-$(RM) $(UDKDOC).tar
    +-$(RENAME) udk* $(PRODUCT_NAME)
    +-$(MY_DELETE_RECURSIVE) "udk*"

docu .PHONY :
    -$(MKDIR) $(DESTDIRAPIDOC) >& $(NULLDEV)
    +-$(MY_COPY_RECURSIVE) $(TARGETDIRDOC) $(DESTDIRAPIDOC)
    +$(RM_CVS_DIRS)

deliver2 .SETDIR=$(DESTDIR)$/.. .PHONY :
    +-$(COPY) $(UDKZIPPATH)$/$(UDKZIP) .
    +-unzip -q -d . $(UDKZIP)
    +-$(RM) $(UDKZIP)
    +-$(RENAME) udk208 $(PRODUCT_NAME)
    +-$(MY_DELETE_RECURSIVE) $(DESTDIRIDL2)
#------------------------------------------------------------------------------------
#       R D B  F I L E S
#------------------------------------------------------------------------------------
    +-$(RM)  $(DESTDIRBIN2)$/udkapi.rdb
    +-$(MY_COPY)  $(BINOUT)$/applicat.rdb $(DESTDIRBIN2)
# ------------------------------------------------------------------------------------
#       I D L files
# ------------------------------------------------------------------------------------
    +-$(MY_COPY_RECURSIVE) $(IDLOUT) $(DESTDIRIDL2)

deliver .PHONY :
# ------------------------------------------------------------------------------------
#           E X E C U T A B L E S
# ------------------------------------------------------------------------------------
    -$(MKDIR) $(DESTDIR) >& $(NULLDEV)
    -$(MKDIR) $(DESTDIRBIN) >& $(NULLDEV)
    -$(MKDIR) $(DESTDIRLIB) >& $(NULLDEV)
    -$(MKDIR) $(DESTDIRINC) >& $(NULLDEV)
    +-$(MY_COPY)  $(BINOUT)$/cppumaker$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/regcomp$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/regmerge$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/regview$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/rdbmaker$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/unocpp$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/unoidl$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/javamaker$(EXEPOSTFIX) $(DESTDIRBIN)
    +-$(MY_COPY)  $(BINOUT)$/uno$(EXEPOSTFIX) $(DESTDIRBIN)
# ------------------------------------------------------------------------------------
#           S H A R E D  L I B R I E S
# ------------------------------------------------------------------------------------
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)uas$(UDK_UPD)$(DLLPOSTFIX)$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)urd$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)rmcxt2$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)sal2$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)simreg$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)smgr$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)store2$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)tdmgr$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)defreg$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)remotebridge$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)impreg$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)$(BINDINGDLL)$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)namingservice$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)prot_uno_uno$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)rdbtdp$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)reg2$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)iiop_uno$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)urp_uno$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)acceptor$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)brdgfctr$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)connectr$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)cpld$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)cppu2$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)urp_uno$(MY_DLLPOSTFIX)    $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)uuresolver$(MY_DLLPOSTFIX)    $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)cppuhelper$(CPPUHELPER_MAJOR)$(COM)$(MY_DLLPOSTFIX) $(DESTDIRDLL)
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)vos$(VOS_MAJOR)$(COM)$(MY_DLLPOSTFIX) $(DESTDIRDLL)
#------------------------------------------------------------------------------------
#       J A R   F I L E S
#------------------------------------------------------------------------------------
    +-$(MY_COPY)  $(BINOUT)$/sandbox.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/jurt.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/ridl.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/unoil.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/jut.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/juh.jar $(DESTDIRJAR)
    +-$(MY_COPY)  $(BINOUT)$/java_uno.jar $(DESTDIRJAR)
#------------------------------------------------------------------------------------
#       R D B  F I L E S
#------------------------------------------------------------------------------------
    +-$(MY_COPY)  $(BINOUT)$/applicat.rdb $(DESTDIRBIN)
#------------------------------------------------------------------------------------
#       I M P O R T   L I B B R I E S
#------------------------------------------------------------------------------------
.IF "$(GUI)"=="WNT"  
    +-$(MY_COPY)  $(LIBOUT)$/ivos.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/istore.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/ireg.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/isal.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/icppu.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/icppuhelper.lib $(DESTDIRLIB)
    +-$(MY_COPY)  $(LIBOUT)$/irmcxt.lib $(DESTDIRLIB)
.ENDIF
#-------------------------------------------------------------------------------------
#       I N C L U D E  f i l e s
#-------------------------------------------------------------------------------------
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/stl $(DESTDIRINC)$/stl
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/sal $(DESTDIRINC)$/sal
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/rtl $(DESTDIRINC)$/rtl
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/osl $(DESTDIRINC)$/osl
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/vos $(DESTDIRINC)$/vos
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/store $(DESTDIRINC)$/store
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/typelib $(DESTDIRINC)$/typelib
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/uno $(DESTDIRINC)$/uno
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/com $(DESTDIRINC)$/com
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/cppu $(DESTDIRINC)$/cppu
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/cppuhelper $(DESTDIRINC)$/cppuhelper
    +-$(MY_COPY_RECURSIVE) $(INCOUT)$/bridges $(DESTDIRINC)$/bridges
# ------------------------------------------------------------------------------------
#       I D L files
# ------------------------------------------------------------------------------------
    +-$(MY_COPY_RECURSIVE) $(IDLOUT) $(DESTDIRIDL)
#----------------------------------------------------------------------
#       E X A M P L E S
#----------------------------------------------------------------------
    -$(MKDIRHIER) -p $(DESTDIREXAMPLES)$/counter >& $(NULLDEV)
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/counter.cxx $(DESTDIREXAMPLES)$/counter
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/countermain.cxx $(DESTDIREXAMPLES)$/counter
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/XCountable.idl $(DESTDIREXAMPLES)$/counter
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/README $(DESTDIREXAMPLES)$/counter
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/makefile.mk $(DESTDIREXAMPLES)$/counter
    +-$(MY_COPY) $(PRJ)$/examples$/counter$/exports.dxp $(DESTDIREXAMPLES)$/counter
#
    -$(MKDIRHIER) -p $(DESTDIREXAMPLES)$/remoteclient >& $(NULLDEV)
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/remoteclient.cxx $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/README $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/makefile.mk $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/exports.dxp $(DESTDIREXAMPLES)$/remoteclient
#
    -$(MKDIRHIER) -p $(DESTDIREXAMPLES)$/officeclient >& $(NULLDEV)
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/officeclient.cxx $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/README $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/makefile.mk $(DESTDIREXAMPLES)$/officeclient
    +-$(MY_COPY) $(PRJ)$/examples$/officeclient$/exports.dxp $(DESTDIREXAMPLES)$/officeclient

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
    -$(MKDIR) $(CHECKDIR) >& $(NULLDEV)
    .IF "$(OS)"=="WNT"
        +unzip -q -d $(CHECKDIR) $(PRODUCT_NAME).zip
    .ELIF "$(GUI)"=="UNX"
        +$(COPY) $(PRODUCT_NAME).tar.gz $(CHECKDIR)
        +cd $(CHECKDIR); gzip -df $(PRODUCT_NAME).tar.gz
        +cd $(CHECKDIR); tar xvf $(PRODUCT_NAME).tar
    .ENDIF
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/bin $(SOLARBINDIR) > $(TMP)$/odkcheck.txt
    +$(PERL) $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/bridges $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/com $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/cppu $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/cppuhelper $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/osl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/rtl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/sal $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/stl $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/store $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/typelib $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/uno $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/include$/vos $(SOLARINCDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/lib $(SOLARLIBDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt
    +dircmp $(CHECKDIR)$/$(PRODUCT_NAME)$/idl $(SOLARIDLDIR) > $(TMP)$/odkcheck.txt
    +perl $(CHECKSRCIPT) $(TMP)$/odkcheck.txt
    +$(RM) $(TMP)$/odkcheck.txt

cleanit .PHONY:
    +-$(MY_DELETE_RECURSIVE) $(OUT)$/bin$/$(CHECKDIR) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(DESTDIR) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(DESTDIRDOC) >& $(NULLDEV)

