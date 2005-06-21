#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
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
             file_images_ooo.par           \
             file_extra_ooo.par            \
             file_font_ooo.par             \
             file_library_ooo.par          \
             file_resource_ooo.par         \
             shortcut_ooo.par              \
             module_ooo.par                \
             module_hidden_ooo.par         \
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
             module_graphicfilter.par      \
             file_graphicfilter.par        \
             module_testtool.par           \
             file_testtool.par             \
             module_lingu.par              \
             file_lingu.par                \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             file_python.par               \
             profileitem_python.par

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
             registryitem_quickstart.par

.IF "$(NETTOOLKIT)"==""
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
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == "" 
SCP1FILES += \
             file_crashrep_dynamic.par
.ENDIF

SCP1FILES += \
             vclcanvas.par
 
# ------------------------------------------------------------------------
# OpenOffice.org with JRE (Windows Only)

.IF "$(GUI)"=="WNT"

SCP2LINK_PRODUCT_TYPE=osl
SCP2TARGET = setup_osljre
SCP2FILES  = installation_ooo.par          \
             scpaction_ooo.par             \
             directory_ooo.par             \
             datacarrier_ooo.par           \
             file_ooo.par                  \
             file_images_ooo.par           \
             file_extra_ooo.par            \
             file_font_ooo.par             \
             file_library_ooo.par          \
             file_resource_ooo.par         \
             shortcut_ooo.par              \
             module_ooo.par                \
             module_hidden_ooo.par         \
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
             canvascommons.par             \
             module_math.par               \
             file_math.par                 \
             module_graphicfilter.par      \
             file_graphicfilter.par        \
             module_testtool.par           \
             file_testtool.par             \
             module_lingu.par              \
             file_lingu.par                \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             module_python.par             \
             file_python.par               \
             profileitem_python.par

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
             registryitem_writer.par   \
             folderitem_writer.par     \
             registryitem_calc.par     \
             folderitem_calc.par       \
             registryitem_draw.par     \
             folderitem_draw.par       \
             registryitem_impress.par  \
             folderitem_impress.par    \
             registryitem_math.par     \
             folderitem_math.par       \
             module_quickstart.par     \
             registryitem_quickstart.par

.IF "$(NETTOOLKIT)"==""
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
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == "" 
SCP2FILES += \
             file_crashrep_dynamic.par
.ENDIF

SCP2FILES += \
             vclcanvas.par

.ENDIF

SCP3LINK_PRODUCT_TYPE = ure
SCP3TARGET = ure
SCP3FILES = ure.par

# --- target -------------------------------------------------------------
.INCLUDE :  target.mk

