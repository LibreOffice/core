#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=sc
TARGET=miscdlgs
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(EXCEPTIONSFILES)

EXCEPTIONSFILES = \
    $(SLO)$/acredlin.obj		\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/autofmt.obj			\
    $(SLO)$/conflictsdlg.obj		\
    $(SLO)$/crdlg.obj			\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/datafdlg.obj		\
    $(SLO)$/delcldlg.obj		\
    $(SLO)$/delcodlg.obj		\
    $(SLO)$/filldlg.obj			\
    $(SLO)$/groupdlg.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/inscldlg.obj		\
    $(SLO)$/inscodlg.obj		\
    $(SLO)$/instbdlg.obj		\
    $(SLO)$/lbseldlg.obj		\
    $(SLO)$/linkarea.obj		\
    $(SLO)$/mtrindlg.obj		\
    $(SLO)$/mvtabdlg.obj		\
    $(SLO)$/namecrea.obj		\
    $(SLO)$/namepast.obj		\
    $(SLO)$/optsolver.obj		\
    $(SLO)$/protectiondlg.obj		\
    $(SLO)$/redcom.obj			\
    $(SLO)$/retypepassdlg.obj		\
    $(SLO)$/scuiautofmt.obj		\
    $(SLO)$/sharedocdlg.obj		\
    $(SLO)$/shtabdlg.obj		\
    $(SLO)$/simpref.obj			\
    $(SLO)$/solveroptions.obj		\
    $(SLO)$/solverutil.obj		\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/strindlg.obj		\
    $(SLO)$/tabbgcolordlg.obj   	\
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/textdlgs.obj		\
    $(SLO)$/warnbox.obj

SRS1NAME=$(TARGET)
SRC1FILES = \
    instbdlg.src				\
    acredlin.src				\
    highred.src					\
    linkarea.src                \
    conflictsdlg.src            \
    sharedocdlg.src \
    protectiondlg.src \
    retypepassdlg.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/autofmt.obj			\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj      \
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/acredlin.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/simpref.obj			\
    $(SLO)$/redcom.obj			\
    $(SLO)$/warnbox.obj         \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj \
    $(SLO)$/protectiondlg.obj \
    $(SLO)$/retypepassdlg.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

