# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# defining extra package for that is a little hacky - maybe use PackageSet instead?
$(eval $(call gb_Package_Package,extras_gallsystemstr,$(gb_CustomTarget_workdir)/extras/gallsysstr))
$(eval $(call gb_Package_use_customtarget,extras_gallsystemstr,extras/gallsysstr))

$(eval $(call gb_Package_add_files,extras_gallsystemstr,$(LIBO_SHARE_FOLDER)/gallery,\
    $(addsuffix .str,$(filter-out fontwork symbolshapes,$(system_galleries))) \
))

# vim: set noet sw=4 ts=4:
