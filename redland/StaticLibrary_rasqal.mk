# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,rasqal))

$(eval $(call gb_StaticLibrary_use_unpacked,rasqal,rasqal))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,rasqal))

$(eval $(call gb_StaticLibrary_use_external,rasqal,raptor_headers))

$(eval $(call gb_StaticLibrary_add_ldflags,rasqal,\
	/DEF:$(SRCDIR)/redland/rasqal/rasqal.def \
))

$(eval $(call gb_StaticLibrary_add_defs,rasqal,\
	-DRASQAL_INTERNAL \
	-DWIN32 \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DWIN32_EXPORTS \
	-D_MT \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,rasqal,\
	UnpackedTarball/rasqal/src/rasqal_datetime \
	UnpackedTarball/rasqal/src/rasqal_decimal \
	UnpackedTarball/rasqal/src/rasqal_engine \
	UnpackedTarball/rasqal/src/rasqal_expr \
	UnpackedTarball/rasqal/src/rasqal_feature \
	UnpackedTarball/rasqal/src/rasqal_general \
	UnpackedTarball/rasqal/src/rasqal_graph_pattern \
	UnpackedTarball/rasqal/src/rasqal_literal \
	UnpackedTarball/rasqal/src/rasqal_map \
	UnpackedTarball/rasqal/src/rasqal_query_results \
	UnpackedTarball/rasqal/src/rasqal_query \
	UnpackedTarball/rasqal/src/rasqal_raptor \
	UnpackedTarball/rasqal/src/rasqal_result_formats \
	UnpackedTarball/rasqal/src/rasqal_rowsource \
	UnpackedTarball/rasqal/src/rasqal_sparql_xml \
	UnpackedTarball/rasqal/src/rasqal_xsd_datatypes \
	UnpackedTarball/rasqal/src/rdql_lexer \
	UnpackedTarball/rasqal/src/rdql_parser \
	UnpackedTarball/rasqal/src/sparql_lexer \
	UnpackedTarball/rasqal/src/sparql_parser \
	UnpackedTarball/rasqal/src/strcasecmp \
))


# vim: set noet sw=4 ts=4:
