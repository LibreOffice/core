/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlimprt.cxx,v $
 *
 *  $Revision: 1.129 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:54:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svtools/zforlist.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmetai.hxx>
#include <sfx2/objsh.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>

#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <svtools/languageoptions.hxx>

#include "xmlimprt.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "xmlbodyi.hxx"
#include "xmlstyli.hxx"
#include "unoguard.hxx"
#include "ViewSettingsSequenceDefines.hxx"

#include "patattr.hxx"

#include "XMLConverter.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include "chgviset.hxx"
#include "XMLStylesImportHelper.hxx"
#include "unonames.hxx"
#include "rangeutl.hxx"
#include "postit.hxx"
#include <comphelper/extract.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/sheet/CellInsertMode.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <tools/urlobj.hxx>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>

#define SC_LOCALE           "Locale"
#define SC_STANDARDFORMAT   "StandardFormat"
#define SC_CURRENCYSYMBOL   "CurrencySymbol"
#define SC_NAMEDRANGES      "NamedRanges"
#define SC_REPEAT_COLUMN "repeat-column"
#define SC_REPEAT_ROW "repeat-row"
#define SC_FILTER "filter"
#define SC_PRINT_RANGE "print-range"

using namespace com::sun::star;
using namespace ::xmloff::token;

OUString SAL_CALL ScXMLImport_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisImporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLImport_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_ALL);
    return (cppu::OWeakObject*)new ScXMLImport( rSMgr, IMPORT_ALL );
}

OUString SAL_CALL ScXMLImport_Meta_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisMetaImporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Meta_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLImport_Meta_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_META);
    return (cppu::OWeakObject*)new ScXMLImport( rSMgr, IMPORT_META );
}

OUString SAL_CALL ScXMLImport_Styles_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisStylesImporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Styles_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLImport_Styles_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES|IMPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLImport( rSMgr, IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES|IMPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLImport_Content_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisContentImporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Content_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLImport_Content_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_META|IMPORT_STYLES|IMPORT_MASTERSTYLES|IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_SETTINGS|IMPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLImport( rSMgr, IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLImport_Settings_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisSettingsImporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Settings_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLImport_Settings_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_SETTINGS);
    return (cppu::OWeakObject*)new ScXMLImport( rSMgr, IMPORT_SETTINGS );
}

//----------------------------------------------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aDocTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,    XML_TOK_DOC_FONTDECLS           },
    { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES              },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES          },
    { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES        },
    { XML_NAMESPACE_OFFICE, XML_META,               XML_TOK_DOC_META                },
    { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPTS             },
    { XML_NAMESPACE_OFFICE, XML_BODY,               XML_TOK_DOC_BODY                },
    { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_STYLE,              XML_TOK_STYLES_STYLE            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_NAME,               XML_TOK_STYLES_STYLE_NAME   },
    { XML_NAMESPACE_STYLE,  XML_FAMILY,             XML_TOK_STYLES_STYLE_FAMILY },
    { XML_NAMESPACE_STYLE,  XML_PARENT_STYLE_NAME,  XML_TOK_STYLES_STYLE_PARENT_STYLE_NAME  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_PROPERTIES,         XML_TOK_STYLE_PROPERTIES    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aBodyTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TRACKED_CHANGES,         XML_TOK_BODY_TRACKED_CHANGES        },
    { XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS,    XML_TOK_BODY_CALCULATION_SETTINGS   },
    { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATIONS,     XML_TOK_BODY_CONTENT_VALIDATIONS    },
    { XML_NAMESPACE_TABLE, XML_LABEL_RANGES,            XML_TOK_BODY_LABEL_RANGES           },
    { XML_NAMESPACE_TABLE, XML_TABLE,                   XML_TOK_BODY_TABLE                  },
    { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS,       XML_TOK_BODY_NAMED_EXPRESSIONS      },
    { XML_NAMESPACE_TABLE, XML_DATABASE_RANGES,         XML_TOK_BODY_DATABASE_RANGES        },
    { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,          XML_TOK_BODY_DATABASE_RANGE         },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES,       XML_TOK_BODY_DATA_PILOT_TABLES      },
    { XML_NAMESPACE_TABLE, XML_CONSOLIDATION,           XML_TOK_BODY_CONSOLIDATION          },
    { XML_NAMESPACE_TABLE, XML_DDE_LINKS,               XML_TOK_BODY_DDE_LINKS              },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION,  XML_TOK_CONTENT_VALIDATION  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  XML_HELP_MESSAGE,    XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE    },
    { XML_NAMESPACE_TABLE,  XML_ERROR_MESSAGE,   XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE   },
    { XML_NAMESPACE_TABLE,  XML_ERROR_MACRO,     XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO     },
    { XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, XML_TOK_CONTENT_VALIDATION_ELEM_EVENT_LISTENERS },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_CONTENT_VALIDATION_NAME                 },
    { XML_NAMESPACE_TABLE, XML_CONDITION,           XML_TOK_CONTENT_VALIDATION_CONDITION            },
    { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS    },
    { XML_NAMESPACE_TABLE, XML_ALLOW_EMPTY_CELL,    XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL     },
    { XML_NAMESPACE_TABLE, XML_DISPLAY_LIST,        XML_TOK_CONTENT_VALIDATION_DISPLAY_LIST         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationMessageElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_P,    XML_TOK_P   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationHelpMessageAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TITLE,   XML_TOK_HELP_MESSAGE_ATTR_TITLE     },
    { XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TOK_HELP_MESSAGE_ATTR_DISPLAY   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationErrorMessageAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TITLE,           XML_TOK_ERROR_MESSAGE_ATTR_TITLE        },
    { XML_NAMESPACE_TABLE, XML_DISPLAY,         XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY      },
    { XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE,    XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aContentValidationErrorMacroAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,    XML_TOK_ERROR_MACRO_ATTR_NAME       },
    { XML_NAMESPACE_TABLE, XML_EXECUTE, XML_TOK_ERROR_MACRO_ATTR_EXECUTE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLabelRangesElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_LABEL_RANGE, XML_TOK_LABEL_RANGE_ELEM    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLabelRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_LABEL_CELL_RANGE_ADDRESS,    XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE    },
    { XML_NAMESPACE_TABLE, XML_DATA_CELL_RANGE_ADDRESS,     XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE     },
    { XML_NAMESPACE_TABLE, XML_ORIENTATION,                 XML_TOK_LABEL_RANGE_ATTR_ORIENTATION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN_GROUP,         XML_TOK_TABLE_COL_GROUP     },
    { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_COLUMNS,       XML_TOK_TABLE_HEADER_COLS   },
    { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMNS,              XML_TOK_TABLE_COLS          },
    { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN,               XML_TOK_TABLE_COL           },
    { XML_NAMESPACE_TABLE,  XML_TABLE_ROW_GROUP,            XML_TOK_TABLE_ROW_GROUP     },
    { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_ROWS,          XML_TOK_TABLE_HEADER_ROWS   },
    { XML_NAMESPACE_TABLE,  XML_TABLE_ROWS,                 XML_TOK_TABLE_ROWS          },
    { XML_NAMESPACE_TABLE,  XML_TABLE_ROW,                  XML_TOK_TABLE_ROW           },
    { XML_NAMESPACE_TABLE,  XML_TABLE_SOURCE,               XML_TOK_TABLE_SOURCE        },
    { XML_NAMESPACE_TABLE,  XML_SCENARIO,                   XML_TOK_TABLE_SCENARIO      },
    { XML_NAMESPACE_TABLE,  XML_SHAPES,                     XML_TOK_TABLE_SHAPES        },
    { XML_NAMESPACE_OFFICE, XML_FORMS,                      XML_TOK_TABLE_FORMS         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TABLE_ROW_GROUP,     XML_TOK_TABLE_ROWS_ROW_GROUP    },
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS,   XML_TOK_TABLE_ROWS_HEADER_ROWS  },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROWS,          XML_TOK_TABLE_ROWS_ROWS         },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROW,           XML_TOK_TABLE_ROWS_ROW          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableColsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN_GROUP,      XML_TOK_TABLE_COLS_COL_GROUP    },
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS,    XML_TOK_TABLE_COLS_HEADER_COLS  },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS,           XML_TOK_TABLE_COLS_COLS         },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN,            XML_TOK_TABLE_COLS_COL          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                        XML_TOK_TABLE_NAME              },
    { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_STYLE_NAME        },
    { XML_NAMESPACE_TABLE, XML_PROTECTED,                   XML_TOK_TABLE_PROTECTION        },
    { XML_NAMESPACE_TABLE, XML_PRINT_RANGES,                XML_TOK_TABLE_PRINT_RANGES      },
    { XML_NAMESPACE_TABLE, XML_PROTECTION_KEY,              XML_TOK_TABLE_PASSWORD          },
    { XML_NAMESPACE_TABLE, XML_PRINT,                       XML_TOK_TABLE_PRINT             },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableScenarioAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER,      XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER  },
    { XML_NAMESPACE_TABLE, XML_BORDER_COLOR,        XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR    },
    { XML_NAMESPACE_TABLE, XML_COPY_BACK,           XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK       },
    { XML_NAMESPACE_TABLE, XML_COPY_STYLES,         XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES     },
    { XML_NAMESPACE_TABLE, XML_COPY_FORMULAS,       XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS   },
    { XML_NAMESPACE_TABLE, XML_IS_ACTIVE,           XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE       },
    { XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES,     XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES },
    { XML_NAMESPACE_TABLE, XML_COMMENT,             XML_TOK_TABLE_SCENARIO_ATTR_COMMENT         },
    { XML_NAMESPACE_TABLE, XML_PROTECTED,           XML_TOK_TABLE_SCENARIO_ATTR_PROTECTED       },
    XML_TOKEN_MAP_END
};


static __FAR_DATA SvXMLTokenMapEntry aTableColAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_COL_ATTR_STYLE_NAME       },
    { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,     XML_TOK_TABLE_COL_ATTR_REPEATED         },
    { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_COL_ATTR_VISIBILITY       },
    { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,    XML_TOK_TABLE_COL_ATTR_DEFAULT_CELL_STYLE_NAME },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TABLE_CELL,      XML_TOK_TABLE_ROW_CELL              },
    { XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL,  XML_TOK_TABLE_ROW_COVERED_CELL      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_ROW_ATTR_STYLE_NAME           },
    { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_ROW_ATTR_VISIBILITY           },
    { XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED,        XML_TOK_TABLE_ROW_ATTR_REPEATED             },
    { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,     XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME },
//  { XML_NAMESPACE_TABLE, XML_USE_OPTIMAL_HEIGHT,          XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowCellTokenMap[] =
{
    { XML_NAMESPACE_TEXT,   XML_P,                  XML_TOK_TABLE_ROW_CELL_P                    },
    { XML_NAMESPACE_TABLE,  XML_SUB_TABLE,          XML_TOK_TABLE_ROW_CELL_TABLE                },
    { XML_NAMESPACE_OFFICE, XML_ANNOTATION,         XML_TOK_TABLE_ROW_CELL_ANNOTATION           },
    { XML_NAMESPACE_TABLE,  XML_DETECTIVE,          XML_TOK_TABLE_ROW_CELL_DETECTIVE            },
    { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_SOURCE,  XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableRowCellAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                      XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME              },
    { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME,         XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME },
    { XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED,             XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS            },
    { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED,          XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS            },
    { XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED,   XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS     },
    { XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED,      XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS     },
    { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,         XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED                },
    { XML_NAMESPACE_TABLE, XML_VALUE_TYPE,                      XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE              },
    { XML_NAMESPACE_TABLE, XML_VALUE,                           XML_TOK_TABLE_ROW_CELL_ATTR_VALUE                   },
    { XML_NAMESPACE_TABLE, XML_DATE_VALUE,                      XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE              },
    { XML_NAMESPACE_TABLE, XML_TIME_VALUE,                      XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE              },
    { XML_NAMESPACE_TABLE, XML_STRING_VALUE,                    XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE            },
    { XML_NAMESPACE_TABLE, XML_BOOLEAN_VALUE,                   XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE           },
    { XML_NAMESPACE_TABLE, XML_FORMULA,                         XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA                 },
    { XML_NAMESPACE_TABLE, XML_CURRENCY,                        XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY                },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableAnnotationAttrTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_AUTHOR,                 XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR                },
    { XML_NAMESPACE_OFFICE, XML_CREATE_DATE,            XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE           },
    { XML_NAMESPACE_OFFICE, XML_CREATE_DATE_STRING,     XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING    },
    { XML_NAMESPACE_OFFICE, XML_DISPLAY,                XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY               },
    { XML_NAMESPACE_SVG,    XML_X,                      XML_TOK_TABLE_ANNOTATION_ATTR_X                     },
    { XML_NAMESPACE_SVG,    XML_Y,                      XML_TOK_TABLE_ANNOTATION_ATTR_Y                     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDetectiveElemTokenMap[]=
{
    { XML_NAMESPACE_TABLE,  XML_HIGHLIGHTED_RANGE,  XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED  },
    { XML_NAMESPACE_TABLE,  XML_OPERATION,          XML_TOK_DETECTIVE_ELEM_OPERATION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDetectiveHighlightedAttrTokenMap[]=
{
    { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_ADDRESS,     XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE       },
    { XML_NAMESPACE_TABLE,  XML_DIRECTION,              XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION        },
    { XML_NAMESPACE_TABLE,  XML_CONTAINS_ERROR,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR   },
    { XML_NAMESPACE_TABLE,  XML_MARKED_INVALID,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_MARKED_INVALID   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDetectiveOperationAttrTokenMap[]=
{
    { XML_NAMESPACE_TABLE,  XML_NAME,   XML_TOK_DETECTIVE_OPERATION_ATTR_NAME   },
    { XML_NAMESPACE_TABLE,  XML_INDEX,  XML_TOK_DETECTIVE_OPERATION_ATTR_INDEX  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableCellRangeSourceAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  XML_NAME,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME           },
    { XML_NAMESPACE_XLINK,  XML_HREF,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF           },
    { XML_NAMESPACE_TABLE,  XML_FILTER_NAME,            XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME    },
    { XML_NAMESPACE_TABLE,  XML_FILTER_OPTIONS,         XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS },
    { XML_NAMESPACE_TABLE,  XML_LAST_COLUMN_SPANNED,    XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN    },
    { XML_NAMESPACE_TABLE,  XML_LAST_ROW_SPANNED,       XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW       },
    { XML_NAMESPACE_TABLE,  XML_REFRESH_DELAY,          XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedExpressionsTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAMED_RANGE,             XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE           },
    { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSION,        XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_RANGE_ATTR_NAME               },
    { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS,  XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS },
    { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS  },
    { XML_NAMESPACE_TABLE, XML_RANGE_USABLE_AS,     XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aNamedExpressionAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_EXPRESSION_ATTR_NAME              },
    { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS },
    { XML_NAMESPACE_TABLE, XML_EXPRESSION,          XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangesTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,  XML_TOK_DATABASE_RANGE      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL,     XML_TOK_DATABASE_RANGE_SOURCE_SQL       },
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATABASE_RANGE_SOURCE_TABLE     },
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATABASE_RANGE_SOURCE_QUERY     },
    { XML_NAMESPACE_TABLE, XML_FILTER,                  XML_TOK_FILTER                          },
    { XML_NAMESPACE_TABLE, XML_SORT,                    XML_TOK_SORT                            },
    { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULES,          XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATABASE_RANGE_ATTR_NAME                    },
    { XML_NAMESPACE_TABLE, XML_IS_SELECTION,            XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION            },
    { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_STYLES,   XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES   },
    { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_SIZE,     XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE     },
    { XML_NAMESPACE_TABLE, XML_HAS_PERSISTENT_DATA,     XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA     },
    { XML_NAMESPACE_TABLE, XML_ORIENTATION,         XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION             },
    { XML_NAMESPACE_TABLE, XML_CONTAINS_HEADER,     XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER         },
    { XML_NAMESPACE_TABLE, XML_DISPLAY_FILTER_BUTTONS,  XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS  },
    { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS    },
    { XML_NAMESPACE_TABLE, XML_REFRESH_DELAY,           XML_TOK_DATABASE_RANGE_ATTR_REFRESH_DELAY           },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceSQLAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME       },
    { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_SQL_ATTR_HREF                },
    { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_SQL_ATTR_CONNECTION_RESSOURCE},
    { XML_NAMESPACE_TABLE, XML_SQL_STATEMENT,           XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT       },
    { XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME         },
    { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_TABLE_ATTR_HREF                  },
    { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_TABLE_ATTR_CONNECTION_RESSOURCE  },
    { XML_NAMESPACE_TABLE, XML_TABLE_NAME,              XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSourceQueryAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME         },
    { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_QUERY_ATTR_HREF                  },
    { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_QUERY_ATTR_CONNECTION_RESSOURCE  },
    { XML_NAMESPACE_TABLE, XML_QUERY_NAME,              XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FILTER_AND,          XML_TOK_FILTER_AND          },
    { XML_NAMESPACE_TABLE, XML_FILTER_OR,           XML_TOK_FILTER_OR           },
    { XML_NAMESPACE_TABLE, XML_FILTER_CONDITION,    XML_TOK_FILTER_CONDITION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,            XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS            },
    { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS,  XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS  },
    { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE,                XML_TOK_FILTER_ATTR_CONDITION_SOURCE                },
    { XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES,              XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES              },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFilterConditionAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_CONDITION_ATTR_FIELD_NUMBER     },
    { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,  XML_TOK_CONDITION_ATTR_CASE_SENSITIVE   },
    { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_CONDITION_ATTR_DATA_TYPE        },
    { XML_NAMESPACE_TABLE, XML_VALUE,           XML_TOK_CONDITION_ATTR_VALUE            },
    { XML_NAMESPACE_TABLE, XML_OPERATOR,        XML_TOK_CONDITION_ATTR_OPERATOR         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_SORT_BY, XML_TOK_SORT_SORT_BY    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,  XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT    },
    { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS      },
    { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,          XML_TOK_SORT_ATTR_CASE_SENSITIVE            },
    { XML_NAMESPACE_TABLE, XML_LANGUAGE,                XML_TOK_SORT_ATTR_LANGUAGE                  },
    { XML_NAMESPACE_TABLE, XML_COUNTRY,                 XML_TOK_SORT_ATTR_COUNTRY                   },
    { XML_NAMESPACE_TABLE, XML_ALGORITHM,               XML_TOK_SORT_ATTR_ALGORITHM                 },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSortSortByAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SORT_BY_ATTR_FIELD_NUMBER   },
    { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_SORT_BY_ATTR_DATA_TYPE      },
    { XML_NAMESPACE_TABLE, XML_ORDER,           XML_TOK_SORT_BY_ATTR_ORDER          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_SORT_GROUPS,     XML_TOK_SUBTOTAL_RULES_SORT_GROUPS      },
    { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULE,   XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,          XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT      },
    { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,                  XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE              },
    { XML_NAMESPACE_TABLE, XML_PAGE_BREAKS_ON_GROUP_CHANGE, XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSortGroupsAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_TYPE,   XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE  },
    { XML_NAMESPACE_TABLE, XML_ORDER,       XML_TOK_SORT_GROUPS_ATTR_ORDER      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_SUBTOTAL_FIELD,  XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_GROUP_BY_FIELD_NUMBER,   XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSubTotalRuleSubTotalFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER    },
    { XML_NAMESPACE_TABLE, XML_FUNCTION,        XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTablesElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE,    XML_TOK_DATA_PILOT_TABLE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATA_PILOT_TABLE_ATTR_NAME                  },
    { XML_NAMESPACE_TABLE, XML_APPLICATION_DATA,        XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA      },
    { XML_NAMESPACE_TABLE, XML_GRAND_TOTAL,             XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL           },
    { XML_NAMESPACE_TABLE, XML_IGNORE_EMPTY_ROWS,       XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS     },
    { XML_NAMESPACE_TABLE, XML_IDENTIFY_CATEGORIES,     XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES   },
    { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS  },
    { XML_NAMESPACE_TABLE, XML_BUTTONS,                 XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS               },
    { XML_NAMESPACE_TABLE, XML_SHOW_FILTER_BUTTON,      XML_TOK_DATA_PILOT_TABLE_ATTR_SHOW_FILTER_BUTTON    },
    { XML_NAMESPACE_TABLE, XML_DRILL_DOWN_ON_DOUBLE_CLICK, XML_TOK_DATA_PILOT_TABLE_ATTR_DRILL_DOWN         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL        },
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE      },
    { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY      },
    { XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE,          XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE    },
    { XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE,       XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD,        XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD  },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceServiceAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_SOURCE_SERVICE_ATTR_NAME                },
    { XML_NAMESPACE_TABLE, XML_SOURCE_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME         },
    { XML_NAMESPACE_TABLE, XML_OBJECT_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME         },
    { XML_NAMESPACE_TABLE, XML_USER_NAME,               XML_TOK_SOURCE_SERVICE_ATTR_USER_NAME           },
    { XML_NAMESPACE_TABLE, XML_PASSWORT,                XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceCellRangeAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS,      XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotTableSourceCellRangeElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FILTER,      XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME,       XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME     },
    { XML_NAMESPACE_TABLE, XML_IS_DATA_LAYOUT_FIELD,    XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD  },
    { XML_NAMESPACE_TABLE, XML_FUNCTION,                XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION              },
    { XML_NAMESPACE_TABLE, XML_ORIENTATION,             XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION           },
    { XML_NAMESPACE_TABLE, XML_SELECTED_PAGE,           XML_TOK_DATA_PILOT_FIELD_ATTR_SELECTED_PAGE         },
    { XML_NAMESPACE_TABLE, XML_USED_HIERARCHY,          XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotFieldElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL,        XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL      },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD_REFERENCE, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_REFERENCE },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS,       XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_GROUPS },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotLevelAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_SHOW_EMPTY,              XML_TOK_DATA_PILOT_LEVEL_ATTR_SHOW_EMPTY            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotLevelElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS,    XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS  },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS,      XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS    },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_DISPLAY_INFO, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_DISPLAY_INFO },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SORT_INFO,    XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_SORT_INFO  },
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LAYOUT_INFO,  XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LAYOUT_INFO },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotSubTotalsElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotSubTotalAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_FUNCTION,                XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION           },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotMembersElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER,       XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDataPilotMemberAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME                 },
    { XML_NAMESPACE_TABLE, XML_DISPLAY,             XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY              },
    { XML_NAMESPACE_TABLE, XML_SHOW_DETAILS,        XML_TOK_DATA_PILOT_MEMBER_ATTR_SHOW_DETAILS     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aConsolidationAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  XML_FUNCTION,                       XML_TOK_CONSOLIDATION_ATTR_FUNCTION         },
    { XML_NAMESPACE_TABLE,  XML_SOURCE_CELL_RANGE_ADDRESSES,    XML_TOK_CONSOLIDATION_ATTR_SOURCE_RANGES    },
    { XML_NAMESPACE_TABLE,  XML_TARGET_CELL_ADDRESS,            XML_TOK_CONSOLIDATION_ATTR_TARGET_ADDRESS   },
    { XML_NAMESPACE_TABLE,  XML_USE_LABEL,                      XML_TOK_CONSOLIDATION_ATTR_USE_LABEL        },
    { XML_NAMESPACE_TABLE,  XML_LINK_TO_SOURCE_DATA,            XML_TOK_CONSOLIDATION_ATTR_LINK_TO_SOURCE   },
    XML_TOKEN_MAP_END
};


// NB: virtually inherit so we can multiply inherit properly
//     in ScXMLFlatDocContext_Impl
class ScXMLDocContext_Impl : public virtual SvXMLImportContext
{
protected:
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
                                        const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{

}

ScXMLDocContext_Impl::~ScXMLDocContext_Impl()
{
}

// context for flat file xml format
class ScXMLFlatDocContext_Impl
    : public ScXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:
    ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
        USHORT i_nPrefix, const OUString & i_rLName,
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Reference<xml::sax::XDocumentHandler>& i_xDocBuilder);

    virtual ~ScXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
        USHORT i_nPrefix, const OUString& i_rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList);
};

ScXMLFlatDocContext_Impl::ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
        USHORT i_nPrefix, const OUString & i_rLName,
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Reference<xml::sax::XDocumentHandler>& i_xDocBuilder) :
    SvXMLImportContext(i_rImport, i_nPrefix, i_rLName),
    ScXMLDocContext_Impl(i_rImport, i_nPrefix, i_rLName, i_xAttrList),
    SvXMLMetaDocumentContext(i_rImport, i_nPrefix, i_rLName,
        i_xDocProps, i_xDocBuilder)
{
}

ScXMLFlatDocContext_Impl::~ScXMLFlatDocContext_Impl() { }


SvXMLImportContext *ScXMLFlatDocContext_Impl::CreateChildContext(
    USHORT i_nPrefix, const OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    // behave like meta base class iff we encounter office:meta
    const SvXMLTokenMap& rTokenMap = GetScImport().GetDocElemTokenMap();
    if ( XML_TOK_DOC_META == rTokenMap.Get( i_nPrefix, i_rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    } else {
        return ScXMLDocContext_Impl::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    }
}

class ScXMLBodyContext_Impl : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const
        { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLBodyContext_Impl( ScXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~ScXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
};

ScXMLBodyContext_Impl::ScXMLBodyContext_Impl( ScXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

ScXMLBodyContext_Impl::~ScXMLBodyContext_Impl()
{
}

SvXMLImportContext *ScXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 /* nPrefix */,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return GetScImport().CreateBodyContext( rLocalName, xAttrList );
}

SvXMLImportContext *ScXMLDocContext_Impl::CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetDocElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_DOC_FONTDECLS:
        if (GetScImport().getImportFlags() & IMPORT_FONTDECLS)
            pContext = GetScImport().CreateFontDeclsContext(nPrefix, rLocalName, xAttrList);
        break;
    case XML_TOK_DOC_STYLES:
        if (GetScImport().getImportFlags() & IMPORT_STYLES)
            pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, sal_False);
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        if (GetScImport().getImportFlags() & IMPORT_AUTOSTYLES)
            pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, sal_True);
        break;
    case XML_TOK_DOC_MASTERSTYLES:
        if (GetScImport().getImportFlags() & IMPORT_MASTERSTYLES)
            pContext = new ScXMLMasterStylesContext( GetImport(), nPrefix, rLocalName,
                                              xAttrList );
        break;
    case XML_TOK_DOC_META:
        DBG_WARNING("XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
        break;
    case XML_TOK_DOC_SCRIPTS:
        if (GetScImport().getImportFlags() & IMPORT_SCRIPTS)
            pContext = GetScImport().CreateScriptContext( rLocalName );
        break;
    case XML_TOK_DOC_BODY:
        if (GetScImport().getImportFlags() & IMPORT_CONTENT)
            pContext = new ScXMLBodyContext_Impl( GetScImport(), nPrefix,
                                              rLocalName, xAttrList );
        break;
    case XML_TOK_DOC_SETTINGS:
        if (GetScImport().getImportFlags() & IMPORT_SETTINGS)
            pContext = new XMLDocumentSettingsContext(GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    }

    if(!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

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

const SvXMLTokenMap& ScXMLImport::GetDetectiveElemTokenMap()
{
    if( !pDetectiveElemTokenMap )
        pDetectiveElemTokenMap = new SvXMLTokenMap( aDetectiveElemTokenMap );
    return *pDetectiveElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDetectiveHighlightedAttrTokenMap()
{
    if( !pDetectiveHighlightedAttrTokenMap )
        pDetectiveHighlightedAttrTokenMap = new SvXMLTokenMap( aDetectiveHighlightedAttrTokenMap );
    return *pDetectiveHighlightedAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDetectiveOperationAttrTokenMap()
{
    if( !pDetectiveOperationAttrTokenMap )
        pDetectiveOperationAttrTokenMap = new SvXMLTokenMap( aDetectiveOperationAttrTokenMap );
    return *pDetectiveOperationAttrTokenMap;
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

const SvXMLTokenMap& ScXMLImport::GetConsolidationAttrTokenMap()
{
    if( !pConsolidationAttrTokenMap )
        pConsolidationAttrTokenMap = new SvXMLTokenMap( aConsolidationAttrTokenMap );
    return *pConsolidationAttrTokenMap;
}


SvXMLImportContext *ScXMLImport::CreateContext( USHORT nPrefix,
                                           const OUString& rLocalName,
                                           const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( (XML_NAMESPACE_OFFICE == nPrefix) &&
        ( IsXMLToken(rLocalName, XML_DOCUMENT_STYLES) ||
          IsXMLToken(rLocalName, XML_DOCUMENT_CONTENT) ||
          IsXMLToken(rLocalName, XML_DOCUMENT_SETTINGS) )) {
        pContext = new ScXMLDocContext_Impl( *this, nPrefix, rLocalName,
                                             xAttrList );
    } else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
                ( IsXMLToken(rLocalName, XML_DOCUMENT_META)) ) {
        pContext = CreateMetaContext(rLocalName);
    } else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
                ( IsXMLToken(rLocalName, XML_DOCUMENT)) ) {
        uno::Reference<xml::sax::XDocumentHandler> xDocBuilder(
            mxServiceFactory->createInstance(::rtl::OUString::createFromAscii(
                "com.sun.star.xml.dom.SAXDocumentBuilder")),
                uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        // flat OpenDocument file format
        pContext = new ScXMLFlatDocContext_Impl( *this, nPrefix, rLocalName,
                        xAttrList, xDPS->getDocumentProperties(), xDocBuilder);
    }
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// #110680#
ScXMLImport::ScXMLImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const sal_uInt16 nImportFlag)
:   SvXMLImport( xServiceFactory, nImportFlag ),
    pDoc( NULL ),
    pChangeTrackingImportHelper(NULL),
    pStylesImportHelper(NULL),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT)),
    sLocale(RTL_CONSTASCII_USTRINGPARAM(SC_LOCALE)),
    sCellStyle(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLSTYL)),
    sStandardFormat(RTL_CONSTASCII_USTRINGPARAM(SC_STANDARDFORMAT)),
    sType(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)),
//  pScAutoStylePool(new SvXMLAutoStylePoolP),
//  pParaItemMapper( 0 ),
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
    pTableScenarioAttrTokenMap( 0 ),
    pTableAttrTokenMap( 0 ),
    pTableColAttrTokenMap( 0 ),
    pTableRowElemTokenMap( 0 ),
    pTableRowAttrTokenMap( 0 ),
    pTableRowCellElemTokenMap( 0 ),
    pTableRowCellAttrTokenMap( 0 ),
    pTableAnnotationAttrTokenMap( 0 ),
    pDetectiveElemTokenMap( 0 ),
    pDetectiveHighlightedAttrTokenMap( 0 ),
    pDetectiveOperationAttrTokenMap( 0 ),
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
    pConsolidationAttrTokenMap( 0 ),
    aTables(*this),
    pMyNamedExpressions(NULL),
    pMyLabelRanges(NULL),
    pValidations(NULL),
    pDetectiveOpArray(NULL),
    pDefaultNotes(NULL),
    pScUnoGuard(NULL),
    pNumberFormatAttributesExportHelper(NULL),
    pStyleNumberFormats(NULL),
    sPrevStyleName(),
    sPrevCurrency(),
    nSolarMutexLocked(0),
    nProgressCount(0),
    nStyleFamilyMask( 0 ),
    nPrevCellType(0),
    bLoadDoc( sal_True ),
    bRemoveLastChar(sal_False),
    bNullDateSetted(sal_False),
    bSelfImportingXMLSet(sal_False),
    bLatinDefaultStyle(sal_False),
    bFromWrapper(sal_False)
{
    pStylesImportHelper = new ScMyStylesImportHelper(*this);

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScCellStylesProperties, xScPropHdlFactory);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScColumnStylesProperties, xScPropHdlFactory);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScRowStylesImportProperties, xScPropHdlFactory);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScTableStylesImportProperties, xScPropHdlFactory);

    // #i66550# needed for 'presentation:event-listener' element for URLs in shapes
    GetNamespaceMap().Add(
        GetXMLToken( XML_NP_PRESENTATION ),
        GetXMLToken( XML_N_PRESENTATION ),
        XML_NAMESPACE_PRESENTATION );
}

ScXMLImport::~ScXMLImport() throw()
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
    delete pDetectiveElemTokenMap;
    delete pDetectiveHighlightedAttrTokenMap;
    delete pDetectiveOperationAttrTokenMap;
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
    delete pConsolidationAttrTokenMap;

//  if (pScAutoStylePool)
//      delete pScAutoStylePool;
    if (pChangeTrackingImportHelper)
        delete pChangeTrackingImportHelper;
    if (pNumberFormatAttributesExportHelper)
        delete pNumberFormatAttributesExportHelper;
    if (pStyleNumberFormats)
        delete pStyleNumberFormats;
    if (pStylesImportHelper)
        delete pStylesImportHelper;

    if (pScUnoGuard)
        delete pScUnoGuard;

    if (pMyNamedExpressions)
        delete pMyNamedExpressions;
    if (pMyLabelRanges)
        delete pMyLabelRanges;
    if (pValidations)
        delete pValidations;
    if (pDefaultNotes)
        delete pDefaultNotes;
    if (pDetectiveOpArray)
        delete pDetectiveOpArray;
}

// ---------------------------------------------------------------------

SvXMLImportContext *ScXMLImport::CreateFontDeclsContext(const USHORT nPrefix, const ::rtl::OUString& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext *pContext = NULL;
    if (!pContext)
    {
        XMLFontStylesContext *pFSContext(
            new XMLFontStylesContext( *this, nPrefix,
                                        rLocalName, xAttrList,
                                        gsl_getSystemTextEncoding() ));
        SetFontDecls( pFSContext );
        pContext = pFSContext;
    }
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateStylesContext(const ::rtl::OUString& rLocalName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList, sal_Bool bIsAutoStyle )
{
    SvXMLImportContext *pContext(NULL);
    if (!pContext)
    {
        pContext = new XMLTableStylesContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, bIsAutoStyle);
        if (bIsAutoStyle)
            //xAutoStyles = pContext;
            SetAutoStyles((SvXMLStylesContext*)pContext);
        else
            //xStyles = pContext;
            SetStyles((SvXMLStylesContext*)pContext);
    }
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateBodyContext(const ::rtl::OUString& rLocalName,
                                                const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    //GetShapeImport()->SetAutoStylesContext((XMLTableStylesContext *)&xAutoStyles);
    //GetChartImport()->SetAutoStylesContext(GetAutoStyles()/*(XMLTableStylesContext *)&xAutoStyles*/);

    return new ScXMLBodyContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList);
}

SvXMLImportContext *ScXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext(0);

    if( !IsStylesOnlyMode() && (getImportFlags() & IMPORT_META))
    {
        uno::Reference<xml::sax::XDocumentHandler> xDocBuilder(
            mxServiceFactory->createInstance(::rtl::OUString::createFromAscii(
                "com.sun.star.xml.dom.SAXDocumentBuilder")),
                uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        pContext = new SvXMLMetaDocumentContext(*this,
                        XML_NAMESPACE_OFFICE, rLocalName,
                        xDPS->getDocumentProperties(), xDocBuilder);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this,
                            XML_NAMESPACE_OFFICE, rLocalName );

    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateScriptContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext(0);

    if( !(IsStylesOnlyMode()) )
    {
        pContext = new XMLScriptContext( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    GetModel() );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );

    return pContext;
}

void ScXMLImport::SetStatistics(
        const uno::Sequence<beans::NamedValue> & i_rStats)
{
    static const char* s_stats[] =
        { "TableCount", "CellCount", "ObjectCount", 0 };

    SvXMLImport::SetStatistics(i_rStats);

    sal_uInt32 nCount(0);
    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (const char** pStat = s_stats; *pStat != 0; ++pStat) {
            if (i_rStats[i].Name.equalsAscii(*pStat)) {
                sal_Int32 val;
                if (i_rStats[i].Value >>= val) {
                    nCount += val;
                } else {
                    DBG_ERROR("ScXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if (nCount)
    {
        GetProgressBarHelper()->SetReference(nCount);
        GetProgressBarHelper()->SetValue(0);
    }
}

XMLShapeImportHelper* ScXMLImport::CreateShapeImport()
{
    /*UniReference < XMLPropertySetMapper > xShapeStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScShapeStylesProperties, xScPropHdlFactory);
    SvXMLImportPropertyMapper *pShapeStylesImportPropertySetMapper = new SvXMLImportPropertyMapper( xShapeStylesPropertySetMapper );*/

    return new XMLTableShapeImportHelper( *this/*, pShapeStylesImportPropertySetMapper*/ );
}

sal_Bool ScXMLImport::GetValidation(const rtl::OUString& sName, ScMyImportValidation& aValidation)
{
    if (pValidations)
    {
        sal_Bool bFound(sal_False);
        ScMyImportValidations::iterator aItr(pValidations->begin());
        ScMyImportValidations::iterator aEndItr(pValidations->end());
        while(aItr != aEndItr && !bFound)
        {
            if (aItr->sName == sName)
            {
                // #b4974740# source position must be set as string,
                // so sBaseCellAddress no longer has to be converted here

                bFound = sal_True;
            }
            else
                ++aItr;
        }
        if (bFound)
            aValidation = *aItr;
        return bFound;
    }
    return sal_False;
}

ScXMLChangeTrackingImportHelper* ScXMLImport::GetChangeTrackingImportHelper()
{
    if (!pChangeTrackingImportHelper)
        pChangeTrackingImportHelper = new ScXMLChangeTrackingImportHelper();
    return pChangeTrackingImportHelper;
}

void ScXMLImport::InsertStyles()
{
    GetStyles()->CopyStylesToDoc(sal_True);

    // if content is going to be loaded with the same import, set bLatinDefaultStyle flag now
    if ( getImportFlags() & IMPORT_CONTENT )
        ExamineDefaultStyle();
}

void ScXMLImport::ExamineDefaultStyle()
{
    if (pDoc)
    {
        // #i62435# after inserting the styles, check if the default style has a latin-script-only
        // number format (then, value cells can be pre-initialized with western script type)

        const ScPatternAttr* pDefPattern = pDoc->GetDefPattern();
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        if ( pFormatter && pDefPattern )
        {
            sal_uInt32 nKey = pDefPattern->GetNumberFormat(pFormatter);
            const SvNumberformat* pFormat = pFormatter->GetEntry(nKey);
            if ( pFormat && pFormat->IsStandard() )
            {
                // The standard format is all-latin if the decimal separator dosen't
                // have a different script type

                String aDecSep;
                LanguageType nFormatLang = pFormat->GetLanguage();
                if ( nFormatLang == LANGUAGE_SYSTEM )
                    aDecSep = ScGlobal::pLocaleData->getNumDecimalSep();
                else
                {
                    LocaleDataWrapper aLocaleData( pDoc->GetServiceManager(),
                            MsLangId::convertLanguageToLocale( nFormatLang ) );
                    aDecSep = aLocaleData.getNumDecimalSep();
                }

                BYTE nScript = pDoc->GetStringScriptType( aDecSep );
                if ( nScript == 0 || nScript == SCRIPTTYPE_LATIN )
                    bLatinDefaultStyle = sal_True;
            }
        }
    }
}

void ScXMLImport::SetChangeTrackingViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rChangeProps)
{
    if (pDoc)
    {
        sal_Int32 nCount(rChangeProps.getLength());
        if (nCount)
        {
            LockSolarMutex();
            sal_Int16 nTemp16(0);
            ScChangeViewSettings* pViewSettings(new ScChangeViewSettings());
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                rtl::OUString sName(rChangeProps[i].Name);
                if (sName.compareToAscii("ShowChanges") == 0)
                    pViewSettings->SetShowChanges(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowAcceptedChanges") == 0)
                    pViewSettings->SetShowAccepted(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowRejectedChanges") == 0)
                    pViewSettings->SetShowRejected(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowChangesByDatetime") == 0)
                    pViewSettings->SetHasDate(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowChangesByDatetimeMode") == 0)
                {
                    if (rChangeProps[i].Value >>= nTemp16)
                        pViewSettings->SetTheDateMode(ScChgsDateMode(nTemp16));
                }
                else if (sName.compareToAscii("ShowChangesByDatetimeFirstDatetime") == 0)
                {
                    util::DateTime aDateTime;
                    if (rChangeProps[i].Value >>= aDateTime)
                    {
                        DateTime aCoreDateTime;
                        ScXMLConverter::ConvertAPIToCoreDateTime(aDateTime, aCoreDateTime);
                        pViewSettings->SetTheFirstDateTime(aCoreDateTime);
                    }
                }
                else if (sName.compareToAscii("ShowChangesByDatetimeSecondDatetime") == 0)
                {
                    util::DateTime aDateTime;
                    if (rChangeProps[i].Value >>= aDateTime)
                    {
                        DateTime aCoreDateTime;
                        ScXMLConverter::ConvertAPIToCoreDateTime(aDateTime, aCoreDateTime);
                        pViewSettings->SetTheLastDateTime(aCoreDateTime);
                    }
                }
                else if (sName.compareToAscii("ShowChangesByAuthor") == 0)
                    pViewSettings->SetHasAuthor(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowChangesByAuthorName") == 0)
                {
                    rtl::OUString sOUName;
                    if (rChangeProps[i].Value >>= sOUName)
                    {
                        String sAuthorName(sOUName);
                        pViewSettings->SetTheAuthorToShow(sAuthorName);
                    }
                }
                else if (sName.compareToAscii("ShowChangesByComment") == 0)
                    pViewSettings->SetHasComment(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowChangesByCommentText") == 0)
                {
                    rtl::OUString sOUComment;
                    if (rChangeProps[i].Value >>= sOUComment)
                    {
                        String sComment(sOUComment);
                        pViewSettings->SetTheComment(sComment);
                    }
                }
                else if (sName.compareToAscii("ShowChangesByRanges") == 0)
                    pViewSettings->SetHasRange(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName.compareToAscii("ShowChangesByRangesList") == 0)
                {
                    rtl::OUString sRanges;
                    if ((rChangeProps[i].Value >>= sRanges) && sRanges.getLength())
                    {
                        ScRangeList aRangeList;
                        ScRangeStringConverter::GetRangeListFromString(aRangeList, sRanges, GetDocument());
                        pViewSettings->SetTheRangeList(aRangeList);
                    }
                }
            }
            pDoc->SetChangeViewSettings(*pViewSettings);
            UnlockSolarMutex();
        }
    }
}

void ScXMLImport::SetViewSettings(const uno::Sequence<beans::PropertyValue>& aViewProps)
{
    sal_Int32 nCount(aViewProps.getLength());
    sal_Int32 nHeight(0);
    sal_Int32 nLeft(0);
    sal_Int32 nTop(0);
    sal_Int32 nWidth(0);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        rtl::OUString sName(aViewProps[i].Name);
        if (sName.compareToAscii("VisibleAreaHeight") == 0)
            aViewProps[i].Value >>= nHeight;
        else if (sName.compareToAscii("VisibleAreaLeft") == 0)
            aViewProps[i].Value >>= nLeft;
        else if (sName.compareToAscii("VisibleAreaTop") == 0)
            aViewProps[i].Value >>= nTop;
        else if (sName.compareToAscii("VisibleAreaWidth") == 0)
            aViewProps[i].Value >>= nWidth;
        else if (sName.compareToAscii("TrackedChangesViewSettings") == 0)
        {
            uno::Sequence<beans::PropertyValue> aChangeProps;
            if(aViewProps[i].Value >>= aChangeProps)
                SetChangeTrackingViewSettings(aChangeProps);
        }
    }
    if (nHeight && nWidth)
    {
        if (GetModel().is())
        {
            ScModelObj* pDocObj(ScModelObj::getImplementation( GetModel() ));
            if (pDocObj)
            {
                SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
                if (pEmbeddedObj)
                {
                    Rectangle aRect;
                    aRect.setX( nLeft );
                    aRect.setY( nTop );
                    aRect.setWidth( nWidth );
                    aRect.setHeight( nHeight );
                    pEmbeddedObj->SetVisArea(aRect);
                }
            }
        }
    }
}

void ScXMLImport::SetConfigurationSettings(const uno::Sequence<beans::PropertyValue>& aConfigProps)
{
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            sal_Int32 nCount(aConfigProps.getLength());
            rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("TrackedChangesProtectionKey"));
            for (sal_Int32 i = nCount - 1; i >= 0; --i)
            {
                if (aConfigProps[i].Name == sName)
                {
                    rtl::OUString sKey;
                    if (aConfigProps[i].Value >>= sKey)
                    {
                        uno::Sequence<sal_Int8> aPass;
                        SvXMLUnitConverter::decodeBase64(aPass, sKey);
                        if (aPass.getLength())
                        {
                            if (pDoc->GetChangeTrack())
                                pDoc->GetChangeTrack()->SetProtection(aPass);
                            else
                            {
                                StrCollection aUsers;
                                ScChangeTrack* pTrack = new ScChangeTrack(pDoc, aUsers);
                                pTrack->SetProtection(aPass);
                                pDoc->SetChangeTrack(pTrack);
                            }
                        }
                    }
                }
            }
            uno::Reference <uno::XInterface> xInterface = xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.SpreadsheetSettings")));
            uno::Reference <beans::XPropertySet> xProperties(xInterface, uno::UNO_QUERY);
            if (xProperties.is())
                SvXMLUnitConverter::convertPropertySet(xProperties, aConfigProps);
        }
    }
}

sal_Int32 ScXMLImport::SetCurrencySymbol(const sal_Int32 nKey, const rtl::OUString& rCurrency)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            rtl::OUString sFormatString;
            try
            {
                uno::Reference <beans::XPropertySet> xProperties(xLocalNumberFormats->getByKey(nKey));
                if (xProperties.is())
                {
                    lang::Locale aLocale;
                    if (GetDocument() && (xProperties->getPropertyValue(sLocale) >>= aLocale))
                    {
                        LockSolarMutex();
                        LocaleDataWrapper aLocaleData( GetDocument()->GetServiceManager(), aLocale );
                        rtl::OUStringBuffer aBuffer(15);
                        aBuffer.appendAscii("#");
                        aBuffer.append( aLocaleData.getNumThousandSep() );
                        aBuffer.appendAscii("##0");
                        aBuffer.append( aLocaleData.getNumDecimalSep() );
                        aBuffer.appendAscii("00 [$");
                        aBuffer.append(rCurrency);
                        aBuffer.appendAscii("]");
                        UnlockSolarMutex();
                        sFormatString = aBuffer.makeStringAndClear();
                        sal_Int32 nNewKey = xLocalNumberFormats->queryKey(sFormatString, aLocale, sal_True);
                        if (nNewKey == -1)
                            nNewKey = xLocalNumberFormats->addNew(sFormatString, aLocale);
                        return nNewKey;
                    }
                }
            }
            catch ( util::MalformedNumberFormatException& rException )
            {
                rtl::OUString sErrorMessage(RTL_CONSTASCII_USTRINGPARAM("Fehler im Formatstring "));
                sErrorMessage += sFormatString;
                sErrorMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" an Position "));
                sErrorMessage += rtl::OUString::valueOf(rException.CheckPos);
                uno::Sequence<rtl::OUString> aSeq(1);
                aSeq[0] = sErrorMessage;
                uno::Reference<xml::sax::XLocator> xLocator;
                SetError(XMLERROR_API | XMLERROR_FLAG_ERROR, aSeq, rException.Message, xLocator);
            }
        }
    }
       return nKey;
}

sal_Bool ScXMLImport::IsCurrencySymbol(const sal_Int32 nNumberFormat, const rtl::OUString& sCurrentCurrency, const rtl::OUString& sBankSymbol)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xLocalNumberFormats->getByKey(nNumberFormat));
                if (xNumberPropertySet.is())
                {
                    rtl::OUString sTemp;
                    if ( xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CURRENCYSYMBOL))) >>= sTemp)
                    {
                        if (sCurrentCurrency.equals(sTemp))
                            return sal_True;
                        // #i61657# This may be a legacy currency symbol that changed in the meantime.
                        if (SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sCurrentCurrency, sBankSymbol) != NULL)
                            return true;
                        // In the rare case that sCurrentCurrency is not the
                        // currency symbol, but a matching ISO code
                        // abbreviation instead that was obtained through
                        // XMLNumberFormatAttributesExportHelper::GetCellType(),
                        // check with the number format's symbol. This happens,
                        // for example, in the es_BO locale, where a legacy
                        // B$,BOB matched B$->BOP, which leads to
                        // sCurrentCurrency being BOP, and the previous call
                        // with BOP,BOB didn't find an entry, but B$,BOB will.
                        return SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sTemp, sBankSymbol) != NULL;
                    }
                }
            }
            catch ( uno::Exception& )
            {
                DBG_ERROR("Numberformat not found");
            }
        }
    }
    return sal_False;
}

void ScXMLImport::SetType(uno::Reference <beans::XPropertySet>& rProperties,
    sal_Int32& rNumberFormat,
    const sal_Int16 nCellType,
    const rtl::OUString& rCurrency)
{
    if ((nCellType != util::NumberFormat::TEXT) && (nCellType != util::NumberFormat::UNDEFINED))
    {
        if (rNumberFormat == -1)
            rProperties->getPropertyValue( sNumberFormat ) >>= rNumberFormat;
        DBG_ASSERT(rNumberFormat != -1, "no NumberFormat");
        sal_Bool bIsStandard;
        // sCurrentCurrency may be the ISO code abbreviation if the currency
        // symbol matches such, or if no match found the symbol itself!
        rtl::OUString sCurrentCurrency;
        sal_Int32 nCurrentCellType(
            GetNumberFormatAttributesExportHelper()->GetCellType(
                rNumberFormat, sCurrentCurrency, bIsStandard) & ~util::NumberFormat::DEFINED);
        if ((nCellType != nCurrentCellType) && !(nCellType == util::NumberFormat::NUMBER &&
            ((nCurrentCellType == util::NumberFormat::SCIENTIFIC) ||
            (nCurrentCellType == util::NumberFormat::FRACTION) ||
            (nCurrentCellType == util::NumberFormat::LOGICAL) ||
            (nCurrentCellType == 0)) || (nCurrentCellType == util::NumberFormat::TEXT)) && !((nCellType == util::NumberFormat::DATETIME) &&
            (nCurrentCellType == util::NumberFormat::DATE)))
        {
            if (!xNumberFormats.is())
            {
                uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
                if (xNumberFormatsSupplier.is())
                    xNumberFormats.set(xNumberFormatsSupplier->getNumberFormats());
            }
            if (xNumberFormats.is())
            {
                try
                {
                    uno::Reference < beans::XPropertySet> xNumberFormatProperties(xNumberFormats->getByKey(rNumberFormat));
                    if (xNumberFormatProperties.is())
                    {
                        if (nCellType != util::NumberFormat::CURRENCY)
                        {
                            lang::Locale aLocale;
                            if ( xNumberFormatProperties->getPropertyValue(sLocale) >>= aLocale )
                            {
                                if (!xNumberFormatTypes.is())
                                    xNumberFormatTypes.set(uno::Reference <util::XNumberFormatTypes>(xNumberFormats, uno::UNO_QUERY));
                                rProperties->setPropertyValue( sNumberFormat, uno::makeAny(xNumberFormatTypes->getStandardFormat(nCellType, aLocale)) );
                            }
                        }
                        else if (rCurrency.getLength() && sCurrentCurrency.getLength())
                        {
                            if (!sCurrentCurrency.equals(rCurrency))
                                if (!IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                                    rProperties->setPropertyValue( sNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
                        }
                    }
                }
                catch ( uno::Exception& )
                {
                    DBG_ERROR("Numberformat not found");
                }
            }
        }
        else
        {
            if ((nCellType == util::NumberFormat::CURRENCY) && rCurrency.getLength() && sCurrentCurrency.getLength() &&
                !sCurrentCurrency.equals(rCurrency) && !IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                rProperties->setPropertyValue( sNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
        }
    }
}

void ScXMLImport::AddStyleRange(const table::CellRangeAddress& rCellRange)
{
    if (!xSheetCellRanges.is() && GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
            xSheetCellRanges.set(uno::Reference <sheet::XSheetCellRangeContainer>(xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SheetCellRanges"))), uno::UNO_QUERY));
        DBG_ASSERT(xSheetCellRanges.is(), "didn't get SheetCellRanges");

    }
    xSheetCellRanges->addRangeAddress(rCellRange, sal_False);
}

void ScXMLImport::SetStyleToRanges()
{
    if (sPrevStyleName.getLength())
    {
        uno::Reference <beans::XPropertySet> xProperties (xSheetCellRanges, uno::UNO_QUERY);
        if (xProperties.is())
        {
            XMLTableStylesContext *pStyles((XMLTableStylesContext *)GetAutoStyles());
            XMLTableStyleContext* pStyle( pStyles ? (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                XML_STYLE_FAMILY_TABLE_CELL, sPrevStyleName, sal_True) : NULL );
            if (pStyle)
            {
                pStyle->FillPropertySet(xProperties);
                sal_Int32 nNumberFormat(pStyle->GetNumberFormat());
                SetType(xProperties, nNumberFormat, nPrevCellType, sPrevCurrency);
            }
            else
            {
                xProperties->setPropertyValue(sCellStyle, uno::makeAny(GetStyleDisplayName( XML_STYLE_FAMILY_TABLE_CELL, sPrevStyleName )));
                sal_Int32 nNumberFormat(GetStyleNumberFormats()->GetStyleNumberFormat(sPrevStyleName));
                sal_Bool bInsert(nNumberFormat == -1);
                SetType(xProperties, nNumberFormat, nPrevCellType, sPrevCurrency);
                if (bInsert)
                    GetStyleNumberFormats()->AddStyleNumberFormat(sPrevStyleName, nNumberFormat);
            }
        }
    }
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
            xSheetCellRanges.set(uno::Reference <sheet::XSheetCellRangeContainer>(
                xMultiServiceFactory->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SheetCellRanges"))),
                uno::UNO_QUERY));
    }
    DBG_ASSERT(xSheetCellRanges.is(), "didn't get SheetCellRanges");
}

void ScXMLImport::SetStyleToRange(const ScRange& rRange, const rtl::OUString* pStyleName,
        const sal_Int16 nCellType, const rtl::OUString* pCurrency)
{
    if (!sPrevStyleName.getLength())
    {
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if (sPrevCurrency.getLength())
            sPrevCurrency = sEmpty;
    }
    else if ((nCellType != nPrevCellType) ||
            ((pStyleName && !pStyleName->equals(sPrevStyleName)) ||
            (!pStyleName && sPrevStyleName.getLength())) ||
            ((pCurrency && !pCurrency->equals(sPrevCurrency)) ||
            (!pCurrency && sPrevCurrency.getLength())))
    {
        SetStyleToRanges();
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        else if(sPrevStyleName.getLength())
            sPrevStyleName = sEmpty;
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if(sPrevCurrency.getLength())
            sPrevCurrency = sEmpty;
    }
    table::CellRangeAddress aCellRange;
    aCellRange.StartColumn = rRange.aStart.Col();
    aCellRange.StartRow = rRange.aStart.Row();
    aCellRange.Sheet = rRange.aStart.Tab();
    aCellRange.EndColumn = rRange.aEnd.Col();
    aCellRange.EndRow = rRange.aEnd.Row();
    AddStyleRange(aCellRange);
}

sal_Bool ScXMLImport::SetNullDateOnUnitConverter()
{
    if (!bNullDateSetted)
        bNullDateSetted = GetMM100UnitConverter().setNullDate(GetModel());
    DBG_ASSERT(bNullDateSetted, "could not set the null date");
    return bNullDateSetted;
}

XMLNumberFormatAttributesExportHelper* ScXMLImport::GetNumberFormatAttributesExportHelper()
{
    if (!pNumberFormatAttributesExportHelper)
        pNumberFormatAttributesExportHelper = new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier());
    return pNumberFormatAttributesExportHelper;
}

ScMyStyleNumberFormats* ScXMLImport::GetStyleNumberFormats()
{
    if (!pStyleNumberFormats)
        pStyleNumberFormats = new ScMyStyleNumberFormats();
    return pStyleNumberFormats;
}

void ScXMLImport::SetStylesToRangesFinished()
{
    SetStyleToRanges();
    sPrevStyleName = sEmpty;
}

// XImporter
void SAL_CALL ScXMLImport::setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    LockSolarMutex();
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
    pDoc = ScXMLConverter::GetScDocument( xModel );
    DBG_ASSERT( pDoc, "ScXMLImport::setTargetDocument - no ScDocument!" );
    if (!pDoc)
        throw lang::IllegalArgumentException();

    bFromWrapper = pDoc->IsXMLFromWrapper();    // UnlockSolarMutex below still works normally

    uno::Reference<document::XActionLockable> xActionLockable(xDoc, uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->addActionLock();
    UnlockSolarMutex();
}

// XServiceInfo
::rtl::OUString SAL_CALL ScXMLImport::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    switch( getImportFlags() )
    {
        case IMPORT_ALL:
            return ScXMLImport_getImplementationName();
        case (IMPORT_STYLES|IMPORT_MASTERSTYLES|IMPORT_AUTOSTYLES|IMPORT_FONTDECLS):
            return ScXMLImport_Styles_getImplementationName();
        case (IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS):
            return ScXMLImport_Content_getImplementationName();
        case IMPORT_META:
            return ScXMLImport_Meta_getImplementationName();
        case IMPORT_SETTINGS:
            return ScXMLImport_Settings_getImplementationName();
        default:
            // generic name for 'unknown' cases
            return ScXMLImport_getImplementationName();
    }
    return SvXMLImport::getImplementationName();
}

// ::com::sun::star::xml::sax::XDocumentHandler
void SAL_CALL ScXMLImport::startDocument(void)
    throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    LockSolarMutex();
    SvXMLImport::startDocument();
    if (pDoc && !pDoc->IsImportingXML())
    {
        ScModelObj::getImplementation(GetModel())->BeforeXMLLoading();
        bSelfImportingXMLSet = sal_True;
    }

    // if content and styles are loaded with separate imports,
    // set bLatinDefaultStyle flag at the start of the content import
    sal_uInt16 nFlags = getImportFlags();
    if ( ( nFlags & IMPORT_CONTENT ) && !( nFlags & IMPORT_STYLES ) )
        ExamineDefaultStyle();

    UnlockSolarMutex();
}

sal_Int32 ScXMLImport::GetRangeType(const rtl::OUString sRangeType) const
{
    sal_Int32 nRangeType(0);
    rtl::OUStringBuffer sBuffer;
    sal_Int16 i = 0;
    while (i <= sRangeType.getLength())
    {
        if ((sRangeType[i] == ' ') || (i == sRangeType.getLength()))
        {
            rtl::OUString sTemp = sBuffer.makeStringAndClear();
            if (sTemp.compareToAscii(SC_REPEAT_COLUMN) == 0)
                nRangeType |= sheet::NamedRangeFlag::COLUMN_HEADER;
            else if (sTemp.compareToAscii(SC_REPEAT_ROW) == 0)
                nRangeType |= sheet::NamedRangeFlag::ROW_HEADER;
            else if (sTemp.compareToAscii(SC_FILTER) == 0)
                 nRangeType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
            else if (sTemp.compareToAscii(SC_PRINT_RANGE) == 0)
                 nRangeType |= sheet::NamedRangeFlag::PRINT_AREA;
        }
        else if (i < sRangeType.getLength())
            sBuffer.append(sRangeType[i]);
        ++i;
    }
    return nRangeType;
}

void ScXMLImport::SetLabelRanges()
{
    ScMyLabelRanges* pLabelRanges = GetLabelRanges();
    if (pLabelRanges)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aColAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_COLLABELRNG)));
            uno::Any aRowAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ROWLABELRNG)));

            uno::Reference< sheet::XLabelRanges > xColRanges;
            uno::Reference< sheet::XLabelRanges > xRowRanges;

            if ( ( aColAny >>= xColRanges ) && ( aRowAny >>= xRowRanges ) )
            {
                table::CellRangeAddress aLabelRange;
                table::CellRangeAddress aDataRange;

                ScMyLabelRanges::iterator aItr = pLabelRanges->begin();
                while (aItr != pLabelRanges->end())
                {
                    sal_Int32 nOffset1(0);
                    sal_Int32 nOffset2(0);

                    if (ScRangeStringConverter::GetRangeFromString( aLabelRange, (*aItr)->sLabelRangeStr, GetDocument(), nOffset1 ) &&
                        ScRangeStringConverter::GetRangeFromString( aDataRange, (*aItr)->sDataRangeStr, GetDocument(), nOffset2 ))
                    {
                        if ( (*aItr)->bColumnOrientation )
                            xColRanges->addNew( aLabelRange, aDataRange );
                        else
                            xRowRanges->addNew( aLabelRange, aDataRange );
                    }

                    delete *aItr;
                    aItr = pLabelRanges->erase(aItr);
                }
            }
        }
    }
}

void ScXMLImport::SetNamedRanges()
{
    ScMyNamedExpressions* pNamedExpressions(GetNamedExpressions());
    if (pNamedExpressions)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Reference <sheet::XNamedRanges> xNamedRanges(xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NAMEDRANGES))), uno::UNO_QUERY);
            if (xNamedRanges.is())
            {
                ScMyNamedExpressions::iterator aItr(pNamedExpressions->begin());
                ScMyNamedExpressions::const_iterator aEndItr(pNamedExpressions->end());
                table::CellAddress aCellAddress;
                rtl::OUString sTempContent(RTL_CONSTASCII_USTRINGPARAM("0"));
                while (aItr != aEndItr)
                {
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetAddressFromString(
                        aCellAddress, (*aItr)->sBaseCellAddress, GetDocument(), nOffset ))
                    {
                        try
                        {
                            xNamedRanges->addNewByName((*aItr)->sName, sTempContent, aCellAddress, GetRangeType((*aItr)->sRangeType));
                        }
                        catch( uno::RuntimeException& )
                        {
                            DBG_ERROR("here are some Named Ranges with the same name");
                            uno::Reference < container::XIndexAccess > xIndex(xNamedRanges, uno::UNO_QUERY);
                            if (xIndex.is())
                            {
                                sal_Int32 nMax(xIndex->getCount());
                                sal_Bool bInserted(sal_False);
                                sal_Int32 nCount(1);
                                rtl::OUStringBuffer sName((*aItr)->sName);
                                sName.append(sal_Unicode('_'));
                                while (!bInserted && nCount <= nMax)
                                {
                                    rtl::OUStringBuffer sTemp(sName);
                                    sTemp.append(rtl::OUString::valueOf(nCount));
                                    try
                                    {
                                        xNamedRanges->addNewByName(sTemp.makeStringAndClear(), sTempContent, aCellAddress, GetRangeType((*aItr)->sRangeType));
                                        bInserted = sal_True;
                                    }
                                    catch( uno::RuntimeException& )
                                    {
                                        ++nCount;
                                    }
                                }
                            }
                        }
                    }
                    ++aItr;
                }
                aItr = pNamedExpressions->begin();
                while (aItr != aEndItr)
                {
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetAddressFromString(
                        aCellAddress, (*aItr)->sBaseCellAddress, GetDocument(), nOffset ))
                    {
                        sTempContent = (*aItr)->sContent;
                        ScXMLConverter::ParseFormula(sTempContent, (*aItr)->bIsExpression);
                        uno::Reference <sheet::XNamedRange> xNamedRange(xNamedRanges->getByName((*aItr)->sName), uno::UNO_QUERY);
                        if (xNamedRange.is())
                            xNamedRange->setContent(sTempContent);
                    }
                    delete *aItr;
                    aItr = pNamedExpressions->erase(aItr);
                }
            }
        }
    }
}

void SAL_CALL ScXMLImport::endDocument(void)
    throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    LockSolarMutex();
    if (getImportFlags() & IMPORT_CONTENT)
    {
        if (GetModel().is())
        {
            uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetModel(), uno::UNO_QUERY);
            if (xViewDataSupplier.is())
            {
                uno::Reference<container::XIndexAccess> xIndexAccess(xViewDataSupplier->getViewData());
                if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
                {
                    uno::Sequence< beans::PropertyValue > aSeq;
                    if (xIndexAccess->getByIndex(0) >>= aSeq)
                    {
                        sal_Int32 nCount (aSeq.getLength());
                        for (sal_Int32 i = 0; i < nCount; ++i)
                        {
                            rtl::OUString sName(aSeq[i].Name);
                            if (sName.compareToAscii(SC_ACTIVETABLE) == 0)
                            {
                                rtl::OUString sValue;
                                if(aSeq[i].Value >>= sValue)
                                {
                                    String sTabName(sValue);
                                    SCTAB nTab(0);
                                    if (pDoc->GetTable(sTabName, nTab))
                                    {
                                        pDoc->SetVisibleTab(nTab);
                                        i = nCount;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            SetLabelRanges();
            SetNamedRanges();
        }
        GetProgressBarHelper()->End();  // make room for subsequent SfxProgressBars
        if (pDoc)
            pDoc->CompileXML();
        aTables.UpdateRowHeights();
        aTables.ResizeShapes();

        SetDefaultNotes();
    }
    if (GetModel().is())
    {
        uno::Reference<document::XActionLockable> xActionLockable(GetModel(), uno::UNO_QUERY);
        if (xActionLockable.is())
            xActionLockable->removeActionLock();
    }
    SvXMLImport::endDocument();

    if (pDoc && bSelfImportingXMLSet)
    {
        ScModelObj::getImplementation(GetModel())->AfterXMLLoading(sal_True);
    }

    UnlockSolarMutex();
}

void ScXMLImport::AddDefaultNote(const table::CellAddress& aCell)
{
    if (!pDefaultNotes)
        pDefaultNotes = new ScMyDefaultNotes();

    pDefaultNotes->push_back(aCell);
}

void ScXMLImport::SetDefaultNotes()
{
    if (pDefaultNotes && pDoc)
    {
        ScMyDefaultNotes::iterator aItr(pDefaultNotes->begin());
        ScMyDefaultNotes::iterator aEndItr(pDefaultNotes->end());
        ScPostIt aNote(pDoc);
        while(aItr != aEndItr)
        {
            if (pDoc->GetNote(static_cast<SCCOL>(aItr->Column), static_cast<SCROW>(aItr->Row), aItr->Sheet, aNote))
            {
                aNote.SetRectangle(aNote.MimicOldRectangle(ScAddress(static_cast<SCCOL>(aItr->Column), static_cast<SCROW>(aItr->Row), aItr->Sheet)));
                pDoc->SetNote(static_cast<SCCOL>(aItr->Column), static_cast<SCROW>(aItr->Row), aItr->Sheet, aNote);
            }
            ++aItr;
        }
    }
}

// XEventListener
void ScXMLImport::DisposingModel()
{
    SvXMLImport::DisposingModel();
    pDoc = NULL;
}

void ScXMLImport::LockSolarMutex()
{
    // #i62677# When called from DocShell/Wrapper, the SolarMutex is already locked,
    // so there's no need to allocate (and later delete) the ScUnoGuard.
    if (bFromWrapper)
    {
        DBG_TESTSOLARMUTEX();
        return;
    }

    if (nSolarMutexLocked == 0)
    {
        DBG_ASSERT(!pScUnoGuard, "Solar Mutex is locked");
        pScUnoGuard = new ScUnoGuard();
    }
    ++nSolarMutexLocked;
}


void ScXMLImport::UnlockSolarMutex()
{
    if (nSolarMutexLocked > 0)
    {
        nSolarMutexLocked--;
        if (nSolarMutexLocked == 0)
        {
            DBG_ASSERT(pScUnoGuard, "Solar Mutex is always unlocked");
            delete pScUnoGuard;
            pScUnoGuard = NULL;
        }
    }
}

void ScXMLImport::SetRangeOverflowType(sal_uInt32 nType)
{
    //  #i31130# Overflow is stored in the document, because the ScXMLImport object
    //  isn't available in ScXMLImportWrapper::ImportFromComponent when using the
    //  OOo->Oasis transformation.

    if ( pDoc )
        pDoc->SetRangeOverflowType( nType );
}

void ScXMLImport::ProgressBarIncrement(sal_Bool bEditCell, sal_Int32 nInc)
{
    nProgressCount += nInc;
    if (bEditCell || nProgressCount > 100)
    {
        GetProgressBarHelper()->Increment(nProgressCount);
        nProgressCount = 0;
    }
}

