#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ka $ $Date: 2000-11-18 11:44:07 $
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
TARGET=dlg

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = $(PRJ)$/util$/sd.cxx \
        inspage.cxx \
        inspagob.cxx \
        present.cxx \
        dlgctrls.cxx \
        dlgchar.cxx \
        paragr.cxx \
        newfoil.cxx \
        tabtempl.cxx \
        dlgpage.cxx  \
        diactrl.cxx  \
        gluectrl.cxx  \
        sdtreelb.cxx \
        tpeffect.cxx \
        tpaction.cxx \
        animobjs.cxx \
        prltempl.cxx \
        prntopts.cxx \
        enumdlg.cxx  \
        dlgsnap.cxx  \
        copydlg.cxx  \
        dlgolbul.cxx \
        tpoption.cxx \
        tpscale.cxx \
        docdlg.cxx   \
        unchss.cxx   \
        dlgassim.cxx	\
        dlgass.cxx   \
        assclass.cxx \
        graphpro.cxx    \
        pubdlg.cxx	 \
        navigatr.cxx    \
        sdpreslt.cxx	\
        preview.cxx    \
        effect.cxx    \
        effcthdl.cxx    \
        effcthlp.cxx    \
        tlborder.cxx    \
        slidechg.cxx    \
        slchghdl.cxx    \
        slchghlp.cxx    \
        morphdlg.cxx	\
        printdlg.cxx	\
        dlgfield.cxx	\
        custsdlg.cxx	\
        brkdlg.cxx      \
        htmlattr.cxx	\
        vectdlg.cxx		\
        docprev.cxx     \
        packgdlg.cxx	

SRCFILES =\
    ins_page.src\
    inspagob.src\
    present.src\
    dlg_char.src\
    paragr.src\
    new_foil.src\
    tabtempl.src\
    dlgpage.src\
    tpeffect.src\
    animobjs.src\
    prltempl.src\
    prntopts.src\
    enumdlg.src\
    dlgsnap.src\
    copydlg.src\
    tpoption.src\
    tpscale.src\
    docdlg.src\
    dlgass.src\
    pubdlg.src\
    navigatr.src\
    sdpreslt.src\
    preview.src\
    effect.src\
    slidechg.src\
    morphdlg.src\
    printdlg.src\
    dlgfield.src\
    custsdlg.src\
    brkdlg.src\
    vectdlg.src\
    dlgolbul.src\
    packgdlg.src 


SLOFILES =  \
        $(SLO)$/inspage.obj \
        $(SLO)$/inspagob.obj \
        $(SLO)$/present.obj  \
        $(SLO)$/dlgctrls.obj \
        $(SLO)$/dlgchar.obj \
        $(SLO)$/paragr.obj \
        $(SLO)$/newfoil.obj \
        $(SLO)$/tabtempl.obj \
        $(SLO)$/dlgpage.obj  \
        $(SLO)$/diactrl.obj  \
        $(SLO)$/gluectrl.obj  \
        $(SLO)$/sdtreelb.obj \
        $(SLO)$/tpeffect.obj \
        $(SLO)$/tpaction.obj \
        $(SLO)$/animobjs.obj \
        $(SLO)$/prltempl.obj \
        $(SLO)$/prntopts.obj \
        $(SLO)$/enumdlg.obj  \
        $(SLO)$/dlgsnap.obj  \
        $(SLO)$/copydlg.obj  \
        $(SLO)$/dlgolbul.obj \
        $(SLO)$/tpoption.obj \
        $(SLO)$/tpscale.obj \
        $(SLO)$/docdlg.obj   \
        $(SLO)$/unchss.obj  \
        $(SLO)$/dlgassim.obj	\
        $(SLO)$/dlgass.obj  \
        $(SLO)$/assclass.obj \
        $(SLO)$/graphpro.obj \
        $(SLO)$/pubdlg.obj	\
        $(SLO)$/navigatr.obj \
        $(SLO)$/preview.obj    \
        $(SLO)$/effect.obj   \
        $(SLO)$/effcthdl.obj   \
        $(SLO)$/effcthlp.obj   \
        $(SLO)$/tlborder.obj   \
        $(SLO)$/slidechg.obj   \
        $(SLO)$/slchghdl.obj   \
        $(SLO)$/slchghlp.obj   \
        $(SLO)$/sdpreslt.obj \
        $(SLO)$/morphdlg.obj	\
        $(SLO)$/printdlg.obj	\
        $(SLO)$/dlgfield.obj	\
        $(SLO)$/custsdlg.obj	\
        $(SLO)$/brkdlg.obj      \
        $(SLO)$/htmlattr.obj	\
        $(SLO)$/vectdlg.obj		\
        $(SLO)$/docprev.obj     \
        $(SLO)$/packgdlg.obj	

.IF "$(GUI)" == "WIN"

NOOPTFILES=\
    $(SLO)$/tpeffect.obj

.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


