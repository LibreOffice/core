# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,wasm-qt5-mandelbrot))

$(eval $(call gb_Executable_use_custom_headers,wasm-qt5-mandelbrot,wasm-qt/qt5-mandelbrot))

$(eval $(call gb_Executable_use_externals,wasm-qt5-mandelbrot,\
    graphite \
    freetype \
    harfbuzz \
    libpng \
    qt5 \
))

$(eval $(call gb_Executable_add_exception_objects,wasm-qt5-mandelbrot,\
    wasm-qt/source/qt5-mandelbrot/main \
    wasm-qt/source/qt5-mandelbrot/mandelbrotwidget \
    wasm-qt/source/qt5-mandelbrot/renderthread \
))

$(eval $(call gb_Executable_add_defs,wasm-qt5-mandelbrot,\
    -DVCL_INTERNALS \
))

# vim: set noet sw=4 ts=4:
