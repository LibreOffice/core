#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 10:22:23 $
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
TARGET=config
LIBTARGET=NO
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------
IMGLST_SRS=$(SRS)$/config.srs
BMP_IN=$(PRJ)$/win/imglst

SRS1NAME=$(TARGET)
SRC1FILES =	\
        mailconfigpage.src \
        optdlg.src \
        optload.src \
        optcomp.src \
        redlopt.src

EXCEPTIONSFILES = \
        $(SLO)$/mailconfigpage.obj \
        $(SLO)$/optcomp.obj \
        $(SLO)$/optload.obj

SLOFILES =	\
        $(EXCEPTIONSFILES) \
        $(SLO)$/barcfg.obj \
        $(SLO)$/caption.obj \
        $(SLO)$/cfgitems.obj \
        $(SLO)$/dbconfig.obj \
        $(SLO)$/fontcfg.obj \
        $(SLO)$/modcfg.obj \
        $(SLO)$/optpage.obj \
        $(SLO)$/prtopt.obj \
        $(SLO)$/uinums.obj \
        $(SLO)$/usrpref.obj \
        $(SLO)$/viewopt.obj \
        $(SLO)$/swlinguconfig.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
        $(SLO)$/barcfg.obj \
        $(SLO)$/caption.obj \
        $(SLO)$/cfgitems.obj \
        $(SLO)$/dbconfig.obj \
        $(SLO)$/fontcfg.obj \
        $(SLO)$/modcfg.obj \
        $(SLO)$/prtopt.obj \
        $(SLO)$/uinums.obj \
        $(SLO)$/usrpref.obj \
        $(SLO)$/viewopt.obj \
        $(SLO)$/swlinguconfig.obj
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

