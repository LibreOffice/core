# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,zxcvbn-c))

$(eval $(call gb_Library_set_warnings_disabled,zxcvbn-c))

$(eval $(call gb_ExternalProject_register_targets,zxcvbn-c,\
	generate-dictionary-nodes \
))

$(call gb_ExternalProject_get_state_target,zxcvbn-c,generate-dictionary-nodes): \
		$(call gb_Executable_get_runtime_dependencies,gcc-wrapper) \
		$(call gb_Executable_get_runtime_dependencies,g++-wrapper) \
		$(if $(CROSS_COMPILING),$(call gb_ExternalProject_get_target_for_build,zxcvbn-c))
	$(call gb_Trace_StartRange,zxcvbn-c,EXTERNAL)
	$(call gb_ExternalProject_run,generate-dictionary-nodes,\
		$(if $(CROSS_COMPILING),\
			cp $(WORKDIR_FOR_BUILD)/UnpackedTarball/zxcvbn-c/dict-src.h \
				$(WORKDIR)/UnpackedTarball/zxcvbn-c/dict-src.h \
		,\
			$(MAKE) dict-src.h \
				AR="$(gb_AR)" \
				CFLAGS="$(gb_CFLAGS)" \
				CXXFLAGS="$(gb_CXXFLAGS)" \
				$(if $(filter MSC,$(COM)),$(gb_AUTOCONF_WRAPPERS) )\
		)\
	)
	$(call gb_Trace_EndRange,zxcvbn-c,EXTERNAL)

# vim: set noet sw=4 ts=4:
