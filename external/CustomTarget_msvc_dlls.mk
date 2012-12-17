# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/msvc_dlls))

VSDLLDIR := $(shell cygpath $(COMPATH))/redist/$(if $(filter INTEL, $(CPUNAME)),x86,amd64)/Microsoft.VC$(VCVER).CRT

VSDLLS := msvcp$(VCVER).dll \
	msvcr$(VCVER).dll \
	$(if $(filter 90,$(VCVER)),msvcm90.dll Microsoft.VC90.CRT.manifest)

$(eval $(call gb_CustomTarget_register_targets,external/msvc_dlls,\
	$(VSDLLS) \
))

$(call gb_CustomTarget_get_target,external/msvc_dlls):
	for file in $(VSDLLS); do \
		if ! test -e $(call gb_CustomTarget_get_workdir,external/msvc_dlls)/$$file; then \
			echo Could not find $$file in $(VSDLLDIR); \
			exit 1; \
		fi \
	done

$(call gb_CustomTarget_get_workdir,external/msvc_dlls)/%.dll: $(VSDLLDIR)/%.dll
	$(call gb_Output_announce,$*.dll,$(true),CPY,1)
	cp $< $@
$(call gb_CustomTarget_get_workdir,external/msvc_dlls)/%.manifest: $(VSDLLDIR)/%.manifest
	$(call gb_Output_announce,$*.manifest,$(true),CPY,1)
	cp $< $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
