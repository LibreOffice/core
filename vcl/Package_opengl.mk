# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,vcl_opengl_shader,$(SRCDIR)/vcl/opengl))

$(eval $(call gb_Package_add_files,vcl_opengl_shader,$(LIBO_ETC_FOLDER)/opengl,\
	areaScaleFastFragmentShader.glsl \
	areaScaleFragmentShader.glsl \
	blendedTextureFragmentShader.glsl \
	blendedTextureVertexShader.glsl \
	dumbVertexShader.glsl \
	diffTextureFragmentShader.glsl \
	convolutionFragmentShader.glsl \
	linearGradientFragmentShader.glsl \
	maskFragmentShader.glsl \
	maskedTextureVertexShader.glsl \
	maskedTextureFragmentShader.glsl \
	radialGradientFragmentShader.glsl \
	replaceColorFragmentShader.glsl \
	solidFragmentShader.glsl \
	textureFragmentShader.glsl \
	textureVertexShader.glsl \
	transformedTextureVertexShader.glsl \
))

# vim: set noet sw=4 ts=4:
