#*************************************************************************
#
#   $RCSfile: MAKEFILE.MK,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2003-10-20 17:50:28 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salsrc

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

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
            50.bmp				\
            sd.ico

RCFILES=	salsrc.rc

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
