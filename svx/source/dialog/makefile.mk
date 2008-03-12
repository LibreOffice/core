#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.63 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 11:31:04 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
PRJ=..$/..
PRJNAME=svx
TARGET=dialogs
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=dialogs
SRC1FILES =  \
        svuidlg.src \
        autocdlg.src \
        connect.src \
        contdlg.src \
        ctredlin.src \
        measure.src \
        dstribut.src \
        passwd.src \
        align.src \
        backgrnd.src \
        bbdlg.src \
        bmpmask.src \
        border.src \
        chardlg.src \
        charmap.src \
        fontwork.src \
        frmsel.src \
        grfpage.src \
        hdft.src \
        hyperdlg.src \
        hyprlink.src \
        hlmarkwn.src\
        hyphen.src \
        iconcdlg.src \
        imapdlg.src \
        impgrf.src \
        language.src \
        lingu.src \
        numfmt.src \
        numpages.src \
        page.src \
        paragrph.src \
        postdlg.src \
        prtqry.src \
        ruler.src \
        rubydialog.src\
        SpellDialog.src \
        swframeposstrings.src \
        swpossizetabpage.src \
        scriptdlg.src \
        selector.src \
        srchdlg.src \
        srchxtra.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        thesdlg.src \
        txenctab.src \
        zoom.src \
        dlgctrl.src \
        grfflt.src \
        langbox.src \
        frmdirlbox.src \
        hangulhanjadlg.src \
        commonlingui.src \
        fontsubs.src \
        readonlyimage.src \
        opthtml.src \
        optgdlg.src \
        optmemory.src \
        treeopt.src \
        optfltr.src \
        connpooloptions.src \
        dbregister.src \
        doclinkdialog.src \
        internationaloptions.src \
        macropg.src \
        eventdlg.src \
        cfg.src \
        optjava.src \
        optupdt.src \
        splitcelldlg.src\
        sdrcelldlg.src \
        docrecovery.src \
        securityoptions.src \
        webconninfo.src \
        newtabledlg.src

SRS2NAME=drawdlgs
SRC2FILES =  \
        dlgname.src \
        sdstring.src \
        tabarea.src \
        tabline.src \
        labdlg.src \
        transfrm.src

.IF "$(GUI)"=="UNX"
SLOFILES=\
    $(SLO)$/sendreportunx.obj
.ELSE
.IF "$(GUI)"=="WNT"
SLOFILES=\
    $(SLO)$/sendreportw32.obj
NOOPTFILES=\
    $(SLO)$/sendreportw32.obj
.ELSE
SLOFILES=\
    $(SLO)$/sendreportgen.obj
.ENDIF
.ENDIF

