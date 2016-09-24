# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,desktop/soffice))

$(call gb_CustomTarget_get_target,desktop/soffice) : \
	$(call gb_CustomTarget_get_workdir,desktop/soffice)/soffice.sh

$(call gb_CustomTarget_get_workdir,desktop/soffice)/soffice.sh : \
		$(SRCDIR)/desktop/scripts/soffice.sh \
		| $(call gb_CustomTarget_get_workdir,desktop/soffice)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
ifneq ($(JITC_PROCESSOR_TYPE),)
	sed -e "s/^#@JITC_PROCESSOR_TYPE_EXPORT@/export JITC_PROCESSOR_TYPE=$(JITC_PROCESSOR_TYPE)/" $< > $@
else
	cp $< $@
endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
