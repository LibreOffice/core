# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,simpress/launcher))

$(eval $(call gb_WinResTarget_set_include,simpress/launcher,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sysui/desktop \
))

$(eval $(call gb_WinResTarget_add_defs,simpress/launcher,\
    -DRES_APP_NAME=simpress \
    -DRES_APP_FILEDESC="$(PRODUCTNAME) Impress" \
    -DRES_APP_ICON=icons/impress_app.ico \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_add_dependencies,simpress/launcher,\
    sysui/desktop/icons/impress_app.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,simpress/launcher,desktop/win32/source/applauncher/launcher))

# vim: set ts=4 sw=4 et:
