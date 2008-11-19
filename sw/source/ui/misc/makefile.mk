#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.14.236.1 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
        $(SLO)$/glshell.obj \
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

