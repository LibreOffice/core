# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rasqal))

$(eval $(call gb_Library_use_unpacked,rasqal,rasqal))

$(eval $(call gb_Library_set_warnings_disabled,rasqal))

$(eval $(call gb_Library_use_external,rasqal,raptor_headers))

$(eval $(call gb_Library_use_libraries,rasqal,raptor2))

$(eval $(call gb_Library_add_defs,rasqal,\
	-DSV_CONFIG \
	-DRASQAL_INTERNAL \
	-DWIN32 \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DWIN32_EXPORTS \
	-D_MT \
	-DHAVE_STDLIB_H \
	-DHAVE_STDINT_H \
	-DHAVE_TIME_H \
	-DHAVE_MATH_H \
	-DHAVE_FLOAT_H \
	-DHAVE___FUNCTION__ \
))

$(eval $(call gb_Library_set_include,rasqal,\
	-I$(gb_UnpackedTarball_workdir)/rasqal/libmtwist \
	-I$(gb_UnpackedTarball_workdir)/rasqal/libsv \
	-I$(gb_UnpackedTarball_workdir)/rasqal/src \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_generated_cobjects,rasqal,\
	UnpackedTarball/rasqal/libmtwist/mt \
	UnpackedTarball/rasqal/libmtwist/seed \
	UnpackedTarball/rasqal/libsv/sv \
	UnpackedTarball/rasqal/src/rasqal_algebra \
	UnpackedTarball/rasqal/src/rasqal_bindings \
	UnpackedTarball/rasqal/src/rasqal_data_graph \
	UnpackedTarball/rasqal/src/rasqal_dataset \
	UnpackedTarball/rasqal/src/rasqal_datetime \
	UnpackedTarball/rasqal/src/rasqal_decimal \
	UnpackedTarball/rasqal/src/rasqal_digest \
	UnpackedTarball/rasqal/src/rasqal_digest_md5 \
	UnpackedTarball/rasqal/src/rasqal_digest_sha1 \
	UnpackedTarball/rasqal/src/rasqal_double \
	UnpackedTarball/rasqal/src/rasqal_engine \
	UnpackedTarball/rasqal/src/rasqal_engine_algebra \
	UnpackedTarball/rasqal/src/rasqal_engine_sort \
	UnpackedTarball/rasqal/src/rasqal_expr \
	UnpackedTarball/rasqal/src/rasqal_expr_datetimes \
	UnpackedTarball/rasqal/src/rasqal_expr_evaluate \
	UnpackedTarball/rasqal/src/rasqal_expr_numerics \
	UnpackedTarball/rasqal/src/rasqal_expr_strings \
	UnpackedTarball/rasqal/src/rasqal_feature \
	UnpackedTarball/rasqal/src/rasqal_format_html \
	UnpackedTarball/rasqal/src/rasqal_format_json \
	UnpackedTarball/rasqal/src/rasqal_format_rdf \
	UnpackedTarball/rasqal/src/rasqal_format_sparql_xml \
	UnpackedTarball/rasqal/src/rasqal_format_sv \
	UnpackedTarball/rasqal/src/rasqal_format_table \
	UnpackedTarball/rasqal/src/rasqal_formula \
	UnpackedTarball/rasqal/src/rasqal_general \
	UnpackedTarball/rasqal/src/rasqal_graph_pattern \
	UnpackedTarball/rasqal/src/rasqal_iostream \
	UnpackedTarball/rasqal/src/rasqal_literal \
	UnpackedTarball/rasqal/src/rasqal_map \
	UnpackedTarball/rasqal/src/rasqal_ntriples \
	UnpackedTarball/rasqal/src/rasqal_prefix \
	UnpackedTarball/rasqal/src/rasqal_projection \
	UnpackedTarball/rasqal/src/rasqal_query \
	UnpackedTarball/rasqal/src/rasqal_query_results \
	UnpackedTarball/rasqal/src/rasqal_query_transform \
	UnpackedTarball/rasqal/src/rasqal_query_write \
	UnpackedTarball/rasqal/src/rasqal_random \
	UnpackedTarball/rasqal/src/rasqal_raptor \
	UnpackedTarball/rasqal/src/rasqal_regex \
	UnpackedTarball/rasqal/src/rasqal_result_formats \
	UnpackedTarball/rasqal/src/rasqal_results_compare \
	UnpackedTarball/rasqal/src/rasqal_row \
	UnpackedTarball/rasqal/src/rasqal_row_compatible \
	UnpackedTarball/rasqal/src/rasqal_rowsource \
	UnpackedTarball/rasqal/src/rasqal_rowsource_aggregation \
	UnpackedTarball/rasqal/src/rasqal_rowsource_assignment \
	UnpackedTarball/rasqal/src/rasqal_rowsource_bindings \
	UnpackedTarball/rasqal/src/rasqal_rowsource_distinct \
	UnpackedTarball/rasqal/src/rasqal_rowsource_empty \
	UnpackedTarball/rasqal/src/rasqal_rowsource_filter \
	UnpackedTarball/rasqal/src/rasqal_rowsource_graph \
	UnpackedTarball/rasqal/src/rasqal_rowsource_groupby \
	UnpackedTarball/rasqal/src/rasqal_rowsource_having \
	UnpackedTarball/rasqal/src/rasqal_rowsource_join \
	UnpackedTarball/rasqal/src/rasqal_rowsource_project \
	UnpackedTarball/rasqal/src/rasqal_rowsource_rowsequence \
	UnpackedTarball/rasqal/src/rasqal_rowsource_service \
	UnpackedTarball/rasqal/src/rasqal_rowsource_slice \
	UnpackedTarball/rasqal/src/rasqal_rowsource_sort \
	UnpackedTarball/rasqal/src/rasqal_rowsource_triples \
	UnpackedTarball/rasqal/src/rasqal_rowsource_union \
	UnpackedTarball/rasqal/src/rasqal_service \
	UnpackedTarball/rasqal/src/rasqal_solution_modifier \
	UnpackedTarball/rasqal/src/rasqal_sort \
	UnpackedTarball/rasqal/src/rasqal_triple \
	UnpackedTarball/rasqal/src/rasqal_triples_source \
	UnpackedTarball/rasqal/src/rasqal_update \
	UnpackedTarball/rasqal/src/rasqal_variable \
	UnpackedTarball/rasqal/src/rasqal_xsd_datatypes \
	UnpackedTarball/rasqal/src/snprintf \
	UnpackedTarball/rasqal/src/sparql_lexer \
	UnpackedTarball/rasqal/src/sparql_parser \
	UnpackedTarball/rasqal/src/timegm \
	UnpackedTarball/rasqal/src/gettimeofday \
))


# vim: set noet sw=4 ts=4:
