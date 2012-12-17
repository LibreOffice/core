# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,msms,$(call gb_CustomTarget_get_workdir,external/msms)))

$(eval $(call gb_Package_add_files,msms,bin,\
	Microsoft_VC$(VCVER)_CRT_x86.msm \
	$(if $(wildcard $(call gb_CustomTarget_get_workdir,external/msms)/Microsoft_VC$(VCVER)_CRT_x64.msm),Microsoft_VC$(VCVER)_CRT_x64.msm) \
	$(if $(filter 90,$(VCVER)),policy_9_0_Microsoft_VC90_CRT_x86.msm) \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
