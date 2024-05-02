# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,spsupp))

$(eval $(call gb_WinResTarget_use_custom_headers,spsupp,\
	shell/source/win32/spsupp/idl \
))

$(eval $(call gb_WinResTarget_add_defs,spsupp,\
	-DTLB_FILE=\"$(gb_CustomTarget_workdir)/shell/source/win32/spsupp/idl/spsupp.tlb\" \
))

$(eval $(call gb_WinResTarget_set_rcfile,spsupp,shell/source/win32/spsupp/res/spsupp))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
