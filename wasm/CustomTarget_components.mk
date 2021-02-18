# vim: set noet sw=4 ts=4:
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,wasm/components))

wasm_WORKDIR := $(call gb_CustomTarget_get_workdir,wasm)

$(call gb_CustomTarget_get_target,wasm/components): \
	$(wasm_WORKDIR)/component_maps.cxx

$(wasm_WORKDIR)/component_maps.cxx: \
	    $(SRCDIR)/solenv/bin/native-code.py \
	    | $(wasm_WORKDIR)/.dir
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	$(call gb_Helper_abbreviate_dirs,$(call gb_ExternalExecutable_get_command,python) $< -g core -g writer) > $@

# vim: set noet sw=4:
