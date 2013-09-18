# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,odk))

$(eval $(call gb_Module_add_targets,odk,\
	$(if $(filter WNT,$(OS)),Package_cli) \
	$(if $(DOXYGEN),\
		CustomTarget_doxygen \
		GeneratedPackage_odk_doxygen \
	) \
	CustomTarget_html \
	CustomTarget_settings \
	Executable_unoapploader \
	Package_config \
	Package_docs \
	Package_html \
	Package_examples \
	Package_lib \
	Package_odk_headers \
	Package_odk_headers_generated \
	Package_settings \
	Package_settings_generated \
	Package_share_readme \
	PackageSet_odk_headers \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_config_win \
	Package_config_win \
))
else
$(eval $(call gb_Module_add_targets,odk,\
	Package_config_notwin \
))
endif

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,odk,\
	$(if $(filter YESGCC,$(BUILD_UNOWINREG)$(COM)),CustomTarget_unowinreg) \
	CustomTarget_classes \
	CustomTarget_javadoc \
	GeneratedPackage_uno_loader_classes \
	$(if $(filter WNT,$(OS)),Library_unowinreg) \
	Package_javadoc \
	Package_unowinreg \
))
endif

$(eval $(call gb_Module_add_check_targets,odk,\
	CppunitTest_checkapi \
	CustomTarget_allheaders \
	CustomTarget_check \
))

# vim: set noet sw=4 ts=4:
