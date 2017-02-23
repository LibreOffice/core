# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#- Env ------------------------------------------------------------------------
BUILDID			:=$(shell cd $(SRCDIR) && git log -1 --format=%H)

#- Targets --------------------------------------------------------------------

.PHONY: LibreOfficeLight_setup

# Register target
$(eval $(call gb_CustomTarget_CustomTarget,ios/LibreOfficeLight))

# Build
# Depend on the custom target that sets up lo.xcconfig
$(call gb_CustomTarget_get_target,ios/LibreOfficeLight): $(call gb_CustomTarget_get_target,ios/Lo_Xcconfig)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	$(call IOSbuild,LibreOfficeLight, clean build)

# Clean
$(call gb_CustomTarget_get_clean_target,ios/LibreOfficeLight):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),APP,2)
	$(call IOSbuild,LibreOfficeLight, clean)

# vim: set noet sw=4 ts=4:
