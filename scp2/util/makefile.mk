#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.45 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:41:43 $
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
             module_testtool.par           \
             file_testtool.par             \
             file_lingu.par                \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             shortcut_python.par           \
             ure_into_ooo.par              \
             ure.par                       \
             common_brand.par              \
             common_brand_readme.par       \
             ooo_brand.par

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
             registryitem_quickstart.par

.IF "$(ENABLE_DIRECTX)"!=""
SCP1FILES += directxcanvas.par
.ENDIF

.IF "$(DISABLE_ACTIVEX)"==""
SCP1FILES +=                           \
             module_activex.par        \
             file_activex.par          \
             module_winexplorerext.par \
             file_winexplorerext.par   \
             registryitem_winexplorerext.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GNOMEVFS)" != ""
SCP1FILES += \
             module_gnome.par   \
             file_gnome.par
.ENDIF

.IF "$(ENABLE_KDE)" == "TRUE"
SCP1FILES += \
             module_kde.par     \
             file_kde.par
.ENDIF

.IF "$(GUIBASE)" == "unx"
SCP1FILES += module_headless.par
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != ""
SCP1FILES += \
             file_crashrep_dynamic.par
.ENDIF

SCP1FILES += \
             vclcanvas.par

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

.IF "$(OS)"=="MACOSX" && "$(GUIBASE)"=="aqua"
SCP1FILES += aqua_ooo.par
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
             module_testtool.par           \
             file_testtool.par             \
             file_lingu.par                \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             module_python_mailmerge.par   \
             file_python.par               \
             profileitem_python.par        \
             shortcut_python.par           \
             ure_into_ooo.par              \
             ure.par                       \
             common_brand.par              \
             common_brand_readme.par       \
             ooo_brand.par

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
             registryitem_quickstart.par

.IF "$(DISABLE_ACTIVEX)"==""
SCP2FILES +=                           \
             module_activex.par        \
             file_activex.par          \
             module_winexplorerext.par \
             file_winexplorerext.par   \
             registryitem_winexplorerext.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GNOMEVFS)" != ""
SCP2FILES += \
             module_gnome.par   \
             file_gnome.par
.ENDIF

.IF "$(ENABLE_KDE)" == "TRUE"
SCP2FILES += \
             module_kde.par     \
             file_kde.par
.ENDIF

.IF "$(GUIBASE)" == "unx"
SCP2FILES += module_headless.par
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != ""
SCP2FILES += \
             file_crashrep_dynamic.par
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

.IF "$(OS)"=="MACOSX" && "$(GUIBASE)"=="aqua"
SCP2FILES += aqua_ooo.par
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

SCP4LINK_PRODUCT_TYPE = sdk
SCP4TARGET = sdkoo
SCP4FILES = sdkoo.par

# --- target -------------------------------------------------------------
.INCLUDE :  target.mk

