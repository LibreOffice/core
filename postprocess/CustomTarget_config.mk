# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/config))

$(eval $(call gb_CustomTarget_register_targets,postprocess/config,\
	uiconfig.zip \
))

.PHONY: $(call gb_CustomTarget_get_workdir,postprocess/config)/uiconfig.zip

# We need this dependency for delivering in package postprocess_config:
$(call gb_CustomTarget_get_workdir,postprocess/config)/uiconfig_%.zip : \
		$(call gb_CustomTarget_get_workdir,postprocess/config)/uiconfig.zip
	@true

$(call gb_CustomTarget_get_workdir,postprocess/config)/uiconfig.zip : \
		$(SRCDIR)/postprocess/packconfig/packconfig.pl $(call gb_Postprocess_get_target,AllPackages)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	cd $(SRCDIR)/postprocess/packconfig \
	&& $(PERL) packconfig.pl -i $(OUTDIR)/xml/uiconfig -o $(dir $@)

# vim: set noet sw=4 ts=4:
