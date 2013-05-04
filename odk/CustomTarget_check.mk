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
	autodoc \
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
		$(if $(DOXYGEN),$(call gb_GeneratedPackage_get_target,odk_cpp_docs)) \
		$(call gb_Package_get_target,odk_bin) \
		$(if $(filter WNT,$(OS)),$(call gb_Package_get_target,odk_cli)) \
		$(call gb_Package_get_target,odk_config) \
		$(if $(filter WNT,$(OS)),\
			$(call gb_Package_get_target,odk_config_win),\
			$(call gb_Package_get_target,odk_config_notwin) \
		) \
		$(call gb_Package_get_target,odk_docs) \
		$(call gb_Package_get_target,odk_html) \
		$(if $(SOLAR_JAVA),$(call gb_Package_get_target,odk_javadoc)) \
		$(call gb_Package_get_target,odk_lib) \
		$(if $(filter MACOSX,$(OS)),$(call gb_Package_get_target,odk_macosx)) \
		$(call gb_Package_get_target,odk_settings) \
		$(call gb_Package_get_target,odk_settings_generated) \
		$(call gb_Package_get_target,odk_uno_loader_classes) \
		$(if $(SOLAR_JAVA),$(call gb_Package_get_target,odk_unowinreg)) \
		$(call gb_PackageSet_get_target,odk_autodoc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	$(call gb_Helper_print_on_error,\
		$(PERL) $< \
			$(INSTDIR)/$(gb_Package_SDKDIRNAME) \
			$(odk_PLATFORM) "$(gb_Executable_EXT)" \
		,$@.log \
	)

.PHONY : $(call gb_CustomTarget_get_workdir,odk/check)/autodoc
$(call gb_CustomTarget_get_workdir,odk/check)/autodoc : $(call gb_PackageSet_get_target,odk_autodoc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	( \
		c=0 && \
		for m in `find $(call gb_CustomTarget_get_workdir,odk/docs/common/ref) -name module-ix.html`; do \
			m=`echo $$m | sed -e s@$(call gb_CustomTarget_get_workdir,odk/docs/common/ref)/@@` && \
			mm=$(INSTDIR)/$(gb_Package_SDKDIRNAME)/docs/common/ref/$${m} && \
			if [ ! -e $${mm} ]; then \
				c=`expr $$c + 1`; \
				echo "autodoc check: $${mm} not found. Please update odk/Package_autodoc.mk ."; \
			fi \
		done && \
		if [ $${c} -gt 0 ]; then exit 1; fi \
	)

# vim: set noet sw=4 ts=4:
