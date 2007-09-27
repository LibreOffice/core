#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 12:22:03 $
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
TARGET=misc
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=\
    bookmark.src \
    docfnote.src \
    glossary.src \
    glosbib.src \
    insfnote.src \
    insrule.src \
    linenum.src \
    num.src \
    numberingtypelistbox.src \
    outline.src \
    pgfnote.src \
    pggrid.src \
    redlndlg.src \
    srtdlg.src

EXCEPTIONSFILES = \
        $(SLO)$/glossary.obj \
        $(SLO)$/glosdoc.obj \
        $(SLO)$/numberingtypelistbox.obj \
        $(SLO)$/outline.obj \
        $(SLO)$/redlndlg.obj
        

SLOFILES =	\
        $(SLO)$/bookmark.obj \
        $(SLO)$/docfnote.obj \
        $(SLO)$/glosbib.obj \
        $(SLO)$/glosdoc.obj \
        $(SLO)$/glossary.obj \
        $(SLO)$/glshell.obj \
        $(SLO)$/insfnote.obj \
        $(SLO)$/insrule.obj \
        $(SLO)$/linenum.obj \
        $(SLO)$/num.obj \
        $(SLO)$/numberingtypelistbox.obj \
        $(SLO)$/outline.obj \
        $(SLO)$/pgfnote.obj \
        $(SLO)$/pggrid.obj \
        $(SLO)$/redlndlg.obj \
        $(SLO)$/swmodalredlineacceptdlg.obj	\
        $(SLO)$/srtdlg.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/glosdoc.obj	\
        $(SLO)$/glshell.obj \
        $(SLO)$/numberingtypelistbox.obj \
        $(SLO)$/redlndlg.obj
       
# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

