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
	CustomTarget_spell \
	Package_packinfo \
	Package_spell \
	Package_misc \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,setup_native,\
	CustomTarget_mac \
	Package_ulf \
))
endif

ifeq ($(OS),WNT)
ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,setup_native,\
        Library_instooofiltmsi \
	Library_jfregca \
	Library_qslnkmsi \
	Library_reg4allmsdoc \
	Library_regactivex \
	Library_regpatchactivex \
	Library_sdqsmsi \
	Library_sellangmsi \
	Library_shlxtmsi \
	Library_sn_tools \
	StaticLibrary_quickstarter \
	StaticLibrary_seterror \
))
else
$(eval $(call gb_Module_add_targets,setup_native,\
	CustomTarget_wintools \
	Package_wintools \
))
endif

else
$(eval $(call gb_Module_add_targets,setup_native,\
	CustomTarget_scripts \
	Package_scripts \
	Package_scripts_generated \
))
endif

# vim: set noet sw=4 ts=4:
