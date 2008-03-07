#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 12:22:58 $
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
TARGET=miscdlgs
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/delcldlg.obj		\
    $(SLO)$/inscldlg.obj		\
    $(SLO)$/delcodlg.obj		\
    $(SLO)$/inscodlg.obj		\
    $(SLO)$/strindlg.obj		\
    $(SLO)$/mtrindlg.obj		\
    $(SLO)$/lbseldlg.obj		\
    $(SLO)$/filldlg.obj			\
    $(SLO)$/autofmt.obj			\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj      \
    $(SLO)$/mvtabdlg.obj		\
    $(SLO)$/groupdlg.obj		\
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/crdlg.obj			\
    $(SLO)$/namecrea.obj		\
    $(SLO)$/namepast.obj		\
    $(SLO)$/teamdlg.obj			\
    $(SLO)$/textdlgs.obj		\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/shtabdlg.obj		\
    $(SLO)$/instbdlg.obj		\
    $(SLO)$/acredlin.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/simpref.obj			\
    $(SLO)$/redcom.obj			\
    $(SLO)$/linkarea.obj		\
    $(SLO)$/warnbox.obj	\
    $(SLO)$/scuiautofmt.obj     \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj

EXCEPTIONSFILES = \
    $(SLO)$/acredlin.obj        \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj

EXCEPTIONSFILES= \
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj

SRS1NAME=$(TARGET)
SRC1FILES = \
    instbdlg.src				\
    acredlin.src				\
    highred.src					\
    linkarea.src                \
    conflictsdlg.src            \
    sharedocdlg.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/autofmt.obj			\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj      \
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/teamdlg.obj			\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/acredlin.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/simpref.obj			\
    $(SLO)$/redcom.obj			\
    $(SLO)$/warnbox.obj         \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

