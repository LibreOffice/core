/*************************************************************************
 *
 *  $RCSfile: xmlimprt.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-26 13:25:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlkywd.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmetai.hxx>
#include <sfx2/objsh.hxx>
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>

#include "xmlimprt.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "xmlbodyi.hxx"
#include "xmlstyli.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCELLRANGE_HPP_
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMERGEABLE_HPP_
#include <com/sun/star/util/XMergeable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_CELLINSERTMODE_HPP_
#include <com/sun/star/sheet/CellInsertMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEMOVEMENT_HPP_
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

using namespace com::sun::star;

//----------------------------------------------------------------------------

sal_Char __READONLY_DATA sXML_np__text[] = "text";
sal_Char __READONLY_DATA sXML_np__table[] = "table";

//----------------------------------------------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aDocTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_styles,            XML_TOK_DOC_STYLES      },
    { XML_NAMESPACE_OFFICE, sXML_automatic_styles,  XML_TOK_DOC_AUTOSTYLES  },
    { XML_NAMESPACE_OFFICE, sXML_master_styles,     XML_TOK_DOC_MASTERSTYLES},
    { XML_NAMESPACE_OFFICE, sXML_meta,              XML_TOK_DOC_META        },
    { XML_NAMESPACE_OFFICE, sXML_body,              XML_TOK_DOC_BODY        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_style,             XML_TOK_STYLES_STYLE            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_name,              XML_TOK_STYLES_STYLE_NAME   },
    { XML_NAMESPACE_STYLE,  sXML_family,            XML_TOK_STYLES_STYLE_FAMILY },
    { XML_NAMESPACE_STYLE,  sXML_parent_style_name, XML_TOK_STYLES_STYLE_PARENT_STYLE_NAME  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_properties,        XML_TOK_STYLE_PROPERTIES    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aBodyTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_content_validations,    XML_TOK_BODY_CONTENT_VALIDATIONS    },
    { XML_NAMESPACE_TABLE, sXML_label_ranges,           XML_TOK_BODY_LABEL_RANGES           },
    { XML_NAMESPACE_TABLE, sXML_table,                  XML_TOK_BODY_TABLE                  },
    { XML_NAMESPACE_TABLE, sXML_named_expressions,      XML_TOK_BODY_NAMED_EXPRESSIONS      },
    { XML_NAMESPACE_TABLE, sXML_database_ranges,        XML_TOK_BODY_DATABASE_RANGES        },
    { XML_NAMESPACE_TABLE, sXML_database_range,         XML_TOK_BODY_DATABASE_RANGE         },
    { XML_NAMESPACE_TABLE, sXML_data_pilot_tables,      XML_TOK_BODY_DATA_PILOT_TABLES      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_content_validation, XML_TOK_CONTENT_VALIDATION  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_help_message,   XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE    },
    { XML_NAMESPACE_TABLE, sXML_error_message,  XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE   },
    { XML_NAMESPACE_TABLE, sXML_error_macro,    XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,               XML_TOK_CONTENT_VALIDATION_NAME                 },
    { XML_NAMESPACE_TABLE, sXML_condition,          XML_TOK_CONTENT_VALIDATION_CONDITION            },
    { XML_NAMESPACE_TABLE, sXML_base_cell_address,  XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS    },
    { XML_NAMESPACE_TABLE, sXML_allow_empty_cell,   XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationMessageElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_p,   XML_TOK_P   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationHelpMessageAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_title,  XML_TOK_HELP_MESSAGE_ATTR_TITLE     },
    { XML_NAMESPACE_TABLE, sXML_display,    XML_TOK_HELP_MESSAGE_ATTR_DISPLAY   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationErrorMessageAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_title,          XML_TOK_ERROR_MESSAGE_ATTR_TITLE        },
    { XML_NAMESPACE_TABLE, sXML_display,            XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY      },
    { XML_NAMESPACE_TABLE, sXML_message_type,   XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationErrorMacroAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,   XML_TOK_ERROR_MACRO_ATTR_NAME       },
    { XML_NAMESPACE_TABLE, sXML_execute,    XML_TOK_ERROR_MACRO_ATTR_EXECUTE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLabelRangesElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_label_range,    XML_TOK_LABEL_RANGE_ELEM    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLabelRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_label_cell_range_address,   XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE    },
    { XML_NAMESPACE_TABLE, sXML_data_cell_range_address,    XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE     },
    { XML_NAMESPACE_TABLE, sXML_orientation,                XML_TOK_LABEL_RANGE_ATTR_ORIENTATION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_table_header_columns,   XML_TOK_TABLE_HEADER_COLS   },
    { XML_NAMESPACE_TABLE, sXML_table_columns,          XML_TOK_TABLE_COLS          },
    { XML_NAMESPACE_TABLE, sXML_table_column,           XML_TOK_TABLE_COL           },
    { XML_NAMESPACE_TABLE, sXML_table_header_rows,      XML_TOK_TABLE_HEADER_ROWS   },
    { XML_NAMESPACE_TABLE, sXML_table_rows,             XML_TOK_TABLE_ROWS          },
    { XML_NAMESPACE_TABLE, sXML_table_row,              XML_TOK_TABLE_ROW           },
    { XML_NAMESPACE_TABLE, sXML_scenario,               XML_TOK_TABLE_SCENARIO      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_table_row,              XML_TOK_TABLE_ROWS_ROW          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableColsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_table_column,           XML_TOK_TABLE_COLS_COL          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,                       XML_TOK_TABLE_NAME              },
    { XML_NAMESPACE_TABLE, sXML_style_name,                 XML_TOK_TABLE_STYLE_NAME        },
    { XML_NAMESPACE_TABLE, sXML_use_cell_protection,        XML_TOK_TABLE_PROTECTION        },
    { XML_NAMESPACE_TABLE, sXML_print_ranges,               XML_TOK_TABLE_PRINT_RANGES      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableScenarioAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_display_border,     XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER  },
    { XML_NAMESPACE_TABLE, sXML_border_color,       XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR    },
    { XML_NAMESPACE_TABLE, sXML_copy_back,          XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK       },
    { XML_NAMESPACE_TABLE, sXML_copy_styles,        XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES     },
    { XML_NAMESPACE_TABLE, sXML_copy_formulas,      XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS   },
    { XML_NAMESPACE_TABLE, sXML_is_active,          XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE       },
    { XML_NAMESPACE_TABLE, sXML_scenario_ranges,    XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES },
    { XML_NAMESPACE_TABLE, sXML_comment,            XML_TOK_TABLE_SCENARIO_ATTR_COMMENT         },
    XML_TOKEN_MAP_END
};


static __FAR_DATA SvXMLTokenMapEntry aTableColAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_style_name,                 XML_TOK_TABLE_COL_ATTR_STYLE_NAME       },
    { XML_NAMESPACE_TABLE, sXML_number_columns_repeated,    XML_TOK_TABLE_COL_ATTR_REPEATED         },
    { XML_NAMESPACE_TABLE, sXML_visibility,                 XML_TOK_TABLE_COL_ATTR_VISIBILITY       },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_table_cell,         XML_TOK_TABLE_ROW_CELL              },
    { XML_NAMESPACE_TABLE, sXML_covered_table_cell, XML_TOK_TABLE_ROW_COVERED_CELL      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_style_name,                 XML_TOK_TABLE_ROW_ATTR_STYLE_NAME           },
    { XML_NAMESPACE_TABLE, sXML_visibility,                 XML_TOK_TABLE_ROW_ATTR_VISIBILITY           },
    { XML_NAMESPACE_TABLE, sXML_number_rows_repeated,       XML_TOK_TABLE_ROW_ATTR_REPEATED             },
//  { XML_NAMESPACE_TABLE, sXML_use_optimal_height,         XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowCellTokenMap[] =
{
    { XML_NAMESPACE_TEXT,   sXML_p,                 XML_TOK_TABLE_ROW_CELL_P                    },
    { XML_NAMESPACE_TABLE,  sXML_sub_table,         XML_TOK_TABLE_ROW_CELL_SUBTABLE             },
    { XML_NAMESPACE_OFFICE, sXML_annotation,        XML_TOK_TABLE_ROW_CELL_ANNOTATION           },
    { XML_NAMESPACE_TABLE,  sXML_cell_range_source, XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowCellAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_style_name,                     XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME              },
    { XML_NAMESPACE_TABLE, sXML_content_validation_name,        XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME },
    { XML_NAMESPACE_TABLE, sXML_number_rows_spanned,            XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS            },
    { XML_NAMESPACE_TABLE, sXML_number_columns_spanned,         XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS            },
    { XML_NAMESPACE_TABLE, sXML_number_matrix_columns_spanned,  XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS     },
    { XML_NAMESPACE_TABLE, sXML_number_matrix_rows_spanned,     XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS     },
    { XML_NAMESPACE_TABLE, sXML_number_columns_repeated,        XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED                },
    { XML_NAMESPACE_TABLE, sXML_value_type,                     XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE              },
    { XML_NAMESPACE_TABLE, sXML_value,                          XML_TOK_TABLE_ROW_CELL_ATTR_VALUE                   },
    { XML_NAMESPACE_TABLE, sXML_date_value,                     XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE              },
    { XML_NAMESPACE_TABLE, sXML_time_value,                     XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE              },
    { XML_NAMESPACE_TABLE, sXML_string_value,                   XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE            },
    { XML_NAMESPACE_TABLE, sXML_boolean_value,                  XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE           },
    { XML_NAMESPACE_TABLE, sXML_formula,                        XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA                 },
    { XML_NAMESPACE_TABLE, sXML_currency,                       XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY                },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableAnnotationAttrTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_author,                XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR                },
    { XML_NAMESPACE_OFFICE, sXML_create_date,           XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE           },
    { XML_NAMESPACE_OFFICE, sXML_create_date_string,    XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING    },
    { XML_NAMESPACE_OFFICE, sXML_display,               XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY               },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableCellRangeSourceAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  sXML_name,                  XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME           },
    { XML_NAMESPACE_XLINK,  sXML_href,                  XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF           },
    { XML_NAMESPACE_TABLE,  sXML_filter_name,           XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME    },
    { XML_NAMESPACE_TABLE,  sXML_filter_options,        XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS },
    { XML_NAMESPACE_TABLE,  sXML_last_column_spanned,   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN    },
    { XML_NAMESPACE_TABLE,  sXML_last_row_spanned,      XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW       },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedExpressionsTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_named_range,            XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE           },
    { XML_NAMESPACE_TABLE, sXML_named_expression,       XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,               XML_TOK_NAMED_RANGE_ATTR_NAME               },
    { XML_NAMESPACE_TABLE, sXML_cell_range_address, XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS },
    { XML_NAMESPACE_TABLE, sXML_base_cell_address,  XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS  },
    { XML_NAMESPACE_TABLE, sXML_range_usable_as,    XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedExpressionAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,               XML_TOK_NAMED_EXPRESSION_ATTR_NAME              },
    { XML_NAMESPACE_TABLE, sXML_base_cell_address,  XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS },
    { XML_NAMESPACE_TABLE, sXML_expression,         XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangesTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_range, XML_TOK_DATABASE_RANGE      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_source_sql,    XML_TOK_DATABASE_RANGE_SOURCE_SQL       },
    { XML_NAMESPACE_TABLE, sXML_database_source_table,  XML_TOK_DATABASE_RANGE_SOURCE_TABLE     },
    { XML_NAMESPACE_TABLE, sXML_database_source_query,  XML_TOK_DATABASE_RANGE_SOURCE_QUERY     },
    { XML_NAMESPACE_TABLE, sXML_filter,                 XML_TOK_FILTER                          },
    { XML_NAMESPACE_TABLE, sXML_sort,                   XML_TOK_SORT                            },
    { XML_NAMESPACE_TABLE, sXML_subtotal_rules,         XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,                   XML_TOK_DATABASE_RANGE_ATTR_NAME                    },
    { XML_NAMESPACE_TABLE, sXML_is_selection,           XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION            },
    { XML_NAMESPACE_TABLE, sXML_on_update_keep_styles,  XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES   },
    { XML_NAMESPACE_TABLE, sXML_on_update_keep_size,    XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE     },
    { XML_NAMESPACE_TABLE, sXML_has_persistent_data,    XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA     },
    { XML_NAMESPACE_TABLE, sXML_orientation,            XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION             },
    { XML_NAMESPACE_TABLE, sXML_contains_header,        XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER         },
    { XML_NAMESPACE_TABLE, sXML_display_filter_buttons, XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS  },
    { XML_NAMESPACE_TABLE, sXML_target_range_address,   XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceSQLAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_name,          XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME       },
    { XML_NAMESPACE_TABLE, sXML_sql_statement,          XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT       },
    { XML_NAMESPACE_TABLE, sXML_parse_sql_statement,    XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_name,  XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME     },
    { XML_NAMESPACE_TABLE, sXML_table_name,     XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceQueryAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_name,  XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME     },
    { XML_NAMESPACE_TABLE, sXML_query_name,     XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_filter_and,     XML_TOK_FILTER_AND          },
    { XML_NAMESPACE_TABLE, sXML_filter_or,          XML_TOK_FILTER_OR           },
    { XML_NAMESPACE_TABLE, sXML_filter_condition,   XML_TOK_FILTER_CONDITION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_target_range_address,           XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS            },
    { XML_NAMESPACE_TABLE, sXML_condition_source_range_address, XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS  },
    { XML_NAMESPACE_TABLE, sXML_condition_source,               XML_TOK_FILTER_ATTR_CONDITION_SOURCE                },
    { XML_NAMESPACE_TABLE, sXML_display_duplicates,             XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES              },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterConditionAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_field_number,   XML_TOK_CONDITION_ATTR_FIELD_NUMBER     },
    { XML_NAMESPACE_TABLE, sXML_case_sensitive, XML_TOK_CONDITION_ATTR_CASE_SENSITIVE   },
    { XML_NAMESPACE_TABLE, sXML_data_type,      XML_TOK_CONDITION_ATTR_DATA_TYPE        },
    { XML_NAMESPACE_TABLE, sXML_value,          XML_TOK_CONDITION_ATTR_VALUE            },
    { XML_NAMESPACE_TABLE, sXML_operator,       XML_TOK_CONDITION_ATTR_OPERATOR         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_sort_by,    XML_TOK_SORT_SORT_BY    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_bind_styles_to_content, XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT    },
    { XML_NAMESPACE_TABLE, sXML_target_range_address,   XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS      },
    { XML_NAMESPACE_TABLE, sXML_case_sensitive,         XML_TOK_SORT_ATTR_CASE_SENSITIVE            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortSortByAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_field_number,   XML_TOK_SORT_BY_ATTR_FIELD_NUMBER   },
    { XML_NAMESPACE_TABLE, sXML_data_type,      XML_TOK_SORT_BY_ATTR_DATA_TYPE      },
    { XML_NAMESPACE_TABLE, sXML_order,          XML_TOK_SORT_BY_ATTR_ORDER          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_sort_groups,    XML_TOK_SUBTOTAL_RULES_SORT_GROUPS      },
    { XML_NAMESPACE_TABLE, sXML_subtotal_rule,  XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_bind_styles_to_content,         XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT      },
    { XML_NAMESPACE_TABLE, sXML_case_sensitive,                 XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE              },
    { XML_NAMESPACE_TABLE, sXML_page_breaks_on_group_change,    XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSortGroupsAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_type,  XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE  },
    { XML_NAMESPACE_TABLE, sXML_order,      XML_TOK_SORT_GROUPS_ATTR_ORDER      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_subtotal_field, XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_group_by_field_number,  XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRuleSubTotalFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_field_number,   XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER    },
    { XML_NAMESPACE_TABLE, sXML_function,       XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTablesElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_pilot_table,   XML_TOK_DATA_PILOT_TABLE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,                   XML_TOK_DATA_PILOT_TABLE_ATTR_NAME                  },
    { XML_NAMESPACE_TABLE, sXML_application_data,       XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA      },
    { XML_NAMESPACE_TABLE, sXML_grand_total,            XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL           },
    { XML_NAMESPACE_TABLE, sXML_ignore_empty_rows,      XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS     },
    { XML_NAMESPACE_TABLE, sXML_identify_categories,    XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES   },
    { XML_NAMESPACE_TABLE, sXML_target_range_address,   XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS  },
    { XML_NAMESPACE_TABLE, sXML_buttons,                XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS               },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_database_source_sql,    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL        },
    { XML_NAMESPACE_TABLE, sXML_database_source_table,  XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE      },
    { XML_NAMESPACE_TABLE, sXML_database_source_query,  XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY      },
    { XML_NAMESPACE_TABLE, sXML_source_service,         XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE    },
    { XML_NAMESPACE_TABLE, sXML_source_cell_range,      XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE },
    { XML_NAMESPACE_TABLE, sXML_data_pilot_field,       XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceServiceAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,                   XML_TOK_SOURCE_SERVICE_ATTR_NAME                },
    { XML_NAMESPACE_TABLE, sXML_source_name,            XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME         },
    { XML_NAMESPACE_TABLE, sXML_object_name,            XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME         },
    { XML_NAMESPACE_TABLE, sXML_username,               XML_TOK_SOURCE_SERVICE_ATTR_USERNAME            },
    { XML_NAMESPACE_TABLE, sXML_passwort,               XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceCellRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_cell_range_address,     XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceCellRangeElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_filter,     XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_source_field_name,      XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME     },
    { XML_NAMESPACE_TABLE, sXML_is_data_layout_field,   XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD  },
    { XML_NAMESPACE_TABLE, sXML_function,               XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION              },
    { XML_NAMESPACE_TABLE, sXML_orientation,            XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION           },
    { XML_NAMESPACE_TABLE, sXML_used_hierarchy,         XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotFieldElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_pilot_level,       XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotLevelAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_display_empty,          XML_TOK_DATA_PILOT_LEVEL_ATTR_DISPLAY_EMPTY         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotLevelElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_pilot_subtotals,   XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS  },
    { XML_NAMESPACE_TABLE, sXML_data_pilot_members,     XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotSubTotalsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_pilot_subtotal,    XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotSubTotalAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_function,               XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION           },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotMembersElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_data_pilot_member,      XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotMemberAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, sXML_name,                   XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME                 },
    { XML_NAMESPACE_TABLE, sXML_display,                XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY              },
    { XML_NAMESPACE_TABLE, sXML_display_details,        XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_DETAILS      },
    XML_TOKEN_MAP_END
};


class ScXMLDocContext_Impl : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDocContext_Impl( ScXMLImport& rImport,
                          USHORT nPrfx,
                             const OUString& rLName,
                             const uno::Reference<xml::sax::XAttributeList>& xAttrList );
    virtual ~ScXMLDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList );
};

ScXMLDocContext_Impl::ScXMLDocContext_Impl( ScXMLImport& rImport, USHORT nPrfx,
                                                 const OUString& rLName,
                                             const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{

}

ScXMLDocContext_Impl::~ScXMLDocContext_Impl()
{
}

SvXMLImportContext *ScXMLDocContext_Impl::CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDocElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_DOC_STYLES:
        pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, sal_False);
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, sal_True);
        break;
        break;
    case XML_TOK_DOC_MASTERSTYLES:
        pContext = new ScXMLMasterStylesContext( GetImport(), nPrefix, rLocalName,
                                              xAttrList );
        break;
    case XML_TOK_DOC_META:
        pContext = GetScImport().CreateMetaContext( rLocalName );
        break;
    case XML_TOK_DOC_BODY:
        pContext = GetScImport().CreateBodyContext( rLocalName );
        break;
    }

    if(!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

//----------------------------------------------------------------------------

const SvXMLTokenMap& ScXMLImport::GetDocElemTokenMap()
{
    if( !pDocElemTokenMap )
        pDocElemTokenMap = new SvXMLTokenMap( aDocTokenMap );

    return *pDocElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetStylesElemTokenMap()
{
    if( !pStylesElemTokenMap )
        pStylesElemTokenMap = new SvXMLTokenMap( aStylesTokenMap );

    return *pStylesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetStylesAttrTokenMap()
{
    if( !pStylesAttrTokenMap )
        pStylesAttrTokenMap = new SvXMLTokenMap( aStylesAttrTokenMap );

    return *pStylesAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetStyleElemTokenMap()
{
    if( !pStyleElemTokenMap )
        pStyleElemTokenMap = new SvXMLTokenMap( aStyleTokenMap );

    return *pStyleElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetBodyElemTokenMap()
{
    if( !pBodyElemTokenMap )
        pBodyElemTokenMap = new SvXMLTokenMap( aBodyTokenMap );
    return *pBodyElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationsElemTokenMap()
{
    if( !pContentValidationsElemTokenMap )
        pContentValidationsElemTokenMap = new SvXMLTokenMap( aContentValidationsElemTokenMap );
    return *pContentValidationsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationElemTokenMap()
{
    if( !pContentValidationElemTokenMap )
        pContentValidationElemTokenMap = new SvXMLTokenMap( aContentValidationElemTokenMap );
    return *pContentValidationElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationAttrTokenMap()
{
    if( !pContentValidationAttrTokenMap )
        pContentValidationAttrTokenMap = new SvXMLTokenMap( aContentValidationAttrTokenMap );
    return *pContentValidationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationMessageElemTokenMap()
{
    if( !pContentValidationMessageElemTokenMap )
        pContentValidationMessageElemTokenMap = new SvXMLTokenMap( aContentValidationMessageElemTokenMap );
    return *pContentValidationMessageElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationHelpMessageAttrTokenMap()
{
    if( !pContentValidationHelpMessageAttrTokenMap )
        pContentValidationHelpMessageAttrTokenMap = new SvXMLTokenMap( aContentValidationHelpMessageAttrTokenMap );
    return *pContentValidationHelpMessageAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationErrorMessageAttrTokenMap()
{
    if( !pContentValidationErrorMessageAttrTokenMap )
        pContentValidationErrorMessageAttrTokenMap = new SvXMLTokenMap( aContentValidationErrorMessageAttrTokenMap );
    return *pContentValidationErrorMessageAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationErrorMacroAttrTokenMap()
{
    if( !pContentValidationErrorMacroAttrTokenMap )
        pContentValidationErrorMacroAttrTokenMap = new SvXMLTokenMap( aContentValidationErrorMacroAttrTokenMap );
    return *pContentValidationErrorMacroAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetLabelRangesElemTokenMap()
{
    if( !pLabelRangesElemTokenMap )
        pLabelRangesElemTokenMap = new SvXMLTokenMap( aLabelRangesElemTokenMap );
    return *pLabelRangesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetLabelRangeAttrTokenMap()
{
    if( !pLabelRangeAttrTokenMap )
        pLabelRangeAttrTokenMap = new SvXMLTokenMap( aLabelRangeAttrTokenMap );
    return *pLabelRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableElemTokenMap()
{
    if( !pTableElemTokenMap )
        pTableElemTokenMap = new SvXMLTokenMap( aTableTokenMap );
    return *pTableElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowsElemTokenMap()
{
    if( !pTableRowsElemTokenMap )
        pTableRowsElemTokenMap = new SvXMLTokenMap( aTableRowsElemTokenMap );
    return *pTableRowsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableColsElemTokenMap()
{
    if( !pTableColsElemTokenMap )
        pTableColsElemTokenMap = new SvXMLTokenMap( aTableColsElemTokenMap );
    return *pTableColsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableAttrTokenMap()
{
    if( !pTableAttrTokenMap )
        pTableAttrTokenMap = new SvXMLTokenMap( aTableAttrTokenMap );
    return *pTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableScenarioAttrTokenMap()
{
    if( !pTableScenarioAttrTokenMap )
        pTableScenarioAttrTokenMap = new SvXMLTokenMap( aTableScenarioAttrTokenMap );
    return *pTableScenarioAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableColAttrTokenMap()
{
    if( !pTableColAttrTokenMap )
        pTableColAttrTokenMap = new SvXMLTokenMap( aTableColAttrTokenMap );
    return *pTableColAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowElemTokenMap()
{
    if( !pTableRowElemTokenMap )
        pTableRowElemTokenMap = new SvXMLTokenMap( aTableRowTokenMap );
    return *pTableRowElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowAttrTokenMap()
{
    if( !pTableRowAttrTokenMap )
        pTableRowAttrTokenMap = new SvXMLTokenMap( aTableRowAttrTokenMap );
    return *pTableRowAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowCellElemTokenMap()
{
    if( !pTableRowCellElemTokenMap )
        pTableRowCellElemTokenMap = new SvXMLTokenMap( aTableRowCellTokenMap );
    return *pTableRowCellElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowCellAttrTokenMap()
{
    if( !pTableRowCellAttrTokenMap )
        pTableRowCellAttrTokenMap = new SvXMLTokenMap( aTableRowCellAttrTokenMap );
    return *pTableRowCellAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableAnnotationAttrTokenMap()
{
    if( !pTableAnnotationAttrTokenMap )
        pTableAnnotationAttrTokenMap = new SvXMLTokenMap( aTableAnnotationAttrTokenMap );
    return *pTableAnnotationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableCellRangeSourceAttrTokenMap()
{
    if( !pTableCellRangeSourceAttrTokenMap )
        pTableCellRangeSourceAttrTokenMap = new SvXMLTokenMap( aTableCellRangeSourceAttrTokenMap );
    return *pTableCellRangeSourceAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedExpressionsElemTokenMap()
{
    if( !pNamedExpressionsElemTokenMap )
        pNamedExpressionsElemTokenMap = new SvXMLTokenMap( aNamedExpressionsTokenMap );
    return *pNamedExpressionsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedRangeAttrTokenMap()
{
    if( !pNamedRangeAttrTokenMap )
        pNamedRangeAttrTokenMap = new SvXMLTokenMap( aNamedRangeAttrTokenMap );
    return *pNamedRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedExpressionAttrTokenMap()
{
    if( !pNamedExpressionAttrTokenMap )
        pNamedExpressionAttrTokenMap = new SvXMLTokenMap( aNamedExpressionAttrTokenMap );
    return *pNamedExpressionAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangesElemTokenMap()
{
    if( !pDatabaseRangesElemTokenMap )
        pDatabaseRangesElemTokenMap = new SvXMLTokenMap( aDatabaseRangesTokenMap );
    return *pDatabaseRangesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeElemTokenMap()
{
    if( !pDatabaseRangeElemTokenMap )
        pDatabaseRangeElemTokenMap = new SvXMLTokenMap( aDatabaseRangeTokenMap );
    return *pDatabaseRangeElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeAttrTokenMap()
{
    if( !pDatabaseRangeAttrTokenMap )
        pDatabaseRangeAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeAttrTokenMap );
    return *pDatabaseRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceSQLAttrTokenMap()
{
    if( !pDatabaseRangeSourceSQLAttrTokenMap )
        pDatabaseRangeSourceSQLAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceSQLAttrTokenMap );
    return *pDatabaseRangeSourceSQLAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceTableAttrTokenMap()
{
    if( !pDatabaseRangeSourceTableAttrTokenMap )
        pDatabaseRangeSourceTableAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceTableAttrTokenMap );
    return *pDatabaseRangeSourceTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceQueryAttrTokenMap()
{
    if( !pDatabaseRangeSourceQueryAttrTokenMap )
        pDatabaseRangeSourceQueryAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceQueryAttrTokenMap );
    return *pDatabaseRangeSourceQueryAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterElemTokenMap()
{
    if( !pFilterElemTokenMap )
        pFilterElemTokenMap = new SvXMLTokenMap( aFilterTokenMap );
    return *pFilterElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterAttrTokenMap()
{
    if( !pFilterAttrTokenMap )
        pFilterAttrTokenMap = new SvXMLTokenMap( aFilterAttrTokenMap );
    return *pFilterAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterConditionAttrTokenMap()
{
    if( !pFilterConditionAttrTokenMap )
        pFilterConditionAttrTokenMap = new SvXMLTokenMap( aFilterConditionAttrTokenMap );
    return *pFilterConditionAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortElemTokenMap()
{
    if( !pSortElemTokenMap )
        pSortElemTokenMap = new SvXMLTokenMap( aSortTokenMap );
    return *pSortElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortAttrTokenMap()
{
    if( !pSortAttrTokenMap )
        pSortAttrTokenMap = new SvXMLTokenMap( aSortAttrTokenMap );
    return *pSortAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortSortByAttrTokenMap()
{
    if( !pSortSortByAttrTokenMap )
        pSortSortByAttrTokenMap = new SvXMLTokenMap( aSortSortByAttrTokenMap );
    return *pSortSortByAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSubTotalRulesElemTokenMap()
{
    if( !pDatabaseRangeSubTotalRulesElemTokenMap )
        pDatabaseRangeSubTotalRulesElemTokenMap = new SvXMLTokenMap( aDatabaseRangeSubTotalRulesTokenMap );
    return *pDatabaseRangeSubTotalRulesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSubTotalRulesAttrTokenMap()
{
    if( !pDatabaseRangeSubTotalRulesAttrTokenMap )
        pDatabaseRangeSubTotalRulesAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSubTotalRulesAttrTokenMap );
    return *pDatabaseRangeSubTotalRulesAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSortGroupsAttrTokenMap()
{
    if( !pSubTotalRulesSortGroupsAttrTokenMap )
        pSubTotalRulesSortGroupsAttrTokenMap = new SvXMLTokenMap( aSubTotalRulesSortGroupsAttrTokenMap );
    return *pSubTotalRulesSortGroupsAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSubTotalRuleElemTokenMap()
{
    if( !pSubTotalRulesSubTotalRuleElemTokenMap )
        pSubTotalRulesSubTotalRuleElemTokenMap = new SvXMLTokenMap( aSubTotalRulesSubTotalRuleTokenMap );
    return *pSubTotalRulesSubTotalRuleElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSubTotalRuleAttrTokenMap()
{
    if( !pSubTotalRulesSubTotalRuleAttrTokenMap )
        pSubTotalRulesSubTotalRuleAttrTokenMap = new SvXMLTokenMap( aSubTotalRulesSubTotalRuleAttrTokenMap );
    return *pSubTotalRulesSubTotalRuleAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRuleSubTotalFieldAttrTokenMap()
{
    if( !pSubTotalRuleSubTotalFieldAttrTokenMap )
        pSubTotalRuleSubTotalFieldAttrTokenMap = new SvXMLTokenMap( aSubTotalRuleSubTotalFieldAttrTokenMap );
    return *pSubTotalRuleSubTotalFieldAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTablesElemTokenMap()
{
    if( !pDataPilotTablesElemTokenMap )
        pDataPilotTablesElemTokenMap = new SvXMLTokenMap( aDataPilotTablesElemTokenMap );
    return *pDataPilotTablesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableAttrTokenMap()
{
    if( !pDataPilotTableAttrTokenMap )
        pDataPilotTableAttrTokenMap = new SvXMLTokenMap( aDataPilotTableAttrTokenMap );
    return *pDataPilotTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableElemTokenMap()
{
    if( !pDataPilotTableElemTokenMap )
        pDataPilotTableElemTokenMap = new SvXMLTokenMap( aDataPilotTableElemTokenMap );
    return *pDataPilotTableElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceServiceAttrTokenMap()
{
    if( !pDataPilotTableSourceServiceAttrTokenMap )
        pDataPilotTableSourceServiceAttrTokenMap = new SvXMLTokenMap( aDataPilotTableSourceServiceAttrTokenMap );
    return *pDataPilotTableSourceServiceAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceCellRangeAttrTokenMap()
{
    if( !pDataPilotTableSourceCellRangeAttrTokenMap )
        pDataPilotTableSourceCellRangeAttrTokenMap = new SvXMLTokenMap( aDataPilotTableSourceCellRangeAttrTokenMap );
    return *pDataPilotTableSourceCellRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceCellRangeElemTokenMap()
{
    if( !pDataPilotTableSourceCellRangeElemTokenMap )
        pDataPilotTableSourceCellRangeElemTokenMap = new SvXMLTokenMap( aDataPilotTableSourceCellRangeElemTokenMap );
    return *pDataPilotTableSourceCellRangeElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotFieldAttrTokenMap()
{
    if( !pDataPilotFieldAttrTokenMap )
        pDataPilotFieldAttrTokenMap = new SvXMLTokenMap( aDataPilotFieldAttrTokenMap );
    return *pDataPilotFieldAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotFieldElemTokenMap()
{
    if( !pDataPilotFieldElemTokenMap )
        pDataPilotFieldElemTokenMap = new SvXMLTokenMap( aDataPilotFieldElemTokenMap );
    return *pDataPilotFieldElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotLevelAttrTokenMap()
{
    if( !pDataPilotLevelAttrTokenMap )
        pDataPilotLevelAttrTokenMap = new SvXMLTokenMap( aDataPilotLevelAttrTokenMap );
    return *pDataPilotLevelAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotLevelElemTokenMap()
{
    if( !pDataPilotLevelElemTokenMap )
        pDataPilotLevelElemTokenMap = new SvXMLTokenMap( aDataPilotLevelElemTokenMap );
    return *pDataPilotLevelElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotSubTotalsElemTokenMap()
{
    if( !pDataPilotSubTotalsElemTokenMap )
        pDataPilotSubTotalsElemTokenMap = new SvXMLTokenMap( aDataPilotSubTotalsElemTokenMap );
    return *pDataPilotSubTotalsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotSubTotalAttrTokenMap()
{
    if( !pDataPilotSubTotalAttrTokenMap )
        pDataPilotSubTotalAttrTokenMap = new SvXMLTokenMap( aDataPilotSubTotalAttrTokenMap );
    return *pDataPilotSubTotalAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotMembersElemTokenMap()
{
    if( !pDataPilotMembersElemTokenMap )
        pDataPilotMembersElemTokenMap = new SvXMLTokenMap( aDataPilotMembersElemTokenMap );
    return *pDataPilotMembersElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotMemberAttrTokenMap()
{
    if( !pDataPilotMemberAttrTokenMap )
        pDataPilotMemberAttrTokenMap = new SvXMLTokenMap( aDataPilotMemberAttrTokenMap );
    return *pDataPilotMemberAttrTokenMap;
}

SvXMLImportContext *ScXMLImport::CreateContext( USHORT nPrefix,
                                           const OUString& rLocalName,
                                           const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_OFFICE==nPrefix &&
        0 == rLocalName.compareToAscii( sXML_document ) )
        pContext = new ScXMLDocContext_Impl( *this, nPrefix, rLocalName,
                                             xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

ScXMLImport::ScXMLImport(   com::sun::star::uno::Reference <com::sun::star::frame::XModel> xTempModel, sal_Bool bLDoc, sal_uInt16 nStyleFamMask ) :
    SvXMLImport( xTempModel ),
    bLoadDoc( bLDoc ),
    nStyleFamilyMask( nStyleFamMask ),
//  rDoc( rD ),
//  pI18NMap( new SvI18NMap ),
    pDocElemTokenMap( 0 ),
    pStylesElemTokenMap( 0 ),
    pStylesAttrTokenMap( 0 ),
    pStyleElemTokenMap( 0 ),
    pBodyElemTokenMap( 0 ),
    pContentValidationsElemTokenMap( 0 ),
    pContentValidationElemTokenMap( 0 ),
    pContentValidationAttrTokenMap( 0 ),
    pContentValidationMessageElemTokenMap( 0 ),
    pContentValidationHelpMessageAttrTokenMap( 0 ),
    pContentValidationErrorMessageAttrTokenMap( 0 ),
    pContentValidationErrorMacroAttrTokenMap( 0 ),
    pLabelRangesElemTokenMap( 0 ),
    pLabelRangeAttrTokenMap( 0 ),
    pTableElemTokenMap( 0 ),
    pTableRowsElemTokenMap( 0 ),
    pTableColsElemTokenMap( 0 ),
    pTableAttrTokenMap( 0 ),
    pTableScenarioAttrTokenMap( 0 ),
    pTableColAttrTokenMap( 0 ),
    pTableRowElemTokenMap( 0 ),
    pTableRowAttrTokenMap( 0 ),
    pTableRowCellElemTokenMap( 0 ),
    pTableRowCellAttrTokenMap( 0 ),
    pTableAnnotationAttrTokenMap( 0 ),
    pTableCellRangeSourceAttrTokenMap( 0 ),
    pNamedExpressionsElemTokenMap( 0 ),
    pNamedRangeAttrTokenMap( 0 ),
    pNamedExpressionAttrTokenMap( 0 ),
    pDatabaseRangesElemTokenMap( 0 ),
    pDatabaseRangeElemTokenMap( 0 ),
    pDatabaseRangeAttrTokenMap( 0 ),
    pDatabaseRangeSourceSQLAttrTokenMap( 0 ),
    pDatabaseRangeSourceTableAttrTokenMap( 0 ),
    pDatabaseRangeSourceQueryAttrTokenMap( 0 ),
    pFilterElemTokenMap( 0 ),
    pFilterAttrTokenMap( 0 ),
    pFilterConditionAttrTokenMap( 0 ),
    pSortElemTokenMap( 0 ),
    pSortAttrTokenMap( 0 ),
    pSortSortByAttrTokenMap( 0 ),
    pDatabaseRangeSubTotalRulesElemTokenMap( 0 ),
    pDatabaseRangeSubTotalRulesAttrTokenMap( 0 ),
    pSubTotalRulesSortGroupsAttrTokenMap( 0 ),
    pSubTotalRulesSubTotalRuleElemTokenMap( 0 ),
    pSubTotalRulesSubTotalRuleAttrTokenMap( 0 ),
    pSubTotalRuleSubTotalFieldAttrTokenMap( 0 ),
    pDataPilotTablesElemTokenMap( 0 ),
    pDataPilotTableAttrTokenMap( 0 ),
    pDataPilotTableElemTokenMap( 0 ),
    pDataPilotTableSourceServiceAttrTokenMap( 0 ),
    pDataPilotTableSourceCellRangeElemTokenMap( 0 ),
    pDataPilotTableSourceCellRangeAttrTokenMap( 0 ),
    pDataPilotFieldAttrTokenMap( 0 ),
    pDataPilotFieldElemTokenMap( 0 ),
    pDataPilotLevelAttrTokenMap( 0 ),
    pDataPilotLevelElemTokenMap( 0 ),
    pDataPilotSubTotalsElemTokenMap( 0 ),
    pDataPilotSubTotalAttrTokenMap( 0 ),
    pDataPilotMembersElemTokenMap( 0 ),
    pDataPilotMemberAttrTokenMap( 0 ),
    aTables(*this),
    aMyNamedExpressions(),
    aValidations(),
//  pScAutoStylePool(new SvXMLAutoStylePoolP),
    pScPropHdlFactory(0L),
    pCellStylesPropertySetMapper(0L),
    pColumnStylesPropertySetMapper(0L),
    pRowStylesPropertySetMapper(0L),
    pTableStylesPropertySetMapper(0L),
    sSC_float(RTL_CONSTASCII_USTRINGPARAM(sXML_float)),
    sSC_time(RTL_CONSTASCII_USTRINGPARAM(sXML_time)),
    sSC_date(RTL_CONSTASCII_USTRINGPARAM(sXML_date)),
    sSC_percentage(RTL_CONSTASCII_USTRINGPARAM(sXML_percentage)),
    sSC_currency(RTL_CONSTASCII_USTRINGPARAM(sXML_currency)),
    sSC_string(RTL_CONSTASCII_USTRINGPARAM(sXML_string)),
    sSC_boolean(RTL_CONSTASCII_USTRINGPARAM(sXML_boolean))

//  pParaItemMapper( 0 ),
{
    /*GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TEXT, sXML_np__text,
                                  sXML_n_text, XML_NAMESPACE_TEXT );
    GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TABLE, sXML_np__table,
                                  sXML_n_table, XML_NAMESPACE_TABLE );*/

    pScPropHdlFactory = new XMLScPropHdlFactory;
    if(pScPropHdlFactory)
    {
        // set lock to avoid deletion
        pScPropHdlFactory->acquire();

        // build one ref
        const UniReference< XMLPropertyHandlerFactory > aFactoryRef = pScPropHdlFactory;

        // construct PropertySetMapper
        pCellStylesPropertySetMapper = new XMLCellStylesPropertySetMapper((XMLPropertyMapEntry*)aXMLScCellStylesProperties, aFactoryRef);
        if(pCellStylesPropertySetMapper)
        {
            // set lock to avoid deletion
            pCellStylesPropertySetMapper->acquire();
        }
        pColumnStylesPropertySetMapper = new XMLColumnStylesPropertySetMapper((XMLPropertyMapEntry*)aXMLScColumnStylesProperties, aFactoryRef);
        if(pColumnStylesPropertySetMapper)
        {
            // set lock to avoid deletion
            pColumnStylesPropertySetMapper->acquire();
        }
        pRowStylesPropertySetMapper = new XMLRowStylesPropertySetMapper((XMLPropertyMapEntry*)aXMLScRowStylesProperties, aFactoryRef);
        if(pRowStylesPropertySetMapper)
        {
            // set lock to avoid deletion
            pRowStylesPropertySetMapper->acquire();
        }
        pTableStylesPropertySetMapper = new XMLTableStylesPropertySetMapper((XMLPropertyMapEntry*)aXMLScTableStylesProperties, aFactoryRef);
        if(pTableStylesPropertySetMapper)
        {
            // set lock to avoid deletion
            pTableStylesPropertySetMapper->acquire();
        }
    }
    uno::Reference<document::XActionLockable> xActionLockable(xTempModel, uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->addActionLock();
}

ScXMLImport::~ScXMLImport()
{
//  RemoveUnusedNumRules();

//  delete pI18NMap;
    delete pDocElemTokenMap;
    delete pStylesElemTokenMap;
    delete pStylesAttrTokenMap;
    delete pStyleElemTokenMap;
    delete pBodyElemTokenMap;
    delete pContentValidationsElemTokenMap;
    delete pContentValidationElemTokenMap;
    delete pContentValidationAttrTokenMap;
    delete pContentValidationMessageElemTokenMap;
    delete pContentValidationHelpMessageAttrTokenMap;
    delete pContentValidationErrorMessageAttrTokenMap;
    delete pContentValidationErrorMacroAttrTokenMap;
    delete pLabelRangesElemTokenMap;
    delete pLabelRangeAttrTokenMap;
    delete pTableElemTokenMap;
    delete pTableRowsElemTokenMap;
    delete pTableColsElemTokenMap;
    delete pTableAttrTokenMap;
    delete pTableScenarioAttrTokenMap;
    delete pTableColAttrTokenMap;
    delete pTableRowElemTokenMap;
    delete pTableRowAttrTokenMap;
    delete pTableRowCellElemTokenMap;
    delete pTableRowCellAttrTokenMap;
    delete pTableAnnotationAttrTokenMap;
    delete pTableCellRangeSourceAttrTokenMap;
    delete pNamedExpressionsElemTokenMap;
    delete pNamedRangeAttrTokenMap;
    delete pNamedExpressionAttrTokenMap;
    delete pDatabaseRangesElemTokenMap;
    delete pDatabaseRangeElemTokenMap;
    delete pDatabaseRangeAttrTokenMap;
    delete pDatabaseRangeSourceSQLAttrTokenMap;
    delete pDatabaseRangeSourceTableAttrTokenMap;
    delete pDatabaseRangeSourceQueryAttrTokenMap;
    delete pFilterElemTokenMap;
    delete pFilterAttrTokenMap;
    delete pFilterConditionAttrTokenMap;
    delete pSortElemTokenMap;
    delete pSortAttrTokenMap;
    delete pSortSortByAttrTokenMap;
    delete pDatabaseRangeSubTotalRulesElemTokenMap;
    delete pDatabaseRangeSubTotalRulesAttrTokenMap;
    delete pSubTotalRulesSortGroupsAttrTokenMap;
    delete pSubTotalRulesSubTotalRuleElemTokenMap;
    delete pSubTotalRulesSubTotalRuleAttrTokenMap;
    delete pSubTotalRuleSubTotalFieldAttrTokenMap;
    delete pDataPilotTablesElemTokenMap;
    delete pDataPilotTableAttrTokenMap;
    delete pDataPilotTableElemTokenMap;
    delete pDataPilotTableSourceServiceAttrTokenMap;
    delete pDataPilotTableSourceCellRangeAttrTokenMap;
    delete pDataPilotTableSourceCellRangeElemTokenMap;
    delete pDataPilotFieldAttrTokenMap;
    delete pDataPilotFieldElemTokenMap;
    delete pDataPilotLevelAttrTokenMap;
    delete pDataPilotLevelElemTokenMap;
    delete pDataPilotSubTotalsElemTokenMap;
    delete pDataPilotSubTotalAttrTokenMap;
    delete pDataPilotMembersElemTokenMap;
    delete pDataPilotMemberAttrTokenMap;

    if (pScPropHdlFactory)
    {
        pScPropHdlFactory->release();
        pScPropHdlFactory = 0L;
    }
    if (pCellStylesPropertySetMapper)
    {
        pCellStylesPropertySetMapper->release();
        pCellStylesPropertySetMapper = 0L;
    }
    if (pColumnStylesPropertySetMapper)
    {
        pColumnStylesPropertySetMapper->release();
        pColumnStylesPropertySetMapper = 0L;
    }
    if (pRowStylesPropertySetMapper)
    {
        pRowStylesPropertySetMapper->release();
        pRowStylesPropertySetMapper = 0L;
    }
    if (pTableStylesPropertySetMapper)
    {
        pTableStylesPropertySetMapper->release();
        pTableStylesPropertySetMapper = 0L;
    }
//  if (pScAutoStylePool)
//      delete pScAutoStylePool;
    uno::Reference<document::XActionLockable> xActionLockable(GetModel(), uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->removeActionLock();
}

// ---------------------------------------------------------------------

SvXMLImportContext *ScXMLImport::CreateStylesContext(const NAMESPACE_RTL(OUString)& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList, sal_Bool bIsAutoStyle )
{
    SvXMLImportContext *pContext = NULL;
    if (!pContext)
    {
        pContext = new XMLTableStylesContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList);
        if (bIsAutoStyle)
            xAutoStyles = pContext;
        else
            xStyles = pContext;
    }
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateBodyContext(const NAMESPACE_RTL(OUString)& rLocalName)
{
    ((XMLTableStylesContext *)&xStyles)->CopyStylesToDoc(sal_True);
    GetShapeImport()->SetAutoStylesContext((XMLTableStylesContext *)&xAutoStyles);

    SvXMLImportContext *pContext = 0;
    pContext = new ScXMLBodyContext(*this, XML_NAMESPACE_OFFICE, rLocalName);
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !IsStylesOnlyMode() )
    {
        pContext = new SfxXMLMetaContext( *this,
                            XML_NAMESPACE_OFFICE, rLocalName,
                            GetModel() );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );

    return pContext;
}

sal_Bool ScXMLImport::GetValidation(const rtl::OUString& sName, ScMyImportValidation& aValidation)
{
    sal_Bool bFound(sal_False);
    ScMyImportValidations::iterator aItr = aValidations.begin();
    while(aItr != aValidations.end() && !bFound)
    {
        if (aItr->sName == sName)
            bFound = sal_True;
        else
            aItr++;
    }
    if (bFound)
        aValidation = *aItr;
    return bFound;
}

void ScXMLImport::GetRangeFromString( const rtl::OUString& rRangeStr, ScRange& rRange )
{
    ScModelObj* pDocObj = ScModelObj::getImplementation( GetModel() );
    if( pDocObj )
    {
        ScDocument* pDoc = pDocObj->GetDocument();
        if( pDoc )
        {
            ScAddress aStartCellAddress;
            ScAddress aEndCellAddress;
            sal_Int16 i( 0 );
            while ((rRangeStr[i] != ':') && (i < rRangeStr.getLength()))
                i++;
            rtl::OUString sStartCellAddress( rRangeStr.copy(0, i) );
            rtl::OUString sEndCellAddress( rRangeStr.copy(i + 1) );
            aStartCellAddress.Parse(sStartCellAddress, pDoc);
            aEndCellAddress.Parse(sEndCellAddress, pDoc);
            rRange = ScRange(aStartCellAddress, aEndCellAddress);
        }
    }
}

void ScXMLImport::GetRangeListFromString( const rtl::OUString& rRangeListStr, ScRangeList& rRangeList )
{
    rtl::OUStringBuffer aBuffer;
    sal_Int32           nLength     = rRangeListStr.getLength();
    sal_Int32           nIndex      = 0;
    sal_Bool            bQuote      = sal_False;
    sal_Bool            bGetRange   = sal_False;
    while( nIndex < nLength )
    {
        sal_Unicode nCode = rRangeListStr[nIndex];
        if( (nCode != ' ') || bQuote )
        {
            aBuffer.append( nCode );
            bQuote = (bQuote != (nCode == '\''));
        }
        else bGetRange = (nCode == ' ');
        if( bGetRange || (nIndex == nLength - 1) )
        {
            ScRange* pRange = new ScRange;
            GetRangeFromString( aBuffer.makeStringAndClear(), *pRange );
            rRangeList.Insert( pRange, LIST_APPEND );
            bGetRange = sal_False;
        }
        nIndex++;
    }
}

sal_Int32 ScXMLImport::GetRangeFromString( const rtl::OUString& rRangeListStr, sal_Int32 nOffset,
                table::CellRangeAddress& rCellRange )
{
    rtl::OUStringBuffer aBuffer;
    sal_Int32           nLength     = rRangeListStr.getLength();
    sal_Int32           nIndex      = nOffset;
    sal_Bool            bQuote      = sal_False;
    sal_Bool            bExitLoop   = sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        sal_Unicode nCode = rRangeListStr[nIndex];
        if( (nCode != ' ') || bQuote )
        {
            aBuffer.append( nCode );
            bQuote = (bQuote != (nCode == '\''));
        }
        else bExitLoop = (nCode == ' ');
        nIndex++;
    }

    ScRange aRange;
    GetRangeFromString( aBuffer.makeStringAndClear(), aRange );
    rCellRange.Sheet = aRange.aStart.Tab();
    rCellRange.StartColumn = aRange.aStart.Col();
    rCellRange.EndColumn = aRange.aEnd.Col();
    rCellRange.StartRow = aRange.aStart.Row();
    rCellRange.EndRow = aRange.aEnd.Row();

    return (nIndex < nLength) ? nIndex : -1;
}

void ScXMLImport::GetCellFromString( const rtl::OUString& rCellStr, table::CellAddress& rCell)
{
    ScModelObj* pDocObj = ScModelObj::getImplementation( GetModel() );
    if( pDocObj )
    {
        ScDocument* pDoc = pDocObj->GetDocument();
        if( pDoc )
        {
            ScAddress aCellAddress;
            sal_Int16 i( 0 );
            aCellAddress.Parse(rCellStr, pDoc);
            rCell.Column = aCellAddress.Col();
            rCell.Row = aCellAddress.Row();
            rCell.Sheet = aCellAddress.Tab();
        }
    }
}

