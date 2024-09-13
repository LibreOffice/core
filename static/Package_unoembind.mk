# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,unoembind,$(call gb_CustomTarget_get_workdir,static/unoembind)))

$(eval $(call gb_Package_use_custom_target,unoembind,static/unoembind))

$(eval $(call gb_Package_add_files,unoembind,$(LIBO_BIN_FOLDER), \
    bindings_uno.js \
))

# vim: set noet sw=4 ts=4:
