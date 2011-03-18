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
             module_helppack.par           \
             module_help_template.par      \
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
             module_extensions.par         \
             module_extensions_sun_templates.par         \
             module_extensions_lightproof.par   \
             directory_extensions.par                     \
             file_extensions.par           \
             module_testtool.par           \
             file_testtool.par             \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             module_accessories.par
.IF "$(WITH_EXTRA_GALLERY)" != ""
SCP1FILES +=                               \
             module_gallery_accessories.par   \
             file_gallery_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_SAMPLE)" != ""
SCP1FILES +=                               \
             module_samples_accessories.par   \
             file_samples_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_TEMPLATE)" != ""
SCP1FILES +=                               \
             module_templates_accessories.par \
             file_templates_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_FONT)" != ""
SCP1FILES +=                               \
             module_font_accessories.par      \
             file_font_accessories.par
.ENDIF
SCP1FILES +=                               \
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

.IF "$(ENABLE_KDE)" == "TRUE" || "$(ENABLE_KDE4)" == "TRUE"
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
             vclcanvas.par \
             mtfrenderer.par

.IF "$(ENABLE_CAIRO)" == "TRUE"
SCP1FILES += cairocanvas.par
.ENDIF

.IF "$(ENABLE_LAYOUT)" == "TRUE"
SCP1FILES += layout.par
.ENDIF # ENABLE_LAYOUT == TRUE

.IF "$(BUILD_SPECIAL)"!=""
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
             module_helppack.par           \
             module_help_template.par      \
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
             module_extensions.par         \
             module_extensions_sun_templates.par         \
             module_extensions_lightproof.par  \
             directory_extensions.par                     \
             file_extensions.par           \
             module_testtool.par           \
             file_testtool.par             \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             module_accessories.par
.IF "$(WITH_EXTRA_GALLERY)" != ""
SCP2FILES +=                               \
             module_gallery_accessories.par   \
             file_gallery_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_SAMPLE)" != ""
SCP2FILES +=                               \
             module_samples_accessories.par   \
             file_samples_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_TEMPLATE)" != ""
SCP2FILES +=                               \
             module_templates_accessories.par \
             file_templates_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_FONT)" != ""
SCP2FILES +=                               \
             module_font_accessories.par      \
             file_font_accessories.par
.ENDIF
SCP2FILES +=                           \
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

.IF "$(ENABLE_KDE)" == "TRUE" || "$(ENABLE_KDE4)" == "TRUE"
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
