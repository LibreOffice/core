# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,static/wasm_fs_image,$(call gb_CustomTarget_get_workdir,static/wasm_fs_image)))

ifeq (EMSCRIPTEN,$(OS))
$(eval $(call gb_Package_add_file,static/wasm_fs_image,$(LIBO_BIN_FOLDER)/soffice.data,soffice.data))
$(eval $(call gb_Package_add_file,static/wasm_fs_image,$(LIBO_BIN_FOLDER)/soffice.data.js.metadata,soffice.data.js.metadata))
endif

# vim: set ts=4 sw=4 noet:
