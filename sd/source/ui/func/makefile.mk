#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util\sd
PRJNAME=sd
TARGET=func

AUTOSEG=true

# --- Settings -----------------------------------------------------

#.INCLUDE :  sdpre.mk
.INCLUDE :  settings.mk
#.INCLUDE :  sd.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk


# cppu code requires name space support.
.IF "$(USE_NAMESPACE)"!=""

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
UNOUCROUT=$(OUT)$/inc

UNOTYPES=\
    com.sun.star.task.XInteractionHandler \
    com.sun.star.uno.XInterface \
    com.sun.star.uno.XWeak \
    com.sun.star.uno.TypeClass


.ENDIF # USE_NAMESPACE

# --- Files --------------------------------------------------------

CXXFILES = $(PRJ)$/util$/sd.cxx \
        fuhtmlex.cxx	\
        fupoor.cxx		\
        fudraw.cxx		\
        fusel.cxx		\
        fuconstr.cxx	\
        fuconrec.cxx	\
        fuconuno.cxx	\
        fuconbez.cxx	\
        fuediglu.cxx	\
        fuzoom.cxx		\
        fuslid.cxx		\
        fuslsel.cxx 	\
        fuslshow.cxx	\
        fuslshw2.cxx	\
        fuslshw3.cxx	\
        fuslshw4.cxx	\
        fuslshw5.cxx	\
        fuslshw6.cxx	\
        fusldlg.cxx		\
        fader.cxx		\
        fader2.cxx		\
        mover.cxx		\
        fuslprms.cxx	\
        fuscale.cxx 	\
        fuslhide.cxx	\
        futransf.cxx	\
        futext.cxx		\
        futext2.cxx		\
        fuline.cxx		\
        sdundo.cxx		\
        sdundogr.cxx	\
        unslprms.cxx	\
        fuoaprms.cxx	\
        unoaprms.cxx	\
        fuarea.cxx		\
        fuchar.cxx		\
        fuconarc.cxx	\
        fuparagr.cxx	\
        fubullet.cxx	\
        futempl.cxx 	\
        fuoutl.cxx		\
        fuoltext.cxx	\
        fuinsert.cxx	\
        fupage.cxx		\
        undopage.cxx	\
        fuprlout.cxx	\
        fuprobjs.cxx	\
        fulinend.cxx	\
        fusnapln.cxx	\
        fuolbull.cxx	\
        fucopy.cxx		\
        fulink.cxx		\
        fuspell.cxx 	\
        futhes.cxx		\
        fusearch.cxx	\
        fuinsfil.cxx	\
        futxtatt.cxx	\
        fumeasur.cxx	\
        fuconnct.cxx	\
        unprlout.cxx	\
        fudspord.cxx	\
        unmovss.cxx 	\
        fucon3d.cxx 	\
        fuexp1.cxx		\
        fuexp2.cxx		\
        fumorph.cxx		\
        fuexpand.cxx	\
        fusumry.cxx     \
        sdmtf.cxx		\
        sprite.cxx		\
        outlinfo.cxx	\
        fucushow.cxx	\
        speedctl.cxx	\
        fuvect.cxx      \
        bulmaper.cxx	\
        fugrfflt.cxx	\
        fupngdlg.cxx

EXCEPTIONSFILES= \
        $(SLO)$/futext2.obj		\
        $(SLO)$/fupngdlg.obj

SLOFILES = \
        $(SLO)$/fuhtmlex.obj	\
        $(SLO)$/fupoor.obj		\
        $(SLO)$/fudraw.obj		\
        $(SLO)$/fuzoom.obj		\
        $(SLO)$/fusel.obj		\
        $(SLO)$/fuconstr.obj 	\
        $(SLO)$/fuconrec.obj 	\
        $(SLO)$/fuconuno.obj 	\
        $(SLO)$/fuconbez.obj 	\
        $(SLO)$/fuediglu.obj 	\
        $(SLO)$/fuslid.obj		\
        $(SLO)$/fuslsel.obj		\
        $(SLO)$/fuslshow.obj 	\
        $(SLO)$/fuslshw2.obj 	\
        $(SLO)$/fuslshw3.obj 	\
        $(SLO)$/fuslshw4.obj 	\
        $(SLO)$/fuslshw5.obj 	\
        $(SLO)$/fuslshw6.obj 	\
        $(SLO)$/fusldlg.obj 	\
        $(SLO)$/fader.obj		\
        $(SLO)$/fader2.obj		\
        $(SLO)$/mover.obj		\
        $(SLO)$/fuslprms.obj 	\
        $(SLO)$/fuscale.obj		\
        $(SLO)$/fuslhide.obj 	\
        $(SLO)$/futransf.obj 	\
        $(SLO)$/futext.obj		\
        $(SLO)$/futext2.obj		\
        $(SLO)$/fuline.obj		\
        $(SLO)$/sdundo.obj		\
        $(SLO)$/sdundogr.obj 	\
        $(SLO)$/unslprms.obj 	\
        $(SLO)$/fuoaprms.obj 	\
        $(SLO)$/unoaprms.obj 	\
        $(SLO)$/fuarea.obj		\
        $(SLO)$/fuchar.obj		\
        $(SLO)$/fuconarc.obj 	\
        $(SLO)$/fuparagr.obj 	\
        $(SLO)$/fubullet.obj 	\
        $(SLO)$/futempl.obj		\
        $(SLO)$/fuoutl.obj		\
        $(SLO)$/fuoltext.obj 	\
        $(SLO)$/fuinsert.obj 	\
        $(SLO)$/fupage.obj		\
        $(SLO)$/undopage.obj 	\
        $(SLO)$/fuprlout.obj 	\
        $(SLO)$/fuprobjs.obj 	\
        $(SLO)$/fulinend.obj 	\
        $(SLO)$/fusnapln.obj 	\
        $(SLO)$/fuolbull.obj 	\
        $(SLO)$/fucopy.obj		\
        $(SLO)$/fulink.obj		\
        $(SLO)$/fuspell.obj		\
        $(SLO)$/futhes.obj		\
        $(SLO)$/fusearch.obj 	\
        $(SLO)$/fuinsfil.obj 	\
        $(SLO)$/futxtatt.obj 	\
        $(SLO)$/fumeasur.obj 	\
        $(SLO)$/fuconnct.obj 	\
        $(SLO)$/unprlout.obj 	\
        $(SLO)$/fudspord.obj 	\
        $(SLO)$/unmovss.obj		\
        $(SLO)$/fucon3d.obj		\
        $(SLO)$/fuexp1.obj		\
        $(SLO)$/fuexp2.obj		\
        $(SLO)$/fumorph.obj		\
        $(SLO)$/fuexpand.obj    \
        $(SLO)$/fusumry.obj     \
        $(SLO)$/sdmtf.obj		\
        $(SLO)$/sprite.obj		\
        $(SLO)$/outlinfo.obj	\
        $(SLO)$/fucushow.obj	\
        $(SLO)$/speedctl.obj	\
        $(SLO)$/fuvect.obj		\
        $(SLO)$/bulmaper.obj	\
        $(SLO)$/fugrfflt.obj	\
        $(SLO)$/fupngdlg.obj


.IF "$(GUI)" == "WNT"

NOOPTFILES=\
    $(SLO)$/futext.obj

.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
