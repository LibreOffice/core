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
DESTDIRLIB=$(DESTDIR)$/lib$/
DESTDIRINC=$(DESTDIR)$/include$/
DESTDIRIDL=$(DESTDIR)$/idl$/
DESTDIRJAR=$(DESTDIRLIB)
DESTDIREXAMPLES=$(DESTDIR)$/examples
DESTDIRDOC=$(OUT)$/misc$/$(PRODUCT_NAME)
DESTDIRCPPAUTODOC=$(DESTDIRDOC)$/www$/cpp$/ref
DESTDIRAPIDOC=$(DESTDIRDOC)$/www$/common
TARGETDIRDOC=..$/..$/www$/common
UDKZIPPATH=$(SOLARBINDIR)
UDKDOC0=$(shell $(FIND) $(UDKZIPPATH) -name "udk*_doc.tar.gz" -print)
UDKDOC=$(UDKDOC0:b:b)
 
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
MY_DELETE_RECURSIVE=rmdir /s

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
all: docugen docu zipdocu deliver zipit 
.ELSE
all: docu deliver convertit zipit
.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISS"
BINDINGDLL=sunpro5_uno
.ELIF "$(OS)$(CPU)"=="SOLARISI"
BINDINGDLL=sunprocc5_uno
.ELIF "$(OS)"=="LINUX"
BINDINGDLL=gcc2_uno
.ELIF "$(OS)"=="WNT"
BINDINGDLL=msci_uno
.ENDIF

docugen .SETDIR=$(DESTDIRDOC)$/.. .PHONY :
    +-$(RM) -rf $(PRODUCT_NAME)
    +-$(COPY) $(UDKZIPPATH)$/$(UDKDOC).tar.gz .
    +-gzip -d $(UDKDOC).tar.gz
    +-tar xvf $(UDKDOC).tar
    +-$(RM) $(UDKDOC).tar
    +-$(RENAME) udk* $(PRODUCT_NAME)
    +-$(RM) -rf "udk*"

docu .PHONY :
    -$(MKDIR) $(DESTDIRAPIDOC) >& $(NULLDEV)
    +-$(MY_COPY_RECURSIVE) $(TARGETDIRDOC) $(DESTDIRAPIDOC)
    +$(RM_CVS_DIRS)

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
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)uas$(UPD)$(DLLPOSTFIX)$(MY_DLLPOSTFIX) $(DESTDIRDLL)
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
    +-$(MY_COPY)  $(DLLOUT)$/$(MY_DLLPREFIX)remote_uno$(MY_DLLPOSTFIX) $(DESTDIRDLL)
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
    -$(MKDIRHIER) -p $(DESTDIREXAMPLES)$/remoteclient >& $(NULLDEV)
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/remoteclient.cxx $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/README $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/makefile.mk $(DESTDIREXAMPLES)$/remoteclient
    +-$(MY_COPY) $(PRJ)$/examples$/remoteclient$/exports.dxp $(DESTDIREXAMPLES)$/remoteclient

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
    +chmod 666 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +chmod 666 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +chmod 666 $(foreach,file,$(HXFILES_CONVERT) $(file))
dos2unx .SETDIR=$(DESTDIR):
    +echo dos2unx
    +-any2all $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-any2all $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-any2all $(foreach,file,$(HXFILES_CONVERT) $(file))
roit .SETDIR=$(DESTDIR):
    +echo roit
    +chmod 444 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +chmod 444 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +chmod 444 $(foreach,file,$(HXFILES_CONVERT) $(file))
.ENDIF
