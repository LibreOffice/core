#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:01:10 $
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
PRJ=..$/..

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch

PRJNAME=svx
TARGET=dialogs
AUTOSEG=true
VERSION=$(UPD)

.IF "$(debug)" != ""
LINK=n:\bin\optlinks\optlinks
.ELSE
OPTLINKS=YES
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(GUI)" != "MAC"
CFLAGS+=-DDG_DLL
.ELSE
CFLAGS+=-D DG_DLL
.ENDIF

# --- Files --------------------------------------------------------

HXX2TARGET= drawdlgs
HXX2EXT=    hxx
HXX2EXCL=   -E:*include*
HXX2DEPN=\
        $(INC)$/dlgctrl.hxx \
        $(INC)$/dlgname.hxx \
        $(INC)$/tabarea.hxx \
        $(INC)$/tabline.hxx \
        $(INC)$/labdlg.hxx \
        $(INC)$/transfrm.hxx

.IF "$(header)" == ""

IMGLST_SRS=$(SRS)$/dialogs.srs
BMP_IN=$(PRJ)$/win/res

CXXFILES = \
        _contdlg.cxx \
        _bmpmask.cxx \
        iconcdlg.cxx\
        ivctrl.cxx\
        imivctl1.cxx\
        imivctl2.cxx\
        contwnd.cxx \
        ctrdial.cxx\
        ctredlin.cxx\
        fontlb.cxx \
        graphctl.cxx \
        nforient.cxx\
        worient.cxx\
        align.cxx \
        backgrnd.cxx \
        bbdlg.cxx \
        border.cxx \
        chardlg.cxx \
        charmap.cxx \
        checklbx.cxx \
        connctrl.cxx \
        connect.cxx \
        dlgctl3d.cxx \
        dlgctrl.cxx \
        dlgname.cxx \
        dlgutil.cxx \
        fntctrl.cxx \
        fontwork.cxx \
        frmsel.cxx \
        hdft.cxx \
        hyperdlg.cxx \
        hltpbase.cxx\
        hlinettp.cxx\
        hlmailtp.cxx\
        hldoctp.cxx\
        hldocntp.cxx\
        hlmarkwn.cxx\
        hyphen.cxx \
        imapdlg.cxx \
        imapwnd.cxx \
        impgrf.cxx \
        labdlg.cxx \
        langbox.cxx \
        langtab.cxx \
        linelink.cxx \
        measctrl.cxx \
        measure.cxx \
        dstribut.cxx \
        numfmt.cxx \
        numpages.cxx \
        page.cxx \
        paragrph.cxx \
        paraprev.cxx \
        passwd.cxx \
        pfiledlg.cxx\
        prtqry.cxx \
        postdlg.cxx \
        relfld.cxx  \
        rlrcitem.cxx \
        rulritem.cxx \
        simptabl.cxx \
        spldlg.cxx \
        splwrap.cxx \
        srchctrl.cxx \
        srchdlg.cxx \
        srchxtra.cxx \
        stddlg.cxx \
        strarray.cxx \
        svxbox.cxx \
        svxruler.cxx \
        tabarea.cxx \
        tabline.cxx \
        tabstpge.cxx \
        textanim.cxx \
        textattr.cxx \
        thesdlg.cxx \
        tp3dattr.cxx \
        tparea.cxx   \
        tpbitmap.cxx \
        tpcolor.cxx  \
        tpgradnt.cxx \
        tphatch.cxx  \
        tpline.cxx \
        tplnedef.cxx \
        tplneend.cxx \
        tpshadow.cxx   \
        transfrm.cxx \
        zoom.cxx \
        $(PROJECTPCHSOURCE).cxx

SRS1NAME=dialogs
SRC1FILES =  \
        connect.src \
        contdlg.src \
        ctredlin.src \
        measure.src \
        dstribut.src \
        passwd.src	\
        align.src \
        backgrnd.src \
        bbdlg.src \
        bmpmask.src \
        border.src \
        chardlg.src \
        charmap.src \
        checklbx.src \
        fontwork.src \
        grfpage.src \
        hdft.src \
    hyperdlg.src \
    hlmarkwn.src\
        hyphen.src \
    iconcdlg.src \
        imapdlg.src \
        impgrf.src \
        langtab.src \
        language.src \
        lingu.src \
        numfmt.src \
        numpages.src \
        page.src \
        paragrph.src \
        postdlg.src \
        prtqry.src \
        ruler.src \
        spldlg.src \
        srchdlg.src \
        srchxtra.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        thesdlg.src \
        tp3dattr.src \
        zoom.src	\
        dlgctrl.src

SRS2NAME=drawdlgs
SRC2FILES =  \
        dlgname.src \
        sdstring.src \
        tabarea.src \
        tabline.src \
        labdlg.src \
        transfrm.src


SLOFILES=\
        $(SLO)$/_bmpmask.obj\
        $(SLO)$/_contdlg.obj\
        $(SLO)$/iconcdlg.obj\
        $(SLO)$/ivctrl.obj\
        $(SLO)$/imivctl1.obj\
        $(SLO)$/imivctl2.obj\
        $(SLO)$/connect.obj \
        $(SLO)$/connctrl.obj \
        $(SLO)$/contwnd.obj	\
        $(SLO)$/fontlb.obj   \
        $(SLO)$/graphctl.obj \
        $(SLO)$/hyperdlg.obj \
        $(SLO)$/hltpbase.obj \
        $(SLO)$/hlinettp.obj \
        $(SLO)$/hlmailtp.obj \
        $(SLO)$/hldoctp.obj \
        $(SLO)$/hldocntp.obj \
        $(SLO)$/hlmarkwn.obj \
        $(SLO)$/imapdlg.obj \
        $(SLO)$/imapwnd.obj \
        $(SLO)$/measure.obj \
        $(SLO)$/dstribut.obj \
        $(SLO)$/measctrl.obj \
        $(SLO)$/passwd.obj \
        $(SLO)$/strarray.obj \
        $(SLO)$/align.obj    \
        $(SLO)$/backgrnd.obj \
        $(SLO)$/bbdlg.obj    \
        $(SLO)$/border.obj   \
        $(SLO)$/charmap.obj  \
        $(SLO)$/chardlg.obj  \
        $(SLO)$/checklbx.obj \
        $(SLO)$/ctrdial.obj    \
        $(SLO)$/ctredlin.obj \
        $(SLO)$/dlgctrl.obj \
        $(SLO)$/dlgctl3d.obj \
        $(SLO)$/dlgname.obj \
        $(SLO)$/dlgutil.obj  \
        $(SLO)$/fntctrl.obj  \
        $(SLO)$/fontwork.obj \
        $(SLO)$/frmsel.obj   \
    $(SLO)$/grfpage.obj \
        $(SLO)$/hdft.obj     \
    $(SLO)$/hyphen.obj   \
        $(SLO)$/impgrf.obj   \
        $(SLO)$/labdlg.obj \
        $(SLO)$/langtab.obj \
        $(SLO)$/langbox.obj \
        $(SLO)$/linelink.obj \
        $(SLO)$/nforient.obj    \
        $(SLO)$/numfmt.obj   \
    $(SLO)$/numpages.obj   \
        $(SLO)$/page.obj     \
        $(SLO)$/pagectrl.obj \
        $(SLO)$/paragrph.obj \
        $(SLO)$/paraprev.obj \
    $(SLO)$/pfiledlg.obj \
        $(SLO)$/postdlg.obj  \
    $(SLO)$/prtqry.obj \
        $(SLO)$/relfld.obj   \
        $(SLO)$/rlrcitem.obj \
        $(SLO)$/rulritem.obj \
    $(SLO)$/simptabl.obj \
    $(SLO)$/spldlg.obj   \
    $(SLO)$/splwrap.obj  \
        $(SLO)$/srchxtra.obj \
        $(SLO)$/srchctrl.obj \
    $(SLO)$/srchdlg.obj  \
        $(SLO)$/stddlg.obj   \
        $(SLO)$/svxbox.obj   \
        $(SLO)$/svxruler.obj \
        $(SLO)$/tabarea.obj \
        $(SLO)$/tabline.obj \
        $(SLO)$/tabstpge.obj \
        $(SLO)$/textattr.obj \
        $(SLO)$/textanim.obj \
    $(SLO)$/thesdlg.obj  \
        $(SLO)$/tp3dattr.obj \
        $(SLO)$/tparea.obj   \
        $(SLO)$/tpbitmap.obj \
        $(SLO)$/tpcolor.obj  \
        $(SLO)$/tpgradnt.obj \
        $(SLO)$/tphatch.obj  \
        $(SLO)$/tpline.obj \
        $(SLO)$/tplneend.obj \
        $(SLO)$/tplnedef.obj \
        $(SLO)$/tpshadow.obj   \
        $(SLO)$/transfrm.obj \
        $(SLO)$/worient.obj    \
        $(SLO)$/zoom.obj

EXCEPTIONSFILES=\
        $(SLO)$/_bmpmask.obj	\
        $(SLO)$/hldocntp.obj	\
        $(SLO)$/hlmarkwn.obj	\
        $(SLO)$/hyphen.obj		\
        $(SLO)$/thesdlg.obj		\
        $(SLO)$/spldlg.obj		\
        $(SLO)$/splwrap.obj

.IF "$(COM)" == "C50"
EXCEPTIONSNOOPTFILES=$(SLO)$/impgrf.obj
.ELSE
EXCEPTIONSFILES+=$(SLO)$/impgrf.obj
.ENDIF

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

