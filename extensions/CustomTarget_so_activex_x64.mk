# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extensions/source/activex))

$(call gb_CustomTarget_get_target,extensions/source/activex) : \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/SOActionsApproval.cxx \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/SOActiveX.cxx \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/SOComWindowPeer.cxx \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/SODispatchInterceptor.cxx \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/StdAfx2.cxx \
	$(call gb_CustomTarget_get_workdir,extensions/source/activex)/so_activex.cxx \

$(call gb_CustomTarget_get_workdir,extensions/source/activex)/% : \
		$(SRCDIR)/extensions/source/activex/% \
		| $(call gb_CustomTarget_get_workdir,extensions/source/activex)/.dir
	cp $< $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
