#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: mt $ $Date: 2001-03-09 18:08:55 $
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

PRJNAME=svx
TARGET=editeng
AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=eeng_pch
PROJECTPCHSOURCE=eeng_pch

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)" != "" || "$(EDITDEBUG)" != ""
CDEFS+=-DEDITDEBUG
.ENDIF

.IF "$(header)" == ""

SLOFILES =	\
            $(SLO)$/txtrange.obj \
            $(SLO)$/svxacorr.obj \
            $(SLO)$/SvXMLAutoCorrectImport.obj \
            $(SLO)$/SvXMLAutoCorrectExport.obj \
            $(SLO)$/forbiddencharacterstable.obj \
            $(SLO)$/swafopt.obj \
            $(SLO)$/editattr.obj \
            $(SLO)$/editdbg.obj	\
            $(SLO)$/editdoc.obj	\
            $(SLO)$/editdoc2.obj \
            $(SLO)$/editeng.obj	\
            $(SLO)$/editobj.obj	\
            $(SLO)$/editsel.obj	\
            $(SLO)$/editundo.obj \
            $(SLO)$/editview.obj  \
            $(SLO)$/edtspell.obj \
            $(SLO)$/eehtml.obj \
            $(SLO)$/eerdll.obj	\
            $(SLO)$/eeobj.obj \
            $(SLO)$/eertfpar.obj \
            $(SLO)$/impedit.obj	\
            $(SLO)$/impedit2.obj \
            $(SLO)$/impedit3.obj \
            $(SLO)$/impedit4.obj \
            $(SLO)$/impedit5.obj \
            $(SLO)$/textconv.obj \
            $(SLO)$/unolingu.obj

SVXLIGHTOBJFILES= \
            $(OBJ)$/txtrange.obj \
            $(OBJ)$/swafopt.obj \
            $(OBJ)$/editattr.obj \
            $(OBJ)$/editdbg.obj	\
            $(OBJ)$/editdoc.obj	\
            $(OBJ)$/editdoc2.obj \
            $(OBJ)$/editeng.obj	\
            $(OBJ)$/editobj.obj	\
            $(OBJ)$/editsel.obj	\
            $(OBJ)$/editundo.obj \
            $(OBJ)$/editview.obj  \
            $(OBJ)$/eerdll.obj	\
            $(OBJ)$/eeobj.obj	\
            $(OBJ)$/impedit.obj	\
            $(OBJ)$/impedit2.obj \
            $(OBJ)$/impedit3.obj \
            $(OBJ)$/impedit4.obj \
            $(OBJ)$/impedit5.obj \
            $(OBJ)$/textconv.obj \
            $(OBJ)$/forbiddencharacterstable.obj \
            $(OBJ)$/unolingu.obj
.ENDIF

SRCFILES=	editeng.src

EXCEPTIONSFILES= \
    $(SLO)$/unolingu.obj    \
    $(OBJ)$/sxl_unolingu.obj

.INCLUDE :  target.mk

