# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,scalc/launcher))

$(eval $(call gb_WinResTarget_set_include,scalc/launcher,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sysui/desktop \
))

$(eval $(call gb_WinResTarget_add_defs,scalc/launcher,\
    -DRES_APP_NAME=scalc \
    -DRES_APP_FILEDESC="$(PRODUCTNAME) Calc" \
    -DRES_APP_ICON=icons/calc_app.ico \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_add_dependencies,scalc/launcher,\
    sysui/desktop/icons/calc_app.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,scalc/launcher,desktop/win32/source/applauncher/launcher))

# vim: set ts=4 sw=4 et:
