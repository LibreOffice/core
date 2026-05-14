# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,setup_native))

$(eval $(call gb_Module_add_targets,setup_native,\
	$(if $(filter MACOSX,$(OS)),CustomTarget_mac) \
	CustomTarget_spell \
	Package_packinfo \
	$(if $(filter LINUX SOLARIS,$(OS)), \
		CustomTarget_scripts \
		$(if $(ENABLE_ONLINE_UPDATE),Package_scripts) \
	) \
))

# vim: set noet sw=4 ts=4:
