# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,raptor))

$(eval $(call gb_StaticLibrary_use_unpacked,raptor,raptor))

$(eval $(call gb_StaticLibrary_use_externals,raptor,\
	libxml2 \
	libxslt \
))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,raptor))

$(eval $(call gb_StaticLibrary_add_ldflags,raptor,\
	/DEF:$(SRCDIR)/redland/raptor/raptor.def \
))

$(eval $(call gb_StaticLibrary_add_defs,raptor,\
	-DRAPTOR_INTERNAL \
	-DWIN32 \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DWIN32_EXPORTS \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,raptor,\
	UnpackedTarball/raptor/src/n3_lexer \
	UnpackedTarball/raptor/src/n3_parser \
	UnpackedTarball/raptor/src/ntriples_parse \
	UnpackedTarball/raptor/src/parsedate \
	UnpackedTarball/raptor/src/raptor_feature \
	UnpackedTarball/raptor/src/raptor_general \
	UnpackedTarball/raptor/src/raptor_guess \
	UnpackedTarball/raptor/src/raptor_identifier \
	UnpackedTarball/raptor/src/raptor_iostream \
	UnpackedTarball/raptor/src/raptor_libxml \
	UnpackedTarball/raptor/src/raptor_locator \
	UnpackedTarball/raptor/src/raptor_namespace \
	UnpackedTarball/raptor/src/raptor_nfc \
	UnpackedTarball/raptor/src/raptor_nfc_data \
	UnpackedTarball/raptor/src/raptor_parse \
	UnpackedTarball/raptor/src/raptor_qname \
	UnpackedTarball/raptor/src/raptor_rdfxml \
	UnpackedTarball/raptor/src/raptor_rfc2396 \
	UnpackedTarball/raptor/src/raptor_rss \
	UnpackedTarball/raptor/src/raptor_rss_common \
	UnpackedTarball/raptor/src/raptor_sax2 \
	UnpackedTarball/raptor/src/raptor_sequence \
	UnpackedTarball/raptor/src/raptor_serialize \
	UnpackedTarball/raptor/src/raptor_serialize_ntriples \
	UnpackedTarball/raptor/src/raptor_serialize_rdfxml \
	UnpackedTarball/raptor/src/raptor_serialize_rdfxmla \
	UnpackedTarball/raptor/src/raptor_serialize_simple \
	UnpackedTarball/raptor/src/raptor_set \
	UnpackedTarball/raptor/src/raptor_stringbuffer \
	UnpackedTarball/raptor/src/raptor_uri \
	UnpackedTarball/raptor/src/raptor_utf8 \
	UnpackedTarball/raptor/src/raptor_win32 \
	UnpackedTarball/raptor/src/raptor_www \
	UnpackedTarball/raptor/src/raptor_www_libxml \
	UnpackedTarball/raptor/src/raptor_xml \
	UnpackedTarball/raptor/src/raptor_xml_writer \
	UnpackedTarball/raptor/src/raptor_xsd \
	UnpackedTarball/raptor/src/strcasecmp \
	UnpackedTarball/raptor/src/turtle_common \
	UnpackedTarball/raptor/src/turtle_lexer \
	UnpackedTarball/raptor/src/turtle_parser \
	UnpackedTarball/raptor/src/raptor_abbrev \
	UnpackedTarball/raptor/src/raptor_avltree \
	UnpackedTarball/raptor/src/raptor_statement \
	UnpackedTarball/raptor/src/raptor_turtle_writer \
))


# vim: set noet sw=4 ts=4:
