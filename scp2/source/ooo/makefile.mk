#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2004-07-05 13:17:39 $
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

PRJPCH=

PRJNAME=scp2
TARGET=ooo
TARGETTYPE=CUI

USE_JAVAVER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_CRASHDUMP)"!=""
SCPDEFS+=-DENABLE_CRASHDUMP
.ENDIF

.IF "$(JAVANUMVER)" >= "000100040000"
SCPDEFS+=-DINCLUDE_JAVA_ACCESSBRIDGE
.ENDIF

.IF "$(ENABLE_GTK)" != ""
SCPDEFS+=-DENABLE_GTK
.ENDIF

SCP_PRODUCT_TYPE=osl

PARFILES=                          \
        installation_ooo.par       \
        scpaction_ooo.par          \
        helptext_ooo.par           \
        directory_ooo.par          \
        starregistry_ooo.par       \
        datacarrier_ooo.par        \
        file_ooo.par               \
        file_extra_ooo.par         \
        file_font_ooo.par          \
        file_library_ooo.par       \
        file_resource_ooo.par      \
        shortcut_ooo.par           \
        custom_ooo.par             \
        procedure_ooo.par          \
        module_ooo.par             \
        profile_ooo.par            \
        profileitem_ooo.par        \
        starregistryitem_ooo.par 

.IF "$(GUI)"=="WNT"
PARFILES +=                        \
        folder_ooo.par             \
        registryitem_ooo.par
.ENDIF

ULFFILES=                          \
        directory_ooo.ulf          \
        helptext_ooo.ulf           \
        module_ooo.ulf             \
        registryitem_ooo.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
