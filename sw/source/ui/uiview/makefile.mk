#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:48 $
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
TARGET=uiview

AUTOSEG=true

PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..\ui_1st\ui_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRCFILES =  \
        view.src \
        pview.src

SLOFILES =  \
        $(SLO)$/view0.obj \
        $(SLO)$/view.obj \
        $(SLO)$/view1.obj \
        $(SLO)$/view2.obj \
        $(SLO)$/viewcoll.obj \
        $(SLO)$/viewdlg2.obj \
        $(SLO)$/viewdlg.obj \
        $(SLO)$/viewdraw.obj \
        $(SLO)$/viewling.obj \
        $(SLO)$/viewmdi.obj \
        $(SLO)$/pview.obj \
        $(SLO)$/viewport.obj \
        $(SLO)$/viewstat.obj \
        $(SLO)$/viewtab.obj \
        $(SLO)$/viewprt.obj \
        $(SLO)$/viewsrch.obj \
        $(SLO)$/scroll.obj \
        $(SLO)$/swcli.obj  \
        $(SLO)$/srcview.obj \
        $(SLO)$/uivwimp.obj

EXCEPTIONSFILES= \
        $(SLO)$/srcview.obj \
        $(SLO)$/uivwimp.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/uiview.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

