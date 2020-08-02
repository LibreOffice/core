# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,toolbarmode_images,$(SRCDIR)/extras/source/toolbarmode))

$(eval $(call gb_Package_add_files_with_dir,toolbarmode_images,$(LIBO_SHARE_FOLDER)/toolbarmode,\
    default.png \
    single.png \
    sidebar.png \
    notebookbar.png \
    notebookbar_compact.png \
    notebookbar_groupedbar_compact.png \
    notebookbar_groupedbar_full.png \
    notebookbar_single.png \
    notebookbar_groups.png \
))

# vim: set noet sw=4 ts=4:
