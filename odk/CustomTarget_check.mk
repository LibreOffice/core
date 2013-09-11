# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/check))

$(eval $(call gb_CustomTarget_register_targets,odk/check,\
	checkbin \
))

odk_PLATFORM := $(if $(filter WNT,$(OS)),\
	$(if $(filter GCC,$(COM)),mingw,windows),\
	$(if $(filter SOLARIS,$(OS)),\
	$(if $(filter SPARC,$(CPUNAME)),solsparc,\
	$(if $(filter U,$(CPU)),solsparc64,solintel)),\
	$(if $(filter LINUX,$(OS)),linux,\
	$(if $(filter NETBSD,$(OS)),netbsd,\
	$(if $(filter FREEBSD,$(OS)),freebsd,\
	$(if $(filter DRAGONFLY,$(OS)),dragonfly,\
	$(if $(filter MACOSX,$(OS)),macosx,\
	$(if $(filter AIX,$(OS)),aix))))))))

$(call gb_CustomTarget_get_workdir,odk/check)/checkbin : \
		$(SRCDIR)/odk/util/check.pl \
		$(if $(DOXYGEN),$(call gb_GeneratedPackage_get_target,odk_doxygen)) \
		$(call gb_Package_get_target,odk_bin) \
		$(if $(filter WNT,$(OS)),$(call gb_Package_get_target,odk_cli)) \
		$(call gb_Package_get_target,odk_config) \
		$(if $(filter WNT,$(OS)),\
			$(call gb_Package_get_target,odk_config_win),\
			$(call gb_Package_get_target,odk_config_notwin) \
		) \
		$(call gb_Package_get_target,odk_docs) \
		$(call gb_Package_get_target,odk_html) \
		$(if $(ENABLE_JAVA),$(call gb_Package_get_target,odk_javadoc)) \
		$(call gb_Package_get_target,odk_lib) \
		$(call gb_Package_get_target,odk_settings) \
		$(call gb_Package_get_target,odk_settings_generated) \
		$(call gb_Package_get_target,odk_uno_loader_classes) \
		$(if $(ENABLE_JAVA),$(call gb_Package_get_target,odk_unowinreg))
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	$(call gb_Helper_print_on_error,\
		$(PERL) $< \
			$(INSTDIR)/$(gb_Package_SDKDIRNAME) \
			$(odk_PLATFORM) "$(gb_Executable_EXT)" \
		,$@.log \
	)

# vim: set noet sw=4 ts=4:
