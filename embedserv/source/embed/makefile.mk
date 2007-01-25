#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 11:36:31 $
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

#MKDEPENDSOLVER=YES

PRJ=..$/..

PRJNAME=embedserv
TARGET=emser


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

LIBTARGET=NO
USE_DEFFILE=NO

INCPRE+= $(ATL_INCLUDE) 

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/register.obj       \
        $(SLO)$/servprov.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/guid.obj           \
        $(SLO)$/esdll.obj          \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj

EXCEPTIONSFILES= \
        $(SLO)$/register.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

