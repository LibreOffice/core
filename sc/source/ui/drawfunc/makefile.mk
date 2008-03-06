#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:22:21 $
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
TARGET=drawfunc

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        objdraw.src\
        drformsh.src

SLOFILES =  \
        $(SLO)$/fupoor.obj \
        $(SLO)$/fumark.obj \
        $(SLO)$/fudraw.obj \
        $(SLO)$/fusel.obj \
        $(SLO)$/fusel2.obj \
        $(SLO)$/fuconstr.obj \
        $(SLO)$/fuconrec.obj \
        $(SLO)$/fuconarc.obj \
        $(SLO)$/fuconuno.obj \
        $(SLO)$/fuconpol.obj \
        $(SLO)$/fuconcustomshape.obj \
        $(SLO)$/fuins1.obj \
        $(SLO)$/fuins2.obj \
        $(SLO)$/futext.obj \
        $(SLO)$/futext2.obj \
        $(SLO)$/futext3.obj \
        $(SLO)$/drawsh.obj \
        $(SLO)$/drawsh2.obj \
        $(SLO)$/drawsh4.obj \
        $(SLO)$/drawsh5.obj \
        $(SLO)$/drtxtob.obj \
        $(SLO)$/drtxtob1.obj \
        $(SLO)$/drtxtob2.obj \
        $(SLO)$/drformsh.obj \
        $(SLO)$/oleobjsh.obj \
        $(SLO)$/chartsh.obj  \
        $(SLO)$/graphsh.obj	\
        $(SLO)$/mediash.obj

EXCEPTIONSFILES= \
        $(SLO)$/fuins2.obj

NOOPTFILES=\
    $(SLO)$/fusel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

