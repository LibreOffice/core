#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 21:16:37 $
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

PRJ=..$/..

PRJNAME=basic
TARGET=app

LIBTARGET = NO

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ------------------------------------------------------------

OBJFILES = \
    $(OBJ)$/ttbasic.obj	\
    $(OBJ)$/basicrt.obj	\
    $(OBJ)$/processw.obj    \
    $(OBJ)$/process.obj     \
    $(OBJ)$/brkpnts.obj     \
    $(OBJ)$/app.obj         \
    $(OBJ)$/mybasic.obj     \
    $(OBJ)$/status.obj      \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/appwin.obj      \
    $(OBJ)$/appedit.obj     \
    $(OBJ)$/appbased.obj    \
    $(OBJ)$/apperror.obj    \
    $(OBJ)$/textedit.obj    \
    $(OBJ)$/msgedit.obj     \
    $(OBJ)$/dialogs.obj     \

EXCEPTIONSFILES = \
    $(OBJ)$/app.obj

SRS1NAME=$(TARGET)
SRC1FILES = \
    basic.src				\
    ttmsg.src				\
    basmsg.src				\
    svtmsg.src				\
    testtool.src

LIB1TARGET=$(LB)$/app.lib
LIB1ARCHIV=$(LB)$/libapp.a
LIB1OBJFILES = \
        $(OBJ)$/basicrt.obj	\
        $(OBJ)$/processw.obj     \
        $(OBJ)$/process.obj      \
        $(OBJ)$/brkpnts.obj      \
        $(OBJ)$/app.obj          \
        $(OBJ)$/mybasic.obj	\
        $(OBJ)$/status.obj       \
        $(OBJ)$/printer.obj      \
        $(OBJ)$/appwin.obj       \
        $(OBJ)$/appedit.obj      \
        $(OBJ)$/appbased.obj     \
        $(OBJ)$/apperror.obj     \
        $(OBJ)$/textedit.obj     \
        $(OBJ)$/msgedit.obj      \
        $(OBJ)$/dialogs.obj      \
        $(OBJ)$/sbintern.obj

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

$(OBJ)$/dialogs.obj : $(INCCOM)$/_version.h

