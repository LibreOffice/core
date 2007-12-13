#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2007-12-13 11:38:50 $
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
TARGET=edit

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
        $(SLO)$/edlingu.obj \
        $(SLO)$/edfldexp.obj \
        $(SLO)$/eddel.obj \
        $(SLO)$/edtab.obj

SLOFILES =	\
        $(EXCEPTIONSFILES)\
        $(SLO)$/acorrect.obj \
        $(SLO)$/autofmt.obj \
        $(SLO)$/edatmisc.obj \
        $(SLO)$/edattr.obj \
        $(SLO)$/edfcol.obj \
        $(SLO)$/edfld.obj \
        $(SLO)$/edfmt.obj \
        $(SLO)$/edglbldc.obj \
        $(SLO)$/edglss.obj \
        $(SLO)$/editsh.obj \
        $(SLO)$/ednumber.obj \
        $(SLO)$/edredln.obj \
        $(SLO)$/edtox.obj \
        $(SLO)$/edundo.obj \
        $(SLO)$/edws.obj \
        $(SLO)$/edsect.obj

# --- Targets -------------------------------------------------------

.INCLUDE :	target.mk

