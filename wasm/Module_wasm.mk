# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Module_Module,wasm))

ifeq ($(OS),EMSCRIPTEN)

$(eval $(call gb_Module_add_targets,wasm,\
    CustomTarget_wasm-qt5-mandelbrot_moc \
    Executable_wasm-qt5-mandelbrot \
))

endif

# vim: set noet sw=4 ts=4:
