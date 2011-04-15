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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dbui))

$(eval $(call gb_AllLangResTarget_set_reslocation,dbui,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,dbui,dbaccess/dbui))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/dbui))

$(eval $(call gb_SrsTarget_set_include,dbaccess/dbui,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(WORKDIR)/inc/dbaccess \
	-I$(SRCDIR)/dbaccess/source/ui/inc \
	-I$(SRCDIR)/dbaccess/inc \
))


$(eval $(call gb_SrsTarget_add_files,dbaccess/dbui,\
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
	dbaccess/source/ui/dlg/dbadmin.src \
	dbaccess/source/ui/dlg/dbadmin2.src \
	dbaccess/source/ui/dlg/dbadminsetup.src \
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
	dbaccess/source/ui/dlg/UserAdmin.src \
	dbaccess/source/ui/dlg/UserAdminDlg.src \
	dbaccess/source/ui/imagelists/dbimagelists.src \
	dbaccess/source/ui/misc/dbumiscres.src \
	dbaccess/source/ui/misc/WizardPages.src \
	dbaccess/source/ui/querydesign/query.src \
	dbaccess/source/ui/querydesign/querydlg.src \
	dbaccess/source/ui/relationdesign/relation.src \
	dbaccess/source/ui/tabledesign/table.src \
	dbaccess/source/ui/uno/copytablewizard.src \
	dbaccess/source/ui/uno/dbinteraction.src \
))

$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/admincontrols.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/AutoControls.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/ConnectionPage.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/dbadmin.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/dbadmin2.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/dbadminsetup.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/textconnectionsettings.src) : $(WORKDIR)/inc/dbaccess/AutoControls.hrc

$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/app/app.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/browser/sbabrw.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/browser/sbagrid.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/dbadmin2.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/dlg/queryfilter.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/querydesign/query.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc
$(call gb_SrsPartTarget_get_target,dbaccess/source/ui/tabledesign/table.src) : $(WORKDIR)/inc/dbaccess/toolbox.hrc

# hack !!!
# just a temporary - globlmn.hrc about to be removed!
ifeq ($(strip $(WITH_LANG)),)
$(WORKDIR)/inc/dbaccess/AutoControls.hrc : $(SRCDIR)/dbaccess/source/ui/dlg/AutoControls_tmpl.hrc
	echo copying $@
	-mkdir -p $(WORKDIR)/inc/dbaccess
	cp $(SRCDIR)/dbaccess/source/ui/dlg/AutoControls_tmpl.hrc $(WORKDIR)/inc/dbaccess/AutoControls.hrc
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk

$(WORKDIR)/inc/dbaccess/toolbox.hrc : $(SRCDIR)/dbaccess/source/ui/inc/toolbox_tmpl.hrc
	echo copying $@
	-mkdir -p $(WORKDIR)/inc/dbaccess
	cp $(SRCDIR)/dbaccess/source/ui/inc/toolbox_tmpl.hrc $(WORKDIR)/inc/dbaccess/toolbox.hrc
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk
else
-include $(WORKDIR)/inc/dbaccess/lastrun.mk
ifneq ($(gb_lastrun_globlmn),MERGED)
.PHONY : $(WORKDIR)/inc/dbaccess/toolbox.hrc $(WORKDIR)/inc/dbaccess/AutoControls.hrc
endif

$(WORKDIR)/inc/dbaccess/AutoControls.hrc : $(SRCDIR)/dbaccess/source/ui/dlg/AutoControls_tmpl.hrc $(gb_SrsPartMergeTarget_SDFLOCATION)/svx/inc/localize.sdf
	echo merging $@
	-mkdir -p $(WORKDIR)/inc/dbaccess
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk
	echo gb_lastrun_globlmn:=MERGED > $(WORKDIR)/inc/dbaccess/lastrun.mk
	$(call gb_Helper_abbreviate_dirs_native, \
		$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-p svx \
		 -i $< -o $@ -m $(gb_SrsPartMergeTarget_SDFLOCATION)/dbaccess/inc/localize.sdf -l all)

$(WORKDIR)/inc/dbaccess/toolbox.hrc : $(SRCDIR)/dbaccess/source/ui/inc/toolbox.hrc $(gb_SrsPartMergeTarget_SDFLOCATION)/svx/inc/localize.sdf
	echo merging $@
	-mkdir -p $(WORKDIR)/inc/dbaccess
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk
	echo gb_lastrun_globlmn:=MERGED > $(WORKDIR)/inc/dbaccess/lastrun.mk
	$(call gb_Helper_abbreviate_dirs_native, \
		$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-p svx \
		 -i $< -o $@ -m $(gb_SrsPartMergeTarget_SDFLOCATION)/dbaccess/inc/localize.sdf -l all)

endif

.PHONY : $(WORKDIR)/inc/dbaccess/toolbox.hrc_clean $(WORKDIR)/inc/dbaccess/AutoControls.hrc_clean
$(WORKDIR)/inc/dbaccess/AutoControls.hrc_clean :
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk \
		$(WORKDIR)/inc/dbaccess/AutoControls.hrc

$(WORKDIR)/inc/dbaccess/toolbox.hrc_clean :
	rm -f $(WORKDIR)/inc/dbaccess/lastrun.mk \
		$(WORKDIR)/inc/dbaccess/toolbox.hrc

# vim: set noet sw=4 ts=4:
