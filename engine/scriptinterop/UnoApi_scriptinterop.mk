# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnoApi_UnoApi,scriptinterop))

# Point the source tree lookup at scriptinterop/unoidl/, so the on-disk layout
# unoidl/cool.idl and unoidl/scriptinterop/*.idl matches the UNOIDL entity
# namespace (root-scope cool, module scriptinterop):
gb_UnoApiTarget_REG_scriptinterop := scriptinterop/unoidl

$(eval $(call gb_UnoApi_use_api,scriptinterop,\
    udkapi \
    offapi \
))

$(eval $(call gb_UnoApi_add_idlfiles,scriptinterop,,\
    cool \
))

$(eval $(call gb_UnoApi_add_idlfiles,scriptinterop,scriptinterop,\
    ImageOptions \
    XBase \
    XDocument \
    XDrawing \
    XFactory \
    XParagraph \
    XPresentation \
    XSelection \
    XSpreadsheet \
))

# vim: set noet sw=4 ts=4:
