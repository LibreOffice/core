#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-06-16 09:57:43 $
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- setup script -------------------------------------------------------
# SCPFLAGS= -w

# ------------------------------------------------------------------------
# Office OSL

SCP1LINK_PRODUCT_TYPE=osl
SCP1TARGET = setup_osl
SCP1FILES  = installation_ooo.par          \
             scpaction_ooo.par             \
             helptext_ooo.par              \
             directory_ooo.par             \
             starregistry_ooo.par          \
             datacarrier_ooo.par           \
             file_ooo.par                  \
             file_extra_ooo.par            \
             file_font_ooo.par             \
             file_library_ooo.par          \
             file_resource_ooo.par         \
             shortcut_ooo.par              \
             custom_ooo.par                \
             procedure_ooo.par             \
             module_ooo.par                \
             profile_ooo.par               \
             profileitem_ooo.par           \
             starregistryitem_ooo.par      \
             configurationitem_ooo.par     \
             module_writer.par             \
             file_writer.par               \
             configurationitem_writer.par  \
             module_calc.par               \
             file_calc.par                 \
             configurationitem_calc.par    \
             module_draw.par               \
             file_draw.par                 \
             configurationitem_draw.par    \
             module_impress.par            \
             file_impress.par              \
             canvascommons.par             \
             configurationitem_impress.par \
             module_math.par               \
             file_math.par                 \
             configurationitem_math.par    \
             module_graphicfilter.par      \
             file_graphicfilter.par        \
             module_testtool.par           \
             file_testtool.par             \
             module_lingu.par              \
             file_lingu.par                \
             module_xsltfilter.par         \
             file_xsltfilter.par           \
             starregistryitem_xsltfilter.par \
             module_python.par             \
             file_python.par               \
             procedure_python.par          \
             configurationitem_python.par  \
             starregistryitem_python.par   \
             profileitem_python.par

.IF "$(SOLAR_JAVA)"!=""
SCP1FILES +=                                  \
             module_javafilter.par            \
             file_javafilter.par              \
             starregistryitem_javafilter.par
.ENDIF

.IF "$(SOLAR_JAVA)"!="" 
.IF "$(GUI)"=="WNT"
SCP1FILES +=                                  \
             registryitem_javafilter.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
SCP1FILES +=                           \
             registryitem_ooo.par      \
             folder_ooo.par            \
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
             module_activex.par        \
             file_activex.par          \
             custom_activex.par
.ENDIF

.IF "$(GUI)"=="UNX"
SCP1FILES += \
             module_cde.par         \
             directory_cde.par      \
             file_cde.par           \
             procedure_cde.par      \
             module_gnome.par       \
             directory_gnome.par    \
             file_gnome.par         \
             module_kde.par         \
             file_kde.par           \
             module_mailcap.par     \
             file_mailcap.par
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == "" 
SCP1FILES += \
             file_crashrep_dynamic.par
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_CRASHDUMP)" == "STATIC" || "$(PRODUCT)" == ""
SCP1FILES += \
             file_crashrep_static.par
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
.IF "$(ENABLE_DIRECTX)" != ""
SCP1FILES += \
             directxcanvas.par
.ELSE # IF "$(ENABLE_DIRECTX)" != ""
SCP1FILES += \
             vclcanvas.par
.ENDIF # IF "$(ENABLE_DIRECTX)" != ""
.ELSE # IF "$(GUI)"=="WNT"
SCP1FILES += \
             vclcanvas.par
.ENDIF #IF "$(GUI)"=="WNT"


# --- target -------------------------------------------------------------
.INCLUDE :  target.mk

