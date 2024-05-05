# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rdf))

$(eval $(call gb_Library_set_include,rdf, \
    -I$(gb_UnpackedTarball_workdir)/redland/src \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_unpacked,rdf,redland))

$(eval $(call gb_Library_set_warnings_disabled,rdf))

$(eval $(call gb_Library_use_libraries,rdf,\
	raptor2 \
	rasqal \
))

$(eval $(call gb_Library_use_externals,rdf,\
	raptor_headers \
	rasqal_headers \
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
	UnpackedTarball/redland/src/rdf_concepts \
	UnpackedTarball/redland/src/rdf_digest \
	UnpackedTarball/redland/src/rdf_digest_md5 \
	UnpackedTarball/redland/src/rdf_digest_sha1 \
	UnpackedTarball/redland/src/rdf_files \
	UnpackedTarball/redland/src/rdf_hash \
	UnpackedTarball/redland/src/rdf_hash_cursor \
	UnpackedTarball/redland/src/rdf_hash_memory \
	UnpackedTarball/redland/src/rdf_heuristics \
	UnpackedTarball/redland/src/rdf_init \
	UnpackedTarball/redland/src/rdf_iterator \
	UnpackedTarball/redland/src/rdf_list \
	UnpackedTarball/redland/src/rdf_log \
	UnpackedTarball/redland/src/rdf_model \
	UnpackedTarball/redland/src/rdf_model_storage \
	UnpackedTarball/redland/src/rdf_node \
	UnpackedTarball/redland/src/rdf_node_common \
	UnpackedTarball/redland/src/rdf_parser \
	UnpackedTarball/redland/src/rdf_parser_raptor \
	UnpackedTarball/redland/src/rdf_raptor \
	UnpackedTarball/redland/src/rdf_query \
	UnpackedTarball/redland/src/rdf_query_rasqal \
	UnpackedTarball/redland/src/rdf_query_results \
	UnpackedTarball/redland/src/rdf_serializer \
	UnpackedTarball/redland/src/rdf_serializer_raptor \
	UnpackedTarball/redland/src/rdf_statement \
	UnpackedTarball/redland/src/rdf_statement_common \
	UnpackedTarball/redland/src/rdf_storage \
	UnpackedTarball/redland/src/rdf_storage_file \
	UnpackedTarball/redland/src/rdf_storage_list \
	UnpackedTarball/redland/src/rdf_storage_hashes \
	UnpackedTarball/redland/src/rdf_storage_trees \
	UnpackedTarball/redland/src/rdf_stream \
	UnpackedTarball/redland/src/rdf_uri \
	UnpackedTarball/redland/src/rdf_utf8 \
))

# vim: set noet sw=4 ts=4:
