#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.70 $
#
#   last change: $Author: rt $ $Date: 2005-01-31 17:21:09 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f | sed -e '/star.portal/d' -e'/star.webservices/d'))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

DESTINCLUDELIST={$(subst,$(SOLARINCDIR),$(DESTDIRINC) $(INCLUDELIST))}
DESTINCDIRLIST={$(subst,$(INCOUT),$(DESTDIRINC) $(INCLUDEDIRLIST))}

COMPONENTLIST = \
    acceptor	\
    bridgefac	\
    connector	\
    reflection		\
    shlibloader		\
    nestedreg		\
    dynamicloader	\
    implreg		\
    introspection		\
    invocation		\
    invocadapt		\
    javavm		\
    namingservice	\
    proxyfac	\
    regtypeprov		\
    remotebridge	\
    simplereg		\
    servicemgr		\
    streams		\
    typeconverter		\
    typemgr		\
    uuresolver \
    javaloader \
    security

XMLLIST= \
    $(foreach,c,$(COMPONENTLIST) $(DESTDIRXML)$/$c.uno.xml) \
    $(DESTDIRXML)$/module-description.dtd

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
    $(DESTDIRBIN)$/unoapploader$(EXEPOSTFIX)

.IF "$(GUI)"=="WNT"
EXELIST += \
    $(DESTDIRBIN)$/regcomp$(EXEPOSTFIX)
.ELSE
EXELIST += \
    $(DESTDIRBIN)$/regcomp.bin 	\
    $(DESTDIRBIN)$/regcomp
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

DOCUSTUDIO4INTEGRATIONHTMLFILES= \
    $(DESTDIRDOCU)$/DevStudioWizards$/CalcAddinWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/DeployingComponents.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/IDLWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/UNOComponentWizard.html \
    $(DESTDIRDOCU)$/DevStudioWizards$/wizards.html

DOCUSTUDIO4INTEGRATIONGRAPHICSFILES= \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/AddInsDescription.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ChooseIDL.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/DesignIDL.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ExecuteAntScript.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/GeneratedJavaCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/InterfacesToImplementCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/InterfacesToImplementCalcAddinsExtended.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/MountWorkingDirectory.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewObjectName.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewOfficeIDL.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/NewOfficeObject.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/OutputWindowBuild.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ReturnCommand.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/SelectModule.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ServiceSetupPanelCalcAddins.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/ServiceSetupPanelCalcAddinsExtended.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/mount_directory.jpg \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office01.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office02.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office03.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office04.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office05.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office06.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office07.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office08.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/office09.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot10.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot11.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot12.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot13.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot14.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot15.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot16b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot17b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot18b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot20b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot21.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot22.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot23.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot24.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot25.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot26.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot27b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot3.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot32b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot33b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot34b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot35b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot42b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot43b.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot8.png \
    $(DESTDIRDOCU)$/DevStudioWizards$/images$/snapshot9.png

DOCUHTMLFILES+= \
    $(DESTDIR)$/index.html \
    $(DESTDIRDOCU)$/tools.html \
    $(DESTDIRDOCU)$/notsupported.html \
    $(DESTDIRDOCU)$/DevelopersGuide_intro.html \
    $(DESTDIRDOCU)$/install.html \
    $(DESTDIRDOCU)$/sdk_styles.css \
    $(DESTDIREXAMPLES)$/examples.html \
    $(DESTDIREXAMPLES)$/DevelopersGuide$/examples.html
#	$(DOCUSTUDIO4INTEGRATIONHTMLFILES) \

DOCUFILES+= \
    $(DOCUHTMLFILES) \
    $(DESTDIRDOCUIMAGES)$/nada.gif \
    $(DESTDIRDOCUIMAGES)$/arrow-1.gif \
    $(DESTDIRDOCUIMAGES)$/arrow-2.gif \
    $(DESTDIRDOCUIMAGES)$/arrow-3.gif \
    $(DESTDIRDOCUIMAGES)$/bg_table.gif \
    $(DESTDIRDOCUIMAGES)$/bluball.gif \
    $(DESTDIRDOCUIMAGES)$/nada.gif \
    $(DESTDIRDOCUIMAGES)$/nav_down.png \
    $(DESTDIRDOCUIMAGES)$/nav_home.png \
    $(DESTDIRDOCUIMAGES)$/nav_left.png \
    $(DESTDIRDOCUIMAGES)$/nav_right.png \
    $(DESTDIRDOCUIMAGES)$/nav_up.png \
    $(DESTDIRDOCUIMAGES)$/sdk_head-1.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_head-2.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_head-3.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_line-1.gif \
    $(DESTDIRDOCUIMAGES)$/sdk_line-2.gif \
    $(DESTDIRDOCUIMAGES)$/so-main-app_32.png \
    $(DESTDIRDOCUIMAGES)$/ooo-main-app_32.png

#	$(DOCUSTUDIO4INTEGRATIONGRAPHICSFILES) \

.IF "$(GUI)"=="UNX"
INSTALLSCRIPT= \
    $(DESTDIR)$/configure \
    $(DESTDIR)$/configure.pl \
    $(DESTDIR)$/setsdkenv_unix \
    $(DESTDIR)$/setsdkenv_unix.sh.in \
    $(DESTDIR)$/setsdkenv_unix.csh.in
.ELSE
INSTALLSCRIPT= \
    $(DESTDIR)$/configureWindows.bat \
    $(DESTDIR)$/setsdkenv_windows.bat \
    $(DESTDIR)$/cfgWin.js
.ENDIF

DIR_FILE_LIST=\
    $(EXELIST) \
    $(XMLLIST) \
    $(LIBLIST) \
    $(SETTINGSLIST) \
    $(DOCUFILES) \
    $(INSTALLSCRIPT) \
    $(DESTIDLLIST)  \
    $(DESTINCLUDELIST) \
    $(DESTCLASSESLIST) \
    $(DESTDIRJAR)$/win$/unowinreg.dll \
    $(DESTDIRDOCU)$/common$/spec$/xml_format$/xml_specification.pdf
#	$(DESTDIRCLASSES)$/oosupport.nbm \

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/copying_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/copying_files.txt

# Special work for simple uno bootstrap mechanism
# zip uno loader class files and winreg helper library for later
# use in the build process (e.g. helper tools)
MYZIPTARGET=$(BIN)$/uno_loader_classes.zip
MYZIPFLAGS=-u -r
MYZIPDIR=$(DESTDIRJAR)
MYZIPLIST=com$/* win$/*

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG) \
    $(MYZIPTARGET)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk

$(MYZIPTARGET) : $(DESTDIRJAR)$/win$/unowinreg.dll $(DESTCLASSESLIST)
    +cd $(MYZIPDIR) && zip $(MYZIPFLAGS) ..$/..$/..$/bin$/$(MYZIPTARGET:b) $(MYZIPLIST)

