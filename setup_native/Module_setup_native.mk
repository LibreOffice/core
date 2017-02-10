# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,setup_native))

$(eval $(call gb_Module_add_targets,setup_native,\
	$(if $(filter LINUX SOLARIS,$(OS)),Library_getuid) \
	$(if $(filter MACOSX,$(OS)),CustomTarget_mac) \
	CustomTarget_spell \
	$(if $(filter WNT,$(OS)),Package_misc) \
	Package_packinfo \
	$(if $(filter LINUX SOLARIS,$(OS)), \
		CustomTarget_scripts \
		$(if $(ENABLE_ONLINE_UPDATE),Package_scripts) \
	) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,setup_native,\
	Library_instooofiltmsi \
	Library_qslnkmsi \
	Library_reg4allmsdoc \
	$(if $(DISABLE_ACTIVEX),,Library_regactivex) \
	Library_sdqsmsi \
	Library_sellangmsi \
	Library_shlxtmsi \
	Library_sn_tools \
	StaticLibrary_quickstarter \
	StaticLibrary_seterror \
))

endif

# vim: set noet sw=4 ts=4:
