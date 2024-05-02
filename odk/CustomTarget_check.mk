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

odk_PLATFORM := $(if $(filter WNT,$(OS)),windows,\
	$(if $(filter SOLARIS,$(OS)),\
		$(if $(filter SPARC,$(CPUNAME)),solsparc,\
			$(if $(filter SPARC64,$(CPUNAME)),solsparc64,solintel)),\
		$(if $(filter LINUX,$(OS)),linux,\
			$(if $(filter NETBSD,$(OS)),netbsd,\
				$(if $(filter FREEBSD,$(OS)),freebsd,\
					$(if $(filter DRAGONFLY,$(OS)),dragonfly,\
						$(if $(filter MACOSX,$(OS)),macosx)))))))

.PHONY: $(gb_CustomTarget_workdir)/odk/check/checkbin
$(gb_CustomTarget_workdir)/odk/check/checkbin : \
		$(SRCDIR)/odk/util/check.pl \
		$(if $(DOXYGEN),$(call gb_GeneratedPackage_get_target,odk_doxygen)) \
		$(foreach exe,$(if $(filter WNT,$(OS)),$(if $(filter-out AARCH64_TRUE,$(CPUNAME)_$(CROSS_COMPILING)),climaker)) cppumaker \
				javamaker uno-skeletonmaker unoapploader unoidl-read unoidl-write,\
			$(call gb_Executable_get_target,$(exe))) \
		$(if $(filter WNT,$(OS)),$(call gb_Package_get_target,odk_cli)) \
		$(call gb_Package_get_target,odk_config) \
		$(if $(filter WNT,$(OS)),$(call gb_Package_get_target,odk_config_win)) \
		$(call gb_Package_get_target,odk_docs) \
		$(call gb_Package_get_target,odk_html) \
		$(if $(ENABLE_JAVA),$(call gb_GeneratedPackage_get_target,odk_javadoc)) \
		$(call gb_Package_get_target,odk_settings) \
		$(call gb_Package_get_target,odk_settings_generated)
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CHK)
	$(call gb_Helper_print_on_error,\
		$(PERL) $< \
			$(INSTDIR)/$(SDKDIRNAME) \
			$(odk_PLATFORM) '$(gb_Executable_EXT)' \
		,$@.log \
	)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CHK)
endif

# vim: set noet sw=4 ts=4:
