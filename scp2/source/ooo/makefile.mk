#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.65 $
#
#   last change: $Author: obo $ $Date: 2008-03-26 12:40:03 $
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
.INCLUDE :  i18npool/version.mk

.IF "$(ENABLE_CRASHDUMP)"!=""
SCPDEFS+=-DENABLE_CRASHDUMP
.ENDIF

.IF "$(BUILD_SPECIAL)"!=""
SCPDEFS+=-DBUILD_SPECIAL
.ENDIF

.IF "$(JAVANUMVER)" >= "000100040000"
SCPDEFS+=-DINCLUDE_JAVA_ACCESSBRIDGE
.ENDIF

.IF "$(PROF_EDITION)"!=""
SCPDEFS+=-DPROF_EDITION
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GTK)" != ""
SCPDEFS+=-DENABLE_GTK
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
GTK_TWO_FOUR=$(shell @+-$(PKG_CONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo YES)
.IF "$(GTK_TWO_FOUR)" != ""
SCPDEFS+=-DGTK_TWO_FOUR
.ENDIF
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(ENABLE_SYSTRAY_GTK)" != ""
SCPDEFS+=-DENABLE_SYSTRAY_GTK
.ENDIF

.IF "$(ENABLE_PASF)" != ""
SCPDEFS+=-DUSE_PASF
.ENDIF

.IF "$(ENABLE_KDE)" != ""
SCPDEFS+=-DENABLE_KDE
.ENDIF

.IF "$(ENABLE_KAB)" != ""
SCPDEFS+=-DENABLE_KAB
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

.IF "$(SYSTEM_CAIRO)" == "YES"
SCPDEFS+=-DSYSTEM_CAIRO
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXML
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXSLT
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
SCPDEFS+=-DSYSTEM_DB
.ENDIF

.IF "$(USE_SYSTEM_STL)" == "YES" || "$(STLPORT4)" != "NO_STLPORT4"
SCPDEFS+=-DUSE_SYSTEM_STL
.ENDIF

.IF "$(WITH_MOZILLA)" == "NO"
SCPDEFS+=-DWITHOUT_MOZILLA
.ENDIF

.IF "$(SYSTEM_MOZILLA)" == "YES"
SCPDEFS+=-DSYSTEM_MOZILLA
.ENDIF

.IF "$(WITH_LDAP)" == "YES"
SCPDEFS+=-DWITH_LDAP
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

.IF "$(SYSTEM_HUNSPELL)" == "YES"
SCPDEFS+=-DSYSTEM_HUNSPELL
.ENDIF

.IF "$(SYSTEM_BSH)" == "YES"
SCPDEFS+=-DSYSTEM_BSH -DBSH_JAR=\""file://$(BSH_JAR)"\"
.ENDIF

.IF "$(SYSTEM_HSQLDB)" == "YES"
SCPDEFS+=-DSYSTEM_HSQLDB -DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\"
.ENDIF

.IF "$(SYSTEM_XML_APIS)" == "YES"
SCPDEFS+=-DSYSTEM_XML_APIS -DXML_APIS_JAR=\""file://$(XML_APIS_JAR)"\"
.ENDIF

.IF "$(SYSTEM_XERCES)" == "YES"
SCPDEFS+=-DSYSTEM_XERCES -DXERCES_JAR=\""file://$(XERCES_JAR)"\"
.ENDIF

.IF "$(SYSTEM_XALAN)" == "YES"
SCPDEFS+=-DSYSTEM_XALAN -DXALAN_JAR=\""file://$(XALAN_JAR)"\"
.IF "$(SERIALIZER_JAR)" != ""
SCPDEFS+=-DSYSTEM_SERIALIZER -DSERIALIZER_JAR=\""file://$(SERIALIZER_JAR)"\"
.ENDIF
.ENDIF

.IF "$(JDK)" == "gcj"
SCPDEFS+=-DGCJ
.ENDIF


.IF "$(ENABLE_CAIRO)" == "TRUE"
SCPDEFS+=-DENABLE_CAIRO
.ENDIF

.IF "$(SYSTEM_ICU)" == "YES"
SCPDEFS+=-DSYSTEM_ICU
.ELSE
.INCLUDE :  icuversion.mk
SCPDEFS+=\
    -DICU_MAJOR=$(ICU_MAJOR) \
    -DICU_MINOR=$(ICU_MINOR) \
    -DICU_MICRO=$(ICU_MICRO)
.ENDIF

SCPDEFS+=-DISOLANG_MAJOR=$(ISOLANG_MAJOR)

.IF "$(DISABLE_NEON)" == "TRUE"
SCPDEFS+=-DDISABLE_NEON
.ENDIF

# if yes or unset (neon not used) -> do not install openssl library!
.IF $(SYSTEM_OPENSSL) != "YES"
SCPDEFS+=-DOPENSSL
.ENDIF

.IF "$(ENABLE_VBA)" == "YES"
    .IF "$(VBA_EXTENSION)" == "YES"
        SCPDEFS+=-DVBA_EXTENSION
    .ENDIF
SCPDEFS+=-DENABLE_VBA
.ENDIF

.IF "$(DISABLE_ATL)"!=""
SCPDEFS+=-DDISABLE_ATL
.ENDIF

SCP_PRODUCT_TYPE=osl

ICUVERSION_DEPENDENT_FILES= \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/file_library_ooo.par \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/shortcut_ooo.par

ISOLANGVERSION_DEPENDENT_FILES= \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/file_library_ooo.par

PARFILES=                          \
        installation_ooo.par       \
        scpaction_ooo.par          \
        directory_ooo.par          \
        datacarrier_ooo.par        \
        file_ooo.par               \
        file_extra_ooo.par         \
        file_font_ooo.par          \
        file_library_ooo.par       \
        file_resource_ooo.par      \
        shortcut_ooo.par           \
        module_ooo.par             \
        module_hidden_ooo.par      \
        module_langpack.par        \
        module_lang_template.par   \
        module_java.par            \
        module_systemint.par       \
        profile_ooo.par            \
        profileitem_ooo.par        \
        ure.par                    \
        ure_into_ooo.par           \
        ure_standalone.par         \
        common_brand.par           \
        common_brand_readme.par    \
        ooo_brand.par

.IF "$(GUI)"=="WNT"
PARFILES +=                        \
        file_jre_ooo.par           \
        folder_ooo.par             \
        folderitem_ooo.par         \
        registryitem_ooo.par       \
        windowscustomaction_ooo.par
.ENDIF

.IF "$(OS)"=="MACOSX" && "$(GUIBASE)"=="aqua"
PARFILES += aqua_ooo.par
.ENDIF

ULFFILES=                          \
        directory_ooo.ulf          \
        folderitem_ooo.ulf         \
        module_ooo.ulf             \
        module_langpack.ulf        \
        module_java.ulf            \
        registryitem_ooo.ulf       \
        module_systemint.ulf

.IF "$(GUIBASE)"=="unx"
    PARFILES+=module_headless.par
    ULFFILES+=module_headless.ulf
.ENDIF

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(SYSTEM_ICU)" != "YES"
$(ICUVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/icuversion.mk
.ENDIF

$(ISOLANGVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/i18npool/version.mk

