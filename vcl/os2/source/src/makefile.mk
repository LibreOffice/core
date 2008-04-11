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
# $Revision: 1.7 $
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

# --- Files --------------------------------------------------------

RCDEPN= 	nullptr.ptr 		\
            help.ptr			\
            cross.ptr			\
            move.ptr			\
            hsplit.ptr			\
            vsplit.ptr			\
            hsizebar.ptr		\
            vsizebar.ptr		\
            hand.ptr			\
            refhand.ptr 		\
            pen.ptr 			\
            magnify.ptr 		\
            fill.ptr			\
            rotate.ptr			\
            hshear.ptr			\
            vshear.ptr			\
            mirror.ptr			\
            crook.ptr			\
            crop.ptr			\
            movept.ptr			\
            movebw.ptr			\
            movedata.ptr		\
            copydata.ptr		\
            linkdata.ptr		\
            movedlnk.ptr		\
            copydlnk.ptr		\
            movef.ptr			\
            copyf.ptr			\
            linkf.ptr			\
            moveflnk.ptr		\
            copyflnk.ptr		\
            movef2.ptr			\
            copyf2.ptr			\
            dline.ptr			\
            drect.ptr			\
            dpolygon.ptr		\
            dbezier.ptr 		\
            darc.ptr			\
            dpie.ptr			\
            dcirccut.ptr		\
            dellipse.ptr		\
            dfree.ptr			\
            dconnect.ptr		\
            dtext.ptr			\
            dcapt.ptr			\
            chart.ptr			\
            detectiv.ptr		\
            pivotcol.ptr		\
            pivotrow.ptr		\
            pivotfld.ptr		\
            chain.ptr			\
            chainnot.ptr		\
            timemove.ptr		\
            timesize.ptr		\
            asn.ptr 			\
            ass.ptr 			\
            asw.ptr 			\
            ase.ptr 			\
            asnw.ptr			\
            asne.ptr			\
            assw.ptr			\
            asse.ptr			\
            asns.ptr			\
            aswe.ptr			\
            asnswe.ptr			\
            sd.ico

RCFILES=	salsrc.rc

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
