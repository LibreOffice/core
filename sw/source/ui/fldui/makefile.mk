#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:25:11 $
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

PRJ=..$/..$/..
LIBTARGET=no
PRJNAME=sw
TARGET=fldui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        changedb.src \
        DropDownFieldDialog.src\
        flddb.src \
        flddinf.src \
        flddok.src \
        fldfunc.src \
        fldref.src \
        fldtdlg.src \
        fldui.src \
        fldvar.src \
        javaedit.src \
        inpdlg.src

SLOFILES =  \
            $(SLO)$/changedb.obj \
        $(SLO)$/DropDownFieldDialog.obj \
        $(SLO)$/flddb.obj \
        $(SLO)$/flddinf.obj \
        $(SLO)$/flddok.obj \
        $(SLO)$/fldedt.obj \
        $(SLO)$/fldfunc.obj \
        $(SLO)$/fldmgr.obj \
        $(SLO)$/fldpage.obj \
        $(SLO)$/fldref.obj \
        $(SLO)$/fldtdlg.obj \
        $(SLO)$/fldvar.obj \
        $(SLO)$/javaedit.obj \
        $(SLO)$/inpdlg.obj \
        $(SLO)$/fldwrap.obj \
                $(SLO)$/xfldui.obj \
                $(SLO)$/FldRefTreeListBox.obj


EXCEPTIONSFILES =  \
        $(SLO)$/fldtdlg.obj \
        $(SLO)$/fldedt.obj \
        $(SLO)$/xfldui.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/fldmgr.obj \
        $(SLO)$/fldwrap.obj \
        $(SLO)$/xfldui.obj

#		$(SLO)$/DropDownFieldDialog.obj \

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

