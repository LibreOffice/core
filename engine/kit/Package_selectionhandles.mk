# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,kit_selectionhandles,$(SRCDIR)/kit/source/gtk))

$(eval $(call gb_Package_add_files,kit_selectionhandles,$(LIBO_SHARE_FOLDER)/kit,\
	handle_image_start.png \
	handle_image_middle.png \
	handle_image_end.png \
))

# vim: set noet sw=4 ts=4:
