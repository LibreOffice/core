# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libtommath))

$(eval $(call gb_ExternalProject_register_targets,libtommath,\
	build \
))

$(eval $(call gb_ExternalProject_use_nmake,libtommath,build))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,libtommath,build):
	$(call gb_Trace_StartRange,libtommath,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		nmake -nologo -f makefile.msvc \
	)
	$(call gb_Trace_EndRange,libtommath,EXTERNAL)
else
$(call gb_ExternalProject_get_state_target,libtommath,build) :
	$(call gb_Trace_StartRange,libtommath,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS \
		&& export CFLAGS=" \
			-fPIC \
		" \
		&& $(MAKE) $(if $(verbose),V=1) NO_ADDTL_WARNINGS=1 \
	)
	$(call gb_Trace_EndRange,libtommath,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
