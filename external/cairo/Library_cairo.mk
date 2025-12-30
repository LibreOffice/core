# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cairo))

$(eval $(call gb_Library_set_include,cairo,\
	-I$(gb_UnpackedTarball_workdir)/cairo \
	-I$(gb_UnpackedTarball_workdir)/cairo/src \
	-I$(gb_UnpackedTarball_workdir)/pixman/pixman \
	-I$(gb_UnpackedTarball_workdir)/freetype/include \
	-I$(gb_UnpackedTarball_workdir)/fontconfig \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,cairo,\
	-wd4057 \
	-wd4100 \
	-wd4132 \
	-wd4146 \
	-wd4200 \
	-wd4245 \
	-wd4267 \
	-wd4295 \
	-wd4456 \
	-wd4701 \
	-wd4702 \
	-wd4703 \
	-wd5294 \
	-wd5286 \
	-wd5287 \
))

$(eval $(call gb_Library_use_static_libraries,cairo, \
	expat \
	fontconfig \
	freetype \
	pixman \
))

$(eval $(call gb_Library_use_externals,cairo,\
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,cairo,\
    dwrite \
    ole32 \
    shell32 \
))

$(eval $(call gb_Library_add_generated_cobjects,cairo,\
	$(addprefix UnpackedTarball/cairo/src/, \
		cairo-analysis-surface \
		cairo-arc \
		cairo-array \
		cairo-atomic \
		cairo-base64-stream \
		cairo-base85-stream \
		cairo-bentley-ottmann-rectangular \
		cairo-bentley-ottmann-rectilinear \
		cairo-bentley-ottmann \
		cairo-botor-scan-converter \
		cairo-boxes-intersect \
		cairo-boxes \
		cairo-cache \
		cairo-clip-boxes \
		cairo-clip-polygon \
		cairo-clip-region \
		cairo-clip-surface \
		cairo-clip-tor-scan-converter \
		cairo-clip \
		cairo-color \
		cairo-composite-rectangles \
		cairo-compositor \
		cairo-contour \
		cairo-damage \
		cairo-debug \
		cairo-default-context \
		cairo-device \
		cairo-error \
		cairo-fallback-compositor \
		cairo-fixed \
		cairo-font-face-twin-data \
		cairo-font-face-twin \
		cairo-font-face \
		cairo-font-options \
		cairo-freed-pool \
		cairo-freelist \
		cairo-gstate \
		cairo-hash \
		cairo-hull \
		cairo-image-compositor \
		cairo-image-info \
		cairo-image-source \
		cairo-image-surface \
		cairo-line \
		cairo-lzw \
		cairo-mask-compositor \
		cairo-matrix \
		cairo-mempool \
		cairo-mesh-pattern-rasterizer \
		cairo-misc \
		cairo-mono-scan-converter \
		cairo-mutex \
		cairo-no-compositor \
		cairo-observer \
		cairo-output-stream \
		cairo-paginated-surface \
		cairo-path-bounds \
		cairo-path-fill \
		cairo-path-fixed \
		cairo-path-in-fill \
		cairo-path-stroke-boxes \
		cairo-path-stroke-polygon \
		cairo-path-stroke-traps \
		cairo-path-stroke-tristrip \
		cairo-path-stroke \
		cairo-path \
		cairo-pattern \
		cairo-pen \
		cairo-polygon-intersect \
		cairo-polygon-reduce \
		cairo-polygon \
		cairo-raster-source-pattern \
		cairo-recording-surface \
		cairo-rectangle \
		cairo-rectangular-scan-converter \
		cairo-region \
		cairo-rtree \
		cairo-scaled-font \
		cairo-shape-mask-compositor \
		cairo-slope \
		cairo-spans-compositor \
		cairo-spans \
		cairo-spline \
		cairo-stroke-dash \
		cairo-stroke-style \
		cairo-surface-clipper \
		cairo-surface-fallback \
		cairo-surface-observer \
		cairo-surface-offset \
		cairo-surface-snapshot \
		cairo-surface-subsurface \
		cairo-surface-wrapper \
		cairo-surface \
		cairo-time \
		cairo-tor-scan-converter \
		cairo-tor22-scan-converter \
		cairo-toy-font-face \
		cairo-traps-compositor \
		cairo-traps \
		cairo-tristrip \
		cairo-unicode \
		cairo-user-font \
		cairo-version \
		cairo-wideint \
		cairo \
		cairo-cff-subset \
		cairo-scaled-font-subsets \
		cairo-truetype-subset \
		cairo-type1-fallback \
		cairo-type1-glyph-names \
		cairo-type1-subset \
		cairo-type3-glyph-surface \
		cairo-script-surface \
		cairo-pdf-surface \
		cairo-pdf-interchange \
		cairo-pdf-operators \
		cairo-pdf-shading \
		cairo-tag-attributes \
		cairo-tag-stack \
		cairo-deflate-stream \
		cairo-ft-font \
	) \
))

# vim: set noet sw=4 ts=4:
