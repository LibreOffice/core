#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 15:09:14 $
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
TARGET=utlui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------


SRS1NAME=$(TARGET)
SRC1FILES =  \
        initui.src \
        gloslst.src \
        navipi.src \
        poolfmt.src \
        attrdesc.src \
        unotools.src \
                utlui.src

EXCEPTIONSFILES= \
        $(SLO)$/unotools.obj	\
        $(SLO)$/swrenamexnameddlg.obj

SLOFILES =  $(EXCEPTIONSFILES) \
        $(SLO)$/bookctrl.obj \
        $(SLO)$/condedit.obj \
        $(SLO)$/content.obj \
        $(SLO)$/gloslst.obj \
        $(SLO)$/glbltree.obj \
        $(SLO)$/initui.obj \
        $(SLO)$/navipi.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/numfmtlb.obj \
        $(SLO)$/prcntfld.obj \
        $(SLO)$/textcontrolcombo.obj \
        $(SLO)$/tmplctrl.obj \
        $(SLO)$/uitool.obj \
        $(SLO)$/uiitems.obj \
        $(SLO)$/attrdesc.obj \
        $(SLO)$/shdwcrsr.obj \
                $(SLO)$/zoomctrl.obj \
                $(SLO)$/viewlayoutctrl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

