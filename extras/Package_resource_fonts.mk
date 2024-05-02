# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,resource_fonts,$(gb_CustomTarget_workdir)/extras/fonts))

$(eval $(call gb_Package_use_customtarget,resource_fonts,extras/fonts))

$(eval $(call gb_Package_add_file,resource_fonts,$(LIBO_SHARE_RESOURCE_FOLDER)/common/fonts/opens___.ttf,opens___.ttf))

# vim: set noet sw=4 ts=4:
