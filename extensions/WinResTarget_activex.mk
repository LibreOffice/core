# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_WinResTarget_WinResTarget,so_activex))

$(eval $(call gb_WinResTarget_use_custom_headers,so_activex,\
    extensions/source/activex/idl \
))

ifneq ($(ENABLE_DBGUTIL),TRUE)
$(eval $(call gb_WinResTarget_add_defs,so_activex,\
	-DPRODUCT \
))
endif

$(eval $(call gb_WinResTarget_set_rcfile,so_activex,extensions/source/activex/so_activex))

$(eval $(call gb_WinResTarget_add_defs,so_activex,\
	$$(DEFS) \
	-DSO_ACTIVEX_TLB=\"$(subst /,\\\\,$(WORKDIR)/CustomTarget/extensions/source/activex/idl/so_activex.tlb\") \
))

# vim:set noet sw=4 ts=4:
