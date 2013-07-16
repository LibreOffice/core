# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,setup_native/mac))

$(eval $(call gb_CustomTarget_register_target,setup_native/mac,macinstall.ulf))

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(call gb_CustomTarget_get_workdir,setup_native/mac)/macinstall.ulf,\
	$(SRCDIR)/setup_native/source/mac/macinstall.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/setup_native/source/mac.po)))

# vim: set noet sw=4 ts=4:
