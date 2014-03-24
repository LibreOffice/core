# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sw))

$(eval $(call gb_AllLangResTarget_add_srs,sw,\
    sw/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sw/res))

$(eval $(call gb_SrsTarget_use_srstargets,sw/res,\
	svx/res \
))

$(eval $(call gb_SrsTarget_set_include,sw/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/core/uibase/inc \
    -I$(SRCDIR)/sw/source/core/uibase/docvw \
    -I$(SRCDIR)/sw/source/core/uibase/envelp \
    -I$(SRCDIR)/sw/source/core/uibase/index \
    -I$(SRCDIR)/sw/source/core/uibase/sidebar \
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
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
    -I$(call gb_SrsTemplateTarget_get_include_dir,sw) \
))

$(eval $(call gb_SrsTarget_add_files,sw/res,\
    sw/source/core/layout/pagefrm.src \
    sw/source/core/undo/undo.src \
    sw/source/core/unocore/unocore.src \
    sw/source/core/uibase/dialog/regionsw.src \
    sw/source/core/uibase/docvw/docvw.src \
    sw/source/core/uibase/lingu/olmenu.src \
    sw/source/core/uibase/misc/redlndlg.src \
    sw/source/core/uibase/ribbar/inputwin.src \
    sw/source/core/uibase/ribbar/workctrl.src \
    sw/source/core/uibase/smartmenu/stmenu.src \
    sw/source/core/uibase/uiview/view.src \
    sw/source/core/uibase/utlui/attrdesc.src \
    sw/source/core/uibase/utlui/navipi.src \
    sw/source/core/uibase/utlui/initui.src \
    sw/source/core/uibase/wrtsh/wrtsh.src \
    sw/source/core/uibase/utlui/unotools.src \
    sw/source/core/uibase/dbui/mailmergechildwindow.src \
    sw/source/ui/app/app.src \
    sw/source/ui/app/error.src \
    sw/source/ui/app/mn.src \
    sw/source/ui/chrdlg/chrdlg.src \
    sw/source/ui/config/optdlg.src \
    sw/source/ui/config/optload.src \
    sw/source/ui/dbui/createaddresslistdialog.src \
    sw/source/ui/dbui/dbtablepreviewdialog.src \
    sw/source/ui/dbui/dbui.src \
    sw/source/ui/dbui/mailmergewizard.src \
    sw/source/ui/dbui/mmaddressblockpage.src \
    sw/source/ui/dbui/mmgreetingspage.src \
    sw/source/ui/dbui/mmmergepage.src \
    sw/source/ui/dbui/mmoutputpage.src \
    sw/source/ui/dbui/selectdbtabledialog.src \
    sw/source/ui/dialog/dialog.src \
    sw/source/ui/dochdl/dochdl.src \
    sw/source/ui/docvw/annotation.src \
    sw/source/ui/docvw/access.src \
    sw/source/ui/envelp/envelp.src \
    sw/source/ui/envelp/label.src \
    sw/source/ui/envelp/labfmt.src \
    sw/source/ui/envelp/mailmrge.src \
    sw/source/ui/fldui/fldui.src \
    sw/source/ui/frmdlg/frmui.src \
    sw/source/ui/globdoc/globdoc.src \
    sw/source/ui/index/cnttab.src \
    sw/source/ui/index/idxmrk.src \
    sw/source/ui/misc/glossary.src \
    sw/source/ui/misc/numberingtypelistbox.src \
    sw/source/ui/misc/srtdlg.src \
    sw/source/ui/misc/swruler.src \
    sw/source/ui/ribbar/tbxanchr.src \
    sw/source/ui/shells/shells.src \
    sw/source/ui/sidebar/PagePropertyPanel.src \
    sw/source/ui/table/table.src \
    sw/source/ui/table/tabledlg.src \
    sw/source/ui/utlui/poolfmt.src \
    sw/source/ui/utlui/utlui.src \
    sw/source/ui/web/web.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalizable_files,sw/res,\
    sw/source/ui/envelp/envprt.src \
))

$(eval $(call gb_SrsTarget_add_templates,sw/res,\
    sw/source/core/uibase/inc/redline_tmpl.hrc \
))

# vim: set noet sw=4 ts=4:
