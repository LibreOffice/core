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


PRJ=..

PRJNAME=scp2
TARGET=setup

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- setup script -------------------------------------------------------
# SCPFLAGS= -w

# ------------------------------------------------------------------------
# Office OSL

SCP1LINK_PRODUCT_TYPE=osl
SCP1TARGET = setup_osl
SCP1FILES  = installation_ooo.par          \
             scpaction_ooo.par             \
             directory_ooo.par             \
             datacarrier_ooo.par           \
             file_ooo.par                  \
             file_extra_ooo.par            \
             file_font_ooo.par             \
             file_library_ooo.par          \
             file_resource_ooo.par         \
             shortcut_ooo.par              \
             module_ooo.par                \
             module_hidden_ooo.par         \
             module_langpack.par           \
             module_lang_template.par      \
             profile_ooo.par               \
             profileitem_ooo.par           \
             module_writer.par             \
             file_writer.par               \
             module_calc.par               \
             file_calc.par                 \
             module_draw.par               \
             file_draw.par                 \
             module_impress.par            \
             file_impress.par              \
             module_base.par               \
             file_base.par                 \
             canvascommons.par             \
             module_math.par               \
             file_math.par                 \
             module_systemint.par          \
             module_graphicfilter.par      \
             file_graphicfilter.par        \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             ure_into_ooo.par              \
             ure.par                       \
             module_improvement.par        \
             file_improvement.par          \
             common_brand.par              \
             common_brand_readme.par       \
             ooo_brand.par

.IF "$(OS)"=="MACOSX" 
SCP1FILES +=                               \
             directory_ooo_macosx.par      
.ENDIF
 
.IF "$(WITH_BINFILTER)" != "NO"
SCP1FILES +=                               \
             module_binfilter.par          \
             registryitem_binfilter.par    \
             file_binfilter.par
.ENDIF

.IF "$(ENABLE_OPENGL)" == "TRUE"
SCP1FILES +=                               \
             module_ogltrans.par
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
SCP1FILES +=                               \
             module_javafilter.par         \
             file_javafilter.par
.ENDIF

.IF "$(SOLAR_JAVA)"!="" 
.IF "$(GUI)"=="WNT"
SCP1FILES +=                               \
             registryitem_javafilter.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
SCP1FILES +=                           \
             registryitem_ooo.par      \
             folder_ooo.par            \
             folderitem_ooo.par        \
             windowscustomaction_ooo.par \
             vc_redist.par             \
             registryitem_writer.par   \
             folderitem_writer.par     \
             registryitem_calc.par     \
             folderitem_calc.par       \
             registryitem_draw.par     \
             folderitem_draw.par       \
             registryitem_impress.par  \
             folderitem_impress.par    \
             registryitem_base.par     \
             folderitem_base.par       \
             registryitem_math.par     \
             folderitem_math.par       \
             module_quickstart.par     \
             file_quickstart.par       \
             registryitem_quickstart.par \
             module_winexplorerext.par \
             file_winexplorerext.par   \
             registryitem_winexplorerext.par

.IF "$(ENABLE_DIRECTX)"!=""
SCP1FILES += directxcanvas.par
.ENDIF

.IF "$(DISABLE_ACTIVEX)"==""
SCP1FILES +=                           \
             module_activex.par        \
             file_activex.par 
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GCONF)"!="" || "$(ENABLE_GNOMEVFS)"!="" || "$(ENABLE_GIO)"!=""
SCP1FILES += \
             module_gnome.par   \
             file_gnome.par
.ENDIF

.IF "$(ENABLE_KDE)" == "TRUE"
SCP1FILES += \
             module_kde.par     \
             file_kde.par
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != ""
SCP1FILES += \
             file_crashrep.par
.ENDIF

SCP1FILES += \
             vclcanvas.par

.IF "$(ENABLE_CAIRO_CANVAS)" == "TRUE"
SCP1FILES += cairocanvas.par
.ENDIF

.IF "$(ENABLE_LAYOUT)" == "TRUE"
SCP1FILES += layout.par
.ENDIF # ENABLE_LAYOUT == TRUE

.IF "$(ENABLE_ONLINE_UPDATE)" != ""
SCP1FILES += \
             module_onlineupdate.par   \
             file_onlineupdate.par
.ENDIF

# ------------------------------------------------------------------------
# OpenOffice.org with JRE

