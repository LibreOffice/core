# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,revenge))

$(eval $(call gb_Library_use_unpacked,revenge,librevenge))

$(eval $(call gb_Library_use_externals,revenge,\
    boost_headers \
))

$(eval $(call gb_Library_set_warnings_disabled,revenge))

$(eval $(call gb_Library_set_include,revenge,\
    -I$(gb_UnpackedTarball_workdir)/librevenge/inc \
    $$(INCLUDE) \
))

# -DLIBREVENGE_STREAM_BUILD is present so that linker does not complain
#  about unresolved external symbol RVNGStream::~RVNGStream
$(eval $(call gb_Library_add_defs,revenge,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBREVENGE_BUILD \
	-DLIBREVENGE_STREAM_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,revenge,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,revenge,\
    UnpackedTarball/librevenge/src/lib/RVNGBinaryData \
    UnpackedTarball/librevenge/src/lib/RVNGMemoryStream \
    UnpackedTarball/librevenge/src/lib/RVNGProperty \
    UnpackedTarball/librevenge/src/lib/RVNGPropertyList \
    UnpackedTarball/librevenge/src/lib/RVNGPropertyListVector \
    UnpackedTarball/librevenge/src/lib/RVNGString \
    UnpackedTarball/librevenge/src/lib/RVNGStringVector \
    UnpackedTarball/librevenge/src/lib/RVNGSVGDrawingGenerator \
))

# vim: set noet sw=4 ts=4:
