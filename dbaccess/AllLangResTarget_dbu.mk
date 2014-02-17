# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dbu))

$(eval $(call gb_AllLangResTarget_set_reslocation,dbu,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,dbu,\
    dbaccess/dbu \
))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/dbu))

$(eval $(call gb_SrsTarget_use_srstargets,dbaccess/dbu,\
	svx/res \
))

$(eval $(call gb_SrsTarget_set_include,dbaccess/dbu,\
    $$(INCLUDE) \
	-I$(call gb_SrsTemplateTarget_get_include_dir,dbaccess) \
    -I$(SRCDIR)/dbaccess/inc \
    -I$(SRCDIR)/dbaccess/source/ui/inc \
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
))

$(eval $(call gb_SrsTarget_add_files,dbaccess/dbu,\
    dbaccess/source/ui/app/app.src \
    dbaccess/source/ui/browser/sbabrw.src \
    dbaccess/source/ui/browser/sbagrid.src \
    dbaccess/source/ui/control/TableGrantCtrl.src \
    dbaccess/source/ui/control/tabletree.src \
    dbaccess/source/ui/control/undosqledit.src \
    dbaccess/source/ui/dlg/admincontrols.src \
    dbaccess/source/ui/dlg/AutoControls.src \
    dbaccess/source/ui/dlg/CollectionView.src \
    dbaccess/source/ui/dlg/ConnectionPage.src \
    dbaccess/source/ui/dlg/dbadmin2.src \
    dbaccess/source/ui/dlg/dbadminsetup.src \
    dbaccess/source/ui/dlg/dbadmin.src \
    dbaccess/source/ui/dlg/directsql.src \
    dbaccess/source/ui/dlg/indexdialog.src \
    dbaccess/source/ui/dlg/paramdialog.src \
    dbaccess/source/ui/dlg/RelationDlg.src \
    dbaccess/source/ui/dlg/sqlmessage.src \
    dbaccess/source/ui/dlg/textconnectionsettings.src \
    dbaccess/source/ui/dlg/UserAdmin.src \
    dbaccess/source/ui/misc/dbumiscres.src \
    dbaccess/source/ui/misc/WizardPages.src \
    dbaccess/source/ui/querydesign/querydlg.src \
    dbaccess/source/ui/querydesign/query.src \
    dbaccess/source/ui/relationdesign/relation.src \
    dbaccess/source/ui/tabledesign/table.src \
    dbaccess/source/ui/uno/copytablewizard.src \
    dbaccess/source/ui/uno/dbinteraction.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalizable_files,dbaccess/dbu,\
    dbaccess/source/ui/imagelists/dbimagelists.src \
))

$(eval $(call gb_SrsTarget_add_templates,dbaccess/dbu,\
    dbaccess/source/ui/dlg/AutoControls_tmpl.hrc \
    dbaccess/source/ui/inc/toolbox_tmpl.hrc \
))

# vim: set noet sw=4 ts=4:
