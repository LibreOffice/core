#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: nn $ $Date: 2000-10-12 10:20:42 $
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
TARGET=unoobj

PROJECTPCH4DLL=TRUE
PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..\pch\ui_pch

ENABLE_EXCEPTIONS=TRUE

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
CXXFILES = \
        docuno.cxx \
        servuno.cxx \
        cellsuno.cxx \
        textuno.cxx \
        notesuno.cxx \
        cursuno.cxx \
        srchuno.cxx \
        fielduno.cxx \
        miscuno.cxx \
        optuno.cxx \
        appluno.cxx \
        funcuno.cxx \
        nameuno.cxx \
        viewuno.cxx \
        datauno.cxx \
        dapiuno.cxx \
        chartuno.cxx \
        styleuno.cxx \
        afmtuno.cxx \
        fmtuno.cxx \
        linkuno.cxx \
        targuno.cxx \
        convuno.cxx \
        editsrc.cxx \
        unoguard.cxx

SLOFILES =  \
        $(SLO)$/docuno.obj \
        $(SLO)$/servuno.obj \
        $(SLO)$/cellsuno.obj \
        $(SLO)$/textuno.obj \
        $(SLO)$/notesuno.obj \
        $(SLO)$/cursuno.obj \
        $(SLO)$/srchuno.obj \
        $(SLO)$/fielduno.obj \
        $(SLO)$/miscuno.obj \
        $(SLO)$/optuno.obj \
        $(SLO)$/appluno.obj \
        $(SLO)$/funcuno.obj \
        $(SLO)$/nameuno.obj \
        $(SLO)$/viewuno.obj \
        $(SLO)$/datauno.obj \
        $(SLO)$/dapiuno.obj \
        $(SLO)$/chartuno.obj \
        $(SLO)$/styleuno.obj \
        $(SLO)$/afmtuno.obj \
        $(SLO)$/fmtuno.obj \
        $(SLO)$/linkuno.obj \
        $(SLO)$/targuno.obj \
        $(SLO)$/convuno.obj \
        $(SLO)$/editsrc.obj \
        $(SLO)$/unoguard.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

