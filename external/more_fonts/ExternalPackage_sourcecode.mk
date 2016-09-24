# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_sourcecode,font_sourcecode))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_sourcecode,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	SourceCodePro-BlackIt.ttf \
	SourceCodePro-Black.ttf \
	SourceCodePro-BoldIt.ttf \
	SourceCodePro-Bold.ttf \
	SourceCodePro-ExtraLightIt.ttf \
	SourceCodePro-ExtraLight.ttf \
	SourceCodePro-It.ttf \
	SourceCodePro-LightIt.ttf \
	SourceCodePro-Light.ttf \
	SourceCodePro-MediumIt.ttf \
	SourceCodePro-Medium.ttf \
	SourceCodePro-Regular.ttf \
	SourceCodePro-SemiboldIt.ttf \
	SourceCodePro-Semibold.ttf \
))

# vim: set noet sw=4 ts=4:
