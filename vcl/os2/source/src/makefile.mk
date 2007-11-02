#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 12:51:01 $
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
