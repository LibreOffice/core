# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dbu))

$(eval $(call gb_AllLangResTarget_set_reslocation,dbu,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,dbu,\
    dbaccess/dbu \
))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/dbu))

$(eval $(call gb_SrsTarget_set_include,dbaccess/dbu,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc/dbaccess \
    -I$(realpath $(SRCDIR)/dbaccess/inc) \
    -I$(realpath $(SRCDIR)/dbaccess/source/ui/inc) \
))

$(eval $(call gb_SrsTarget_add_files,dbaccess/dbu,\
    dbaccess/source/ui/app/app.src \
    dbaccess/source/ui/browser/sbabrw.src \
    dbaccess/source/ui/browser/sbagrid.src \
    dbaccess/source/ui/control/TableGrantCtrl.src \
    dbaccess/source/ui/control/tabletree.src \
    dbaccess/source/ui/control/undosqledit.src \
    dbaccess/source/ui/dlg/AdabasStat.src \
    dbaccess/source/ui/dlg/admincontrols.src \
    dbaccess/source/ui/dlg/adtabdlg.src \
    dbaccess/source/ui/dlg/advancedsettings.src \
    dbaccess/source/ui/dlg/AutoControls.src \
    dbaccess/source/ui/dlg/CollectionView.src \
    dbaccess/source/ui/dlg/ConnectionPage.src \
    dbaccess/source/ui/dlg/dbadmin2.src \
    dbaccess/source/ui/dlg/dbadminsetup.src \
    dbaccess/source/ui/dlg/dbadmin.src \
    dbaccess/source/ui/dlg/dbfindex.src \
    dbaccess/source/ui/dlg/directsql.src \
    dbaccess/source/ui/dlg/dlgattr.src \
    dbaccess/source/ui/dlg/dlgsave.src \
    dbaccess/source/ui/dlg/dlgsize.src \
    dbaccess/source/ui/dlg/dsselect.src \
    dbaccess/source/ui/dlg/indexdialog.src \
    dbaccess/source/ui/dlg/paramdialog.src \
    dbaccess/source/ui/dlg/queryfilter.src \
    dbaccess/source/ui/dlg/queryorder.src \
    dbaccess/source/ui/dlg/RelationDlg.src \
    dbaccess/source/ui/dlg/sqlmessage.src \
    dbaccess/source/ui/dlg/textconnectionsettings.src \
    dbaccess/source/ui/dlg/UserAdminDlg.src \
    dbaccess/source/ui/dlg/UserAdmin.src \
    dbaccess/source/ui/imagelists/dbimagelists.src \
    dbaccess/source/ui/misc/dbumiscres.src \
    dbaccess/source/ui/misc/WizardPages.src \
    dbaccess/source/ui/querydesign/querydlg.src \
    dbaccess/source/ui/querydesign/query.src \
    dbaccess/source/ui/relationdesign/relation.src \
    dbaccess/source/ui/tabledesign/table.src \
    dbaccess/source/ui/uno/copytablewizard.src \
    dbaccess/source/ui/uno/dbinteraction.src \
))

$(eval $(call gb_SrsTarget_add_templates,dbaccess/dbu,\
    dbaccess/source/ui/dlg/AutoControls_tmpl.hrc \
    dbaccess/source/ui/inc/toolbox_tmpl.hrc \
))

$(call gb_SrsTarget_get_target,dbaccess/dbu) :| $(OUTDIR)/inc/svx/globlmn.hrc

# vim: set noet sw=4 ts=4:
