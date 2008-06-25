#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.88 $
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

PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

IDLLIST:={$(subst,/,$/ $(shell @$(FIND) $(IDLOUT) -type f | sed -e '/star.portal/d' -e'/star.webservices/d'))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

DESTINCLUDELIST={$(subst,$(SOLARINCDIR),$(DESTDIRINC) $(INCLUDELIST))}
DESTINCLUDELIST+=$(DESTDIRINC)$/udkversion.mk

DESTINCDIRLIST={$(subst,$(INCOUT),$(DESTDIRINC) $(INCLUDEDIRLIST))}

#COMPONENTLIST = \
#	acceptor	\
#	bridgefac	\
#	connector	\
#	reflection		\
#	shlibloader		\
#	nestedreg		\
#	dynamicloader	\
#	implreg		\
#	introspection		\
#	invocation		\
#	invocadapt		\
#	javavm		\
#	namingservice	\
#	proxyfac	\
#	regtypeprov		\
#	remotebridge	\
#	simplereg		\
#	servicemgr		\
#	streams		\
#	typeconverter		\
#	typemgr		\
#	uuresolver \
#	javaloader \
#	security

#XMLLIST= \
#	$(foreach,c,$(COMPONENTLIST) $(DESTDIRXML)$/$c.uno.xml) \
#	$(DESTDIRXML)$/module-description.dtd

EXELIST = \
    $(DESTDIRBIN)$/cppumaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/sp2bv$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regcompare$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regmerge$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/regview$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/rdbmaker$(EXEPOSTFIX)	\
    $(DESTDIRBIN)$/idlcpp$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/idlc$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/javamaker$(EXEPOSTFIX) 	\
    $(DESTDIRBIN)$/xml2cmp$(EXEPOSTFIX)	\
    $(DESTDIRBIN)$/autodoc$(EXEPOSTFIX) \
    $(DESTDIRBIN)$/unoapploader$(EXEPOSTFIX) \
    $(DESTDIRBIN)$/uno-skeletonmaker$(EXEPOSTFIX)

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
EXELIST += \
    $(DESTDIRBIN)$/regcomp$(EXEPOSTFIX)
.ELSE
EXELIST += \
    $(DESTDIRBIN)$/regcomp$(EXEPOSTFIX) \
    $(DESTDIRBIN)$/climaker$(EXEPOSTFIX)
.ENDIF
.ELSE
EXELIST += \
    $(DESTDIRBIN)$/regcomp.bin 	\
    $(DESTDIRBIN)$/regcomp
.ENDIF

.IF "$(GUI)"=="WNT"
CLILIST = \
    $(DESTDIRCLI)$/cli_basetypes.dll 	\
    $(DESTDIRCLI)$/cli_uretypes.dll 	\
    $(DESTDIRCLI)$/cli_oootypes.dll 	\
    $(DESTDIRCLI)$/cli_ure.dll 	\
    $(DESTDIRCLI)$/cli_cppuhelper.dll
.ENDIF


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
.ELIF "$(COMEX)"=="10"
    LIBLIST += $(DESTDIRLIB)$/stlport_vc71.lib
.ELSE
    LIBLIST += $(DESTDIRLIB)$/stlport_vc6.lib
.ENDIF
.ENDIF

.IF "$(OS)"=="LINUX"
    LIBLIST = $(DESTDIRLIB)$/libsalcpprt.a
.ENDIF

# note, that inner class files are copied implicitly
DESTCLASSESLIST= \
    $(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/Loader.class \
    $(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/InstallationFinder.class \
    $(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/WinRegKey.class \
    $(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/WinRegKeyException.class
#	$(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/Loader$$CustomURLClassLoader.class \
#	$(DESTDIRJAR)$/com$/sun$/star$/lib$/loader$/InstallationFinder$$StreamGobbler.class \

SETTINGSLIST= \
    $(DESTDIRSETTINGS)$/settings.mk \
    $(DESTDIRSETTINGS)$/std.mk \
    $(DESTDIRSETTINGS)$/stdtarget.mk

.IF "$(GUI)"=="WNT"
SETTINGSLIST+=$(DESTDIRSETTINGS)$/component.uno.def
.ENDIF

.IF "$(GUI)"=="UNX"
SETTINGSLIST+=$(DESTDIRSETTINGS)$/component.uno.map
.ENDIF


DOCUHTMLFILES+= \
    $(DESTDIR2)$/index.html \
    $(DESTDIRDOCU2)$/tools.html \
    $(DESTDIRDOCU2)$/notsupported.html \
    $(DESTDIRDOCU2)$/install.html \
    $(DESTDIREXAMPLES2)$/examples.html \
    $(DESTDIREXAMPLES2)$/DevelopersGuide$/examples.html

DOCUFILES= \
    $(DESTDIRDOCUIMAGES)$/nada.gif \
    $(DESTDIRDOCUIMAGES)$/arrow-2.gif \
    $(DESTDIRDOCUIMAGES)$/bluball.gif \
    $(DESTDIRDOCUIMAGES)$/so-main-app_32.png \
    $(DESTDIRDOCUIMAGES)$/ooo-main-app_32.png

DOCUFILES_OO= \
    $(DOCUHTMLFILES) \
    $(DESTDIRDOCUIMAGES2)$/arrow-1.gif \
    $(DESTDIRDOCUIMAGES2)$/arrow-3.gif \
    $(DESTDIRDOCUIMAGES2)$/bg_table.gif \
    $(DESTDIRDOCUIMAGES2)$/bg_table2.gif \
    $(DESTDIRDOCUIMAGES2)$/bg_table3.gif \
    $(DESTDIRDOCUIMAGES2)$/nav_down.png \
    $(DESTDIRDOCUIMAGES2)$/nav_home.png \
    $(DESTDIRDOCUIMAGES2)$/nav_left.png \
    $(DESTDIRDOCUIMAGES2)$/nav_right.png \
    $(DESTDIRDOCUIMAGES2)$/nav_up.png \
    $(DESTDIRDOCUIMAGES2)$/sdk_head-1.gif \
    $(DESTDIRDOCUIMAGES2)$/sdk_head-2.gif \
    $(DESTDIRDOCUIMAGES2)$/sdk_head-3.gif \
    $(DESTDIRDOCUIMAGES2)$/sdk_line-1.gif \
    $(DESTDIRDOCUIMAGES2)$/sdk_line-2.gif

.IF "$(GUI)"=="UNX"
INSTALLSCRIPT= \
    $(DESTDIR)$/config.guess \
    $(DESTDIR)$/config.sub \
    $(DESTDIR)$/configure.pl \
    $(DESTDIR)$/setsdkenv_unix \
    $(DESTDIR)$/setsdkenv_unix.sh.in \
    $(DESTDIR)$/setsdkenv_unix.csh.in
.ELSE
INSTALLSCRIPT= \
    $(DESTDIR)$/setsdkenv_windows.bat \
    $(DESTDIR)$/setsdkenv_windows.template \
    $(DESTDIR)$/setsdkname.bat \
    $(DESTDIR)$/cfgWin.js
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
    $(DESTCLASSESLIST)

.IF "$(SOLAR_JAVA)" != ""
DIR_FILE_LIST += $(DESTDIRJAR)$/win$/unowinreg.dll
.ENDIF

DIR_DIRECTORY_LIST:=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG:=$(MISC)$/copying_dirs_created.txt
DIR_FILE_FLAG:=$(MISC)$/copying_files.txt

.IF "$(SOLAR_JAVA)" != ""
# Special work for simple uno bootstrap mechanism
# zip uno loader class files and winreg helper library for later
# use in the build process (e.g. helper tools)
MYZIPTARGET=$(BIN)$/uno_loader_classes.zip
MYZIPFLAGS=-u -r
MYZIPDIR=$(DESTDIRJAR)
MYZIPLIST=com$/* win$/*

.ENDIF

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG) \

#--------------------------------------------------
# use global rules
#--------------------------------------------------
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk

.IF "$(SOLAR_JAVA)" != ""
$(MYZIPTARGET) : $(BIN)$/unowinreg.dll $(DESTCLASSESLIST)
    cd $(MYZIPDIR) && zip $(MYZIPFLAGS) ..$/..$/..$/bin$/$(MYZIPTARGET:b) $(MYZIPLIST)
.ENDIF
