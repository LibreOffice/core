# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rdf))

$(eval $(call gb_Library_use_unpacked,rdf,redland))

$(eval $(call gb_Library_set_warnings_not_errors,rdf))

$(eval $(call gb_Library_use_libraries,rdf,\
	raptor \
	rasqal \
))

$(eval $(call gb_Library_use_externals,rdf,\
	openssl \
	raptor_headers \
	rasqal_headers \
))

#$(eval $(call gb_Library_add_ldflags,rdf,\
	/DEF:$(SRCDIR)/redland/redland/rdf.def \
))

$(eval $(call gb_Library_add_defs,rdf,\
	-DLIBRDF_INTERNAL \
	-DWIN32 \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DWIN32_EXPORTS \
))

$(eval $(call gb_Library_add_generated_cobjects,rdf,\
	UnpackedTarball/redland/librdf/rdf_concepts \
	UnpackedTarball/redland/librdf/rdf_digest \
	UnpackedTarball/redland/librdf/rdf_digest_md5 \
	UnpackedTarball/redland/librdf/rdf_digest_openssl \
	UnpackedTarball/redland/librdf/rdf_digest_sha1 \
	UnpackedTarball/redland/librdf/rdf_files \
	UnpackedTarball/redland/librdf/rdf_hash \
	UnpackedTarball/redland/librdf/rdf_hash_cursor \
	UnpackedTarball/redland/librdf/rdf_hash_memory \
	UnpackedTarball/redland/librdf/rdf_heuristics \
	UnpackedTarball/redland/librdf/rdf_init \
	UnpackedTarball/redland/librdf/rdf_iterator \
	UnpackedTarball/redland/librdf/rdf_list \
	UnpackedTarball/redland/librdf/rdf_log \
	UnpackedTarball/redland/librdf/rdf_model \
	UnpackedTarball/redland/librdf/rdf_model_storage \
	UnpackedTarball/redland/librdf/rdf_node \
	UnpackedTarball/redland/librdf/rdf_parser \
	UnpackedTarball/redland/librdf/rdf_parser_raptor \
	UnpackedTarball/redland/librdf/rdf_query \
	UnpackedTarball/redland/librdf/rdf_query_rasqal \
	UnpackedTarball/redland/librdf/rdf_query_results \
	UnpackedTarball/redland/librdf/rdf_query_triples \
	UnpackedTarball/redland/librdf/rdf_serializer \
	UnpackedTarball/redland/librdf/rdf_serializer_raptor \
	UnpackedTarball/redland/librdf/rdf_statement \
	UnpackedTarball/redland/librdf/rdf_storage \
	UnpackedTarball/redland/librdf/rdf_storage_file \
	UnpackedTarball/redland/librdf/rdf_storage_list \
	UnpackedTarball/redland/librdf/rdf_storage_hashes \
	UnpackedTarball/redland/librdf/rdf_stream \
	UnpackedTarball/redland/librdf/rdf_uri \
	UnpackedTarball/redland/librdf/rdf_utf8 \
))

# vim: set noet sw=4 ts=4:
