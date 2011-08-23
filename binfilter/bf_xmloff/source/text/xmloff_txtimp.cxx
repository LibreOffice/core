/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/solar.h>
#ifndef _SVSTDARR_STRINGSDTOR_DECL
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>

#include "xmlnmspe.hxx"
#include "txtstyli.hxx"

#include "xmlnumi.hxx"

#include "txtparai.hxx"
#include "txtprmap.hxx"
#include "txtimppr.hxx"
#include "xmlimp.hxx"
#include "txtvfldi.hxx"
#include "i18nmap.hxx"
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"
#include "XMLSectionImportContext.hxx"
#include "XMLIndexTOCContext.hxx"
#include "XMLFontStylesContext.hxx"
#include "nmspmap.hxx"
#include "XMLEventsImportContext.hxx"
#include "XMLTrackedChangesImportContext.hxx"
#include "XMLChangeImportContext.hxx"
#include "XMLAutoMarkFileContext.hxx"

#include "XMLCalculationSettingsContext.hxx"
#include "XMLNumberStylesImport.hxx"
namespace binfilter {

using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::binfilter::xmloff::token;
using ::com::sun::star::util::DateTime;
using namespace ::com::sun::star::ucb;
using ::comphelper::UStringLess;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

static __FAR_DATA SvXMLTokenMapEntry aTextElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_P, 				XML_TOK_TEXT_P				},
    { XML_NAMESPACE_TEXT, XML_H, 				XML_TOK_TEXT_H				},
    { XML_NAMESPACE_TEXT, XML_ORDERED_LIST, 	XML_TOK_TEXT_ORDERED_LIST	},
    { XML_NAMESPACE_TEXT, XML_UNORDERED_LIST,	XML_TOK_TEXT_UNORDERED_LIST },
    { XML_NAMESPACE_DRAW, XML_TEXT_BOX, 		XML_TOK_TEXT_TEXTBOX_PAGE },
    { XML_NAMESPACE_DRAW, XML_IMAGE, 			XML_TOK_TEXT_IMAGE_PAGE },
    { XML_NAMESPACE_DRAW, XML_OBJECT, 			XML_TOK_TEXT_OBJECT_PAGE },
    { XML_NAMESPACE_DRAW, XML_OBJECT_OLE,		XML_TOK_TEXT_OBJECT_OLE_PAGE },
    { XML_NAMESPACE_DRAW, XML_APPLET,			XML_TOK_TEXT_APPLET_PAGE },
    { XML_NAMESPACE_DRAW, XML_PLUGIN,			XML_TOK_TEXT_PLUGIN_PAGE },
    { XML_NAMESPACE_DRAW, XML_FLOATING_FRAME,	XML_TOK_TEXT_FLOATING_FRAME_PAGE },
    { XML_NAMESPACE_DRAW, XML_A, 				XML_TOK_DRAW_A_PAGE },
    { XML_NAMESPACE_TABLE,XML_TABLE,			XML_TOK_TABLE_TABLE 		},
//	{ XML_NAMESPACE_TABLE,XML_SUB_TABLE,		XML_TOK_TABLE_SUBTABLE 		},
    { XML_NAMESPACE_TEXT, XML_VARIABLE_DECLS,	XML_TOK_TEXT_VARFIELD_DECLS },
    { XML_NAMESPACE_TEXT, XML_USER_FIELD_DECLS, XML_TOK_TEXT_USERFIELD_DECLS },
    { XML_NAMESPACE_TEXT, XML_SEQUENCE_DECLS,	XML_TOK_TEXT_SEQUENCE_DECLS },
    { XML_NAMESPACE_TEXT, XML_DDE_CONNECTION_DECLS, XML_TOK_TEXT_DDE_DECLS },
    { XML_NAMESPACE_TEXT, XML_SECTION,			XML_TOK_TEXT_SECTION },
    { XML_NAMESPACE_TEXT, XML_TABLE_OF_CONTENT, XML_TOK_TEXT_TOC },
    { XML_NAMESPACE_TEXT, XML_OBJECT_INDEX, 	XML_TOK_TEXT_OBJECT_INDEX },
    { XML_NAMESPACE_TEXT, XML_TABLE_INDEX,		XML_TOK_TEXT_TABLE_INDEX },
    { XML_NAMESPACE_TEXT, XML_ILLUSTRATION_INDEX, XML_TOK_TEXT_ILLUSTRATION_INDEX },
    { XML_NAMESPACE_TEXT, XML_USER_INDEX,		XML_TOK_TEXT_USER_INDEX },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX, XML_TOK_TEXT_ALPHABETICAL_INDEX },
    { XML_NAMESPACE_TEXT, XML_BIBLIOGRAPHY,     XML_TOK_TEXT_BIBLIOGRAPHY_INDEX },
    { XML_NAMESPACE_TEXT, XML_INDEX_TITLE,		XML_TOK_TEXT_INDEX_TITLE },
    { XML_NAMESPACE_TEXT, XML_TRACKED_CHANGES,  XML_TOK_TEXT_TRACKED_CHANGES },
    { XML_NAMESPACE_TEXT, XML_CHANGE_START,	    XML_TOK_TEXT_CHANGE_START },
    { XML_NAMESPACE_TEXT, XML_CHANGE_END, 		XML_TOK_TEXT_CHANGE_END },
    { XML_NAMESPACE_TEXT, XML_CHANGE, 			XML_TOK_TEXT_CHANGE },
    { XML_NAMESPACE_OFFICE, XML_FORMS,			XML_TOK_TEXT_FORMS },
    { XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS,	XML_TOK_TEXT_CALCULATION_SETTINGS },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX_AUTO_MARK_FILE, XML_TOK_TEXT_AUTOMARK },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_SPAN, XML_TOK_TEXT_SPAN },
    { XML_NAMESPACE_TEXT, XML_TAB_STOP, XML_TOK_TEXT_TAB_STOP },
    { XML_NAMESPACE_TEXT, XML_LINE_BREAK, XML_TOK_TEXT_LINE_BREAK },
    { XML_NAMESPACE_TEXT, XML_S, XML_TOK_TEXT_S },
    { XML_NAMESPACE_TEXT, XML_A, XML_TOK_TEXT_HYPERLINK },
    { XML_NAMESPACE_TEXT, XML_RUBY, XML_TOK_TEXT_RUBY },

    { XML_NAMESPACE_TEXT, XML_FOOTNOTE, XML_TOK_TEXT_FOOTNOTE },
    { XML_NAMESPACE_TEXT, XML_ENDNOTE, XML_TOK_TEXT_ENDNOTE },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK, XML_TOK_TEXT_BOOKMARK },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK_START, XML_TOK_TEXT_BOOKMARK_START },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK_END, XML_TOK_TEXT_BOOKMARK_END },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK, XML_TOK_TEXT_REFERENCE },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK_START,
      XML_TOK_TEXT_REFERENCE_START },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK_END,
      XML_TOK_TEXT_REFERENCE_END },

    { XML_NAMESPACE_DRAW, XML_TEXT_BOX, XML_TOK_TEXT_TEXTBOX },
    { XML_NAMESPACE_DRAW, XML_IMAGE, XML_TOK_TEXT_IMAGE },
    { XML_NAMESPACE_DRAW, XML_OBJECT, XML_TOK_TEXT_OBJECT },
    { XML_NAMESPACE_DRAW, XML_OBJECT_OLE, XML_TOK_TEXT_OBJECT_OLE },
    { XML_NAMESPACE_DRAW, XML_APPLET, 			XML_TOK_TEXT_APPLET },
    { XML_NAMESPACE_DRAW, XML_FLOATING_FRAME, 	XML_TOK_TEXT_FLOATING_FRAME },
    { XML_NAMESPACE_DRAW, XML_PLUGIN, 			XML_TOK_TEXT_PLUGIN },
    { XML_NAMESPACE_DRAW, XML_A, 				XML_TOK_DRAW_A },

    // index marks
    { XML_NAMESPACE_TEXT, XML_TOC_MARK, XML_TOK_TEXT_TOC_MARK },
    { XML_NAMESPACE_TEXT, XML_TOC_MARK_START, XML_TOK_TEXT_TOC_MARK_START },
    { XML_NAMESPACE_TEXT, XML_TOC_MARK_END, XML_TOK_TEXT_TOC_MARK_END },
    { XML_NAMESPACE_TEXT, XML_USER_INDEX_MARK, XML_TOK_TEXT_USER_INDEX_MARK },
    { XML_NAMESPACE_TEXT, XML_USER_INDEX_MARK_START,
      XML_TOK_TEXT_USER_INDEX_MARK_START },
    { XML_NAMESPACE_TEXT, XML_USER_INDEX_MARK_END,
      XML_TOK_TEXT_USER_INDEX_MARK_END },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX_MARK,
      XML_TOK_TEXT_ALPHA_INDEX_MARK },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX_MARK_START,
      XML_TOK_TEXT_ALPHA_INDEX_MARK_START },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX_MARK_END,
      XML_TOK_TEXT_ALPHA_INDEX_MARK_END },

    // sender fields
    { XML_NAMESPACE_TEXT, XML_SENDER_FIRSTNAME,XML_TOK_TEXT_SENDER_FIRSTNAME},
    { XML_NAMESPACE_TEXT, XML_SENDER_LASTNAME, XML_TOK_TEXT_SENDER_LASTNAME },
    { XML_NAMESPACE_TEXT, XML_SENDER_INITIALS, XML_TOK_TEXT_SENDER_INITIALS },
    { XML_NAMESPACE_TEXT, XML_SENDER_TITLE, XML_TOK_TEXT_SENDER_TITLE },
    { XML_NAMESPACE_TEXT, XML_SENDER_POSITION, XML_TOK_TEXT_SENDER_POSITION },
    { XML_NAMESPACE_TEXT, XML_SENDER_EMAIL, XML_TOK_TEXT_SENDER_EMAIL },
    { XML_NAMESPACE_TEXT, XML_SENDER_PHONE_PRIVATE,
      XML_TOK_TEXT_SENDER_PHONE_PRIVATE },
    { XML_NAMESPACE_TEXT, XML_SENDER_FAX, XML_TOK_TEXT_SENDER_FAX },
    { XML_NAMESPACE_TEXT, XML_SENDER_COMPANY, XML_TOK_TEXT_SENDER_COMPANY },
    { XML_NAMESPACE_TEXT, XML_SENDER_PHONE_WORK,
      XML_TOK_TEXT_SENDER_PHONE_WORK },
    { XML_NAMESPACE_TEXT, XML_SENDER_STREET, XML_TOK_TEXT_SENDER_STREET },
    { XML_NAMESPACE_TEXT, XML_SENDER_CITY, XML_TOK_TEXT_SENDER_CITY },
    { XML_NAMESPACE_TEXT, XML_SENDER_POSTAL_CODE,
      XML_TOK_TEXT_SENDER_POSTAL_CODE },
    { XML_NAMESPACE_TEXT, XML_SENDER_COUNTRY, XML_TOK_TEXT_SENDER_COUNTRY },
    { XML_NAMESPACE_TEXT, XML_SENDER_STATE_OR_PROVINCE,
      XML_TOK_TEXT_SENDER_STATE_OR_PROVINCE },

    // misc. document fields
    { XML_NAMESPACE_TEXT, XML_AUTHOR_NAME, XML_TOK_TEXT_AUTHOR_NAME },
    { XML_NAMESPACE_TEXT, XML_AUTHOR_INITIALS, XML_TOK_TEXT_AUTHOR_INITIALS },
    { XML_NAMESPACE_TEXT, XML_DATE, XML_TOK_TEXT_DATE },
    { XML_NAMESPACE_TEXT, XML_TIME, XML_TOK_TEXT_TIME },
    { XML_NAMESPACE_TEXT, XML_PAGE_NUMBER, XML_TOK_TEXT_PAGE_NUMBER },
    { XML_NAMESPACE_TEXT, XML_PAGE_CONTINUATION_STRING,
      XML_TOK_TEXT_PAGE_CONTINUATION_STRING },

    // variable fields
    { XML_NAMESPACE_TEXT, XML_VARIABLE_SET, XML_TOK_TEXT_VARIABLE_SET },
    { XML_NAMESPACE_TEXT, XML_VARIABLE_GET, XML_TOK_TEXT_VARIABLE_GET },
    { XML_NAMESPACE_TEXT, XML_VARIABLE_INPUT, XML_TOK_TEXT_VARIABLE_INPUT },
    { XML_NAMESPACE_TEXT, XML_USER_FIELD_GET, XML_TOK_TEXT_USER_FIELD_GET },
    { XML_NAMESPACE_TEXT, XML_USER_FIELD_INPUT,XML_TOK_TEXT_USER_FIELD_INPUT},
    { XML_NAMESPACE_TEXT, XML_SEQUENCE, XML_TOK_TEXT_SEQUENCE },
    { XML_NAMESPACE_TEXT, XML_EXPRESSION, XML_TOK_TEXT_EXPRESSION },
    { XML_NAMESPACE_TEXT, XML_TEXT_INPUT, XML_TOK_TEXT_TEXT_INPUT },

    // database fields
    { XML_NAMESPACE_TEXT, XML_DATABASE_DISPLAY,
      XML_TOK_TEXT_DATABASE_DISPLAY },
    { XML_NAMESPACE_TEXT, XML_DATABASE_NEXT,
      XML_TOK_TEXT_DATABASE_NEXT },
    { XML_NAMESPACE_TEXT, XML_DATABASE_SELECT,
      XML_TOK_TEXT_DATABASE_SELECT },
    { XML_NAMESPACE_TEXT, XML_DATABASE_ROW_NUMBER,
      XML_TOK_TEXT_DATABASE_ROW_NUMBER },
    { XML_NAMESPACE_TEXT, XML_DATABASE_NAME, XML_TOK_TEXT_DATABASE_NAME },

    // docinfo fields
    { XML_NAMESPACE_TEXT, XML_INITIAL_CREATOR,
      XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR },
    { XML_NAMESPACE_TEXT, XML_DESCRIPTION, XML_TOK_TEXT_DOCUMENT_DESCRIPTION},
    { XML_NAMESPACE_TEXT, XML_USER_INFO_0,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_0 },
    { XML_NAMESPACE_TEXT, XML_USER_INFO_1,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_1 },
    { XML_NAMESPACE_TEXT, XML_USER_INFO_2,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_2 },
    { XML_NAMESPACE_TEXT, XML_USER_INFO_3,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_3 },
    { XML_NAMESPACE_TEXT, XML_PRINTED_BY, XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR},
    { XML_NAMESPACE_TEXT, XML_TITLE, XML_TOK_TEXT_DOCUMENT_TITLE },
    { XML_NAMESPACE_TEXT, XML_SUBJECT, XML_TOK_TEXT_DOCUMENT_SUBJECT },
    { XML_NAMESPACE_TEXT, XML_KEYWORDS, XML_TOK_TEXT_DOCUMENT_KEYWORDS },
    { XML_NAMESPACE_TEXT, XML_CREATOR, XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR },
    { XML_NAMESPACE_TEXT, XML_EDITING_CYCLES,
      XML_TOK_TEXT_DOCUMENT_REVISION },
    { XML_NAMESPACE_TEXT, XML_CREATION_DATE,
      XML_TOK_TEXT_DOCUMENT_CREATION_DATE },
    { XML_NAMESPACE_TEXT, XML_CREATION_TIME,
      XML_TOK_TEXT_DOCUMENT_CREATION_TIME },
    { XML_NAMESPACE_TEXT, XML_PRINT_DATE, XML_TOK_TEXT_DOCUMENT_PRINT_DATE },
    { XML_NAMESPACE_TEXT, XML_PRINT_TIME, XML_TOK_TEXT_DOCUMENT_PRINT_TIME },
    { XML_NAMESPACE_TEXT, XML_MODIFICATION_DATE,
      XML_TOK_TEXT_DOCUMENT_SAVE_DATE },
    { XML_NAMESPACE_TEXT, XML_MODIFICATION_TIME,
      XML_TOK_TEXT_DOCUMENT_SAVE_TIME },
    { XML_NAMESPACE_TEXT, XML_EDITING_DURATION,
      XML_TOK_TEXT_DOCUMENT_EDIT_DURATION },
    { XML_NAMESPACE_TEXT, XML_USER_DEFINED,
      XML_TOK_TEXT_DOCUMENT_USER_DEFINED },

    // misc fields
    { XML_NAMESPACE_TEXT, XML_PLACEHOLDER, XML_TOK_TEXT_PLACEHOLDER },
    { XML_NAMESPACE_TEXT, XML_HIDDEN_TEXT, XML_TOK_TEXT_HIDDEN_TEXT },
    { XML_NAMESPACE_TEXT, XML_HIDDEN_PARAGRAPH,
      XML_TOK_TEXT_HIDDEN_PARAGRAPH },
    { XML_NAMESPACE_TEXT, XML_CONDITIONAL_TEXT,
      XML_TOK_TEXT_CONDITIONAL_TEXT },
    { XML_NAMESPACE_TEXT, XML_FILE_NAME, XML_TOK_TEXT_FILENAME },
    { XML_NAMESPACE_TEXT, XML_CHAPTER,	XML_TOK_TEXT_CHAPTER },
    { XML_NAMESPACE_TEXT, XML_TEMPLATE_NAME, XML_TOK_TEXT_TEMPLATENAME },
    { XML_NAMESPACE_TEXT, XML_PARAGRAPH_COUNT, XML_TOK_TEXT_PARAGRAPH_COUNT },
    { XML_NAMESPACE_TEXT, XML_WORD_COUNT, XML_TOK_TEXT_WORD_COUNT },
    { XML_NAMESPACE_TEXT, XML_TABLE_COUNT, XML_TOK_TEXT_TABLE_COUNT },
    { XML_NAMESPACE_TEXT, XML_CHARACTER_COUNT, XML_TOK_TEXT_CHARACTER_COUNT },
    { XML_NAMESPACE_TEXT, XML_IMAGE_COUNT, XML_TOK_TEXT_IMAGE_COUNT },
    { XML_NAMESPACE_TEXT, XML_OBJECT_COUNT, XML_TOK_TEXT_OBJECT_COUNT },
    { XML_NAMESPACE_TEXT, XML_PAGE_COUNT, XML_TOK_TEXT_PAGE_COUNT },
    { XML_NAMESPACE_TEXT, XML_PAGE_VARIABLE_GET, XML_TOK_TEXT_GET_PAGE_VAR },
    { XML_NAMESPACE_TEXT, XML_PAGE_VARIABLE_SET, XML_TOK_TEXT_SET_PAGE_VAR },
    { XML_NAMESPACE_TEXT, XML_EXECUTE_MACRO, XML_TOK_TEXT_MACRO },
    { XML_NAMESPACE_TEXT, XML_DDE_CONNECTION, XML_TOK_TEXT_DDE },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_REF, XML_TOK_TEXT_REFERENCE_REF },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK_REF, XML_TOK_TEXT_BOOKMARK_REF },
    { XML_NAMESPACE_TEXT, XML_SEQUENCE_REF, XML_TOK_TEXT_SEQUENCE_REF },
    { XML_NAMESPACE_TEXT, XML_FOOTNOTE_REF, XML_TOK_TEXT_FOOTNOTE_REF },
    { XML_NAMESPACE_TEXT, XML_ENDNOTE_REF, XML_TOK_TEXT_ENDNOTE_REF },
    { XML_NAMESPACE_TEXT, XML_BIBLIOGRAPHY_MARK,
      XML_TOK_TEXT_BIBLIOGRAPHY_MARK },
    { XML_NAMESPACE_OFFICE, XML_ANNOTATION, XML_TOK_TEXT_ANNOTATION },
    { XML_NAMESPACE_TEXT, XML_SCRIPT, XML_TOK_TEXT_SCRIPT },
    { XML_NAMESPACE_TEXT, XML_TABLE_FORMULA, XML_TOK_TEXT_TABLE_FORMULA },
    { XML_NAMESPACE_TEXT, XML_DROPDOWN, XML_TOK_TEXT_DROPDOWN },

    // Calc fields
    { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_TEXT_SHEET_NAME },

    // draw fields
    { XML_NAMESPACE_TEXT, XML_MEASURE,	XML_TOK_TEXT_MEASURE },

    // redlining (aka change tracking)
    { XML_NAMESPACE_TEXT, XML_CHANGE_START, XML_TOK_TEXTP_CHANGE_START },
    { XML_NAMESPACE_TEXT, XML_CHANGE_END  , XML_TOK_TEXTP_CHANGE_END },
    { XML_NAMESPACE_TEXT, XML_CHANGE, XML_TOK_TEXTP_CHANGE },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,	XML_TOK_TEXT_P_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_COND_STYLE_NAME,
                                            XML_TOK_TEXT_P_COND_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_LEVEL, 		XML_TOK_TEXT_P_LEVEL },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextListBlockAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,
            XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_CONTINUE_NUMBERING,
            XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextListBlockElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_LIST_HEADER, XML_TOK_TEXT_LIST_HEADER },
    { XML_NAMESPACE_TEXT, XML_LIST_ITEM, 	XML_TOK_TEXT_LIST_ITEM	 },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextFrameAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_STYLE_NAME, XML_TOK_TEXT_FRAME_STYLE_NAME },
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_TEXT_FRAME_NAME },
    { XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_TOK_TEXT_FRAME_ANCHOR_TYPE },
    { XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER, XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER },
    { XML_NAMESPACE_SVG, XML_X, XML_TOK_TEXT_FRAME_X },
    { XML_NAMESPACE_SVG, XML_Y, XML_TOK_TEXT_FRAME_Y },
    { XML_NAMESPACE_SVG, XML_WIDTH, XML_TOK_TEXT_FRAME_WIDTH },
    { XML_NAMESPACE_STYLE, XML_REL_WIDTH, XML_TOK_TEXT_FRAME_REL_WIDTH },
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_TOK_TEXT_FRAME_HEIGHT },
    { XML_NAMESPACE_FO, XML_MIN_HEIGHT, XML_TOK_TEXT_FRAME_MIN_HEIGHT },
    { XML_NAMESPACE_STYLE, XML_REL_HEIGHT, XML_TOK_TEXT_FRAME_REL_HEIGHT },
    { XML_NAMESPACE_DRAW, XML_CHAIN_NEXT_NAME, XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME },
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_TEXT_FRAME_HREF },
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_TEXT_FRAME_FILTER_NAME },
    { XML_NAMESPACE_DRAW, XML_ZINDEX, XML_TOK_TEXT_FRAME_Z_INDEX },
    { XML_NAMESPACE_SVG, XML_TRANSFORM, XML_TOK_TEXT_FRAME_TRANSFORM },
    { XML_NAMESPACE_DRAW, XML_CLASS_ID, XML_TOK_TEXT_FRAME_CLASS_ID },
    { XML_NAMESPACE_DRAW,  	XML_CODE,       	XML_TOK_TEXT_FRAME_CODE },
    { XML_NAMESPACE_DRAW,  	XML_OBJECT, 		XML_TOK_TEXT_FRAME_OBJECT },
    { XML_NAMESPACE_DRAW,  	XML_ARCHIVE, 		XML_TOK_TEXT_FRAME_ARCHIVE },
    { XML_NAMESPACE_DRAW,   XML_MAY_SCRIPT, 	XML_TOK_TEXT_FRAME_MAY_SCRIPT },
    { XML_NAMESPACE_DRAW,   XML_MIME_TYPE, 	XML_TOK_TEXT_FRAME_MIME_TYPE },
    { XML_NAMESPACE_DRAW, XML_APPLET_NAME, XML_TOK_TEXT_FRAME_APPLET_NAME },
    { XML_NAMESPACE_DRAW, XML_FRAME_NAME, XML_TOK_TEXT_FRAME_FRAME_NAME },
    { XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE },
    { XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_TABLE, XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextContourAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG, XML_WIDTH, 	XML_TOK_TEXT_CONTOUR_WIDTH		},
    { XML_NAMESPACE_SVG, XML_HEIGHT,	XML_TOK_TEXT_CONTOUR_HEIGHT		},
    { XML_NAMESPACE_SVG, XML_VIEWBOX,	XML_TOK_TEXT_CONTOUR_VIEWBOX	},
    { XML_NAMESPACE_SVG, XML_D,	    	XML_TOK_TEXT_CONTOUR_D			},
    { XML_NAMESPACE_DRAW,XML_POINTS,	XML_TOK_TEXT_CONTOUR_POINTS		},
    { XML_NAMESPACE_DRAW,XML_RECREATE_ON_EDIT,	XML_TOK_TEXT_CONTOUR_AUTO	},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextHyperlinkAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_TEXT_HYPERLINK_HREF },
    { XML_NAMESPACE_OFFICE, XML_NAME, XML_TOK_TEXT_HYPERLINK_NAME },
    { XML_NAMESPACE_XLINK, XML_SHOW, XML_TOK_TEXT_HYPERLINK_SHOW },
    { XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME, XML_TOK_TEXT_HYPERLINK_TARGET_FRAME },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_TEXT_HYPERLINK_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_VISITED_STYLE_NAME, XML_TOK_TEXT_HYPERLINK_VIS_STYLE_NAME },
    { XML_NAMESPACE_OFFICE, XML_SERVER_MAP, XML_TOK_TEXT_HYPERLINK_SERVER_MAP },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextMasterPageElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_HEADER, XML_TOK_TEXT_MP_HEADER },
    { XML_NAMESPACE_STYLE, XML_FOOTER, XML_TOK_TEXT_MP_FOOTER },
    { XML_NAMESPACE_STYLE, XML_HEADER_LEFT, XML_TOK_TEXT_MP_HEADER_LEFT },
    { XML_NAMESPACE_STYLE, XML_FOOTER_LEFT, XML_TOK_TEXT_MP_FOOTER_LEFT },

    XML_TOKEN_MAP_END
};

// maximum allowed length of combined characters field
#define MAX_COMBINED_CHARACTERS 6


XMLTextImportHelper::XMLTextImportHelper(
        const Reference < XModel >& rModel,
        SvXMLImport& rImport,
        sal_Bool bInsertM, sal_Bool bStylesOnlyM,
        sal_Bool bPrg,
        sal_Bool bBlockM,
        sal_Bool bOrganizerM ) :
    pTextElemTokenMap( 0 ),
    pTextPElemTokenMap( 0 ),
    pTextPAttrTokenMap( 0 ),
    pTextListBlockAttrTokenMap( 0 ),
    pTextListBlockElemTokenMap( 0 ),
    pTextFieldAttrTokenMap( 0 ),
    pTextFrameAttrTokenMap( 0 ),
    pTextContourAttrTokenMap( 0 ),
    pTextHyperlinkAttrTokenMap( 0 ),
    pTextMasterPageElemTokenMap( 0 ),
    pPrevFrmNames( 0 ),
    pNextFrmNames( 0 ),
    pRenameMap( 0 ),
    pOutlineStyles( 0 ),
    bInsertMode( bInsertM ),
    bBlockMode( bBlockM ),
    bStylesOnlyMode( bStylesOnlyM ),
    bProgress( bPrg ),
    bOrganizerMode( bOrganizerM ),
    bBodyContentStarted( sal_True ),
    
    // #107848#
    // Initialize inside_deleted_section flag correctly
    bInsideDeleteContext( sal_False ),

    pFootnoteBackpatcher( NULL ),
    pSequenceIdBackpatcher( NULL ),
    pSequenceNameBackpatcher( NULL ),
    xServiceFactory( rModel, UNO_QUERY ),
    sParaStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")),
    sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
    sHeadingStyleName(RTL_CONSTASCII_USTRINGPARAM("HeadingStyleName")),
    sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel")),
    sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue")),
    sParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart")),
    sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")),
    sSequenceNumber(RTL_CONSTASCII_USTRINGPARAM("SequenceNumber")),
    sSourceName(RTL_CONSTASCII_USTRINGPARAM("SourceName")),
    sCurrentPresentation(RTL_CONSTASCII_USTRINGPARAM("CurrentPresentation")),
    sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sChainPrevName(RTL_CONSTASCII_USTRINGPARAM("ChainPrevName")),
    sHyperLinkURL(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")),
    sHyperLinkName(RTL_CONSTASCII_USTRINGPARAM("HyperLinkName")),
    sHyperLinkTarget(RTL_CONSTASCII_USTRINGPARAM("HyperLinkTarget")),
    sUnvisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("UnvisitedCharStyleName")),
    sVisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("VisitedCharStyleName")),
    sTextFrame(RTL_CONSTASCII_USTRINGPARAM("TextFrame")),
    sPageDescName(RTL_CONSTASCII_USTRINGPARAM("PageDescName")),
    sServerMap(RTL_CONSTASCII_USTRINGPARAM("ServerMap")),
    sHyperLinkEvents(RTL_CONSTASCII_USTRINGPARAM("HyperLinkEvents")),
    sContent(RTL_CONSTASCII_USTRINGPARAM("Content")),
    sServiceCombinedCharacters(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.text.TextField.CombinedCharacters"))
{
    Reference< XChapterNumberingSupplier > xCNSupplier( rModel, UNO_QUERY );

    if( xCNSupplier.is() )
        xChapterNumbering = xCNSupplier->getChapterNumberingRules();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( rModel, UNO_QUERY );
//	DBG_ASSERT( xFamiliesSupp.is(), "no chapter numbering supplier" ); for clipboard there may be documents without styles

    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies = xFamiliesSupp->getStyleFamilies();

        const OUString aParaStyles(RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles"));
        if( xFamilies->hasByName( aParaStyles ) )
        {
            Any aAny( xFamilies->getByName( aParaStyles ) );
            aAny >>= xParaStyles;
        }

        const OUString aCharStyles(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles"));
        if( xFamilies->hasByName( aCharStyles ) )
        {
            Any	aAny( xFamilies->getByName( aCharStyles ) );
            aAny >>= xTextStyles;
        }

        const OUString aNumStyles(RTL_CONSTASCII_USTRINGPARAM("NumberingStyles"));
        if( xFamilies->hasByName( aNumStyles ) )
        {
            Any	aAny( xFamilies->getByName( aNumStyles ) );
            aAny >>= xNumStyles;
        }

        const OUString aFrameStyles(RTL_CONSTASCII_USTRINGPARAM("FrameStyles"));
        if( xFamilies->hasByName( aFrameStyles ) )
        {
            Any	aAny( xFamilies->getByName( aFrameStyles ) );
            aAny >>= xFrameStyles;
        }

        const OUString aPageStyles(RTL_CONSTASCII_USTRINGPARAM("PageStyles"));
        if( xFamilies->hasByName( aPageStyles ) )
        {
            Any aAny( xFamilies->getByName( aPageStyles ) );
            aAny >>= xPageStyles;
        }
    }

    Reference < XTextFramesSupplier > xTFS( rModel, UNO_QUERY );
    if( xTFS.is() )
        xTextFrames = xTFS->getTextFrames();

    Reference < XTextGraphicObjectsSupplier > xTGOS( rModel, UNO_QUERY );
    if( xTGOS.is() )
        xGraphics = xTGOS->getGraphicObjects();

    Reference < XTextEmbeddedObjectsSupplier > xTEOS( rModel, UNO_QUERY );
    if( xTEOS.is() )
        xObjects = xTEOS->getEmbeddedObjects();

    XMLPropertySetMapper *pPropMapper =
            new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA );
    xParaImpPrMap = new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    xTextImpPrMap = new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME );
    xFrameImpPrMap = new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_SECTION );
    xSectionImpPrMap = new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_RUBY );
    xRubyImpPrMap = new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

XMLTextImportHelper::~XMLTextImportHelper()
{
    delete pTextElemTokenMap;
    delete pTextPElemTokenMap;
    delete pTextPAttrTokenMap;
    delete pTextListBlockAttrTokenMap;
    delete pTextListBlockElemTokenMap;
    delete pTextFieldAttrTokenMap;
    delete pTextFrameAttrTokenMap;
    delete pTextContourAttrTokenMap;
    delete pTextHyperlinkAttrTokenMap;
    delete pTextMasterPageElemTokenMap;

    delete pRenameMap;

    delete pPrevFrmNames;
    delete pNextFrmNames;
    delete [] pOutlineStyles;

    _FinitBackpatcher();
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateShapeExtPropMapper(SvXMLImport& rImport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME );		
    return new XMLTextImportPropertyMapper( pPropMapper, rImport, 
                   const_cast<XMLFontStylesContext*>(rImport.GetFontDecls()) );
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateCharExtPropMapper(SvXMLImport& rImport, XMLFontStylesContext *pFontDecls)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    if (!pFontDecls)
        pFontDecls = const_cast<XMLFontStylesContext*>(rImport.GetFontDecls());
    return new XMLTextImportPropertyMapper( pPropMapper, rImport, pFontDecls );
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateParaExtPropMapper(SvXMLImport& rImport, XMLFontStylesContext *pFontDecls)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE_PARA );
    if (!pFontDecls)
        pFontDecls = const_cast<XMLFontStylesContext*>(rImport.GetFontDecls());
    return new XMLTextImportPropertyMapper( pPropMapper, rImport, pFontDecls );
}

void XMLTextImportHelper::SetCursor( const Reference < XTextCursor > & rCursor )
{
    xCursor = rCursor;
    xText = rCursor->getText();
    xCursorAsRange = Reference < XTextRange >( rCursor, UNO_QUERY );
}

void XMLTextImportHelper::ResetCursor()
{
    xCursor = 0;
    xText = 0;
    xCursorAsRange = 0;
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextElemTokenMap()
{
    return new SvXMLTokenMap( aTextElemTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextPElemTokenMap()
{
    return new SvXMLTokenMap( aTextPElemTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextPAttrTokenMap()
{
    return new SvXMLTokenMap( aTextPAttrTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextFrameAttrTokenMap()
{
    return new SvXMLTokenMap( aTextFrameAttrTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextContourAttrTokenMap()
{
    return new SvXMLTokenMap( aTextContourAttrTokenMap );
}


SvXMLTokenMap *XMLTextImportHelper::_GetTextHyperlinkAttrTokenMap()
{
    return new SvXMLTokenMap( aTextHyperlinkAttrTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextMasterPageElemTokenMap()
{
    return new SvXMLTokenMap( aTextMasterPageElemTokenMap );
}

sal_Bool XMLTextImportHelper::HasFrameByName( const OUString& rName ) const
{
    return ( xTextFrames.is() && xTextFrames->hasByName( rName ) ) ||
           ( xGraphics.is() && xGraphics->hasByName( rName ) ) ||
           ( xObjects.is() && xObjects->hasByName( rName ) );
}

void XMLTextImportHelper::InsertString( const OUString& rChars )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertString( xCursorAsRange, rChars, sal_False );
}

void XMLTextImportHelper::InsertString( const OUString& rChars,
                                         sal_Bool& rIgnoreLeadingSpace )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
    {
        sal_Int32 nLen = rChars.getLength();
        OUStringBuffer sChars( nLen );

        for( sal_Int32 i=0; i < nLen; i++ )
        {
            sal_Unicode c = rChars[i];
            switch( c )
            {
                case 0x20:
                case 0x09:
                case 0x0a:
                case 0x0d:
                    if( !rIgnoreLeadingSpace )
                        sChars.append( (sal_Unicode)0x20 );
                    rIgnoreLeadingSpace = sal_True;
                    break;
                default:
                    rIgnoreLeadingSpace = sal_False;
                    sChars.append( c );
                    break;
            }
        }
        xText->insertString( xCursorAsRange, sChars.makeStringAndClear(),
                             sal_False );
    }
}
void XMLTextImportHelper::InsertControlCharacter( sal_Int16 nControl )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertControlCharacter( xCursorAsRange, nControl, sal_False );
}

void XMLTextImportHelper::InsertTextContent(
    Reference < XTextContent > & xContent )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertTextContent( xCursorAsRange, xContent, sal_False );
}

void XMLTextImportHelper::DeleteParagraph()
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursor.is(), "no cursor" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );

    sal_Bool bDelete = sal_True;
    Reference < XEnumerationAccess > xEnumAccess( xCursor, UNO_QUERY );
    if( xEnumAccess.is() )
    {
        Reference < XEnumeration > xEnum = xEnumAccess->createEnumeration();
        DBG_ASSERT( xEnum->hasMoreElements(), "empty text enumeration" );
        if( xEnum->hasMoreElements() )
        {
            Any aAny = xEnum->nextElement();
            Reference < XTextRange > xTxtRange;
            aAny >>= xTxtRange;
            Reference < XComponent > xComp( xTxtRange, UNO_QUERY );
            DBG_ASSERT( xComp.is(), "got no component" );
            if( xComp.is() )
            {
                xComp->dispose();
                bDelete = sal_False;
            }
        }
    }
    if( bDelete )
    {
        if( xCursor->goLeft( 1, sal_True ) )
        {
            OUString sEmpty;
            xText->insertString( xCursorAsRange, sEmpty, sal_True );
        }
    }
}

#ifdef CONV_STAR_FONTS
OUString XMLTextImportHelper::ConvertStarFonts( const OUString& rChars,
                                                const OUString& rStyleName,
                                                sal_uInt8& rFlags,
                                                 sal_Bool bPara,
                                                SvXMLImport& rImport ) const
{
    OUStringBuffer sChars( rChars );
    sal_Bool bConverted = sal_False;
    for( sal_Int32 i=0; i<rChars.getLength(); i++ )
    {
        sal_Unicode c = rChars[i];
        if( c >= 0xf000 && c <= 0xf0ff )
        {
            if( (rFlags & CONV_STAR_FONT_FLAGS_VALID) == 0 )
            {
                XMLTextStyleContext *pStyle = 0;
                sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                           : XML_STYLE_FAMILY_TEXT_TEXT;
                if( rStyleName.getLength() && xAutoStyles.Is() )
                {
                    pStyle = PTR_CAST( XMLTextStyleContext,
                          ((SvXMLStylesContext *)&xAutoStyles)->
                                FindStyleChildContext( nFamily, rStyleName,
                                                       sal_True ) );
                }

                if( pStyle )
                {
                    sal_Int32 nCount = pStyle->_GetProperties().size();
                    if( nCount )
                    {
                        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                            ((SvXMLStylesContext *)&xAutoStyles)->GetImportPropertyMapper(nFamily);
                        if( xImpPrMap.is() )
                        {
                            UniReference<XMLPropertySetMapper> rPropMapper =
                                xImpPrMap->getPropertySetMapper();
                            for( sal_Int32 i=0; i < nCount; i++ )
                            {
                                const XMLPropertyState& rProp = pStyle->_GetProperties()[i];
                                sal_Int32 nIdx = rProp.mnIndex;
                                sal_uInt32 nContextId = rPropMapper->GetEntryContextId(nIdx);
                                if( CTF_FONTFAMILYNAME == nContextId )
                                {
                                    rFlags &= ~(CONV_FROM_STAR_BATS|CONV_FROM_STAR_MATH);
                                    OUString sFontName;
                                    rProp.maValue >>= sFontName;
                                    OUString sStarBats( RTL_CONSTASCII_USTRINGPARAM("StarBats" ) );
                                    OUString sStarMath( RTL_CONSTASCII_USTRINGPARAM("StarMath" ) );
                                    if( sFontName.equalsIgnoreAsciiCase( sStarBats  ) )
                                        rFlags |= CONV_FROM_STAR_BATS;
                                    else if( sFontName.equalsIgnoreAsciiCase( sStarMath ) )
                                        rFlags |= CONV_FROM_STAR_MATH;
                                    break;
                                }
                            }
                        }
                    }

                }

                rFlags |= CONV_STAR_FONT_FLAGS_VALID;
            }
            if( (rFlags & CONV_FROM_STAR_BATS ) != 0 )
            {
                sChars.setCharAt( i, rImport.ConvStarBatsCharToStarSymbol( c ) );
                bConverted = sal_True;
            }
            else if( (rFlags & CONV_FROM_STAR_MATH ) != 0 )
            {
                sChars.setCharAt( i, rImport.ConvStarMathCharToStarSymbol( c ) );
                bConverted = sal_True;
            }
        }
    }

    return bConverted ? sChars.makeStringAndClear() : rChars;
}
#endif

OUString XMLTextImportHelper::SetStyleAndAttrs(
        const Reference < XTextCursor >& rCursor,
        const OUString& rStyleName,
        sal_Bool bPara )
{
    XMLTextStyleContext *pStyle = 0;
    OUString sStyleName( rStyleName );
    if( sStyleName.getLength() && xAutoStyles.Is() )
    {
        sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                   : XML_STYLE_FAMILY_TEXT_TEXT;
        pStyle = PTR_CAST( XMLTextStyleContext,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( nFamily, sStyleName, sal_True ) );
    }
    if( pStyle )
        sStyleName = pStyle->GetParent();

    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();

    // style
    if( sStyleName.getLength() )
    {
        const String& rPropName = bPara ? sParaStyleName : sCharStyleName;
        const Reference < XNameContainer > & rStyles = bPara ? xParaStyles
                                                          : xTextStyles;
        if( rStyles.is() &&
            xPropSetInfo->hasPropertyByName( rPropName ) &&
            rStyles->hasByName( sStyleName ) )
        {
            Any aAny;
            aAny <<= sStyleName;
            xPropSet->setPropertyValue( rPropName, aAny );
        }
        else
            sStyleName = OUString();
    }

    if( bPara && xPropSetInfo->hasPropertyByName( sNumberingRules )  )
    {
        // Set numbering rules
        Reference < XIndexReplace > xNumRules;
        Any aAny = xPropSet->getPropertyValue( sNumberingRules );
        aAny >>= xNumRules;

        if( IsInList() )
        {
            XMLTextListBlockContext *pListBlock = GetListBlock();
            Reference < XIndexReplace > xNewNumRules =
                pListBlock->GetNumRules();

            sal_Bool bSameNumRules = xNewNumRules == xNumRules;
            if( !bSameNumRules && xNewNumRules.is() && xNumRules.is() )
            {
                // If the interface pointers are different then this does
                // not mean that the num rules are different. Further tests
                // are rquired then. However, if only one num rule is
                // set, no tests are required of course.
                Reference< XNamed > xNewNamed( xNewNumRules, UNO_QUERY );
                Reference< XNamed > xNamed( xNumRules, UNO_QUERY );
                if( xNewNamed.is() && xNamed.is() )
                {
                    bSameNumRules = xNewNamed->getName() == xNamed->getName();
                }
                else
                {
                    if( !xNumRuleCompare.is() )
                    {
                        Reference<XAnyCompareFactory> xCompareFac( xServiceFactory, UNO_QUERY );
                        OSL_ENSURE( xCompareFac.is(), "got no XAnyCompareFactory" );
                        if( xCompareFac.is() )
                        {
                            xNumRuleCompare =
                                xCompareFac->createAnyCompareByName(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            "NumberingRules" ) ) );
                            OSL_ENSURE( xNumRuleCompare .is(),
                                    "got no Numbering Rules comparison" );
                        }
                    }
                    if( xNumRuleCompare.is() )
                    {
                        Any aNewAny;
                        aNewAny <<= xNewNumRules;
                        bSameNumRules = (xNumRuleCompare->compare( aAny,
                                                                aNewAny ) == 0);
                    }
                }
            }

            if( !bSameNumRules )
            {
                aAny <<= xNewNumRules;

                // #102607# This may except when xNewNumRules contains
                // a Writer-NumRule-Implementation bug gets applied to
                // a shape. Since this may occur inside a document
                // (e.g. when edited), this must be handled
                // gracefully.
                try
                {
                    xPropSet->setPropertyValue( sNumberingRules, aAny );
                }
                catch( Exception e )
                {
                    ; // I would really like to use a warning here,
                      // but I can't access the XMLErrorHandler from
                      // here.
                }
            }

            XMLTextListItemContext *pListItem = GetListItem();

            sal_Int8 nLevel = (sal_Int8)pListBlock->GetLevel();
            if( !pListItem &&
                xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
            {
                sal_Bool bTmp = sal_False;
                aAny.setValue( &bTmp, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sNumberingIsNumber, aAny );
            }

            aAny <<= nLevel;
            xPropSet->setPropertyValue( sNumberingLevel, aAny );

            if( pListBlock->IsRestartNumbering() )
            {
                // TODO: property missing
                if( xPropSetInfo->hasPropertyByName( sParaIsNumberingRestart ) )
                {
                    sal_Bool bTmp = sal_True;
                    aAny.setValue( &bTmp, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sParaIsNumberingRestart, aAny );
                }
                pListBlock->ResetRestartNumbering();
            }
            if( pListItem && pListItem->HasStartValue() &&
                xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                aAny <<= pListItem->GetStartValue();
                xPropSet->setPropertyValue( sNumberingStartValue, aAny );
            }
            SetListItem( (XMLTextListItemContext *)0 );
        }
        else
        {
            // If the paragraph is not in a list but its style, remove it from
            // the list.
            if( xNumRules.is() )
            {
                aAny.clear();
                xPropSet->setPropertyValue( sNumberingRules, aAny );
            }
        }
    }

    // hard paragraph properties
    if( pStyle )
    {
        pStyle->FillPropertySet( xPropSet );
        if( bPara && pStyle->HasMasterPageName() &&
            xPropSetInfo->hasPropertyByName( sPageDescName ) )
        {
            const OUString& rMasterPageName = pStyle->GetMasterPageName();
            if( !rMasterPageName.getLength() ||
                (xPageStyles.is() &&
                 xPageStyles->hasByName( rMasterPageName )) )
            {
                Any aAny;
                aAny <<= rMasterPageName;
                xPropSet->setPropertyValue( sPageDescName, aAny );
            }
        }
        if( bPara && pStyle->GetDropCapStyleName().getLength() &&
            xTextStyles.is() &&
            xTextStyles->hasByName( pStyle->GetDropCapStyleName() ) &&
            xPropSetInfo->hasPropertyByName( pStyle->sDropCapCharStyleName ) )
        {
            Any aAny;
            aAny <<= pStyle->GetDropCapStyleName();
            xPropSet->setPropertyValue( pStyle->sDropCapCharStyleName, aAny );
        }

        // combined characters special treatment
        if (!bPara && pStyle->HasCombinedCharactersLetter())
        {
            // insert combined characters text field
            if( xServiceFactory.is() )
            {
                Reference<XInterface> xIfc =
                   xServiceFactory->createInstance(sServiceCombinedCharacters);
                if( xIfc.is() )
                {
                    // fix cursor if larger than possible for
                    // combined characters field
                    if (rCursor->getString().getLength() >
                            MAX_COMBINED_CHARACTERS)
                    {
                        rCursor->gotoRange(rCursor->getStart(), sal_False);
                        rCursor->goRight(MAX_COMBINED_CHARACTERS, sal_True);
                    }

                    // set field value (the combined character string)
                    Reference<XPropertySet> xTmp( xIfc, UNO_QUERY );
                    Any aAny;
                    aAny <<= rCursor->getString();
                    xTmp->setPropertyValue(sContent, aAny);

                    // insert the field over it's original text
                    Reference<XTextRange> xRange(rCursor, UNO_QUERY);
                    Reference<XTextContent> xTextContent(xTmp, UNO_QUERY);
                    if (xText.is() && xRange.is())
                    {
                        xText->insertTextContent( xRange, xTextContent,
                                                  sal_True );
                    }
                }
            }
        }
    }

    return sStyleName;
}

void XMLTextImportHelper::FindOutlineStyleName( 
    ::rtl::OUString& rStyleName, 
    sal_Int8 nLevel )
{
    // style name empty?
    if( rStyleName.getLength() == 0 )
    {
        // Empty? Then we need o do stuff. Let's do error checking first.
        if( xChapterNumbering.is() && 
            ( nLevel > 0 ) && 
            ( nLevel <= xChapterNumbering->getCount() ) )
        {
            nLevel--;   // for the remainder, the level's are 0-based

            // empty style name: look-up previously used name

            // if we don't have a previously used name, we'll use the default
            if( pOutlineStyles == NULL )
                pOutlineStyles = new OUString[xChapterNumbering->getCount()];

            if( pOutlineStyles[nLevel] == NULL )
            {
                // no other name used previously? Then use default

                // iterate over property value sequence to find the style name
                Sequence<PropertyValue> aProperties;
                xChapterNumbering->getByIndex( nLevel ) >>= aProperties;
                for( sal_Int32 i = 0; i < aProperties.getLength(); i++ )
                {
                    if( aProperties[i].Name == sHeadingStyleName )
                    {
                        aProperties[i].Value >>= pOutlineStyles[nLevel];
                        break;  // early out, if we found it!.
                    }
                }
            }

            // finally, we'll use the previously used style name for this
            // format (or the default we've just put into that style)
            rStyleName = pOutlineStyles[nLevel];
        }
        // else: nothing we can do, so we'll leave it empty
    }
    // else: we already had a style name, so we let it pass.
}

void XMLTextImportHelper::SetOutlineStyle(
        sal_Int8 nLevel,
        const OUString& rStyleName )
{
    if( rStyleName.getLength() &&
        xChapterNumbering.is() &&
        nLevel > 0 && nLevel <= xChapterNumbering->getCount() )
    {
        if( !pOutlineStyles )
        {
            pOutlineStyles = new OUString[xChapterNumbering->getCount()];
        }
        pOutlineStyles[nLevel-1] = rStyleName;
    }
}

void XMLTextImportHelper::SetOutlineStyles()
{
    if( pOutlineStyles &&
        xChapterNumbering.is() && 
        !( IsInsertMode() || IsStylesOnlyMode() ) )
    {
        sal_Int32 nCount = xChapterNumbering->getCount();
        for( sal_Int32 i=0; i < nCount; i++ )
        {
            Sequence < PropertyValue > aProps( 1 );
            PropertyValue *pProps = aProps.getArray();
            pProps->Name = sHeadingStyleName;
            pProps->Value <<= pOutlineStyles[i];

            Any aAny;
            aAny <<= aProps;
            xChapterNumbering->replaceByIndex( i, aAny );
        }
    }
}

void XMLTextImportHelper::SetHyperlink(
    const Reference < XTextCursor >& rCursor,
    const OUString& rHRef,
    const OUString& rName,
    const OUString& rTargetFrameName,
    const OUString& rStyleName,
    const OUString& rVisitedStyleName,
    XMLEventsImportContext* pEvents)
{
    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference < XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( !xPropSetInfo.is() || !xPropSetInfo->hasPropertyByName(sHyperLinkURL) )
        return;

    Any aAny;
    aAny <<= rHRef;
    xPropSet->setPropertyValue( sHyperLinkURL, aAny );

    if( xPropSetInfo->hasPropertyByName( sHyperLinkName ) )
    {
        aAny <<= rName;
        xPropSet->setPropertyValue( sHyperLinkName, aAny );
    }

    if( xPropSetInfo->hasPropertyByName( sHyperLinkTarget ) )
    {
        aAny <<= rTargetFrameName;
        xPropSet->setPropertyValue( sHyperLinkTarget, aAny );
    }

    if ( (pEvents != NULL) &&
         xPropSetInfo->hasPropertyByName( sHyperLinkEvents ))
    {
        // The API treats events at hyperlinks differently from most
        // other properties: You have to set a name replace with the
        // events in it. The easiest way to to this is to 1) get
        // events, 2) set new ones, and 3) then put events back.
        aAny = xPropSet->getPropertyValue( sHyperLinkEvents );
        Reference<XNameReplace> xReplace;
        aAny >>= xReplace;
        if (xReplace.is())
        {
            // set events
            pEvents->SetEvents(xReplace);

            // put events
            aAny <<= xReplace;
            xPropSet->setPropertyValue( sHyperLinkEvents, aAny );
        }
    }

    if( xTextStyles.is() )
    {
        if( rStyleName.getLength() &&
            xPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName ) &&
            xTextStyles->hasByName( rStyleName ) )
        {
            aAny <<= rStyleName;
            xPropSet->setPropertyValue( sUnvisitedCharStyleName, aAny );
        }

        if( rVisitedStyleName.getLength() &&
            xPropSetInfo->hasPropertyByName( sVisitedCharStyleName ) &&
            xTextStyles->hasByName( rVisitedStyleName ) )
        {
            aAny <<= rVisitedStyleName;
            xPropSet->setPropertyValue( sVisitedCharStyleName, aAny );
        }
    }
}

void XMLTextImportHelper::SetRuby(
    const Reference < XTextCursor >& rCursor,
    const OUString& rStyleName,
    const OUString& rTextStyleName,
    const OUString& rText )
{
    Reference<XPropertySet> xPropSet(rCursor, UNO_QUERY);

    OUString sRubyText(RTL_CONSTASCII_USTRINGPARAM("RubyText"));
    OUString sRubyCharStyleName(RTL_CONSTASCII_USTRINGPARAM("RubyCharStyleName"));

    // if we have one Ruby property, we assume all of them are present
    if (xPropSet.is() &&
        xPropSet->getPropertySetInfo()->hasPropertyByName( sRubyText ))
    {
        Any aAny;

        // the ruby text
        aAny <<= rText;
        xPropSet->setPropertyValue(sRubyText, aAny);

        // the ruby style (ruby-adjust)
        XMLPropStyleContext *pStyle = 0;
        if( rStyleName.getLength() && xAutoStyles.Is() )
        {
            pStyle = PTR_CAST(
                XMLPropStyleContext,
                ((SvXMLStylesContext *)&xAutoStyles)->
                FindStyleChildContext( XML_STYLE_FAMILY_TEXT_RUBY,
                                       rStyleName, sal_True ) );

            if (NULL != pStyle)
                pStyle->FillPropertySet( xPropSet );
        }

        // the ruby text character style
        if( xTextStyles.is() )
        {
            if( (rTextStyleName.getLength() > 0) &&
//				xPropSetInfo->hasPropertyByName( sRubyCharStyleName ) &&
                xTextStyles->hasByName( rTextStyleName ) )
            {
                aAny <<= rTextStyleName;
                xPropSet->setPropertyValue(sRubyCharStyleName, aAny);
            }
        }
    }
}

void XMLTextImportHelper::SetAutoStyles( SvXMLStylesContext *pStyles )
{
    xAutoStyles = pStyles;
}

void XMLTextImportHelper::SetFontDecls( XMLFontStylesContext *pFontDecls )
{
    xFontDecls = pFontDecls;
    ((XMLTextImportPropertyMapper *)xParaImpPrMap.get())
        ->SetFontDecls( pFontDecls );
    ((XMLTextImportPropertyMapper *)xTextImpPrMap.get())
        ->SetFontDecls( pFontDecls );
}

const XMLFontStylesContext *XMLTextImportHelper::GetFontDecls() const
{
    return (XMLFontStylesContext *)&xFontDecls;
}

sal_Bool XMLTextImportHelper::HasDrawNameAttribute(
        const Reference< XAttributeList > & xAttrList,
        SvXMLNamespaceMap& rNamespaceMap )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            rNamespaceMap.GetKeyByAttrName( rAttrName, &aLocalName );
        if( XML_NAMESPACE_DRAW == nPrefix &&
            IsXMLToken( aLocalName, XML_NAME ) )
        {
            return xAttrList->getValueByIndex(i).getLength() != 0;
        }
    }

    return sal_False;
}

SvXMLImportContext *XMLTextImportHelper::CreateTextChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList,
        XMLTextType eType )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetTextElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;
    sal_Bool bContent = sal_True;
    sal_Bool bObjectOLE = sal_False;
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
    case XML_TOK_TEXT_H:
        bHeading = sal_True;
    case XML_TOK_TEXT_P:
        pContext = new XMLParaContext( rImport,
                                       nPrefix, rLocalName,
                                       xAttrList, bHeading );
        if( bProgress && XML_TEXT_TYPE_SHAPE != eType )
        {
            rImport.GetProgressBarHelper()->Increment();
        }
        break;
    case XML_TOK_TEXT_ORDERED_LIST:
        bOrdered = sal_True;
    case XML_TOK_TEXT_UNORDERED_LIST:
        pContext = new XMLTextListBlockContext( rImport, *this,
                                                nPrefix, rLocalName,
                                                xAttrList, bOrdered );
        break;
    case XML_TOK_TABLE_TABLE:
        if( XML_TEXT_TYPE_BODY == eType ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_SECTION == eType ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType ||
            XML_TEXT_TYPE_CHANGED_REGION == eType)
            pContext = CreateTableChildContext( rImport, nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TEXT_SEQUENCE_DECLS:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this,	nPrefix, rLocalName, VarTypeSequence);
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_VARFIELD_DECLS:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSimple);
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_USERFIELD_DECLS:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted)||
            XML_TEXT_TYPE_HEADER_FOOTER == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeUserField);
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_DDE_DECLS:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType )
        {
            pContext = new XMLDdeFieldDeclsImportContext(
                rImport, nPrefix, rLocalName);
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_TEXTBOX_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
            XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            if( HasDrawNameAttribute( xAttrList, rImport.GetNamespaceMap() ) )
            {
                TextContentAnchorType eAnchorType =
                    XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                                   : TextContentAnchorType_AT_PAGE;
                pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                    rLocalName, xAttrList,
                                                    eAnchorType,
                                                    XML_TEXT_FRAME_TEXTBOX );
            }
            else
            {
                Reference < XShapes > xShapes;
                pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                        rImport, nPrefix, rLocalName, xAttrList, xShapes );
            }
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_IMAGE_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
            XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            if( HasDrawNameAttribute( xAttrList, rImport.GetNamespaceMap() ) )
            {
                TextContentAnchorType eAnchorType =
                    XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                                   : TextContentAnchorType_AT_PAGE;
                pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                    rLocalName, xAttrList,
                                                    eAnchorType,
                                                    XML_TEXT_FRAME_GRAPHIC );
            }
            else
            {
                Reference < XShapes > xShapes;
                pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                        rImport, nPrefix, rLocalName, xAttrList, xShapes );
            }
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_OBJECT_OLE_PAGE:
        bObjectOLE = sal_True;
    case XML_TOK_TEXT_OBJECT_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                    rLocalName, xAttrList, eAnchorType,
                                    bObjectOLE ? XML_TEXT_FRAME_OBJECT_OLE
                                               : XML_TEXT_FRAME_OBJECT );
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_APPLET_PAGE:
    case XML_TOK_TEXT_PLUGIN_PAGE:
    case XML_TOK_TEXT_FLOATING_FRAME_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            sal_uInt16 nType;
            switch( nToken )
            {
            case XML_TOK_TEXT_APPLET_PAGE:
                nType = XML_TEXT_FRAME_APPLET;
                break;
            case XML_TOK_TEXT_PLUGIN_PAGE:
                nType = XML_TEXT_FRAME_PLUGIN;
                break;
            case XML_TOK_TEXT_FLOATING_FRAME_PAGE:
                nType = XML_TEXT_FRAME_FLOATING_FRAME;
                break;
            }
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                    rLocalName, xAttrList, eAnchorType,
                                    nType );
            bContent = sal_False;
        }
        break;

    case XML_TOK_DRAW_A_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_CHANGED_REGION == eType)
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameHyperlinkContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType );
            bContent = sal_False;
        }
        break;

    case XML_TOK_TEXT_INDEX_TITLE:
    case XML_TOK_TEXT_SECTION:
#ifndef SVX_LIGHT
        pContext = new XMLSectionImportContext( rImport, nPrefix, rLocalName );
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        break;

    case XML_TOK_TEXT_TOC:
    case XML_TOK_TEXT_OBJECT_INDEX:
    case XML_TOK_TEXT_TABLE_INDEX:
    case XML_TOK_TEXT_ILLUSTRATION_INDEX:
    case XML_TOK_TEXT_USER_INDEX:
    case XML_TOK_TEXT_ALPHABETICAL_INDEX:
    case XML_TOK_TEXT_BIBLIOGRAPHY_INDEX:
#ifndef SVX_LIGHT
        if( XML_TEXT_TYPE_SHAPE != eType )
            pContext = new XMLIndexTOCContext( rImport, nPrefix, rLocalName );
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        break;

    case XML_TOK_TEXT_TRACKED_CHANGES:
#ifndef SVX_LIGHT
        pContext = new XMLTrackedChangesImportContext( rImport, nPrefix,
                                                       rLocalName);
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        bContent = sal_False;
        break;

    case XML_TOK_TEXT_CHANGE:
    case XML_TOK_TEXT_CHANGE_START:
    case XML_TOK_TEXT_CHANGE_END:
        pContext = new XMLChangeImportContext(
            rImport, nPrefix, rLocalName,
            (XML_TOK_TEXT_CHANGE_END != nToken),
            (XML_TOK_TEXT_CHANGE_START != nToken),
            sal_True);
        break;

    case XML_TOK_TEXT_FORMS:
#ifndef SVX_LIGHT
        pContext = rImport.GetFormImport()->createOfficeFormsContext(rImport, nPrefix, rLocalName);
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        bContent = sal_False;
        break;

    case XML_TOK_TEXT_AUTOMARK:
        if( XML_TEXT_TYPE_BODY == eType )
        {
            pContext = new XMLAutoMarkFileContext(rImport, nPrefix,rLocalName);
        }
        bContent = sal_False;
        break;

    case XML_TOK_TEXT_CALCULATION_SETTINGS:
#ifndef SVX_LIGHT
        pContext = new XMLCalculationSettingsContext ( rImport, nPrefix, rLocalName, xAttrList);
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        bContent = sal_False;
    break;

    default:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            Reference < XShapes > xShapes;
            pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                    rImport, nPrefix, rLocalName, xAttrList, xShapes );
            bContent = sal_False;
        }
    }

//	if( !pContext )
//		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    // handle open redlines
    if ( (XML_TOK_TEXT_CHANGE != nToken) &&
         (XML_TOK_TEXT_CHANGE_END != nToken) &&
         (XML_TOK_TEXT_CHANGE_START != nToken) )
    {
//		ResetOpenRedlineId();
    }

    if( XML_TEXT_TYPE_BODY == eType && bContent )
        bBodyContentStarted = sal_False;

    return pContext;
}

SvXMLImportContext *XMLTextImportHelper::CreateTableChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    return 0;
}

/// get data style key for use with NumberFormat property
sal_Int32 XMLTextImportHelper::GetDataStyleKey(const OUString& sStyleName,
                                               sal_Bool* pIsSystemLanguage )
{
    const SvXMLStyleContext* pStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                  FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE,
                                              sStyleName, sal_True );

    // get appropriate context


    // first check if its a impress and draw only number format
    // this is needed since its also a SvXMLNumFormatContext,
    // that was needed to support them for controls in impress/draw also
    SdXMLNumberFormatImportContext* pSdNumStyle = PTR_CAST( SdXMLNumberFormatImportContext, pStyle );
    if( pSdNumStyle )
    {
        return pSdNumStyle->GetDrawKey();
    }
    else
    {
        SvXMLNumFormatContext* pNumStyle = PTR_CAST( SvXMLNumFormatContext, pStyle );
        if( pNumStyle )
        {
            if( pIsSystemLanguage != NULL )
                *pIsSystemLanguage = pNumStyle->IsSystemLanguage();

            // return key
            return pNumStyle->GetKey();
        }
    }
    return -1;
}

const SvxXMLListStyleContext *XMLTextImportHelper::FindAutoListStyle( const OUString& rName ) const
{
    const SvxXMLListStyleContext *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( SvxXMLListStyleContext ,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_TEXT_LIST, rName,
                                           sal_True ) );
    }

    return pStyle;
}

XMLPropStyleContext *XMLTextImportHelper::FindAutoFrameStyle( const OUString& rName ) const
{
    XMLPropStyleContext *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( XMLPropStyleContext ,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rName,
                                           sal_True ) );
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindSectionStyle(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = NULL;
    if (xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( XMLPropStyleContext,
                           ((SvXMLStylesContext *)&xAutoStyles)->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_TEXT_SECTION,
                               rName, sal_True ) );
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindPageMaster(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = NULL;
    if (xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( XMLPropStyleContext,
                           ((SvXMLStylesContext *)&xAutoStyles)->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_PAGE_MASTER,
                               rName, sal_True ) );
    }

    return pStyle;
}

XMLTextListItemContext *XMLTextImportHelper::GetListItem()
{
    return (XMLTextListItemContext *)&xListItem;
}

void XMLTextImportHelper::SetListItem( XMLTextListItemContext *pListItem )
{
    xListItem = pListItem;
}

void XMLTextImportHelper::_SetListItem( SvXMLImportContext *pListItem )
{
    xListItem = PTR_CAST( XMLTextListItemContext, pListItem );
}

XMLTextListBlockContext *XMLTextImportHelper::GetListBlock()
{
    return (XMLTextListBlockContext *)&xListBlock;
}

void XMLTextImportHelper::SetListBlock( XMLTextListBlockContext *pListBlock )
{
    xListBlock = pListBlock;
}

void XMLTextImportHelper::_SetListBlock( SvXMLImportContext *pListBlock )
{
    xListBlock = PTR_CAST( XMLTextListBlockContext, pListBlock );
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockAttrTokenMap()
{
    if( !pTextListBlockAttrTokenMap )
        pTextListBlockAttrTokenMap =
            new SvXMLTokenMap( aTextListBlockAttrTokenMap );

    return *pTextListBlockAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockElemTokenMap()
{
    if( !pTextListBlockElemTokenMap )
        pTextListBlockElemTokenMap =
            new SvXMLTokenMap( aTextListBlockElemTokenMap );

    return *pTextListBlockElemTokenMap;
}

SvI18NMap& XMLTextImportHelper::GetRenameMap()
{
    if( 0 == pRenameMap )
        pRenameMap = new SvI18NMap();
    return *pRenameMap;
}

void XMLTextImportHelper::InsertBookmarkStartRange(
    const OUString sName,
    const Reference<XTextRange> & rRange)
{
    aBookmarkStartRanges[sName] = rRange;
}

sal_Bool XMLTextImportHelper::FindAndRemoveBookmarkStartRange(
    Reference<XTextRange> & rRange,
    const OUString sName)
{
    if (aBookmarkStartRanges.count(sName))
    {
        rRange = aBookmarkStartRanges[sName];
        aBookmarkStartRanges.erase(sName);
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}


void XMLTextImportHelper::ConnectFrameChains(
        const OUString& rFrmName,
        const OUString& rNextFrmName,
        const Reference < XPropertySet >& rFrmPropSet )
{
    if( !rFrmName.getLength() )
        return;

    if( rNextFrmName.getLength() )
    {
        OUString sNextFrmName = GetRenameMap().Get( XML_TEXT_RENAME_TYPE_FRAME,
                                                    rNextFrmName );
        if( xTextFrames.is() && xTextFrames->hasByName( sNextFrmName ) )
        {
            Any aAny;
            aAny <<= sNextFrmName;
            rFrmPropSet->setPropertyValue( sChainNextName, aAny );
        }
        else
        {
            if( !pPrevFrmNames )
            {
                pPrevFrmNames = new SvStringsDtor;
                pNextFrmNames = new SvStringsDtor;
            }
            pPrevFrmNames->Insert( new String( rFrmName ),
                                   pPrevFrmNames->Count() );
            pNextFrmNames->Insert( new String( sNextFrmName ),
                                   pNextFrmNames->Count() );
        }
    }
    if( pPrevFrmNames && pPrevFrmNames->Count() )
    {
        sal_uInt16 nCount = pPrevFrmNames->Count();
        for( sal_uInt16 i=0; i<nCount; i++ )
        {
            String *pNext = (*pNextFrmNames)[i];
            if( OUString(*pNext) == rFrmName )
            {
                // The previuous frame must exist, because it existing than
                // inserting the entry
                String *pPrev = (*pPrevFrmNames)[i];

                Any aAny;
                aAny <<= OUString( *pPrev );
                rFrmPropSet->setPropertyValue( sChainPrevName, aAny );

                pPrevFrmNames->Remove( i, 1 );
                pNextFrmNames->Remove( i, 1 );
                delete pPrev;
                delete pNext;

                // There cannot be more than one previous frames
                break;
            }
        }
    }
}

sal_Bool XMLTextImportHelper::IsInFrame() const
{
    sal_Bool bIsInFrame = sal_False;

    // are we currently in a text frame? yes, if the cursor has a
    // TextFrame property and it's non-NULL
    Reference<XPropertySet> xPropSet(((XMLTextImportHelper *)this)->GetCursor(), UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextFrame))
        {
            Any aAny = xPropSet->getPropertyValue(sTextFrame);
            Reference<XTextFrame> xFrame;
            aAny >>= xFrame;

            if (xFrame.is())
            {
                bIsInFrame = sal_True;
            }
        }
    }

    return bIsInFrame;
}

sal_Bool XMLTextImportHelper::IsInHeaderFooter() const
{
    return sal_False;
}

Reference< XPropertySet> XMLTextImportHelper::createAndInsertOLEObject(
                                        SvXMLImport& rImport,
                                        const OUString& rHRef,
                                        const OUString& rStyleName,
                                        const OUString& rTblName,
                                        sal_Int32 nWidth, sal_Int32 nHeight )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}


Reference< XPropertySet> XMLTextImportHelper::createAndInsertApplet(
                                        const OUString& rCode,
                                          const OUString& rName,
                                          sal_Bool bMayScript,
                                          const OUString& rHRef,
                                        sal_Int32 nWidth, sal_Int32 nHeight )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}
Reference< XPropertySet> XMLTextImportHelper::createAndInsertPlugin(
                                          const OUString& rMimeType,
                                          const OUString& rHRef,
                                        sal_Int32 nWidth, sal_Int32 nHeight )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}
Reference< XPropertySet> XMLTextImportHelper::createAndInsertFloatingFrame(
                                          const OUString& rName,
                                          const OUString& rHRef,
                                          const OUString& rStyleName,
                                        sal_Int32 nWidth, sal_Int32 nHeight )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}

void XMLTextImportHelper::endAppletOrPlugin(
        Reference < XPropertySet> & rPropSet,
        ::std::map < const ::rtl::OUString, ::rtl::OUString, UStringLess > &rParamMap)
{
}
// redline helper: dummy implementation to be overridden in sw/filter/xml
void XMLTextImportHelper::RedlineAdd(
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const DateTime& rDateTime,
    sal_Bool bMergeLastPara)
{
    // dummy implementation: do nothing
}

Reference<XTextCursor> XMLTextImportHelper::RedlineCreateText(
    Reference<XTextCursor> & rOldCursor,
    const OUString& rId)
{
    // dummy implementation: do nothing
    Reference<XTextCursor> xRet;
    return xRet;
}

void XMLTextImportHelper::RedlineSetCursor(
    const OUString& rId,
    sal_Bool bStart,
    sal_Bool bIsOutsideOfParagraph)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::RedlineAdjustStartNodeCursor(
    sal_Bool bStart)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetShowChanges( sal_Bool bShowChanges )
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetRecordChanges( sal_Bool bRecordChanges )
{
    // dummy implementation: do nothing
}
void XMLTextImportHelper::SetChangesProtectionKey(
    const Sequence<sal_Int8> & rProtectionKey )
{
    // dummy implementation: do nothing
}


OUString XMLTextImportHelper::GetOpenRedlineId()
{
    return sOpenRedlineIdentifier;
}

void XMLTextImportHelper::SetOpenRedlineId( ::rtl::OUString& rId)
{
    sOpenRedlineIdentifier = rId;
}

void XMLTextImportHelper::ResetOpenRedlineId()
{
    OUString sEmpty;
    SetOpenRedlineId(sEmpty);
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