SCP2LINK_PRODUCT_TYPE=osl
SCP2TARGET = setup_osljre
SCP2FILES  = installation_ooo.par          \
             scpaction_ooo.par             \
             directory_ooo.par             \
             datacarrier_ooo.par           \
             file_ooo.par                  \
             file_extra_ooo.par            \
             file_font_ooo.par             \
             file_library_ooo.par          \
             file_resource_ooo.par         \
             shortcut_ooo.par              \
             module_ooo.par                \
             module_hidden_ooo.par         \
             module_langpack.par           \
             module_lang_template.par      \
             module_java.par               \
             profile_ooo.par               \
             profileitem_ooo.par           \
             module_writer.par             \
             file_writer.par               \
             module_calc.par               \
             file_calc.par                 \
             module_draw.par               \
             file_draw.par                 \
             module_impress.par            \
             file_impress.par              \
             module_base.par               \
             file_base.par                 \
             canvascommons.par             \
             module_math.par               \
             file_math.par                 \
             module_systemint.par          \
             module_graphicfilter.par      \
             file_graphicfilter.par        \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             ure_into_ooo.par              \
             ure.par                       \
             module_improvement.par        \
             file_improvement.par          \
             common_brand.par              \
             common_brand_readme.par       \
             ooo_brand.par

.IF "$(OS)"=="MACOSX" 
SCP2FILES +=                               \
             directory_ooo_macosx.par      
.ENDIF
 
.IF "$(WITH_BINFILTER)" != "NO"
SCP2FILES +=                               \
             module_binfilter.par          \
             registryitem_binfilter.par    \
             file_binfilter.par
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
SCP2FILES +=                               \
             module_javafilter.par         \
             file_javafilter.par
.ENDIF

.IF "$(SOLAR_JAVA)"!="" 
.IF "$(GUI)"=="WNT"
SCP2FILES +=                               \
             registryitem_javafilter.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
SCP2FILES +=                           \
             file_jre_ooo.par          \
             registryitem_ooo.par      \
             folder_ooo.par            \
             folderitem_ooo.par        \
             windowscustomaction_ooo.par \
             vc_redist.par             \
             registryitem_writer.par   \
             folderitem_writer.par     \
             registryitem_calc.par     \
             folderitem_calc.par       \
             registryitem_draw.par     \
             folderitem_draw.par       \
             registryitem_impress.par  \
             folderitem_impress.par    \
             registryitem_base.par     \
             folderitem_base.par       \
             registryitem_math.par     \
             folderitem_math.par       \
             module_quickstart.par     \
             file_quickstart.par       \
             registryitem_quickstart.par \
             module_winexplorerext.par \
             file_winexplorerext.par   \
             registryitem_winexplorerext.par

.IF "$(ENABLE_DIRECTX)"!=""
SCP2FILES += directxcanvas.par
.ENDIF

.IF "$(DISABLE_ACTIVEX)"==""
SCP2FILES +=                           \
             module_activex.par        \
             file_activex.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GCONF)"!="" || "$(ENABLE_GNOMEVFS)"!="" || "$(ENABLE_GIO)"!=""
SCP2FILES += \
             module_gnome.par   \
             file_gnome.par
.ENDIF

.IF "$(ENABLE_KDE)" == "TRUE"
SCP2FILES += \
             module_kde.par     \
             file_kde.par
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != ""
SCP2FILES += \
             file_crashrep.par
.ENDIF

SCP2FILES += \
             vclcanvas.par

.IF "$(ENABLE_LAYOUT)" == "TRUE"
SCP2FILES += layout.par
.ENDIF # ENABLE_LAYOUT == TRUE

.IF "$(BUILD_SPECIAL)"!=""
SCP2FILES += \
             module_onlineupdate.par   \
             file_onlineupdate.par
.ENDIF

# ------------------------------------------------------------------------
# URE

.IF "$(OS)" != "MACOSX"
SCP3LINK_PRODUCT_TYPE = osl
SCP3TARGET = ure
SCP3FILES = ure_standalone.par  \
            ure.par
.ENDIF

# ------------------------------------------------------------------------
# SDK

SCP4LINK_PRODUCT_TYPE = osl
SCP4TARGET = sdkoo
SCP4FILES = sdkoo.par

# --- target -------------------------------------------------------------
.INCLUDE :  target.mk
