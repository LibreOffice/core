#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:20:38 $
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

PRJNAME=sc
TARGET=dbgui
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/sortdlg.obj		\
    $(SLO)$/tpsort.obj		\
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/pfiltdlg.obj	\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/subtdlg.obj		\
    $(SLO)$/tpsubt.obj		\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/dpgroupdlg.obj	\
    $(SLO)$/dapitype.obj	\
    $(SLO)$/dapidata.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/scendlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/validate.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj	\
    $(SLO)$/scuiasciiopt.obj	\
     $(SLO)$/scuiimoptdlg.obj

EXCEPTIONSFILES= \
    $(SLO)$/csvgrid.obj \
    $(SLO)$/csvruler.obj \
    $(SLO)$/csvsplits.obj \
    $(SLO)$/csvtablebox.obj \
    $(SLO)$/fieldwnd.obj \
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/dapidata.obj

SRS1NAME=$(TARGET)
SRC1FILES =  \
        pivot.src		\
        pvfundlg.src	\
        dpgroupdlg.src	\
        dapitype.src	\
        consdlg.src		\
        scendlg.src     \
        imoptdlg.src    \
        validate.src    \
        asciiopt.src    \
        outline.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


