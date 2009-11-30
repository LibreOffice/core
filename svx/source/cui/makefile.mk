#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.65 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
PRJ=..$/..
PRJNAME=svx
TARGET=cui
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=cui
SRC1FILES =  \
        acccfg.src \
        align.src \
        autocdlg.src \
        backgrnd.src \
        bbdlg.src \
        border.src \
        cfg.src \
        chardlg.src \
        charmap.src \
        commonlingui.src \
        connect.src \
        connpooloptions.src \
        cuiimapdlg.src \
        dbregister.src \
        doclinkdialog.src \
        dstribut.src \
        eventdlg.src \
        fontsubs.src \
        grfflt.src \
        grfpage.src \
        hangulhanjadlg.src \
        hlmarkwn.src\
        iconcdlg.src \
        internationaloptions.src \
        macroass.src \
        macropg.src \
        measure.src \
        newtabledlg.src \
        numfmt.src \
        numpages.src \
        optfltr.src \
        optgdlg.src \
        opthtml.src \
        optjava.src \
        optmemory.src \
        optupdt.src \
        page.src \
        paragrph.src \
        postdlg.src \
        readonlyimage.src \
        scriptdlg.src \
        sdrcelldlg.src \
        securityoptions.src \
        selector.src \
        SpellDialog.src \
        splitcelldlg.src\
        srchxtra.src \
        svuidlg.src \
        swpossizetabpage.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        treeopt.src \
        webconninfo.src \
        zoom.src

SRS2NAME=cuidrawdlgs
SRC2FILES =  \
        dlgname.src \
        labdlg.src \
        tabarea.src \
        tabline.src \
        transfrm.src

SLOFILES+=\
        $(SLO)$/tparea.obj \
        $(SLO)$/borderconn.obj \
        $(SLO)$/cuiexp.obj \
        $(SLO)$/dlgfact.obj \
        $(SLO)$/init.obj \
        $(SLO)$/scriptdlg.obj \
        $(SLO)$/commonlingui.obj \
        $(SLO)$/selector.obj \
        $(SLO)$/fontsubs.obj \
        $(SLO)$/linkdlg.obj \
        $(SLO)$/pastedlg.obj \
        $(SLO)$/plfilter.obj \
        $(SLO)$/insdlg.obj \
        $(SLO)$/optHeaderTabListbox.obj \
        $(SLO)$/sdbcdriverenum.obj \
        $(SLO)$/connpoolsettings.obj \
        $(SLO)$/dbregisterednamesconfig.obj \
        $(SLO)$/dbregistersettings.obj \
        $(SLO)$/dbregister.obj \
        $(SLO)$/doclinkdialog.obj \
        $(SLO)$/connpoolconfig.obj \
        $(SLO)$/internationaloptions.obj \
        $(SLO)$/connpooloptions.obj \
        $(SLO)$/optgdlg.obj \
        $(SLO)$/optmemory.obj \
        $(SLO)$/opthtml.obj \
        $(SLO)$/optfltr.obj \
        $(SLO)$/treeopt.obj \
        $(SLO)$/autocdlg.obj \
        $(SLO)$/textanim.obj \
        $(SLO)$/labdlg.obj \
        $(SLO)$/dstribut.obj \
        $(SLO)$/hangulhanjadlg.obj \
        $(SLO)$/showcols.obj \
        $(SLO)$/zoom.obj \
        $(SLO)$/cuigaldlg.obj \
        $(SLO)$/cuiimapwnd.obj \
        $(SLO)$/hlmarkwn.obj \
        $(SLO)$/cuicharmap.obj \
        $(SLO)$/srchxtra.obj \
        $(SLO)$/bbdlg.obj \
        $(SLO)$/swpossizetabpage.obj \
        $(SLO)$/transfrm.obj \
        $(SLO)$/cuisrchdlg.obj \
        $(SLO)$/cuitbxform.obj \
        $(SLO)$/optdict.obj \
        $(SLO)$/dlgname.obj \
        $(SLO)$/multipat.obj \
        $(SLO)$/multifil.obj \
        $(SLO)$/iconcdlg.obj \
        $(SLO)$/hltpbase.obj \
        $(SLO)$/hldocntp.obj \
        $(SLO)$/hldoctp.obj \
        $(SLO)$/hlinettp.obj \
        $(SLO)$/hlmailtp.obj \
        $(SLO)$/cuihyperdlg.obj \
        $(SLO)$/cuifmsearch.obj \
        $(SLO)$/cuigrfflt.obj \
        $(SLO)$/readonlyimage.obj \
        $(SLO)$/optaccessibility.obj \
        $(SLO)$/optsave.obj \
        $(SLO)$/optasian.obj \
        $(SLO)$/optpath.obj \
        $(SLO)$/optcolor.obj \
        $(SLO)$/optjsearch.obj \
        $(SLO)$/optinet2.obj \
        $(SLO)$/securityoptions.obj \
        $(SLO)$/webconninfo.obj \
        $(SLO)$/optctl.obj \
        $(SLO)$/optjava.obj \
        $(SLO)$/optupdt.obj \
        $(SLO)$/optimprove.obj \
        $(SLO)$/optimprove2.obj \
        $(SLO)$/optgenrl.obj \
        $(SLO)$/connect.obj \
        $(SLO)$/tabarea.obj \
        $(SLO)$/tabline.obj \
        $(SLO)$/measure.obj \
        $(SLO)$/tpshadow.obj \
        $(SLO)$/tpline.obj \
        $(SLO)$/tpgradnt.obj \
        $(SLO)$/tpcolor.obj \
        $(SLO)$/tpbitmap.obj \
        $(SLO)$/tphatch.obj \
        $(SLO)$/tplneend.obj \
        $(SLO)$/tplnedef.obj \
        $(SLO)$/SpellAttrib.obj \
        $(SLO)$/SpellDialog.obj \
        $(SLO)$/numpages.obj \
        $(SLO)$/paragrph.obj \
        $(SLO)$/tabstpge.obj \
        $(SLO)$/textattr.obj \
        $(SLO)$/align.obj \
        $(SLO)$/backgrnd.obj \
        $(SLO)$/border.obj \
        $(SLO)$/chardlg.obj \
        $(SLO)$/numfmt.obj \
        $(SLO)$/page.obj \
        $(SLO)$/postdlg.obj \
        $(SLO)$/sdrcelldlg.obj \
        $(SLO)$/splitcelldlg.obj \
        $(SLO)$/newtabledlg.obj \
        $(SLO)$/grfpage.obj \
        $(SLO)$/cfgchart.obj \
        $(SLO)$/eventdlg.obj \
        $(SLO)$/macropg.obj \
        $(SLO)$/acccfg.obj\
        $(SLO)$/macroass.obj \
        $(SLO)$/cfg.obj \
        $(SLO)$/cfgutil.obj \
        $(SLO)$/optchart.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=$(SLO)$/winpluginlib.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
