# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_WinResTarget_WinResTarget,UAccCOM))

$(eval $(call gb_WinResTarget_use_custom_headers,UAccCOM,\
    winaccessibility/ia2/idl \
))

ifneq ($(ENABLE_DBGUTIL),TRUE)
$(eval $(call gb_WinResTarget_add_defs,UAccCOM,\
	-DPRODUCT \
))
endif

$(eval $(call gb_WinResTarget_set_include,UAccCOM,\
	$$(INCLUDE) \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_WinResTarget_set_rcfile,UAccCOM,winaccessibility/source/UAccCOM/UAccCOM))

# vim:set noet sw=4 ts=4:
