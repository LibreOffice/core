# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,librsvg,rsvg))

$(eval $(call gb_ExternalPackage_use_external_project,librsvg,librsvg))

$(eval $(call gb_ExternalPackage_add_files,librsvg,lib,\
    .libs/librsvg-2.2.dylib \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,librsvg,inc/external/librsvg-2.0/librsvg,\
    rsvg-cairo.h \
    librsvg-enum-types.h \
    librsvg-features.h \
    rsvg.h \
))

# vim: set noet sw=4 ts=4:
