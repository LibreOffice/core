#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 09:59:34 $
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
TARGET=ww8

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = \
        $(SLO)$/wrtww8gr.obj \
        $(SLO)$/ww8par2.obj \
        $(SLO)$/ww8par3.obj \
        $(SLO)$/ww8par5.obj \
        $(SLO)$/tracer.obj \
        $(SLO)$/fields.obj \
        $(SLO)$/styles.obj \
        $(SLO)$/ww8graf.obj \
        $(SLO)$/ww8graf2.obj \
        $(SLO)$/wrtw8esh.obj \
        $(SLO)$/wrtw8nds.obj \
        $(SLO)$/wrtw8num.obj \
        $(SLO)$/wrtw8sty.obj \
        $(SLO)$/wrtww8.obj \
        $(SLO)$/ww8atr.obj \
        $(SLO)$/ww8par.obj \
        $(SLO)$/ww8par6.obj \
        $(SLO)$/writerhelper.obj \
        $(SLO)$/writerwordglue.obj \
        $(SLO)$/ww8scan.obj


SLOFILES =	\
        $(SLO)$/wrtw8esh.obj \
        $(SLO)$/wrtw8nds.obj \
        $(SLO)$/wrtw8num.obj \
        $(SLO)$/wrtw8sty.obj \
        $(SLO)$/wrtww8.obj \
        $(SLO)$/wrtww8gr.obj \
        $(SLO)$/ww8atr.obj \
        $(SLO)$/ww8graf.obj \
        $(SLO)$/ww8graf2.obj \
        $(SLO)$/ww8par.obj \
        $(SLO)$/ww8par2.obj \
        $(SLO)$/ww8par3.obj \
        $(SLO)$/ww8par4.obj \
        $(SLO)$/ww8par5.obj \
        $(SLO)$/ww8par6.obj \
        $(SLO)$/ww8glsy.obj \
        $(SLO)$/tracer.obj \
        $(SLO)$/fields.obj \
        $(SLO)$/styles.obj \
        $(SLO)$/ww8scan.obj \
        $(SLO)$/writerhelper.obj \
        $(SLO)$/writerwordglue.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

