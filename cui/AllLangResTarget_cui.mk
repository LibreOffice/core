# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,cui))

$(eval $(call gb_AllLangResTarget_set_reslocation,cui,svx))

$(eval $(call gb_AllLangResTarget_add_srs,cui,\
    cui/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,cui/res))

$(eval $(call gb_SrsTarget_set_include,cui/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/cui/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,cui/res,\
    cui/source/customize/acccfg.src \
    cui/source/customize/cfg.src \
    cui/source/customize/eventdlg.src \
    cui/source/customize/macropg.src \
    cui/source/dialogs/colorpicker.src \
    cui/source/dialogs/commonlingui.src \
    cui/source/dialogs/cuires.src \
    cui/source/dialogs/fmsearch.src \
    cui/source/dialogs/gallery.src \
    cui/source/dialogs/grfflt.src \
    cui/source/dialogs/hangulhanjadlg.src \
    cui/source/dialogs/hlmarkwn.src \
    cui/source/dialogs/hyperdlg.src \
    cui/source/dialogs/iconcdlg.src \
    cui/source/dialogs/multipat.src \
    cui/source/dialogs/passwdomdlg.src \
    cui/source/dialogs/scriptdlg.src \
    cui/source/dialogs/sdrcelldlg.src \
    cui/source/dialogs/showcols.src \
    cui/source/dialogs/srchxtra.src \
    cui/source/dialogs/svuidlg.src \
    cui/source/dialogs/tbxform.src \
    cui/source/options/certpath.src \
    cui/source/options/connpooloptions.src \
    cui/source/options/dbregister.src \
    cui/source/options/doclinkdialog.src \
    cui/source/options/internationaloptions.src \
    cui/source/options/optasian.src \
    cui/source/options/optchart.src \
    cui/source/options/optcolor.src \
    cui/source/options/optdict.src \
    cui/source/options/optfltr.src \
    cui/source/options/optgdlg.src \
    cui/source/options/optinet2.src \
    cui/source/options/optjava.src \
    cui/source/options/optlingu.src \
    cui/source/options/optpath.src \
    cui/source/options/treeopt.src \
    cui/source/tabpages/align.src \
    cui/source/tabpages/autocdlg.src \
    cui/source/tabpages/bbdlg.src \
    cui/source/tabpages/border.src \
    cui/source/tabpages/dstribut.src \
    cui/source/tabpages/frmdirlbox.src \
    cui/source/tabpages/grfpage.src \
    cui/source/tabpages/labdlg.src \
    cui/source/tabpages/macroass.src \
    cui/source/tabpages/page.src \
    cui/source/tabpages/paragrph.src \
    cui/source/tabpages/strings.src \
))

# vim: set noet sw=4 ts=4:
