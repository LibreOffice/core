# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizard,$(gb_CustomTarget_workdir)/extras/source/templates/wizard))

$(eval $(call gb_Package_add_files_with_dir,extras_tplwizard,$(LIBO_SHARE_FOLDER)/template/common/wizard,\
    styles/blackberry.ots \
    styles/black_white.ots \
    styles/default.ots \
    styles/diner.ots \
    styles/fall.ots \
    styles/glacier.ots \
    styles/green_grapes.ots \
    styles/jeans.ots \
    styles/marine.ots \
    styles/millennium.ots \
    styles/nature.ots \
    styles/neon.ots \
    styles/night.ots \
    styles/nostalgic.ots \
    styles/pastell.ots \
    styles/pool.ots \
    styles/pumpkin.ots \
    styles/xos.ots \
))

# vim: set noet sw=4 ts=4:
