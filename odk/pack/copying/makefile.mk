#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=../..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)/util/makefile.pmk
#----------------------------------------------------------------

IDLLIST:={$(subst,/,/ $(shell @$(FIND) $(IDLOUT)/com -type f))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

DESTINCLUDELIST={$(subst,$(SOLARINCDIR),$(DESTDIRINC) $(INCLUDELIST))}
DESTINCLUDELIST+=$(DESTDIRINC)/udkversion.mk

DESTINCDIRLIST={$(subst,$(INCOUT),$(DESTDIRINC) $(INCLUDEDIRLIST))}

IDL_DOCU_INDEX_FILE = $(DESTDIRGENIDLREF)/module-ix.html
IDL_DOCU_CLEANUP_FLAG = $(MISC)/idl_docu_cleanup.flag

IDL_CHAPTER_REFS=idl_chapter_refs.txt

EXELIST = \
    $(DESTDIRBIN)/cppumaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)/regcompare$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)/idlc$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)/javamaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)/autodoc$(EXEPOSTFIX) \
    $(DESTDIRBIN)/unoapploader$(EXEPOSTFIX) \
    $(DESTDIRBIN)/uno-skeletonmaker$(EXEPOSTFIX)

.IF "$(SYSTEM_UCPP)" == ""
EXELIST += \
	$(DESTDIRBIN)/ucpp$(EXEPOSTFIX)
.ENDIF

.IF "$(GUI)"=="WNT"
EXELIST += \
    $(DESTDIRBIN)/climaker$(EXEPOSTFIX)
.ENDIF

.IF "$(GUI)"=="WNT"
CLILIST = \
    $(DESTDIRCLI)/cli_basetypes.dll 	\
    $(DESTDIRCLI)/cli_uretypes.dll 	\
    $(DESTDIRCLI)/cli_oootypes.dll 	\
    $(DESTDIRCLI)/cli_ure.dll 	\
    $(DESTDIRCLI)/cli_cppuhelper.dll
.ENDIF


.IF "$(GUI)"=="WNT"
LIBLIST = \
    $(DESTDIRLIB)/istore.lib 	\
    $(DESTDIRLIB)/ireg.lib 	\
    $(DESTDIRLIB)/isal.lib 	\
    $(DESTDIRLIB)/isalhelper.lib 	\
    $(DESTDIRLIB)/icppu.lib 	\
    $(DESTDIRLIB)/icppuhelper.lib 	\
    $(DESTDIRLIB)/irmcxt.lib \
    $(DESTDIRLIB)/ipurpenvhelper3MSC.lib
.ENDIF

.IF "$(OS)"=="LINUX"
    LIBLIST = $(DESTDIRLIB)/libsalcpprt.a
.ENDIF

# note, that inner class files are copied implicitly
DESTCLASSESLIST= \
    $(DESTDIRJAR)/com/sun/star/lib/loader/Loader.class \
    $(DESTDIRJAR)/com/sun/star/lib/loader/InstallationFinder.class \
    $(DESTDIRJAR)/com/sun/star/lib/loader/WinRegKey.class \
    $(DESTDIRJAR)/com/sun/star/lib/loader/WinRegKeyException.class
#	$(DESTDIRJAR)/com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class \
#	$(DESTDIRJAR)/com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class \

SETTINGSLIST= \
    $(DESTDIRSETTINGS)/settings.mk \
    $(DESTDIRSETTINGS)/std.mk \
    $(DESTDIRSETTINGS)/stdtarget.mk \
    $(DESTDIRSETTINGS)/dk.mk


.IF "$(GUI)"=="WNT"
SETTINGSLIST+=$(DESTDIRSETTINGS)/component.uno.def
.ENDIF

.IF "$(GUI)"=="UNX"
SETTINGSLIST+=$(DESTDIRSETTINGS)/component.uno.map
.ENDIF


DOCUHTMLFILES+= \
    $(DESTDIR)/index.html \
    $(DESTDIRDOCU)/tools.html \
    $(DESTDIRDOCU)/notsupported.html \
    $(DESTDIRDOCU)/install.html \
    $(DESTDIREXAMPLES)/examples.html \
    $(DESTDIREXAMPLES)/DevelopersGuide/examples.html

DOCUFILES= \
    $(DOCUHTMLFILES) \
    $(DESTDIRDOCU)/sdk_styles.css \
    $(DESTDIRGENIDLREF)/idl.css \
    $(DESTDIRDOCUIMAGES)/nada.gif \
    $(DESTDIRDOCUIMAGES)/arrow-2.gif \
    $(DESTDIRDOCUIMAGES)/bluball.gif \
    $(DESTDIRDOCUIMAGES)/orc-main-app_32.png \
    $(DESTDIRDOCUIMAGES)/ooo-main-app_32.png \
    $(DESTDIRDOCUIMAGES)/odk-footer-logo.gif \
    $(DESTDIRDOCUIMAGES)/arrow-1.gif \
    $(DESTDIRDOCUIMAGES)/arrow-3.gif \
    $(DESTDIRDOCUIMAGES)/bg_table.png \
    $(DESTDIRDOCUIMAGES)/bg_table2.png \
    $(DESTDIRDOCUIMAGES)/bg_table3.png \
    $(DESTDIRDOCUIMAGES)/nav_down.png \
    $(DESTDIRDOCUIMAGES)/nav_home.png \
    $(DESTDIRDOCUIMAGES)/nav_left.png \
    $(DESTDIRDOCUIMAGES)/nav_right.png \
    $(DESTDIRDOCUIMAGES)/nav_up.png \
    $(DESTDIRDOCUIMAGES)/sdk_head-1.png \
    $(DESTDIRDOCUIMAGES)/sdk_head-2.png \
    $(DESTDIRDOCUIMAGES)/sdk_line-1.gif \
    $(DESTDIRDOCUIMAGES)/sdk_line-2.gif

.IF "$(GUI)"=="UNX"
INSTALLSCRIPT= \
    $(DESTDIR)/config.guess \
    $(DESTDIR)/config.sub \
    $(DESTDIR)/configure.pl \
    $(DESTDIR)/setsdkenv_unix \
    $(DESTDIR)/setsdkenv_unix.sh.in \
    $(DESTDIR)/setsdkenv_unix.csh.in
.ELSE
INSTALLSCRIPT= \
    $(DESTDIR)/setsdkenv_windows.bat \
    $(DESTDIR)/setsdkenv_windows.template \
    $(DESTDIR)/setsdkname.bat \
    $(DESTDIR)/cfgWin.js
.ENDIF

DIR_FILE_LIST=\
    $(EXELIST) \
    $(CLILIST) \
    $(LIBLIST) \
    $(SETTINGSLIST) \
    $(DOCUFILES) \
    $(INSTALLSCRIPT) \
    $(DESTIDLLIST)  \
    $(DESTINCLUDELIST) \
    $(DESTCLASSESLIST) \
    $(IDL_DOCU_INDEX_FILE) \
    $(CONVERTTAGFLAG) 

.IF "$(OS)"=="MACOSX"
DIR_FILE_LIST += $(DESTDIRBIN)/addsym-macosx.sh
.ENDIF
    
.IF "$(SOLAR_JAVA)" != ""
DIR_FILE_LIST += $(DESTDIRJAR)/win/unowinreg.dll
.ENDIF

DIR_DIRECTORY_LIST:=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG:=$(MISC)/copying_dirs_created.txt
DIR_FILE_FLAG:=$(MISC)/copying_files.txt

.IF "$(SOLAR_JAVA)" != ""
# Special work for simple uno bootstrap mechanism
# zip uno loader class files and winreg helper library for later
# use in the build process (e.g. helper tools)
MYZIPTARGET=$(BIN)/uno_loader_classes.zip
MYZIPFLAGS=-u -r
MYZIPDIR=$(DESTDIRJAR)
MYZIPLIST=com/* win/*

.ENDIF

#--------------------------------------------------
# TARGETS
#--------------------------------------------------

.IF "$(L10N_framework)"==""

all : CLEAN_DKVERSION_CHECK \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG) \

.ENDIF

#--------------------------------------------------
# use global rules
#--------------------------------------------------
.INCLUDE: $(PRJ)/util/odk_rules.pmk


CLEANUP_DKVERSION_FILE=$(DESTDIRSETTINGS)/dk.mk

    
CLEAN_DKVERSION_CHECK:
    @@-rm -f $(CLEANUP_DKVERSION_FILE)

$(DESTDIRBIN)/addsym-macosx.sh : addsym-macosx.sh
    @@-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $< $(MY_TEXTCOPY_TARGETPRE) $@
    -chmod 755 $@

$(DESTDIRSETTINGS)/dk.mk : dk.mk
    @@-rm -f $@
    -$(MKDIRHIER) $(@:d)
    tr -d "\015" < dk.mk | sed -e 's/@@RELEASE@@/$(PRODUCT_RELEASE)/' -e 's/@@BUILDID@@/$(RSCREVISION)/'> $@

$(IDL_DOCU_CLEANUP_FLAG) : $(IDL_CHAPTER_REFS) $(PRJ)/docs/common/ref/idl.css $(DESTIDLLIST)
    @@-$(MY_DELETE_RECURSIVE) $(DESTDIRGENIDLREF)
    $(TOUCH) $@

$(IDL_DOCU_INDEX_FILE) : $(IDL_DOCU_CLEANUP_FLAG) 
    -$(MKDIRHIER) $(@:d) 
    $(MY_AUTODOC) -html $(DESTDIRGENIDLREF) -dvgroot "http://wiki.services.openoffice.org/wiki" \
        -name $(IDLDOCREFNAME) -lg idl -dvgfile $(IDL_CHAPTER_REFS) -t $(DESTDIRIDL)
    -rm $(@:d)/idl.css
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(PRJ)/docs/common/ref/idl.css $(MY_TEXTCOPY_TARGETPRE) $(@:d:d)/idl.css

 
.IF "$(SOLAR_JAVA)" != ""
$(MYZIPTARGET) : $(BIN)/unowinreg.dll $(DESTCLASSESLIST)
    cd $(MYZIPDIR) && zip $(MYZIPFLAGS) ../../../bin/$(MYZIPTARGET:b) $(MYZIPLIST)
.ENDIF
