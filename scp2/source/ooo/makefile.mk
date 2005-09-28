#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.35 $
#
#   last change: $Author: hr $ $Date: 2005-09-28 15:47:30 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
PRJ=..$/..

PRJPCH=

PRJNAME=scp2
TARGET=ooo
TARGETTYPE=CUI

USE_JAVAVER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  icuversion.mk

.IF "$(ENABLE_CRASHDUMP)"!=""
SCPDEFS+=-DENABLE_CRASHDUMP
.ENDIF

.IF "$(JAVANUMVER)" >= "000100040000"
SCPDEFS+=-DINCLUDE_JAVA_ACCESSBRIDGE
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GTK)" != ""
SCPDEFS+=-DENABLE_GTK
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
GTK_TWO_FOUR=$(shell +-$(PKGCONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo YES)
.IF "$(GTK_TWO_FOUR)" != ""
SCPDEFS+=-DGTK_TWO_FOUR
.ENDIF
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(ENABLE_PASF)" != ""
SCPDEFS+=-DUSE_PASF
.ENDIF

.IF "$(ENABLE_KDE)" != ""
SCPDEFS+=-DENABLE_KDE
.ENDIF

.IF "$(ENABLE_EVOAB2)" != ""
SCPDEFS+=-DENABLE_EVOAB2
.ENDIF

.IF "$(ENABLE_DIRECTX)" != ""
SCPDEFS+=-DENABLE_DIRECTX
.ENDIF

.IF "$(SYSTEM_FREETYPE)" == "YES"
SCPDEFS+=-DSYSTEM_FREETYPE
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXML
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
SCPDEFS+=-DSYSTEM_DB
.ENDIF

.IF "$(USE_SYSTEM_STL)" == "YES"
SCPDEFS+=-DUSE_SYSTEM_STL
.ENDIF

.IF "$(WITH_MOZILLA)" == "NO"
SCPDEFS+=-DWITHOUT_MOZILLA
.ENDIF

.IF "$(SYSTEM_MOZILLA)" == "YES"
SCPDEFS+=-DSYSTEM_MOZILLA
.ENDIF

.IF "$(WITH_OPENLDAP)" == "YES"
SCPDEFS+=-DWITH_OPENLDAP
.ENDIF

.IF "$(SYSTEM_CURL)" == "YES"
SCPDEFS+=-DSYSTEM_CURL
.ENDIF

.IF "$(WITH_BINFILTER)" == "NO"
SCPDEFS+=-DWITHOUT_BINFILTER
.ENDIF

.IF "$(SYSTEM_STDLIBS)" == "YES"
SCPDEFS+=-DSYSTEM_STDLIBS
.ENDIF

.IF "$(SYSTEM_SNDFILE)" == "YES"
SCPDEFS+=-DSYSTEM_SNDFILE
.ENDIF

.IF "$(SYSTEM_PORTAUDIO)" == "YES"
SCPDEFS+=-DSYSTEM_PORTAUDIO
.ENDIF

.IF "$(SYSTEM_MYSPELL)" == "YES"
SCPDEFS+=-DSYSTEM_MYSPELL
.ENDIF

.IF "$(SYSTEM_XALAN)" == "YES"
SCPDEFS+=-DSYSTEM_XALAN
.ENDIF

SCPDEFS+=\
    -DICU_MAJOR=$(ICU_MAJOR) \
    -DICU_MINOR=$(ICU_MINOR) \
    -DICU_MICRO=$(ICU_MICRO)

SCP_PRODUCT_TYPE=osl

PARFILES=                          \
        installation_ooo.par       \
        scpaction_ooo.par          \
        directory_ooo.par          \
        datacarrier_ooo.par        \
        file_ooo.par               \
        file_images_ooo.par        \
        file_extra_ooo.par         \
        file_font_ooo.par          \
        file_library_ooo.par       \
        file_resource_ooo.par      \
        shortcut_ooo.par           \
        module_ooo.par             \
        module_hidden_ooo.par      \
        profile_ooo.par            \
        profileitem_ooo.par

.IF "$(GUI)"=="WNT"
PARFILES +=                        \
        file_jre_ooo.par           \
        folder_ooo.par             \
        folderitem_ooo.par         \
        registryitem_ooo.par
.ENDIF

ULFFILES=                          \
        directory_ooo.ulf          \
        folderitem_ooo.ulf         \
        module_ooo.ulf             \
        registryitem_ooo.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
