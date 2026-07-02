# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,desktop/unitedlibs-link-map))

$(gb_CustomTarget_workdir)/desktop/united.map : $(call gb_Executable_get_target_for_build,unitedlibs-link-map-generator)
	$(call gb_Executable_get_command,unitedlibs-link-map-generator) > $(gb_CustomTarget_workdir)/desktop/united.map

# vim: set noet sw=4 ts=4:
