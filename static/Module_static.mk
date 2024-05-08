# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Module_Module,static))

ifeq ($(ENABLE_CUSTOMTARGET_COMPONENTS),TRUE)

$(eval $(call gb_Module_add_targets,static,\
    CustomTarget_components \
    Library_components \
))

ifeq (EMSCRIPTEN,$(OS))
$(eval $(call gb_Module_add_targets,static,\
    CustomTarget_emscripten_fs_image \
    CustomTarget_unoembind \
    Package_favicon \
    Package_unoembind \
    StaticLibrary_unoembind \
    $(if $(ENABLE_QT5), \
        CustomTarget_wasm-qt5-mandelbrot_moc \
        Executable_wasm-qt5-mandelbrot \
    ) \
))
endif

endif

ifneq ($(filter EMSCRIPTEN,$(BUILD_TYPE_FOR_HOST)),)
$(eval $(call gb_Module_add_targets,static, \
    Executable_embindmaker \
    Executable_wasmcallgen \
))
endif

# vim: set noet sw=4 ts=4:
