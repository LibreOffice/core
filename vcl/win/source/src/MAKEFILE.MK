#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: MAKEFILE.MK,v $
#
# $Revision: 1.10 $
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

PRJNAME=vcl
TARGET=salsrc

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- #105371#
CFLAGS += -DWINVER=0x0400

# --- Files --------------------------------------------------------

RCDEPN= 	nullptr.cur 		\
            help.cur			\
            hsize.cur			\
            vsize.cur			\
            neswsize.cur		\
            nwsesize.cur		\
            cross.cur			\
            move.cur			\
            hsplit.cur			\
            vsplit.cur			\
            hsizebar.cur		\
            vsizebar.cur		\
            hand.cur			\
            refhand.cur 		\
            pen.cur 			\
            magnify.cur 		\
            fill.cur			\
            rotate.cur			\
            hshear.cur			\
            vshear.cur			\
            mirror.cur			\
            crook.cur			\
            crop.cur			\
            movept.cur			\
            movebw.cur			\
            movedata.cur		\
            copydata.cur		\
            linkdata.cur		\
            movedlnk.cur		\
            copydlnk.cur		\
            movef.cur			\
            copyf.cur			\
            linkf.cur			\
            moveflnk.cur		\
            copyflnk.cur		\
            movef2.cur			\
            copyf2.cur			\
            notallow.cur		\
            dline.cur			\
            drect.cur			\
            dpolygon.cur		\
            dbezier.cur 		\
            darc.cur			\
            dpie.cur			\
            dcirccut.cur		\
            dellipse.cur		\
            dfree.cur			\
            dconnect.cur		\
            dtext.cur			\
            dcapt.cur			\
            chart.cur			\
            detectiv.cur		\
            pivotcol.cur		\
            pivotrow.cur		\
            pivotfld.cur		\
            pivotdel.cur		\
            chain.cur			\
            chainnot.cur		\
            timemove.cur		\
            timesize.cur		\
            asn.cur 			\
            ass.cur 			\
            asw.cur 			\
            ase.cur 			\
            asnw.cur			\
            asne.cur			\
            assw.cur			\
            asse.cur			\
            asns.cur			\
            aswe.cur			\
            asnswe.cur			\
            airbrush.cur		\
            vtext.cur			\
                        tblsels.cur                     \
                        tblsele.cur                     \
                        tblselse.cur                     \
                        tblselw.cur                     \
                        tblselsw.cur                     \
                        pntbrsh.cur                  \
                        50.bmp                          \
            sd.ico

RCFILES=	salsrc.rc

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