SLOFILES+=\
    $(SLO)$/SpellDialogChildWindow.obj \
    $(SLO)$/thesdlg.obj     \
    $(SLO)$/rubydialog.obj  \
    $(SLO)$/hyphen.obj      \
    $(SLO)$/opengrf.obj     \
    $(SLO)$/internationaloptions.obj	\
    $(SLO)$/optHeaderTabListbox.obj		\
    $(SLO)$/charmap.obj     \
    $(SLO)$/connpooloptions.obj         \
    $(SLO)$/dbregistersettings.obj      \
    $(SLO)$/fontsubs.obj                \
    $(SLO)$/_bmpmask.obj    \
    $(SLO)$/optmemory.obj				\
    $(SLO)$/opthtml.obj					\
    $(SLO)$/optfltr.obj					\
    $(SLO)$/autocdlg.obj     \
    $(SLO)$/cuiexp.obj     \
    $(SLO)$/dlgfact.obj     \
    $(SLO)$/svxdlg.obj     \
    $(SLO)$/_contdlg.obj\
    $(SLO)$/iconcdlg.obj\
    $(SLO)$/connect.obj \
    $(SLO)$/connctrl.obj \
    $(SLO)$/contwnd.obj     \
    $(SLO)$/fontlb.obj   \
    $(SLO)$/graphctl.obj \
    $(SLO)$/hyperdlg.obj \
    $(SLO)$/hyprlink.obj \
    $(SLO)$/hlinettp.obj \
    $(SLO)$/hlmailtp.obj \
    $(SLO)$/hldoctp.obj \
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
    $(SLO)$/borderconn.obj   \
    $(SLO)$/chardlg.obj  \
    $(SLO)$/checklbx.obj \
    $(SLO)$/dialcontrol.obj    \
    $(SLO)$/ctredlin.obj \
    $(SLO)$/dlgctrl.obj \
    $(SLO)$/dlgctl3d.obj \
    $(SLO)$/dlgname.obj \
    $(SLO)$/dlgutil.obj  \
    $(SLO)$/fntctrl.obj  \
    $(SLO)$/fontwork.obj \
    $(SLO)$/grfpage.obj \
    $(SLO)$/hdft.obj     \
    $(SLO)$/impgrf.obj   \
    $(SLO)$/labdlg.obj \
    $(SLO)$/langbox.obj \
    $(SLO)$/wrapfield.obj \
    $(SLO)$/numfmt.obj   \
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
    $(SLO)$/srchxtra.obj \
    $(SLO)$/srchctrl.obj \
    $(SLO)$/srchdlg.obj  \
    $(SLO)$/stddlg.obj   \
    $(SLO)$/svxbox.obj   \
    $(SLO)$/svxruler.obj \
    $(SLO)$/swpossizetabpage.obj \
    $(SLO)$/tabarea.obj \
    $(SLO)$/tabline.obj \
    $(SLO)$/tabstpge.obj \
    $(SLO)$/textattr.obj \
    $(SLO)$/textanim.obj \
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
    $(SLO)$/orienthelper.obj    \
    $(SLO)$/txenctab.obj \
    $(SLO)$/txencbox.obj \
    $(SLO)$/zoom.obj         \
    $(SLO)$/grfflt.obj      \
    $(SLO)$/frmdirlbox.obj \
    $(SLO)$/commonlingui.obj \
    $(SLO)$/dialmgr.obj \
    $(SLO)$/cuiimapwnd.obj \
    $(SLO)$/cuisrchdlg.obj \
    $(SLO)$/cuitbxform.obj \
    $(SLO)$/cuicharmap.obj  \
    $(SLO)$/cuihyperdlg.obj	\
    $(SLO)$/cuifmsearch.obj \
    $(SLO)$/cuigrfflt.obj \
    $(SLO)$/multipat.obj \
    $(SLO)$/optcolor.obj    \
    $(SLO)$/optjsearch.obj  \
    $(SLO)$/optgenrl.obj    \
    $(SLO)$/optctl.obj  \
    $(SLO)$/svxgrahicitem.obj \
    $(SLO)$/SpellAttrib.obj  \
    $(SLO)$/SpellDialog.obj  \
    $(SLO)$/swframeposstrings.obj\
    $(SLO)$/swframeexample.obj \
    $(SLO)$/svxbmpnumvalueset.obj	\
    $(SLO)$/svxgrahicitem.obj \
    $(SLO)$/frmsel.obj	\
    $(SLO)$/framelink.obj	\
    $(SLO)$/framelinkarray.obj \
    $(SLO)$/databaseregistrationui.obj \
    $(SLO)$/radiobtnbox.obj \
    $(SLO)$/cfgchart.obj \
    $(SLO)$/optupdt.obj \
    $(SLO)$/optchart.obj \
    $(SLO)$/optinet2.obj    \
    $(SLO)$/securityoptions.obj \
    $(SLO)$/webconninfo.obj \
    $(SLO)$/optgdlg.obj \
    $(SLO)$/optdict.obj \
    $(SLO)$/optaccessibility.obj    \
    $(SLO)$/optsave.obj \
    $(SLO)$/optasian.obj \
    $(SLO)$/optpath.obj    \
    $(SLO)$/optjava.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES= \
    $(SLO)$/fontsubs.obj \
    $(SLO)$/svxdlg.obj \
    $(SLO)$/charmap.obj     \
    $(SLO)$/_bmpmask.obj    \
    $(SLO)$/frmsel.obj \
    $(SLO)$/opengrf.obj     \
    $(SLO)$/hyphen.obj      \
    $(SLO)$/rubydialog.obj  \
    $(SLO)$/thesdlg.obj     \
    $(SLO)$/splwrap.obj     \
    $(SLO)$/hangulhanja.obj \
    $(SLO)$/_contdlg.obj \
    $(SLO)$/connctrl.obj \
    $(SLO)$/contwnd.obj \
    $(SLO)$/fontlb.obj   \
    $(SLO)$/graphctl.obj \
    $(SLO)$/hyperdlg.obj \
    $(SLO)$/hyprlink.obj \
    $(SLO)$/imapdlg.obj \
    $(SLO)$/imapwnd.obj \
    $(SLO)$/measctrl.obj \
    $(SLO)$/passwd.obj \
    $(SLO)$/strarray.obj \
    $(SLO)$/borderconn.obj    \
    $(SLO)$/checklbx.obj \
    $(SLO)$/dialcontrol.obj    \
    $(SLO)$/ctredlin.obj \
    $(SLO)$/dlgctrl.obj \
    $(SLO)$/dlgctl3d.obj \
    $(SLO)$/dlgutil.obj  \
    $(SLO)$/docrecovery.obj  \
    $(SLO)$/fntctrl.obj  \
    $(SLO)$/fontwork.obj \
    $(SLO)$/hdft.obj     \
    $(SLO)$/impgrf.obj   \
    $(SLO)$/langbox.obj \
    $(SLO)$/wrapfield.obj    \
    $(SLO)$/pagectrl.obj \
    $(SLO)$/paraprev.obj \
    $(SLO)$/pfiledlg.obj \
    $(SLO)$/prtqry.obj \
    $(SLO)$/relfld.obj   \
    $(SLO)$/rlrcitem.obj \
    $(SLO)$/rulritem.obj \
    $(SLO)$/scriptdlg.obj \
    $(SLO)$/selector.obj \
    $(SLO)$/simptabl.obj \
    $(SLO)$/srchctrl.obj \
    $(SLO)$/srchdlg.obj  \
    $(SLO)$/stddlg.obj   \
    $(SLO)$/svxbox.obj   \
    $(SLO)$/svxruler.obj \
    $(SLO)$/tparea.obj   \
    $(SLO)$/orienthelper.obj    \
    $(SLO)$/txenctab.obj \
    $(SLO)$/txencbox.obj \
    $(SLO)$/grfflt.obj \
    $(SLO)$/frmdirlbox.obj \
    $(SLO)$/commonlingui.obj \
    $(SLO)$/SpellDialogChildWindow.obj \
    $(SLO)$/svxbmpnumvalueset.obj	\
    $(SLO)$/dialmgr.obj	\
    $(SLO)$/svxgrahicitem.obj \
    $(SLO)$/swframeexample.obj \
    $(SLO)$/swframeposstrings.obj \
    $(SLO)$/eventdlg.obj \
    $(SLO)$/macropg.obj \
    $(SLO)$/cfg.obj	\
    $(SLO)$/framelink.obj	\
    $(SLO)$/framelinkarray.obj \
    $(SLO)$/databaseregistrationui.obj \
    $(SLO)$/radiobtnbox.obj

