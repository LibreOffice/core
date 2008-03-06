#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:25:25 $
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
TARGET=unoobj

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
SLO1FILES =  \
        $(SLO)$/docuno.obj \
        $(SLO)$/servuno.obj \
        $(SLO)$/defltuno.obj \
        $(SLO)$/drdefuno.obj \
        $(SLO)$/cellsuno.obj \
        $(SLO)$/tokenuno.obj \
        $(SLO)$/textuno.obj \
        $(SLO)$/notesuno.obj \
        $(SLO)$/cursuno.obj \
        $(SLO)$/srchuno.obj \
        $(SLO)$/fielduno.obj \
        $(SLO)$/miscuno.obj \
        $(SLO)$/optuno.obj \
        $(SLO)$/appluno.obj \
        $(SLO)$/funcuno.obj \
        $(SLO)$/nameuno.obj \
        $(SLO)$/viewuno.obj \
        $(SLO)$/dispuno.obj \
        $(SLO)$/datauno.obj \
        $(SLO)$/dapiuno.obj \
        $(SLO)$/chartuno.obj \
        $(SLO)$/chart2uno.obj \
        $(SLO)$/shapeuno.obj \
        $(SLO)$/pageuno.obj \
        $(SLO)$/forbiuno.obj \
        $(SLO)$/styleuno.obj \
        $(SLO)$/afmtuno.obj \
        $(SLO)$/fmtuno.obj \
        $(SLO)$/linkuno.obj \
        $(SLO)$/targuno.obj \
        $(SLO)$/convuno.obj \
        $(SLO)$/editsrc.obj \
        $(SLO)$/unoguard.obj \
        $(SLO)$/confuno.obj \
        $(SLO)$/filtuno.obj \
        $(SLO)$/unodoc.obj \
        $(SLO)$/addruno.obj \
        $(SLO)$/listenercalls.obj \
        $(SLO)$/cellvaluebinding.obj \
        $(SLO)$/celllistsource.obj \
        $(SLO)$/warnpassword.obj \
        $(SLO)$/unoreflist.obj \
        $(SLO)$/ChartRangeSelectionListener.obj

SLO2FILES = \
    $(SLO)$/scdetect.obj								\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
             $(SLO)$/cellsuno.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

