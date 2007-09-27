#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 11:32:44 $
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
        $(SLO)$/maildispatcher.obj \
        $(SLO)$/mailmergechildwindow.obj \
        $(SLO)$/mailmergehelper.obj \
        $(SLO)$/mmaddressblockpage.obj \
        $(SLO)$/mmconfigitem.obj \
        $(SLO)$/mmlayoutpage.obj \
        $(SLO)$/mmoutputpage.obj

SLOFILES =  \
        $(SLO)$/addresslistdialog.obj \
        $(SLO)$/createaddresslistdialog.obj \
        $(SLO)$/customizeaddresslistdialog.obj \
        $(SLO)$/dbinsdlg.obj \
        $(SLO)$/dbmgr.obj \
        $(SLO)$/dbtree.obj \
        $(SLO)$/dbtablepreviewdialog.obj \
        $(SLO)$/maildispatcher.obj \
        $(SLO)$/mailmergehelper.obj \
        $(SLO)$/mailmergewizard.obj \
        $(SLO)$/mailmergechildwindow.obj \
        $(SLO)$/mmconfigitem.obj \
        $(SLO)$/mmdocselectpage.obj \
        $(SLO)$/mmlayoutpage.obj \
        $(SLO)$/mmoutputpage.obj \
        $(SLO)$/mmoutputtypepage.obj \
        $(SLO)$/mmaddressblockpage.obj \
        $(SLO)$/mmgreetingspage.obj \
        $(SLO)$/mmmergepage.obj \
        $(SLO)$/mmpreparemergepage.obj \
        $(SLO)$/dbui.obj \
        $(SLO)$/selectdbtabledialog.obj \
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

