#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: kz $ $Date: 2005-10-05 14:39:36 $
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

PRJNAME=svx
TARGET=editeng
AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=eeng_pch
PROJECTPCHSOURCE=eeng_pch

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)" != "" || "$(EDITDEBUG)" != ""
CDEFS+=-DEDITDEBUG
.ENDIF

.IF "$(header)" == ""

SLOFILES =	\
            $(SLO)$/svxacorr.obj \
            $(SLO)$/acorrcfg.obj \
            $(SLO)$/txtrange.obj \
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
            $(OBJ)$/editview.obj  \
            $(OBJ)$/eerdll.obj	\
            $(OBJ)$/impedit.obj	\
            $(OBJ)$/impedit2.obj \
            $(OBJ)$/impedit3.obj \
            $(OBJ)$/impedit4.obj \
            $(OBJ)$/impedit5.obj \
            $(OBJ)$/forbiddencharacterstable.obj \
            $(OBJ)$/unolingu.obj
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=	editeng.src

EXCEPTIONSFILES= \
    $(SLO)$/unolingu.obj

.IF "$(SVXLIGHT)"!=""
EXCEPTIONSFILES+= \
    $(OBJ)$/sxl_unolingu.obj
.ENDIF          # "$(SVXLIGHT)"!=""

.INCLUDE :  target.mk

