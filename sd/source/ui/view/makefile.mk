#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:48:44 $
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
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=view

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = $(PRJ)$/util$/sd.cxx \
        sdview.cxx		\
        sdview2.cxx 	\
        sdview3.cxx 	\
        sdview4.cxx 	\
        sdview5.cxx 	\
        viewshel.cxx	\
        viewshe2.cxx	\
        viewshe3.cxx	\
        sdwindow.cxx	\
        drviewsh.cxx	\
        drviews1.cxx	\
        drviews2.cxx	\
        drviews3.cxx	\
        drviews4.cxx	\
        drviews5.cxx	\
        drviews6.cxx	\
        drviews7.cxx	\
        drviews8.cxx	\
        drviews9.cxx	\
        drviewsa.cxx	\
        drviewsb.cxx	\
        drviewsc.cxx	\
        drviewsd.cxx	\
        drviewse.cxx	\
        drviewsf.cxx	\
        drviewsg.cxx	\
        drviewsi.cxx	\
        drviewsj.cxx	\
        drvwshrg.cxx	\
        drawview.cxx	\
        grviewsh.cxx	\
        slidvish.cxx	\
        slidview.cxx	\
        outlnvsh.cxx	\
        outlnvs2.cxx	\
        outlview.cxx	\
        tabcontr.cxx	\
        layertab.cxx	\
        showview.cxx	\
        showwin.cxx 	\
        bmcache.cxx 	\
        drstdob.cxx 	\
        grstdob.cxx 	\
        drbezob.cxx 	\
        drglueob.cxx 	\
        drtxtob.cxx 	\
        drtxtob1.cxx 	\
        sdruler.cxx 	\
        sdoutl.cxx		\
        frmview.cxx 	\
        clview.cxx		\
        zoomlist.cxx	\
        unmodpg.cxx		\
        drgrfob.cxx

EXCEPTIONSFILES= \
        $(SLO)$/sdoutl.obj  \
        $(SLO)$/drviews8.obj

SLOFILES =	\
        $(SLO)$/sdview.obj	\
        $(SLO)$/sdview2.obj	\
        $(SLO)$/sdview3.obj	\
        $(SLO)$/sdview4.obj	\
        $(SLO)$/sdview5.obj	\
        $(SLO)$/viewshel.obj \
        $(SLO)$/viewshe2.obj \
        $(SLO)$/viewshe3.obj \
        $(SLO)$/sdwindow.obj \
        $(SLO)$/drviewsh.obj \
        $(SLO)$/drviews1.obj \
        $(SLO)$/drviews2.obj \
        $(SLO)$/drviews3.obj \
        $(SLO)$/drviews4.obj \
        $(SLO)$/drviews5.obj \
        $(SLO)$/drviews6.obj \
        $(SLO)$/drviews7.obj \
        $(SLO)$/drviews8.obj \
        $(SLO)$/drviews9.obj \
        $(SLO)$/drviewsa.obj \
        $(SLO)$/drviewsb.obj \
        $(SLO)$/drviewsc.obj \
        $(SLO)$/drviewsd.obj \
        $(SLO)$/drviewse.obj \
        $(SLO)$/drviewsf.obj \
        $(SLO)$/drviewsg.obj \
        $(SLO)$/drviewsi.obj \
        $(SLO)$/drviewsj.obj \
        $(SLO)$/drvwshrg.obj \
        $(SLO)$/drawview.obj \
        $(SLO)$/grviewsh.obj \
        $(SLO)$/slidvish.obj \
        $(SLO)$/slidview.obj \
        $(SLO)$/outlnvsh.obj \
        $(SLO)$/outlnvs2.obj \
        $(SLO)$/outlview.obj \
        $(SLO)$/tabcontr.obj \
        $(SLO)$/layertab.obj \
        $(SLO)$/showview.obj \
        $(SLO)$/showwin.obj	\
        $(SLO)$/bmcache.obj	\
        $(SLO)$/drstdob.obj	\
        $(SLO)$/grstdob.obj	\
        $(SLO)$/drbezob.obj	\
        $(SLO)$/drglueob.obj	\
        $(SLO)$/drtxtob.obj	\
        $(SLO)$/drtxtob1.obj	\
        $(SLO)$/sdruler.obj	\
        $(SLO)$/sdoutl.obj  \
        $(SLO)$/frmview.obj	\
        $(SLO)$/clview.obj  \
        $(SLO)$/zoomlist.obj	\
        $(SLO)$/unmodpg.obj	\
        $(SLO)$/drgrfob.obj


.IF "$(GUI)" == "WIN"
NOOPTFILES=\
    $(SLO)$/drviews5.obj
.ENDIF

.IF "$(GUI)" == "WNT"
NOOPTFILES=\
    $(SLO)$/drviews5.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

