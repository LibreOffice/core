# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,orcus))

$(eval $(call gb_Library_use_unpacked,orcus,liborcus))

$(eval $(call gb_Library_use_externals,orcus,\
	boost_headers \
	boost_filesystem \
	boost_iostreams \
	boost_system \
	mdds_headers \
	zlib \
))

$(eval $(call gb_Library_set_warnings_disabled,orcus))

$(eval $(call gb_Library_set_precompiled_header,orcus,external/liborcus/inc/pch/precompiled_orcus))

$(eval $(call gb_Library_set_include,orcus,\
	-I$(call gb_UnpackedTarball_get_dir,liborcus)/include \
	-I$(call gb_UnpackedTarball_get_dir,liborcus)/src/include \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,orcus,\
	-DBOOST_ALL_NO_LIB \
	-D__ORCUS_BUILDING_DLL \
	-D__ORCUS_GNUMERIC \
	-D__ORCUS_ODS \
	-D__ORCUS_XLSX \
	-D__ORCUS_XLS_XML \
))

# Needed when building against MSVC in C++17 mode, as
# workdir/UnpackedTarball/liborcus/src/liborcus/css_document_tree.cpp uses std::unary_function:
$(eval $(call gb_Library_add_defs,orcus, \
    -D_HAS_AUTO_PTR_ETC=1 \
))

$(eval $(call gb_Library_use_libraries,orcus,\
	orcus-parser \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,orcus,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,orcus,\
	UnpackedTarball/liborcus/src/liborcus/config \
	UnpackedTarball/liborcus/src/liborcus/css_document_tree \
	UnpackedTarball/liborcus/src/liborcus/css_selector \
	UnpackedTarball/liborcus/src/liborcus/detection_result \
	UnpackedTarball/liborcus/src/liborcus/dom_tree \
	UnpackedTarball/liborcus/src/liborcus/format_detection \
	UnpackedTarball/liborcus/src/liborcus/formula_result \
	UnpackedTarball/liborcus/src/liborcus/global \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_cell_context \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_context \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_detection_handler \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_handler \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_helper \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_namespace_types \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_sheet_context \
	UnpackedTarball/liborcus/src/liborcus/gnumeric_tokens \
	UnpackedTarball/liborcus/src/liborcus/info \
	UnpackedTarball/liborcus/src/liborcus/interface \
	UnpackedTarball/liborcus/src/liborcus/json_document_tree \
	UnpackedTarball/liborcus/src/liborcus/json_map_tree \
	UnpackedTarball/liborcus/src/liborcus/json_structure_mapper \
	UnpackedTarball/liborcus/src/liborcus/json_structure_tree \
	UnpackedTarball/liborcus/src/liborcus/json_util \
	UnpackedTarball/liborcus/src/liborcus/measurement \
	UnpackedTarball/liborcus/src/liborcus/odf_helper \
	UnpackedTarball/liborcus/src/liborcus/odf_namespace_types \
	UnpackedTarball/liborcus/src/liborcus/odf_number_formatting_context \
	UnpackedTarball/liborcus/src/liborcus/odf_para_context \
	UnpackedTarball/liborcus/src/liborcus/odf_styles \
	UnpackedTarball/liborcus/src/liborcus/odf_styles_context \
	UnpackedTarball/liborcus/src/liborcus/odf_tokens \
	UnpackedTarball/liborcus/src/liborcus/ods_content_xml_context \
	UnpackedTarball/liborcus/src/liborcus/ods_content_xml_handler \
	UnpackedTarball/liborcus/src/liborcus/ods_dde_links_context \
	UnpackedTarball/liborcus/src/liborcus/ods_session_data \
	UnpackedTarball/liborcus/src/liborcus/ooxml_content_types \
	UnpackedTarball/liborcus/src/liborcus/ooxml_global \
	UnpackedTarball/liborcus/src/liborcus/ooxml_namespace_types \
	UnpackedTarball/liborcus/src/liborcus/ooxml_schemas \
	UnpackedTarball/liborcus/src/liborcus/ooxml_tokens \
	UnpackedTarball/liborcus/src/liborcus/ooxml_types \
	UnpackedTarball/liborcus/src/liborcus/opc_context \
	UnpackedTarball/liborcus/src/liborcus/opc_reader \
	UnpackedTarball/liborcus/src/liborcus/orcus_csv \
	UnpackedTarball/liborcus/src/liborcus/orcus_gnumeric \
	UnpackedTarball/liborcus/src/liborcus/orcus_import_ods \
	UnpackedTarball/liborcus/src/liborcus/orcus_import_xlsx \
	UnpackedTarball/liborcus/src/liborcus/orcus_json \
	UnpackedTarball/liborcus/src/liborcus/orcus_ods \
	UnpackedTarball/liborcus/src/liborcus/orcus_xls_xml \
	UnpackedTarball/liborcus/src/liborcus/orcus_xlsx \
	UnpackedTarball/liborcus/src/liborcus/orcus_xml \
	UnpackedTarball/liborcus/src/liborcus/orcus_xml_impl \
	UnpackedTarball/liborcus/src/liborcus/orcus_xml_map_def \
	UnpackedTarball/liborcus/src/liborcus/session_context \
	UnpackedTarball/liborcus/src/liborcus/spreadsheet_iface_util \
	UnpackedTarball/liborcus/src/liborcus/spreadsheet_impl_types \
	UnpackedTarball/liborcus/src/liborcus/spreadsheet_interface \
	UnpackedTarball/liborcus/src/liborcus/spreadsheet_types \
	UnpackedTarball/liborcus/src/liborcus/string_helper \
	UnpackedTarball/liborcus/src/liborcus/xls_xml_context \
	UnpackedTarball/liborcus/src/liborcus/xls_xml_detection_handler \
	UnpackedTarball/liborcus/src/liborcus/xls_xml_handler \
	UnpackedTarball/liborcus/src/liborcus/xls_xml_namespace_types \
	UnpackedTarball/liborcus/src/liborcus/xls_xml_tokens \
	UnpackedTarball/liborcus/src/liborcus/xlsx_autofilter_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_conditional_format_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_drawing_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_handler \
	UnpackedTarball/liborcus/src/liborcus/xlsx_helper \
	UnpackedTarball/liborcus/src/liborcus/xlsx_pivot_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_revision_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_session_data \
	UnpackedTarball/liborcus/src/liborcus/xlsx_sheet_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_table_context \
	UnpackedTarball/liborcus/src/liborcus/xlsx_types \
	UnpackedTarball/liborcus/src/liborcus/xlsx_workbook_context \
	UnpackedTarball/liborcus/src/liborcus/xml_context_base \
	UnpackedTarball/liborcus/src/liborcus/xml_context_global \
	UnpackedTarball/liborcus/src/liborcus/xml_element_validator \
	UnpackedTarball/liborcus/src/liborcus/xml_empty_context \
	UnpackedTarball/liborcus/src/liborcus/xml_map_tree \
	UnpackedTarball/liborcus/src/liborcus/xml_simple_stream_handler \
	UnpackedTarball/liborcus/src/liborcus/xml_stream_handler \
	UnpackedTarball/liborcus/src/liborcus/xml_stream_parser \
	UnpackedTarball/liborcus/src/liborcus/xml_structure_mapper \
	UnpackedTarball/liborcus/src/liborcus/xml_structure_tree \
	UnpackedTarball/liborcus/src/liborcus/xml_util \
	UnpackedTarball/liborcus/src/liborcus/xpath_parser \
	UnpackedTarball/liborcus/src/liborcus/yaml_document_tree \
))

# vim: set noet sw=4 ts=4:
