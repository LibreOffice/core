# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,desktop/soffice))

ifeq (,$(ENABLE_RELEASE_BUILD))

# Add entitlements if this is a non-release build. Just to be safe,
# this target will always be run and MACOSX_CODESIGNING_IDENTITY is
# set to empty as we don't want to sign $(INSTROOTBASE). Ignore
# failures as it appears that setting only entitlements can fail
# with certain macOS SDKs.
$(call gb_CustomTarget_get_target,desktop/soffice) : \
	$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice
	-MACOSX_CODESIGNING_IDENTITY= $(SRCDIR)/solenv/bin/macosx-codesign-app-bundle $(INSTROOTBASE)

endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
