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
	$(gb_CustomTarget_workdir)/extensions/source/activex/SOActionsApproval.cxx \
	$(gb_CustomTarget_workdir)/extensions/source/activex/SOActiveX.cxx \
	$(gb_CustomTarget_workdir)/extensions/source/activex/SOComWindowPeer.cxx \
	$(gb_CustomTarget_workdir)/extensions/source/activex/SODispatchInterceptor.cxx \
	$(gb_CustomTarget_workdir)/extensions/source/activex/StdAfx2.cxx \
	$(gb_CustomTarget_workdir)/extensions/source/activex/so_activex.cxx \

$(gb_CustomTarget_workdir)/extensions/source/activex/% : \
		$(SRCDIR)/extensions/source/activex/% \
		| $(gb_CustomTarget_workdir)/extensions/source/activex/.dir
	cp $< $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
