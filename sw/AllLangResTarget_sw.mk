# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sw))

$(eval $(call gb_AllLangResTarget_add_srs,sw,\
    sw/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sw/res))

$(eval $(call gb_SrsTarget_use_packages,sw/res,\
	editeng_inc \
	sfx2_inc \
	svl_inc \
	svtools_inc \
	svx_globlmn_hrc \
	svx_inc \
	tools_inc \
))

$(eval $(call gb_SrsTarget_set_include,sw/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/core/undo \
    -I$(SRCDIR)/sw/source/core/unocore \
    -I$(SRCDIR)/sw/source/ui/chrdlg \
    -I$(SRCDIR)/sw/source/ui/config \
    -I$(SRCDIR)/sw/source/ui/dbui \
    -I$(SRCDIR)/sw/source/ui/dialog \
    -I$(SRCDIR)/sw/source/ui/docvw \
    -I$(SRCDIR)/sw/source/ui/envelp \
    -I$(SRCDIR)/sw/source/ui/fldui \
    -I$(SRCDIR)/sw/source/ui/fmtui \
    -I$(SRCDIR)/sw/source/ui/frmdlg \
    -I$(SRCDIR)/sw/source/ui/globdoc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/source/ui/index \
    -I$(SRCDIR)/sw/source/ui/lingu \
    -I$(SRCDIR)/sw/source/ui/misc \
    -I$(SRCDIR)/sw/source/ui/ribbar \
    -I$(SRCDIR)/sw/source/ui/shells \
    -I$(SRCDIR)/sw/source/ui/smartmenu \
    -I$(SRCDIR)/sw/source/ui/table \
    -I$(SRCDIR)/sw/source/ui/uiview \
    -I$(SRCDIR)/sw/source/ui/utlui \
    -I$(SRCDIR)/sw/source/ui/web \
    -I$(SRCDIR)/sw/source/ui/wrtsh \
))

$(eval $(call gb_SrsTarget_add_files,sw/res,\
    sw/source/core/layout/pagefrm.src \
    sw/source/core/undo/undo.src \
    sw/source/core/unocore/unocore.src \
    sw/source/ui/app/app.src \
    sw/source/ui/app/error.src \
    sw/source/ui/app/mn.src \
    sw/source/ui/chrdlg/ccoll.src \
    sw/source/ui/chrdlg/chardlg.src \
    sw/source/ui/chrdlg/chrdlg.src \
    sw/source/ui/chrdlg/drpcps.src \
    sw/source/ui/chrdlg/numpara.src \
    sw/source/ui/chrdlg/paradlg.src \
    sw/source/ui/config/mailconfigpage.src \
    sw/source/ui/config/optcomp.src \
    sw/source/ui/config/optdlg.src \
    sw/source/ui/config/optload.src \
    sw/source/ui/config/redlopt.src \
    sw/source/ui/dbui/addresslistdialog.src \
    sw/source/ui/dbui/createaddresslistdialog.src \
    sw/source/ui/dbui/customizeaddresslistdialog.src \
    sw/source/ui/dbui/dbinsdlg.src \
    sw/source/ui/dbui/dbtablepreviewdialog.src \
    sw/source/ui/dbui/dbui.src \
    sw/source/ui/dbui/mailmergechildwindow.src \
    sw/source/ui/dbui/mailmergewizard.src \
    sw/source/ui/dbui/mmaddressblockpage.src \
    sw/source/ui/dbui/mmdocselectpage.src \
    sw/source/ui/dbui/mmgreetingspage.src \
    sw/source/ui/dbui/mmlayoutpage.src \
    sw/source/ui/dbui/mmmergepage.src \
    sw/source/ui/dbui/mmoutputpage.src \
    sw/source/ui/dbui/mmoutputtypepage.src \
    sw/source/ui/dbui/mmpreparemergepage.src \
    sw/source/ui/dbui/selectdbtabledialog.src \
    sw/source/ui/dialog/abstract.src \
    sw/source/ui/dialog/ascfldlg.src \
    sw/source/ui/dialog/dialog.src \
    sw/source/ui/dialog/docstdlg.src \
    sw/source/ui/dialog/regionsw.src \
    sw/source/ui/dochdl/dochdl.src \
    sw/source/ui/dochdl/selglos.src \
    sw/source/ui/docvw/annotation.src \
    sw/source/ui/docvw/access.src \
    sw/source/ui/docvw/docvw.src \
    sw/source/ui/envelp/envelp.src \
    sw/source/ui/envelp/envfmt.src \
    sw/source/ui/envelp/envlop.src \
    sw/source/ui/envelp/envprt.src \
    sw/source/ui/envelp/label.src \
    sw/source/ui/envelp/labfmt.src \
    sw/source/ui/envelp/labprt.src \
    sw/source/ui/envelp/mailmrge.src \
    sw/source/ui/fldui/DropDownFieldDialog.src \
    sw/source/ui/fldui/changedb.src \
    sw/source/ui/fldui/flddb.src \
    sw/source/ui/fldui/flddinf.src \
    sw/source/ui/fldui/flddok.src \
    sw/source/ui/fldui/fldfunc.src \
    sw/source/ui/fldui/fldref.src \
    sw/source/ui/fldui/fldtdlg.src \
    sw/source/ui/fldui/fldui.src \
    sw/source/ui/fldui/fldvar.src \
    sw/source/ui/fldui/inpdlg.src \
    sw/source/ui/fldui/javaedit.src \
    sw/source/ui/fmtui/tmpdlg.src \
    sw/source/ui/frmdlg/column.src \
    sw/source/ui/frmdlg/cption.src \
    sw/source/ui/frmdlg/frmpage.src \
    sw/source/ui/frmdlg/frmui.src \
    sw/source/ui/frmdlg/wrap.src \
    sw/source/ui/globdoc/globdoc.src \
    sw/source/ui/index/cnttab.src \
    sw/source/ui/index/idxmrk.src \
    sw/source/ui/index/multmrk.src \
    sw/source/ui/lingu/olmenu.src \
    sw/source/ui/misc/autocorr.src \
    sw/source/ui/misc/bookmark.src \
    sw/source/ui/misc/docfnote.src \
    sw/source/ui/misc/glosbib.src \
    sw/source/ui/misc/glossary.src \
    sw/source/ui/misc/insfnote.src \
    sw/source/ui/misc/num.src \
    sw/source/ui/misc/numberingtypelistbox.src \
    sw/source/ui/misc/outline.src \
    sw/source/ui/misc/pgfnote.src \
    sw/source/ui/misc/pggrid.src \
    sw/source/ui/misc/redlndlg.src \
    sw/source/ui/misc/srtdlg.src \
    sw/source/ui/misc/titlepage.src \
    sw/source/ui/ribbar/inputwin.src \
    sw/source/ui/ribbar/tbxanchr.src \
    sw/source/ui/ribbar/workctrl.src \
    sw/source/ui/shells/shells.src \
    sw/source/ui/smartmenu/stmenu.src \
    sw/source/ui/table/chartins.src \
    sw/source/ui/table/mergetbl.src \
    sw/source/ui/table/rowht.src \
    sw/source/ui/table/table.src \
    sw/source/ui/table/tabledlg.src \
    sw/source/ui/uiview/pview.src \
    sw/source/ui/uiview/view.src \
    sw/source/ui/utlui/attrdesc.src \
    sw/source/ui/utlui/gloslst.src \
    sw/source/ui/utlui/initui.src \
    sw/source/ui/utlui/navipi.src \
    sw/source/ui/utlui/poolfmt.src \
    sw/source/ui/utlui/unotools.src \
    sw/source/ui/utlui/utlui.src \
    sw/source/ui/web/web.src \
    sw/source/ui/wrtsh/wrtsh.src \
))

# vim: set noet sw=4 ts=4:
