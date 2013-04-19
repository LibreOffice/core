# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,sweb/src))

$(eval $(call gb_WinResTarget_set_include,sweb/src,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sysui/desktop \
))

$(eval $(call gb_WinResTarget_add_defs,sweb/src,\
    -DRES_APP_NAME=sweb \
    -DRES_APP_FILEDESC="$(PRODUCTNAME) Writer (Web)" \
    -DRES_APP_ICON=icons/writer_app.ico \
    -DVERVARIANT=$(LIBO_VERSION_PATCH) \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_add_dependencies,sweb/src,\
    sysui/desktop/icons/writer_app.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,sweb/src,desktop/win32/source/applauncher/launcher))

# vim: set ts=4 sw=4 et:
