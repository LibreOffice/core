# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_sourceserif,font_sourceserif))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_sourceserif,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	SourceSerifPro-BoldIt.ttf \
	SourceSerifPro-ExtraLight.ttf \
	SourceSerifPro-Light.ttf \
	SourceSerifPro-Semibold.ttf \
	SourceSerifPro-BlackIt.ttf \
	SourceSerifPro-Bold.ttf \
	SourceSerifPro-It.ttf \
	SourceSerifPro-Regular.ttf \
	SourceSerifPro-Black.ttf \
	SourceSerifPro-ExtraLightIt.ttf \
	SourceSerifPro-LightIt.ttf \
	SourceSerifPro-SemiboldIt.ttf \
))

# vim: set noet sw=4 ts=4:
