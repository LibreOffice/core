# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_cfgsrvnolang,$(SRCDIR)/extras/source/misc_config))

$(eval $(call gb_Package_add_files_with_dir,extras_cfgsrvnolang,$(LIBO_SHARE_FOLDER)/config,\
	wizard/form/styles/beige.css \
	wizard/form/styles/bgr.css \
	wizard/form/styles/dark.css \
	wizard/form/styles/grey.css \
	wizard/form/styles/ibg.css \
	wizard/form/styles/ice.css \
	wizard/form/styles/orange.css \
	wizard/form/styles/red.css \
	wizard/form/styles/violet.css \
	wizard/form/styles/water.css \
))

# vim: set noet sw=4 ts=4:
