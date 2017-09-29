#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


PRJ=..$/..

PRJPCH=

PRJNAME=scp2
TARGET=ooo
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  i18npool/version.mk

.IF "$(COM)" == "MSC"
SCPDEFS+=-D_MSC
.ENDIF

.IF "$(BUILD_SPECIAL)"!=""
SCPDEFS+=-DBUILD_SPECIAL
.ENDIF

.IF "$(BUILD_X64)"!=""
SCPDEFS+=-DBUILD_X64
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

.IF "$(ENABLE_GSTREAMER)" != ""
SCPDEFS+=-DENABLE_GSTREAMER
.ENDIF

.IF "$(ENABLE_KDE)" != ""
SCPDEFS+=-DENABLE_KDE
.ENDIF

.IF "$(ENABLE_KDE4)" != ""
SCPDEFS+=-DENABLE_KDE4
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

.IF "$(ENABLE_CAIRO)" == "YES"
SCPDEFS+=-DENABLE_CAIRO
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXML
.ENDIF

.IF "$(SYSTEM_LIBWPD)" == "YES"
SCPDEFS+=-DSYSTEM_LIBWPD
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXSLT
.ELSE
.INCLUDE :  libxsltversion.mk
SCPDEFS+=-DLIBXSLT_MAJOR=$(LIBXSLT_MAJOR)
.ENDIF

.IF "$(WITH_MYSPELL_DICTS)" == "NO"
SCPDEFS+=-DWITHOUT_MYSPELL_DICTS
.ENDIF

.IF "$(ENABLE_NSS_MODULE)" == "YES"
SCPDEFS+=-DENABLE_XMLSEC
    .IF "$(SYSTEM_NSS)" != "YES"
    SCPDEFS+=-DBUNDLE_NSS_LIBS
    .ENDIF
.ENDIF

.IF "$(WITH_LDAP)" == "YES"
SCPDEFS+=-DWITH_LDAP
.ENDIF

.IF "$(SYSTEM_CURL)" == "YES"
SCPDEFS+=-DSYSTEM_CURL
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

.IF "$(SYSTEM_REDLAND)" == "YES"
SCPDEFS+=-DSYSTEM_REDLAND
.ELSE
.INCLUDE :  redlandversion.mk
SCPDEFS+=\
    -DRAPTOR_MAJOR=$(RAPTOR_MAJOR) \
    -DRASQAL_MAJOR=$(RASQAL_MAJOR) \
    -DREDLAND_MAJOR=$(REDLAND_MAJOR)
.ENDIF

.IF "$(SYSTEM_BSH)" == "YES"
SCPDEFS+=-DSYSTEM_BSH -DBSH_JAR=\""file://$(BSH_JAR)"\"
.ENDIF

.IF "$(ENABLE_BEANSHELL)" == "YES"
SCPDEFS+=-DENABLE_BEANSHELL
.ENDIF

.IF "$(ENABLE_JAVASCRIPT)" == "YES"
SCPDEFS+=-DENABLE_JAVASCRIPT
.ENDIF

.IF "$(SYSTEM_HSQLDB)" == "YES"
SCPDEFS+=-DSYSTEM_HSQLDB -DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\"
.ENDIF

.IF "$(SYSTEM_SAXON)" == "YES"
SCPDEFS+=-DSYSTEM_SAXON -DSAXON_JAR=\""file://$(SAXON_JAR)"\"
.ENDIF

.IF "$(DISABLE_SAXON)" == "YES"
SCPDEFS+=-DDISABLE_SAXON
.ENDIF

.IF "$(DISABLE_LIBWPD)" == "TRUE"
SCPDEFS+=-DDISABLE_LIBWPD
.ENDIF

.IF "$(ENABLE_HUNSPELL)" != "YES"
SCPDEFS+=-DDISABLE_HUNSPELL
.ENDIF

.IF "$(ENABLE_HYPHEN)" != "YES"
SCPDEFS+=-DDISABLE_HYPHEN
.ENDIF

.IF "$(JDK)" == "gcj"
SCPDEFS+=-DGCJ
.ENDIF


.IF "$(ENABLE_CAIRO)" == "TRUE"
SCPDEFS+=-DENABLE_CAIRO
.ENDIF

.IF "$(SYSTEM_LUCENE)" == "YES"
SCPDEFS+=-DSYSTEM_LUCENE
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

.IF "$(SYSTEM_NEON)" == "YES"
SCPDEFS+=-DSYSTEM_NEON
.ENDIF

# if yes or unset (neon not used) -> do not install openssl library!
.IF $(SYSTEM_OPENSSL) != "YES"
SCPDEFS+=-DOPENSSL
.ENDIF

.IF "$(DISABLE_ATL)"!=""
SCPDEFS+=-DDISABLE_ATL
.ENDIF

# Detect version numbers for apr, apr-util, serf.
.IF "$(SYSTEM_APR)"!="YES"
.INCLUDE : apr_version.mk
SCPDEFS+=\
    -DAPR_MAJOR=$(APR_MAJOR)		\
    -DAPR_MINOR=$(APR_MINOR)		\
    -DAPR_MICRO=$(APR_MICRO)
.ENDIF
.IF "$(SYSTEM_APR_UTIL)"!="YES"
.INCLUDE : aprutil_version.mk
SCPDEFS+=\
    -DAPR_UTIL_MAJOR=$(APR_UTIL_MAJOR)	\
    -DAPR_UTIL_MINOR=$(APR_UTIL_MINOR)	\
    -DAPR_UTIL_MICRO=$(APR_UTIL_MICRO)
.ENDIF
.IF "$(SYSTEM_SERF)"!="YES"
.INCLUDE : serf_version.mk
SCPDEFS+=\
    -DSERF_MAJOR=$(SERF_MAJOR)		\
    -DSERF_MINOR=$(SERF_MINOR)		\
    -DSERF_MICRO=$(SERF_MICRO)
.ENDIF

.IF "$(SYSTEM_PYTHON)" == "YES"
SCPDEFS+=-DSYSTEM_PYTHON
.ENDIF

.IF "$(SYSTEM_LIBTEXTCAT)" == "YES"
SCPDEFS+=-DSYSTEM_LIBTEXTCAT
.ENDIF

.IF "$(SYSTEM_LIBTEXTCAT_DATA)" != ""
SCPDEFS+=-DSYSTEM_LIBTEXTCAT_DATA
.ENDIF

.IF "$(ENABLE_SVCTAGS)" == "YES"
SCPDEFS+=-DENABLE_SVCTAGS
.ENDIF

.IF "$(WITH_VC_REDIST)" == "YES"
SCPDEFS+=-DWITH_VC_REDIST
.ENDIF

.IF "$(MINGW_GCCDLL)"!=""
SCPDEFS+=-DMINGW_GCCDLL=\""$(MINGW_GCCDLL)"\"
.ENDIF

.IF "$(MINGW_GXXDLL)"!=""
SCPDEFS+=-DMINGW_GXXDLL=\""$(MINGW_GXXDLL)"\"
.ENDIF

.IF "$(SYSTEM_GDKPIXBUF)" == "YES"
SCPDEFS+=-DSYSTEM_GDKPIXBUF
.ENDIF

.IF "$(SYSTEM_GETTEXT)" == "YES"
SCPDEFS+=-DSYSTEM_GETTEXT
.ENDIF

.IF "$(SYSTEM_GLIB)" == "YES"
SCPDEFS+=-DSYSTEM_GLIB
.ENDIF

.IF "$(SYSTEM_LIBJPEG)" == "YES"
SCPDEFS+=-DSYSTEM_LIBJPEG
.ENDIF

.IF "$(SYSTEM_PANGO)" == "YES"
SCPDEFS+=-DSYSTEM_PANGO
.ENDIF

.IF "$(SYSTEM_APACHE_COMMONS)" == "YES"
SCPDEFS+=-DSYSTEM_APACHE_COMMONS
.ENDIF

.IF "${MACOSX_DEPLOYMENT_TARGET}" != ""
SCPDEFS+=-DMACOSX_DEPLOYMENT_TARGET_NUM=${MACOSX_DEPLOYMENT_TARGET:s/.//}
.ENDIF

# The variables ENABLE_ONLINE_UPDATE, WITHOUT_MATHMLDTD, WITH_CATA_FONTS, WITH_CATB_FONTS,
# and WITHOUT_PPDS are added (or not) in configure to SCPDEFS.

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
        directory_ooo_macosx.par   \
        datacarrier_ooo.par        \
        file_ooo.par               \
        file_extra_ooo.par         \
        file_font_ooo.par          \
        file_library_ooo.par       \
        file_resource_ooo.par      \
        file_improvement.par       \
        shortcut_ooo.par           \
        module_ooo.par             \
        module_hidden_ooo.par      \
        module_langpack.par        \
        module_lang_template.par   \
        module_java.par            \
        module_systemint.par       \
        module_improvement.par     \
        profile_ooo.par            \
        profileitem_ooo.par        \
        ure.par                    \
        ure_into_ooo.par           \
        ure_standalone.par	   \
        common_brand.par           \
        common_brand_readme.par    \
    ooo_brand.par


.IF "$(GUI)"=="WNT"
PARFILES +=                        \
        file_jre_ooo.par           \
        folder_ooo.par             \
        folderitem_ooo.par         \
        registryitem_ooo.par       \
        vc_redist.par              \
        windowscustomaction_ooo.par
.ENDIF

ULFFILES=                          \
        directory_ooo.ulf          \
        folderitem_ooo.ulf         \
        module_ooo.ulf             \
        module_langpack.ulf        \
        module_java.ulf            \
        registryitem_ooo.ulf       \
        module_systemint.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(SYSTEM_ICU)" != "YES"
$(ICUVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/icuversion.mk
.ENDIF

$(ISOLANGVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/i18npool/version.mk