.IF "$(GUI)"=="UNX"
LIB1OBJFILES +=	$(SLO)$/sendreportunx.obj
.ELSE
.IF "$(GUI)"=="WNT"
LIB1OBJFILES +=	$(SLO)$/sendreportw32.obj
.ELSE
LIB1OBJFILES +=	$(SLO)$/sendreportgen.obj
.ENDIF
.ENDIF

LIB2TARGET= $(SLB)$/cui.lib

LIB2OBJFILES= \
        $(SLO)$/linkdlg.obj \
        $(SLO)$/pastedlg.obj \
        $(SLO)$/plfilter.obj \
        $(SLO)$/insdlg.obj \
    $(SLO)$/optHeaderTabListbox.obj		\
    $(SLO)$/sdbcdriverenum.obj	\
    $(SLO)$/connpoolsettings.obj \
    $(SLO)$/dbregisterednamesconfig.obj \
    $(SLO)$/dbregistersettings.obj \
    $(SLO)$/dbregister.obj				\
    $(SLO)$/doclinkdialog.obj				\
    $(SLO)$/connpoolconfig.obj \
    $(SLO)$/internationaloptions.obj	\
    $(SLO)$/connpooloptions.obj			\
    $(SLO)$/optgdlg.obj					\
    $(SLO)$/optmemory.obj				\
    $(SLO)$/opthtml.obj					\
    $(SLO)$/optfltr.obj					\
    $(SLO)$/treeopt.obj \
    $(SLO)$/autocdlg.obj     \
    $(SLO)$/dialmgr.obj \
    $(SLO)$/textanim.obj \
    $(SLO)$/labdlg.obj	\
    $(SLO)$/dstribut.obj  \
    $(SLO)$/hangulhanjadlg.obj	\
    $(SLO)$/showcols.obj	\
    $(SLO)$/zoom.obj	\
    $(SLO)$/cuigaldlg.obj \
    $(SLO)$/cuiimapwnd.obj \
    $(SLO)$/hlmarkwn.obj	\
    $(SLO)$/cuicharmap.obj  \
    $(SLO)$/srchxtra.obj	\
    $(SLO)$/bbdlg.obj	\
    $(SLO)$/swpossizetabpage.obj \
    $(SLO)$/transfrm.obj \
    $(SLO)$/cuisrchdlg.obj \
    $(SLO)$/cuitbxform.obj \
    $(SLO)$/optdict.obj \
    $(SLO)$/dlgname.obj \
    $(SLO)$/multipat.obj \
    $(SLO)$/multifil.obj	\
    $(SLO)$/iconcdlg.obj	\
    $(SLO)$/hltpbase.obj    \
    $(SLO)$/hldocntp.obj    \
    $(SLO)$/hldoctp.obj \
    $(SLO)$/hlinettp.obj \
    $(SLO)$/hlmailtp.obj 	\
    $(SLO)$/cuihyperdlg.obj	\
    $(SLO)$/cuifmsearch.obj \
    $(SLO)$/cuigrfflt.obj \
    $(SLO)$/readonlyimage.obj \
    $(SLO)$/optaccessibility.obj    \
    $(SLO)$/optsave.obj		\
    $(SLO)$/optasian.obj	\
    $(SLO)$/optpath.obj		\
    $(SLO)$/optcolor.obj	\
    $(SLO)$/optjsearch.obj	\
    $(SLO)$/optinet2.obj    \
    $(SLO)$/securityoptions.obj \
    $(SLO)$/webconninfo.obj \
    $(SLO)$/optctl.obj	\
    $(SLO)$/optjava.obj	\
    $(SLO)$/optupdt.obj	\
    $(SLO)$/optgenrl.obj	\
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
    $(SLO)$/SpellAttrib.obj  \
    $(SLO)$/SpellDialog.obj  \
    $(SLO)$/numpages.obj \
    $(SLO)$/paragrph.obj \
    $(SLO)$/tabstpge.obj \
    $(SLO)$/textattr.obj  	\
    $(SLO)$/align.obj \
    $(SLO)$/backgrnd.obj \
    $(SLO)$/border.obj \
    $(SLO)$/chardlg.obj \
    $(SLO)$/numfmt.obj \
    $(SLO)$/page.obj \
    $(SLO)$/postdlg.obj \
    $(SLO)$/sdrcelldlg.obj \
    $(SLO)$/splitcelldlg.obj \
    $(SLO)$/newtabledlg.obj\
    $(SLO)$/grfpage.obj \
    $(SLO)$/cfgchart.obj \
    $(SLO)$/optchart.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=$(SLO)$/winpluginlib.obj
LIB2OBJFILES+=$(SLO)$/winpluginlib.obj
.ENDIF


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"libcui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/svxdlg.obj : $(INCCOM)$/cuilib.hxx
