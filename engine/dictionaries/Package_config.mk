# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,dictionaries_config,$(gb_CustomTarget_workdir)/dictionaries/registry))

$(eval $(call gb_Package_add_files,dictionaries_config,$(LIBO_SHARE_FOLDER)/registry,\
	dictionaries.xcd \
))

$(eval $(call gb_Package_use_custom_target,dictionaries_config,dictionaries/registry))

# vim: set noet sw=4 ts=4:
