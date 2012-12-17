# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/msms))

MSMS := Microsoft_VC$(VCVER)_CRT_x86.msm \
	Microsoft_VC$(VCVER)_CRT_x64.msm \
	$(if $(filter 90,$(VCVER)),policy_9_0_Microsoft_VC90_CRT_x86.msm)

$(eval $(call gb_CustomTarget_register_targets,external/msms,\
	$(MSMS) \
))

$(call gb_CustomTarget_get_target,external/msms):
	for file in $(MSMS); do \
		if ! test -e $(call gb_CustomTarget_get_workdir,external/msms)/$$file; then \
			if `echo $$file | grep x86`; then \
				echo Failed to copy Merge module $$file; \
				exit 1; \
			else \
				echo Failed to copy x64 merge module $$file, installation will lack the 64-bit Explorer extension; \
			fi \
		fi \
	done

$(call gb_CustomTarget_get_workdir,external/msms)/%.msm: $(MSM_PATH)/%.msm
	$(call gb_Output_announce,$*,$(true),CPY,1)
	cp $< $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
