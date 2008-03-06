#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:25:08 $
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
TARGET=undo

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        target.cxx \
        refundo.cxx \
        areasave.cxx \
        undobase.cxx \
        undoutil.cxx \
        undocell.cxx \
        undostyl.cxx \
        undoolk.cxx \
        undoblk.cxx \
        undoblk2.cxx \
        undoblk3.cxx \
        undodat.cxx \
        undodraw.cxx \
        undotab.cxx



SLOFILES =  \
        $(SLO)$/target.obj \
        $(SLO)$/refundo.obj \
        $(SLO)$/areasave.obj \
        $(SLO)$/undobase.obj \
        $(SLO)$/undoutil.obj \
        $(SLO)$/undocell.obj \
        $(SLO)$/undostyl.obj \
        $(SLO)$/undoolk.obj \
        $(SLO)$/undoblk.obj \
        $(SLO)$/undoblk2.obj \
        $(SLO)$/undoblk3.obj \
        $(SLO)$/undodat.obj \
        $(SLO)$/undodraw.obj \
        $(SLO)$/undotab.obj

EXCEPTIONSFILES= \
    $(SLO)$/undostyl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

