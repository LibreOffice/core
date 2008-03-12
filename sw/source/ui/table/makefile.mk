#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 12:55:40 $
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
TARGET=table
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=   \
        $(SLO)$/chartins.obj\

SRS1NAME=$(TARGET)
SRC1FILES =  \
        colwd.src \
        convert.src \
        chartins.src \
        insrc.src \
        instable.src \
        mergetbl.src \
        rowht.src \
        splittbl.src \
        table.src \
        tabledlg.src \
        tautofmt.src

SLOFILES =  \
        $(SLO)$/colwd.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/chartins.obj \
        $(SLO)$/insrc.obj \
        $(SLO)$/instable.obj \
        $(SLO)$/mergetbl.obj \
        $(SLO)$/rowht.obj \
        $(SLO)$/splittbl.obj \
        $(SLO)$/tabledlg.obj \
        $(SLO)$/tablemgr.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tautofmt.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/chartins.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tablemgr.obj 
        
        
# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

