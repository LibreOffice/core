#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 14:07:02 $
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
TARGET=docshell
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        docsh.cxx	\
        docsh2.cxx	\
        docsh3.cxx	\
        docsh4.cxx 	\
        docsh5.cxx	\
        docsh6.cxx 	\
        docsh7.cxx 	\
        docsh8.cxx 	\
        tablink.cxx 	\
        arealink.cxx 	\
        dbdocfun.cxx 	\
        dbdocimp.cxx 	\
        impex.cxx	\
        docfunc.cxx	\
        olinefun.cxx	\
        servobj.cxx	\
        tpstat.cxx	\
        autostyl.cxx	\
        pagedata.cxx \
        hiranges.cxx \
        pntlock.cxx \
        sizedev.cxx \
        editable.cxx


SLOFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/tpstat.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj


EXCEPTIONSFILES= \
        $(SLO)$/docsh.obj \
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj \
        $(SLO)$/docsh8.obj \
        $(SLO)$/dbdocimp.obj

SRS1NAME=$(TARGET)
SRC1FILES =  tpstat.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

