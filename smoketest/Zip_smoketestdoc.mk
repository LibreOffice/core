# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

SMOKETESTDOC_DIR:=$(SRCDIR)/smoketest/data

$(eval $(call gb_Zip_Zip,smoketestdoc,$(WORKDIR)/Zip/smoketestdoc))

$(eval $(call gb_Zip_add_commandoptions,smoketestdoc,-D))

$(eval $(call gb_Zip_add_file,smoketestdoc,mimetype))

$(eval $(call gb_Zip_add_file,smoketestdoc,content.xml,$(SMOKETESTDOC_DIR)/content.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,meta.xml,$(SMOKETESTDOC_DIR)/meta.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,settings.xml,$(SMOKETESTDOC_DIR)/settings.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,styles.xml,$(SMOKETESTDOC_DIR)/styles.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,META-INF/manifest.xml,$(SMOKETESTDOC_DIR)/manifest.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/script-lc.xml,$(SMOKETESTDOC_DIR)/script-lc.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/script-lb.xml,$(SMOKETESTDOC_DIR)/script-lb.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Events.xml,$(SMOKETESTDOC_DIR)/Events.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Global.xml,$(SMOKETESTDOC_DIR)/Global.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_10er.xml,$(SMOKETESTDOC_DIR)/Test_10er.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_DB.xml,$(SMOKETESTDOC_DIR)/Test_DB.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_Ext.xml,$(SMOKETESTDOC_DIR)/Test_Ext.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/dialog-lc.xml,$(SMOKETESTDOC_DIR)/dialog-lc.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/Standard/dialog-lb.xml,$(SMOKETESTDOC_DIR)/dialog-lb.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/Standard/OptionsDlg.xml,$(SMOKETESTDOC_DIR)/OptionsDlg.xml))

$(WORKDIR)/Zip/smoketestdoc/mimetype :
	mkdir -p $(dir $@)
	printf application/vnd.sun.xml.writer > $@

$(call gb_Zip_get_final_target,smoketestdoc) : $(OUTDIR)/bin/smoketestdoc.sxw
$(call gb_Zip_get_clean_target,smoketestdoc) : Clean_smoketestdoc_sxw

$(OUTDIR)/bin/smoketestdoc.sxw : $(call gb_Zip_get_target,smoketestdoc) | $(OUTDIR)/bin/.dir
	$(call gb_Deliver_deliver,$<,$@)

.PHONY : Clean_smoketestdoc_sxw
Clean_smoketestdoc_sxw :
	rm -f $(OUTDIR)/bin/smoketestdoc.sxw

# vim: set noet sw=4 ts=4:
