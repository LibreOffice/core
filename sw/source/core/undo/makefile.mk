#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 09:29:37 $
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
TARGET=undo

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        undo.src

EXCEPTIONSFILES = \
        $(SLO)$/SwRewriter.obj \
        $(SLO)$/unattr.obj \
        $(SLO)$/undobj.obj \
        $(SLO)$/undraw.obj \
        $(SLO)$/untbl.obj

SLOFILES =	\
        $(SLO)$/SwRewriter.obj \
        $(SLO)$/SwUndoField.obj \
        $(SLO)$/SwUndoPageDesc.obj \
        $(SLO)$/SwUndoFmt.obj \
        $(SLO)$/SwUndoTOXChange.obj \
        $(SLO)$/docundo.obj \
        $(SLO)$/rolbck.obj \
        $(SLO)$/unattr.obj \
        $(SLO)$/unbkmk.obj \
        $(SLO)$/undel.obj \
        $(SLO)$/undobj.obj \
        $(SLO)$/undobj1.obj \
        $(SLO)$/undraw.obj \
        $(SLO)$/unfmco.obj \
        $(SLO)$/unins.obj \
        $(SLO)$/unmove.obj \
        $(SLO)$/unnum.obj \
        $(SLO)$/unoutl.obj \
        $(SLO)$/unovwr.obj \
        $(SLO)$/unredln.obj \
        $(SLO)$/unsect.obj \
        $(SLO)$/unsort.obj \
        $(SLO)$/unspnd.obj \
        $(SLO)$/untbl.obj \
        $(SLO)$/untblk.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

