# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,freetype,freetype))

$(eval $(call gb_ExternalPackage_use_external_project,freetype,freetype))

$(eval $(call gb_ExternalPackage_add_files,freetype,lib,\
	objs/.libs/libfreetype.a \
))

$(eval $(call gb_ExternalPackage_add_files,freetype,bin,\
	builds/unix/freetype-config \
))

# vim: set noet sw=4 ts=4:
