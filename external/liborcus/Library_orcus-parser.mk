# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,orcus-parser))

$(eval $(call gb_Library_use_unpacked,orcus-parser,liborcus))

$(eval $(call gb_Library_use_externals,orcus-parser,\
	boost_headers \
	boost_system \
	mdds_headers \
	zlib \
))

$(eval $(call gb_Library_set_warnings_not_errors,orcus-parser))

$(eval $(call gb_Library_set_include,orcus-parser,\
	-I$(call gb_UnpackedTarball_get_dir,liborcus)/include \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,orcus-parser,\
	-DBOOST_ALL_NO_LIB \
	-D__ORCUS_PSR_BUILDING_DLL \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,orcus-parser,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,orcus-parser,\
	UnpackedTarball/liborcus/src/parser/base64 \
	UnpackedTarball/liborcus/src/parser/cell_buffer \
	UnpackedTarball/liborcus/src/parser/css_parser_base \
	UnpackedTarball/liborcus/src/parser/css_types \
	UnpackedTarball/liborcus/src/parser/csv_parser_base \
	UnpackedTarball/liborcus/src/parser/exception \
	UnpackedTarball/liborcus/src/parser/parser_global \
	UnpackedTarball/liborcus/src/parser/pstring \
	UnpackedTarball/liborcus/src/parser/sax_parser_base \
	UnpackedTarball/liborcus/src/parser/sax_token_parser \
	UnpackedTarball/liborcus/src/parser/stream \
	UnpackedTarball/liborcus/src/parser/string_pool \
	UnpackedTarball/liborcus/src/parser/tokens \
	UnpackedTarball/liborcus/src/parser/types \
	UnpackedTarball/liborcus/src/parser/xml_namespace \
	UnpackedTarball/liborcus/src/parser/zip_archive \
	UnpackedTarball/liborcus/src/parser/zip_archive_stream \
))

# vim: set noet sw=4 ts=4:
