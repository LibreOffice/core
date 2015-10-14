# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,soffice/launcher))

$(eval $(call gb_WinResTarget_set_include,soffice/launcher,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sysui/desktop \
))

$(eval $(call gb_WinResTarget_add_defs,soffice/launcher,\
    -DRES_APP_NAME=soffice \
    -DRES_APP_FILEDESC="$(PRODUCTNAME)" \
    -DRES_APP_ICON=icons/soffice.ico \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_add_dependencies,soffice/launcher,\
    sysui/desktop/icons/soffice.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,soffice/launcher,desktop/win32/source/applauncher/launcher))

# vim: set ts=4 sw=4 et:
