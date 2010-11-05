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

PRJNAME=sw
TARGET=dbui
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------
IMGLST_SRS=$(SRS)$/dbui.srs
BMP_IN=$(PRJ)$/win/imglst


SRS1NAME=$(TARGET)
SRC1FILES =  \
        addresslistdialog.src \
        createaddresslistdialog.src \
        customizeaddresslistdialog.src \
        dbinsdlg.src    \
        dbui.src \
        dbtablepreviewdialog.src \
        mailmergechildwindow.src \
        mailmergewizard.src \
        mmdocselectpage.src \
        mmlayoutpage.src \
        mmoutputpage.src \
        mmoutputtypepage.src \
        mmaddressblockpage.src \
        mmgreetingspage.src \
        mmmergepage.src \
        mmpreparemergepage.src \
        selectdbtabledialog.src \

EXCEPTIONSFILES= \
        $(SLO)$/addresslistdialog.obj \
        $(SLO)$/createaddresslistdialog.obj \
        $(SLO)$/customizeaddresslistdialog.obj \
        $(SLO)$/dbinsdlg.obj \
        $(SLO)$/dbmgr.obj \
        $(SLO)$/dbtablepreviewdialog.obj \
        $(SLO)$/dbtree.obj \
        $(SLO)$/dbui.obj \
        $(SLO)$/maildispatcher.obj \
        $(SLO)$/mailmergechildwindow.obj \
        $(SLO)$/mailmergehelper.obj \
        $(SLO)$/mailmergewizard.obj \
        $(SLO)$/mmaddressblockpage.obj \
        $(SLO)$/mmconfigitem.obj \
        $(SLO)$/mmdocselectpage.obj \
        $(SLO)$/mmgreetingspage.obj \
        $(SLO)$/mmlayoutpage.obj \
        $(SLO)$/mmmergepage.obj \
        $(SLO)$/mmoutputpage.obj \
        $(SLO)$/mmoutputtypepage.obj \
        $(SLO)$/mmpreparemergepage.obj \
        $(SLO)$/selectdbtabledialog.obj

SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/swdbtoolsclient.obj

        
LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/dbmgr.obj \
        $(SLO)$/dbtree.obj \
        $(SLO)$/dbui.obj \
        $(SLO)$/maildispatcher.obj \
        $(SLO)$/mailmergehelper.obj \
        $(SLO)$/mailmergechildwindow.obj \
        $(SLO)$/mmconfigitem.obj \
        $(SLO)$/swdbtoolsclient.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

