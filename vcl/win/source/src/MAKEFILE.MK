#*************************************************************************
#*
#*	  $Workfile:   makefile.  $
#*
#*	  Ersterstellung	TH 01.04.97
#*	  Letzte Aenderung	$Author: hr $ $Date: 2000-09-18 17:05:49 $
#*	  $Revision: 1.1.1.1 $
#*
#*	  $Logfile:   T:/vcl/win/source/src/makefile.__v  $
#*
#*	  Copyright (c) 1990 - 2000, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salsrc

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

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
            50.bmp				\
            sd.ico

RCFILES=	salsrc.rc

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
