#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:50 $
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

PRJNAME=sw
TARGET=utlui

AUTOSEG=true

PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..\ui_1st\ui_pch

# future: DEMO\finder.obj

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

IMGLST_SRS=$(SRS)$/utlui.srs
BMP_IN=$(PRJ)$/win/imglst


# --- Files --------------------------------------------------------

CXXFILES = \
        bookctrl.cxx \
        condedit.cxx \
        content.cxx \
        finder.cxx \
        gloslst.cxx \
        glbltree.cxx \
        initui.cxx \
        navipi.cxx \
        navicfg.cxx \
        numfmtlb.cxx \
        prcntfld.cxx \
        tmplctrl.cxx \
        uitool.cxx \
        uiitems.cxx \
        attrdesc.cxx \
        swstbctl.cxx \
        shdwcrsr.cxx \
        unotools.cxx \
        zoomctrl.cxx

SRCFILES =  \
        initui.src \
        gloslst.src \
        navipi.src \
        poolfmt.src \
        attrdesc.src \
        unotools.src \
        utlui.src

SLOFILES =  \
        $(SLO)$/bookctrl.obj \
        $(SLO)$/condedit.obj \
        $(SLO)$/content.obj \
        $(SLO)$/finder.obj \
        $(SLO)$/gloslst.obj \
        $(SLO)$/glbltree.obj \
        $(SLO)$/initui.obj \
        $(SLO)$/navipi.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/numfmtlb.obj \
        $(SLO)$/prcntfld.obj \
        $(SLO)$/tmplctrl.obj \
        $(SLO)$/uitool.obj \
        $(SLO)$/uiitems.obj \
        $(SLO)$/attrdesc.obj \
        $(SLO)$/swstbctl.obj \
        $(SLO)$/shdwcrsr.obj \
        $(SLO)$/unotools.obj \
        $(SLO)$/zoomctrl.obj

EXCEPTIONSFILES= \
        $(SLO)$/unotools.obj\
        $(SLO)$/gloslst.obj \

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

