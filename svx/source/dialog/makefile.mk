#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.35 $
#
#   last change: $Author: hr $ $Date: 2004-08-02 16:39:53 $
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
PRJNAME=svx
TARGET=dialogs
VERSION=$(UPD)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

CFLAGS+=-DDG_DLL
RSCDEFS += -Dause_hack

# --- Files --------------------------------------------------------

SRS1NAME=dialogs
SRC1FILES =  \
        autocdlg.src \
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
        rubydialog.src\
        swframeposstrings.src \
        swpossizetabpage.src \
        scriptdlg.src \
        selector.src \
        spldlg.src \
        srchdlg.src \
        srchxtra.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        thesdlg.src \
        txenctab.src \
        zoom.src	\
        dlgctrl.src \
        grfflt.src \
        langbox.src \
        frmdirlbox.src \
        hangulhanjadlg.src \
        commonlingui.src \
        fontsubs.src				\
        readonlyimage.src           \
        opthtml.src					\
        optgdlg.src					\
        optmemory.src				\
        treeopt.src					\
        optfltr.src					\
        connpooloptions.src			\
        dbregister.src				\
        doclinkdialog.src			\
        internationaloptions.src	\
        macropg.src \
        eventdlg.src \
        cfg.src \
        optjava.src

SRS2NAME=drawdlgs
SRC2FILES =  \
        dlgname.src \
        sdstring.src \
        tabarea.src \
        tabline.src \
        labdlg.src \
        transfrm.src

#EXCEPTIONSFILES=\
#	$(SLO)$/connpoolconfig.obj \
#	$(SLO)$/connpoolsettings.obj \
#	$(SLO)$/sdbcdriverenum.obj	\
#	$(SLO)$/optgdlg.obj	\
#		$(SLO)$/charmap.obj		\
#		$(SLO)$/_bmpmask.obj	\
#        $(SLO)$/frmsel.obj   \
#		$(SLO)$/hltpbase.obj	\
#		$(SLO)$/hldocntp.obj	\
#		$(SLO)$/hlmarkwn.obj	\
#		$(SLO)$/opengrf.obj		\
#		$(SLO)$/hyphen.obj		\
#		$(SLO)$/rubydialog.obj 	\
#		$(SLO)$/thesdlg.obj		\
#		$(SLO)$/spldlg.obj		\
#		$(SLO)$/numpages.obj	\
#		$(SLO)$/splwrap.obj		\
#		$(SLO)$/hangulhanja.obj


EXCEPTIONSFILES=\
    $(SLO)$/connpoolconfig.obj \
    $(SLO)$/connpoolsettings.obj \
    $(SLO)$/sdbcdriverenum.obj	\
    $(SLO)$/optgdlg.obj	\
    $(SLO)$/charmap.obj		\
    $(SLO)$/_bmpmask.obj	\
    $(SLO)$/frmsel.obj   \
    $(SLO)$/opengrf.obj		\
    $(SLO)$/hltpbase.obj	\
    $(SLO)$/hldocntp.obj	\
    $(SLO)$/hlmarkwn.obj	\
    $(SLO)$/hyphen.obj		\
    $(SLO)$/rubydialog.obj 	\
    $(SLO)$/thesdlg.obj		\
    $(SLO)$/spldlg.obj		\
    $(SLO)$/numpages.obj	\
    $(SLO)$/splwrap.obj		\
    $(SLO)$/hangulhanja.obj \
    $(SLO)$/hangulhanjadlg.obj \
    $(SLO)$/optasian.obj		\
    $(SLO)$/optaccessibility.obj	\
    $(SLO)$/optsave.obj		\
    $(SLO)$/impgrf.obj      \
    $(SLO)$/optpath.obj	\
    $(SLO)$/multifil.obj	\
    $(SLO)$/optdict.obj	\
    $(SLO)$/cuigaldlg.obj \
    $(SLO)$/dbregister.obj				\
    $(SLO)$/doclinkdialog.obj				\
    $(SLO)$/dbregisterednamesconfig.obj \
    $(SLO)$/scriptdlg.obj \
    $(SLO)$/selector.obj \
    $(SLO)$/showcols.obj \
    $(SLO)$/svxbmpnumvalueset.obj \
    $(SLO)$/macropg.obj \
    $(SLO)$/cfg.obj \
    $(SLO)$/optjava.obj

SLOFILES=\
    $(EXCEPTIONSFILES)	\
    $(SLO)$/internationaloptions.obj	\
    $(SLO)$/optHeaderTabListbox.obj		\
    $(SLO)$/connpooloptions.obj			\
    $(SLO)$/dbregistersettings.obj		\
    $(SLO)$/fontsubs.obj				\
    $(SLO)$/optmemory.obj				\
    $(SLO)$/opthtml.obj					\
    $(SLO)$/optfltr.obj					\
    $(SLO)$/treeopt.obj \
                $(SLO)$/autocdlg.obj     \
                $(SLO)$/cuiexp.obj     \
                $(SLO)$/dlgfact.obj     \
                $(SLO)$/svxdlg.obj     \
        $(SLO)$/_contdlg.obj\
        $(SLO)$/iconcdlg.obj\
        $(SLO)$/connect.obj \
        $(SLO)$/connctrl.obj \
        $(SLO)$/contwnd.obj	\
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
        $(SLO)$/grfpage.obj \
                $(SLO)$/hdft.obj     \
                $(SLO)$/impgrf.obj   \
                $(SLO)$/labdlg.obj \
                $(SLO)$/langtab.obj \
                $(SLO)$/langbox.obj \
                $(SLO)$/linelink.obj \
                $(SLO)$/nforient.obj    \
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
                $(SLO)$/worient.obj    \
                $(SLO)$/txenctab.obj \
                $(SLO)$/txencbox.obj \
                $(SLO)$/zoom.obj	 \
                $(SLO)$/grfflt.obj	\
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
    $(SLO)$/optgenrl.obj	\
    $(SLO)$/optinet2.obj	\
    $(SLO)$/optctl.obj	\
    $(SLO)$/svxgrahicitem.obj \
    $(SLO)$/framelink.obj \
    $(SLO)$/swframeposstrings.obj\
    $(SLO)$/swframeexample.obj \
    $(SLO)$/svxbmpnumvalueset.obj	\
    $(SLO)$/svxgrahicitem.obj

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
    $(SLO)$/spldlg.obj      \
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
    $(SLO)$/checklbx.obj \
    $(SLO)$/ctrdial.obj    \
    $(SLO)$/ctredlin.obj \
    $(SLO)$/dlgctrl.obj \
    $(SLO)$/dlgctl3d.obj \
    $(SLO)$/dlgutil.obj  \
    $(SLO)$/fntctrl.obj  \
    $(SLO)$/fontwork.obj \
    $(SLO)$/hdft.obj     \
    $(SLO)$/impgrf.obj   \
    $(SLO)$/langtab.obj \
    $(SLO)$/langbox.obj \
    $(SLO)$/linelink.obj \
    $(SLO)$/nforient.obj    \
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
    $(SLO)$/worient.obj    \
    $(SLO)$/txenctab.obj \
    $(SLO)$/txencbox.obj \
    $(SLO)$/grfflt.obj \
    $(SLO)$/frmdirlbox.obj \
    $(SLO)$/commonlingui.obj \
    $(SLO)$/dialmgr.obj	\
    $(SLO)$/svxbmpnumvalueset.obj   \
    $(SLO)$/svxgrahicitem.obj \
    $(SLO)$/framelink.obj\
    $(SLO)$/swframeexample.obj \
    $(SLO)$/swframeposstrings.obj \
    $(SLO)$/eventdlg.obj \
    $(SLO)$/macropg.obj \
    $(SLO)$/cfg.obj

LIB2TARGET= $(SLB)$/cui.lib

LIB2OBJFILES= \
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
    $(SLO)$/optsave.obj \
    $(SLO)$/optasian.obj		\
    $(SLO)$/optpath.obj	\
    $(SLO)$/optcolor.obj    \
    $(SLO)$/optjsearch.obj  \
    $(SLO)$/optinet2.obj	\
    $(SLO)$/optctl.obj	\
    $(SLO)$/optjava.obj	\
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
    $(SLO)$/grfpage.obj


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    +echo \#define DLL_NAME \"libcui$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
.IF "$(USE_SHELL)"!="4nt"
    +echo \#define DLL_NAME \"cui$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE          # "$(USE_SHELL)"!="4nt"
    +echo #define DLL_NAME "cui$(UPD)$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF          # "$(USE_SHELL)"!="4nt"
.ENDIF

$(SLO)$/svxdlg.obj : $(INCCOM)$/cuilib.hxx
