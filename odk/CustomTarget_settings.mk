# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/settings))

$(eval $(call gb_CustomTarget_register_target,odk/settings,dk.mk))

$(call gb_CustomTarget_get_workdir,odk/settings)/dk.mk : $(SRCDIR)/odk/settings/dk.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,SED,1)
	tr -d "\015" < $< | sed \
		-e 's/@@PLATFORM@@/$(firstword $(subst _, ,$(PLATFORMID)))/' \
		-e 's/@@PROCTYPE@@/$(subst $(WHITESPACE),_,$(wordlist 2,$(words $(subst _, ,$(PLATFORMID))),$(subst _, ,$(PLATFORMID))))/' \
		-e 's/@@RELEASE@@/$(PRODUCTVERSION)/' \
		-e 's/@@BUILDID@@/$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO)/' \
		-e 's/@@ENABLE_DBGUTIL@@/$(ENABLE_DBGUTIL)/' > $@

# vim: set noet sw=4 ts=4:
