#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
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

PRJNAME=sc
TARGET=miscdlgs

PROJECTPCH4DLL=TRUE
PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..$/pch$/ui_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        delcldlg.cxx inscldlg.cxx 	\
        delcodlg.cxx inscodlg.cxx	\
        strindlg.cxx mtrindlg.cxx 	\
        lbseldlg.cxx filldlg.cxx  	\
        autofmt.cxx  solvrdlg.cxx 	\
        mvtabdlg.cxx groupdlg.cxx 	\
        tabopdlg.cxx crdlg.cxx 		\
    namecrea.cxx namepast.cxx 	\
    teamdlg.cxx textdlgs.cxx 	\
    chartdlg.cxx anyrefdg.cxx	\
    crnrdlg.cxx shtabdlg.cxx \
    instbdlg.cxx acredlin.cxx \
    highred.cxx simpref.cxx	\
    redcom.cxx

SLOFILES =  \
        $(SLO)$/delcldlg.obj $(SLO)$/inscldlg.obj \
        $(SLO)$/delcodlg.obj $(SLO)$/inscodlg.obj \
        $(SLO)$/strindlg.obj $(SLO)$/mtrindlg.obj \
        $(SLO)$/lbseldlg.obj $(SLO)$/filldlg.obj  \
        $(SLO)$/autofmt.obj  $(SLO)$/solvrdlg.obj \
        $(SLO)$/mvtabdlg.obj $(SLO)$/groupdlg.obj \
        $(SLO)$/tabopdlg.obj $(SLO)$/crdlg.obj 	\
    $(SLO)$/namecrea.obj $(SLO)$/namepast.obj \
    $(SLO)$/teamdlg.obj $(SLO)$/textdlgs.obj 	\
    $(SLO)$/chartdlg.obj $(SLO)$/anyrefdg.obj	\
    $(SLO)$/crnrdlg.obj $(SLO)$/shtabdlg.obj \
    $(SLO)$/instbdlg.obj $(SLO)$/acredlin.obj \
    $(SLO)$/highred.obj	 $(SLO)$/simpref.obj  \
    $(SLO)$/redcom.obj	

SRCFILES = \
    instbdlg.src acredlin.src \
    highred.src

NOOPTFILES=\
    $(SLO)$/chartdlg.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

