# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_docs,$(SRCDIR)/odk))

$(eval $(call gb_Package_set_outdir,odk_docs,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,odk_docs,$(SDKDIRNAME),\
	docs/images/arrow-1.gif \
	docs/images/arrow-2.gif \
	docs/images/arrow-3.gif \
	docs/images/bg_table.png \
	docs/images/bg_table2.png \
	docs/images/bg_table3.png \
	docs/images/bluball.gif \
	docs/images/nada.gif \
	docs/images/nav_down.png \
	docs/images/nav_home.png \
	docs/images/nav_left.png \
	docs/images/nav_right.png \
	docs/images/nav_up.png \
	docs/images/odk-footer-logo.gif \
	docs/images/ooo-main-app_32.png \
	docs/images/sdk_head-1.png \
	docs/images/sdk_head-2.png \
	docs/images/sdk_line-1.gif \
	docs/images/sdk_line-2.gif \
	docs/sdk_styles.css \
))

# vim: set noet sw=4 ts=4:
