# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,IAccessible2))

$(eval $(call gb_ExternalProject_register_targets,IAccessible2,\
	build \
))

# run IAccessible2's 'concatidl.sh' script that creates the merged .idl file,
# 'ia2_api_all.idl'
$(call gb_ExternalProject_get_state_target,IAccessible2,build) :
	$(call gb_Trace_StartRange,IAccessible2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		cd $(gb_UnpackedTarball_workdir)/IAccessible2\
		&& ./concatidl.sh\
	)
	$(call gb_Trace_EndRange,IAccessible2,EXTERNAL)

# vim: set noet sw=4 ts=4:
