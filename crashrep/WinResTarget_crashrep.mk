# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,crashrep))

$(eval $(call gb_WinResTarget_use_custom_headers,crashrep,\
	crashrep/source/win32 \
))

$(eval $(call gb_WinResTarget_add_defs,crashrep,\
    -DRES_APP_NAME=crashrep \
    -DRES_APP_FILEDESC="$(PRODUCTNAME) Crashreporter" \
    -DVERVARIANT=$(LIBO_VERSION_PATCH) \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_set_rcfile,crashrep,crashrep/source/win32/crashrep))

# vim: set noet sw=4 ts=4:
