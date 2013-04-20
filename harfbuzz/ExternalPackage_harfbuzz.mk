# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,harfbuzz,harfbuzz))

$(eval $(call gb_ExternalPackage_add_unpacked_files,harfbuzz,inc/external/harfbuzz,\
	src/hb.h \
	src/hb-blob.h \
	src/hb-buffer.h \
	src/hb-common.h \
	src/hb-font.h \
	src/hb-icu.h \
	src/hb-ot.h \
	src/hb-ot-layout.h \
	src/hb-ot-tag.h \
	src/hb-set.h \
	src/hb-shape.h \
	src/hb-shape-plan.h \
	src/hb-unicode.h \
	src/hb-version.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,harfbuzz,harfbuzz))

$(eval $(call gb_ExternalPackage_add_file,harfbuzz,lib/libharfbuzz.a,src/.libs/libharfbuzz.a))


# vim: set noet sw=4 ts=4:
