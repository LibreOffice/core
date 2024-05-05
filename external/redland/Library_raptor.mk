# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,raptor2))

$(eval $(call gb_Library_set_include,raptor2, \
    -I$(gb_UnpackedTarball_workdir)/raptor/src \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_unpacked,raptor2,raptor))

$(eval $(call gb_Library_use_externals,raptor2,\
	libxml2 \
	libxslt \
))

$(eval $(call gb_Library_set_warnings_disabled,raptor2))

$(eval $(call gb_Library_add_defs,raptor2,\
	-DHAVE_CONFIG_H \
	-DRAPTOR_INTERNAL \
	-DWIN32 \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DWIN32_EXPORTS \
	-DYY_NO_UNISTD_H \
	-DHAVE__ACCESS \
	-UHAVE_ACCESS \
))

$(eval $(call gb_Library_add_generated_cobjects,raptor2,\
	UnpackedTarball/raptor/src/ntriples_parse \
	UnpackedTarball/raptor/src/parsedate \
	UnpackedTarball/raptor/src/raptor_abbrev \
	UnpackedTarball/raptor/src/raptor_avltree \
	UnpackedTarball/raptor/src/raptor_concepts \
	UnpackedTarball/raptor/src/raptor_escaped \
	UnpackedTarball/raptor/src/raptor_general \
	UnpackedTarball/raptor/src/raptor_guess \
	UnpackedTarball/raptor/src/raptor_iostream \
	UnpackedTarball/raptor/src/raptor_json_writer \
	UnpackedTarball/raptor/src/raptor_libxml \
	UnpackedTarball/raptor/src/raptor_locator \
	UnpackedTarball/raptor/src/raptor_log \
	UnpackedTarball/raptor/src/raptor_memstr \
	UnpackedTarball/raptor/src/raptor_namespace \
	UnpackedTarball/raptor/src/raptor_ntriples \
	UnpackedTarball/raptor/src/raptor_option \
	UnpackedTarball/raptor/src/raptor_parse \
	UnpackedTarball/raptor/src/raptor_qname \
	UnpackedTarball/raptor/src/raptor_rdfxml \
	UnpackedTarball/raptor/src/raptor_rfc2396 \
	UnpackedTarball/raptor/src/raptor_sax2 \
	UnpackedTarball/raptor/src/raptor_sequence \
	UnpackedTarball/raptor/src/raptor_serialize \
	UnpackedTarball/raptor/src/raptor_serialize_ntriples \
	UnpackedTarball/raptor/src/raptor_serialize_rdfxml \
	UnpackedTarball/raptor/src/raptor_serialize_rdfxmla \
	UnpackedTarball/raptor/src/raptor_serialize_turtle \
	UnpackedTarball/raptor/src/raptor_set \
	UnpackedTarball/raptor/src/raptor_statement \
	UnpackedTarball/raptor/src/raptor_stringbuffer \
	UnpackedTarball/raptor/src/raptor_syntax_description \
	UnpackedTarball/raptor/src/raptor_term \
	UnpackedTarball/raptor/src/raptor_turtle_writer \
	UnpackedTarball/raptor/src/raptor_unicode \
	UnpackedTarball/raptor/src/raptor_uri \
	UnpackedTarball/raptor/src/raptor_www \
	UnpackedTarball/raptor/src/raptor_xml \
	UnpackedTarball/raptor/src/raptor_xml_writer \
	UnpackedTarball/raptor/src/snprintf \
	UnpackedTarball/raptor/src/sort_r \
	UnpackedTarball/raptor/src/strcasecmp \
	UnpackedTarball/raptor/src/turtle_common \
	UnpackedTarball/raptor/src/turtle_lexer \
	UnpackedTarball/raptor/src/turtle_parser \
))


# vim: set noet sw=4 ts=4:
