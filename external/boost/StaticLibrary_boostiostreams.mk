# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boostiostreams))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boostiostreams))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boostiostreams,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_external,boostiostreams,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boostiostreams,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boostiostreams,\
	UnpackedTarball/boost/libs/iostreams/src/zlib \
	UnpackedTarball/boost/libs/iostreams/src/gzip \
))

# vim: set noet sw=4 ts=4:
