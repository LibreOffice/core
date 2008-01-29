#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2008-01-29 08:38:31 $
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
TARGET=frmedt

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk


.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
        fecopy.cxx \
        fedesc.cxx \
        fefly1.cxx \
        feflyole.cxx \
        feshview.cxx \
        fetab.cxx \
        fews.cxx \
        tblsel.cxx



SLOFILES =  $(EXCEPTIONSFILES) \
        $(SLO)$/fedesc.obj \
        $(SLO)$/feflyole.obj \
        $(SLO)$/feshview.obj \
        $(SLO)$/fetab.obj \
        $(SLO)$/tblsel.obj
        
EXCEPTIONSFILES=\
        $(SLO)$/fecopy.obj \
        $(SLO)$/fefly1.obj \
        $(SLO)$/fews.obj
        

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

