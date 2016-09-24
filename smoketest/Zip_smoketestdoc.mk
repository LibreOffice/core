# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

SMOKETESTDOC_DIR:=$(SRCDIR)/smoketest/data

$(eval $(call gb_Zip_Zip,smoketestdoc,$(SRCDIR)/smoketest/data))

$(eval $(call gb_Zip_add_commandoptions,smoketestdoc,-D))

$(eval $(call gb_Zip_add_file,smoketestdoc,mimetype))

$(eval $(call gb_Zip_add_file,smoketestdoc,content.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,meta.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,settings.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,styles.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,META-INF/manifest.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/script-lc.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/script-lb.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Events.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Global.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_10er.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_DB.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Basic/Standard/Test_Ext.xml))

$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/dialog-lc.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/Standard/dialog-lb.xml))
$(eval $(call gb_Zip_add_file,smoketestdoc,Dialogs/Standard/OptionsDlg.xml))

# vim: set noet sw=4 ts=4:
