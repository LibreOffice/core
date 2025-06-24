# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,y-crdt))

$(eval $(call gb_ExternalProject_register_targets,y-crdt,\
	build \
))

$(call gb_ExternalProject_get_state_target,y-crdt,build):
	$(call gb_Trace_StartRange,y-crdt,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		CARGO_HOME=$(TARFILE_LOCATION)/cargo cargo build --offline --locked -p yffi \
	)
	$(call gb_Trace_EndRange,y-crdt,EXTERNAL)

# vim: set noet sw=4 ts=4:
