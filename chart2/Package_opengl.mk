# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,chart2_opengl_shader,$(SRCDIR)/chart2/opengl))

$(eval $(call gb_Package_add_files,chart2_opengl_shader,$(LIBO_ETC_FOLDER)/opengl,\
	backgroundFragmentShader.glsl \
	backgroundVertexShader.glsl \
	commonFragmentShader.glsl \
	commonVertexShader.glsl \
	debugFragmentShader.glsl \
	debugVertexShader.glsl \
	pickingVertexShader.glsl \
	pickingFragmentShader.glsl \
	symbolFragmentShader.glsl \
	symbolVertexShader.glsl \
	textFragmentShader.glsl \
	textVertexShader.glsl \
	screenTextFragmentShader.glsl \
	screenTextVertexShader.glsl \
	shape3DFragmentShader.glsl \
	shape3DVertexShader.glsl \
	renderTextureVertexShader.glsl \
	renderTextureFragmentShader.glsl \
	shape3DFragmentShaderBatch.glsl \
	shape3DVertexShaderBatch.glsl \
	shape3DFragmentShaderV300.glsl \
	shape3DVertexShaderV300.glsl \
	textFragmentShaderBatch.glsl \
	textVertexShaderBatch.glsl \
	shape3DFragmentShaderBatchScroll.glsl \
	shape3DVertexShaderBatchScroll.glsl \
))

# vim: set noet sw=4 ts=4:
