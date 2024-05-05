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
	boost_filesystem \
	boost_system \
	mdds_headers \
	zlib \
))

$(eval $(call gb_Library_set_warnings_disabled,orcus-parser))

$(eval $(call gb_Library_set_precompiled_header,orcus-parser,external/liborcus/inc/pch/precompiled_orcus-parser))

$(eval $(call gb_Library_set_include,orcus-parser,\
	-I$(gb_UnpackedTarball_workdir)/liborcus/include \
	-I$(gb_UnpackedTarball_workdir)/liborcus/src/include \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,orcus-parser,\
	-DBOOST_ALL_NO_LIB \
	-D__ORCUS_PSR_BUILDING_DLL \
))

# Needed when building against MSVC in C++17 mode, as
# workdir/UnpackedTarball/liborcus/include/orcus/global.hpp uses std::unary_function:
$(eval $(call gb_Library_add_defs,orcus-parser, \
    -D_HAS_AUTO_PTR_ETC=1 \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,orcus-parser,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,orcus-parser,\
	UnpackedTarball/liborcus/src/parser/base64 \
	UnpackedTarball/liborcus/src/parser/cell_buffer \
	UnpackedTarball/liborcus/src/parser/css_parser_base \
	UnpackedTarball/liborcus/src/parser/css_types \
	UnpackedTarball/liborcus/src/parser/csv_parser_base \
	UnpackedTarball/liborcus/src/parser/exception \
	UnpackedTarball/liborcus/src/parser/json_global \
	UnpackedTarball/liborcus/src/parser/json_parser_base \
	UnpackedTarball/liborcus/src/parser/json_parser_thread \
	UnpackedTarball/liborcus/src/parser/parser_base \
	UnpackedTarball/liborcus/src/parser/parser_global \
	UnpackedTarball/liborcus/src/parser/sax_parser_base \
	UnpackedTarball/liborcus/src/parser/sax_token_parser \
	UnpackedTarball/liborcus/src/parser/sax_token_parser_thread \
	UnpackedTarball/liborcus/src/parser/stream \
	UnpackedTarball/liborcus/src/parser/string_pool \
	UnpackedTarball/liborcus/src/parser/tokens \
	UnpackedTarball/liborcus/src/parser/types \
	UnpackedTarball/liborcus/src/parser/utf8 \
	UnpackedTarball/liborcus/src/parser/xml_namespace \
	UnpackedTarball/liborcus/src/parser/xml_writer \
	UnpackedTarball/liborcus/src/parser/yaml_parser_base \
	UnpackedTarball/liborcus/src/parser/zip_archive \
	UnpackedTarball/liborcus/src/parser/zip_archive_stream \
))

# vim: set noet sw=4 ts=4:
