#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Package_Package,filter_xslt,$(SRCDIR)/filter/source/xslt))

$(eval $(call gb_Package_set_outdir,filter_xslt,$(INSTDIR)))

$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/common/copy.xsl,common/copy.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/common/math.xsl,common/math.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/common/measure_conversion.xsl,common/measure_conversion.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/ooo2ms_docpr.xsl,export/common/ooo2ms_docpr.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/spreadsheetml/formular.xsl,export/spreadsheetml/formular.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/spreadsheetml/ooo2spreadsheetml.xsl,export/spreadsheetml/ooo2spreadsheetml.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/spreadsheetml/style_mapping.xsl,export/spreadsheetml/style_mapping.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/spreadsheetml/styles.xsl,export/spreadsheetml/styles.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/spreadsheetml/table.xsl,export/spreadsheetml/table.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/uof/odf2uof_presentation.xsl,export/uof/odf2uof_presentation.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/uof/odf2uof_spreadsheet.xsl,export/uof/odf2uof_spreadsheet.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/uof/odf2uof_text.xsl,export/uof/odf2uof_text.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml.xsl,export/wordml/ooo2wordml.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_border.xsl,export/wordml/ooo2wordml_border.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_custom_draw.xsl,export/wordml/ooo2wordml_custom_draw.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_draw.xsl,export/wordml/ooo2wordml_draw.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_field.xsl,export/wordml/ooo2wordml_field.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_list.xsl,export/wordml/ooo2wordml_list.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_page.xsl,export/wordml/ooo2wordml_page.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_path.xsl,export/wordml/ooo2wordml_path.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_settings.xsl,export/wordml/ooo2wordml_settings.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_table.xsl,export/wordml/ooo2wordml_table.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/wordml/ooo2wordml_text.xsl,export/wordml/ooo2wordml_text.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/common/ms2ooo_docpr.xsl,import/common/ms2ooo_docpr.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/spreadsheetml/spreadsheetml2ooo.xsl,import/spreadsheetml/spreadsheetml2ooo.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/uof/uof2odf_presentation.xsl,import/uof/uof2odf_presentation.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/uof/uof2odf_spreadsheet.xsl,import/uof/uof2odf_spreadsheet.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/uof/uof2odf_text.xsl,import/uof/uof2odf_text.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo.xsl,import/wordml/wordml2ooo.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_custom_draw.xsl,import/wordml/wordml2ooo_custom_draw.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_draw.xsl,import/wordml/wordml2ooo_draw.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_field.xsl,import/wordml/wordml2ooo_field.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_list.xsl,import/wordml/wordml2ooo_list.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_page.xsl,import/wordml/wordml2ooo_page.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_path.xsl,import/wordml/wordml2ooo_path.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_props.xsl,import/wordml/wordml2ooo_props.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_settings.xsl,import/wordml/wordml2ooo_settings.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_table.xsl,import/wordml/wordml2ooo_table.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/import/wordml/wordml2ooo_text.xsl,import/wordml/wordml2ooo_text.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/body.xsl,odf2xhtml/export/common/body.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/styles/style_collector.xsl,odf2xhtml/export/common/styles/style_collector.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/styles/style_mapping_css.xsl,odf2xhtml/export/common/styles/style_mapping_css.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/table/table.xsl,odf2xhtml/export/common/table/table.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/table/table_cells.xsl,odf2xhtml/export/common/table/table_cells.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/table/table_columns.xsl,odf2xhtml/export/common/table/table_columns.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/table/table_rows.xsl,odf2xhtml/export/common/table/table_rows.xsl))
$(eval $(call gb_Package_add_file,filter_xslt,$(LIBO_SHARE_FOLDER)/xslt/export/common/table_of_content.xsl,odf2xhtml/export/common/table_of_content.xsl))

# vim: set noet sw=4 ts=4:
