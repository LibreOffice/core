#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:17:24 $
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

PRJNAME=sal
TARGET=cpprtl
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

SLOFILES=   $(SLO)$/alloc.obj	\
            $(SLO)$/memory.obj  \
        $(SLO)$/cipher.obj	\
            $(SLO)$/crc.obj	\
            $(SLO)$/digest.obj  \
            $(SLO)$/random.obj  \
            $(SLO)$/char.obj	\
            $(SLO)$/ustring.obj \
            $(SLO)$/locale.obj	\
            $(SLO)$/rtl_process.obj \
            $(SLO)$/string.obj	\
            $(SLO)$/strbuf.obj	\
            $(SLO)$/uuid.obj	\
            $(SLO)$/ustrbuf.obj \
            $(SLO)$/byteseq.obj

.IF "$(GUI)" == "WIN"
SLOFILES+=  $(SLO)$/tcwin16.obj
.ENDIF

#.IF "$(UPDATER)"=="YES"
OBJFILES=   $(OBJ)$/alloc.obj	\
            $(OBJ)$/memory.obj  \
        $(OBJ)$/cipher.obj	\
            $(OBJ)$/crc.obj	\
            $(OBJ)$/digest.obj	\
            $(OBJ)$/random.obj	\
            $(OBJ)$/char.obj	\
            $(OBJ)$/ustring.obj	\
            $(OBJ)$/locale.obj	\
            $(OBJ)$/rtl_process.obj \
            $(OBJ)$/string.obj	\
            $(OBJ)$/strbuf.obj	\
            $(OBJ)$/uuid.obj	\
            $(OBJ)$/ustrbuf.obj \
            $(OBJ)$/byteseq.obj
#.ENDIF

.IF "$(GUI)" == "WIN"
OBJFILES+=  $(OBJ)$/tcwin16.obj

.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

