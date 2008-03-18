#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 16:03:50 $
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

PRJNAME=sw
TARGET=frmdlg
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        column.src \
        cption.src \
        frmpage.src \
        frmui.src \
        wrap.src

SLOFILES =  \
        $(SLO)$/colmgr.obj \
        $(SLO)$/column.obj \
        $(SLO)$/colex.obj \
        $(SLO)$/cption.obj \
        $(SLO)$/frmdlg.obj \
        $(SLO)$/frmmgr.obj \
        $(SLO)$/frmpage.obj \
        $(SLO)$/pattern.obj \
        $(SLO)$/uiborder.obj \
        $(SLO)$/wrap.obj

EXCEPTIONSFILES =  \
        $(SLO)$/frmpage.obj \
        $(SLO)$/cption.obj 

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/colmgr.obj \
        $(SLO)$/colex.obj \
        $(SLO)$/frmmgr.obj




# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

