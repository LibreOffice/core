/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <tuple>
#include <vector>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/maptype.hxx>

#include <sal/log.hxx>
#include "txtparai.hxx"
#include <xmloff/txtprmap.hxx>
#include <xmloff/txtimppr.hxx>
#include <xmloff/xmlimp.hxx>
#include <txtvfldi.hxx>
#include <xmloff/i18nmap.hxx>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"
#include "XMLSectionImportContext.hxx"
#include "XMLIndexTOCContext.hxx"
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include "XMLTrackedChangesImportContext.hxx"
#include "XMLChangeImportContext.hxx"
#include "XMLAutoMarkFileContext.hxx"

#include "XMLCalculationSettingsContext.hxx"
#include <XMLNumberStylesImport.hxx>
// XML import: reconstrution of assignment of paragraph style to outline levels (#i69629#)
#include <com/sun/star/beans/XPropertyState.hpp>
#include <txtlists.hxx>
#include <xmloff/odffields.hxx>

using ::com::sun::star::ucb::XAnyCompare;

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;
using namespace ::com::sun::star::ucb;

static const SvXMLTokenMapEntry aTextElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_P,                XML_TOK_TEXT_P              },
    { XML_NAMESPACE_LO_EXT, XML_P,                XML_TOK_TEXT_P              },
    { XML_NAMESPACE_TEXT, XML_H,                XML_TOK_TEXT_H              },
    { XML_NAMESPACE_TEXT, XML_LIST,             XML_TOK_TEXT_LIST           },
    { XML_NAMESPACE_DRAW, XML_FRAME,            XML_TOK_TEXT_FRAME_PAGE     },
    { XML_NAMESPACE_DRAW, XML_A,                XML_TOK_DRAW_A_PAGE },
    { XML_NAMESPACE_TABLE,XML_TABLE,            XML_TOK_TABLE_TABLE         },
    { XML_NAMESPACE_LO_EXT,XML_TABLE,           XML_TOK_TABLE_TABLE         },
    { XML_NAMESPACE_TEXT, XML_VARIABLE_DECLS,   XML_TOK_TEXT_VARFIELD_DECLS },
    { XML_NAMESPACE_TEXT, XML_USER_FIELD_DECLS, XML_TOK_TEXT_USERFIELD_DECLS },
    { XML_NAMESPACE_TEXT, XML_SEQUENCE_DECLS,   XML_TOK_TEXT_SEQUENCE_DECLS },
    { XML_NAMESPACE_TEXT, XML_DDE_CONNECTION_DECLS, XML_TOK_TEXT_DDE_DECLS },
    { XML_NAMESPACE_TEXT, XML_SECTION,          XML_TOK_TEXT_SECTION },
    { XML_NAMESPACE_TEXT, XML_TABLE_OF_CONTENT, XML_TOK_TEXT_TOC },
    { XML_NAMESPACE_TEXT, XML_OBJECT_INDEX,     XML_TOK_TEXT_OBJECT_INDEX },
    { XML_NAMESPACE_TEXT, XML_TABLE_INDEX,      XML_TOK_TEXT_TABLE_INDEX },
    { XML_NAMESPACE_TEXT, XML_ILLUSTRATION_INDEX, XML_TOK_TEXT_ILLUSTRATION_INDEX },
    { XML_NAMESPACE_TEXT, XML_USER_INDEX,       XML_TOK_TEXT_USER_INDEX },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX, XML_TOK_TEXT_ALPHABETICAL_INDEX },
    { XML_NAMESPACE_TEXT, XML_BIBLIOGRAPHY,     XML_TOK_TEXT_BIBLIOGRAPHY_INDEX },
    { XML_NAMESPACE_TEXT, XML_INDEX_TITLE,      XML_TOK_TEXT_INDEX_TITLE },
    { XML_NAMESPACE_TEXT, XML_TRACKED_CHANGES,  XML_TOK_TEXT_TRACKED_CHANGES },
    { XML_NAMESPACE_TEXT, XML_CHANGE_START,     XML_TOK_TEXT_CHANGE_START },
    { XML_NAMESPACE_TEXT, XML_CHANGE_END,       XML_TOK_TEXT_CHANGE_END },
    { XML_NAMESPACE_TEXT, XML_CHANGE,           XML_TOK_TEXT_CHANGE },
    { XML_NAMESPACE_OFFICE, XML_FORMS,          XML_TOK_TEXT_FORMS },
    { XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS,    XML_TOK_TEXT_CALCULATION_SETTINGS },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_INDEX_AUTO_MARK_FILE, XML_TOK_TEXT_AUTOMARK },
    // #i52127#
    { XML_NAMESPACE_TEXT, XML_NUMBERED_PARAGRAPH, XML_TOK_TEXT_NUMBERED_PARAGRAPH   },

    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextPElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_SPAN, XML_TOK_TEXT_SPAN },
    { XML_NAMESPACE_TEXT, XML_TAB, XML_TOK_TEXT_TAB_STOP },
    { XML_NAMESPACE_TEXT, XML_LINE_BREAK, XML_TOK_TEXT_LINE_BREAK },
    { XML_NAMESPACE_TEXT, XML_SOFT_PAGE_BREAK, XML_TOK_TEXT_SOFT_PAGE_BREAK },
    { XML_NAMESPACE_TEXT, XML_S, XML_TOK_TEXT_S },
    { XML_NAMESPACE_TEXT, XML_A, XML_TOK_TEXT_HYPERLINK },
    { XML_NAMESPACE_TEXT, XML_RUBY, XML_TOK_TEXT_RUBY },

    { XML_NAMESPACE_TEXT, XML_NOTE, XML_TOK_TEXT_NOTE },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK, XML_TOK_TEXT_BOOKMARK },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK_START, XML_TOK_TEXT_BOOKMARK_START },
    { XML_NAMESPACE_TEXT, XML_BOOKMARK_END, XML_TOK_TEXT_BOOKMARK_END },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK, XML_TOK_TEXT_REFERENCE },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK_START,
      XML_TOK_TEXT_REFERENCE_START },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_MARK_END,
      XML_TOK_TEXT_REFERENCE_END },

    { XML_NAMESPACE_DRAW, XML_FRAME, XML_TOK_TEXT_FRAME },
    { XML_NAMESPACE_DRAW, XML_A,                XML_TOK_DRAW_A },

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
    // note: loext was written by accident in some LO versions, don't remove!
    { XML_NAMESPACE_LO_EXT, XML_SENDER_INITIALS, XML_TOK_TEXT_SENDER_INITIALS },
    { XML_NAMESPACE_TEXT,   XML_SENDER_INITIALS, XML_TOK_TEXT_SENDER_INITIALS },
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
    { XML_NAMESPACE_TEXT, XML_DATABASE_ROW_SELECT,
      XML_TOK_TEXT_DATABASE_SELECT },
    { XML_NAMESPACE_TEXT, XML_DATABASE_ROW_NUMBER,
      XML_TOK_TEXT_DATABASE_ROW_NUMBER },
    { XML_NAMESPACE_TEXT, XML_DATABASE_NAME, XML_TOK_TEXT_DATABASE_NAME },

    // docinfo fields
    { XML_NAMESPACE_TEXT, XML_INITIAL_CREATOR,
      XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR },
    { XML_NAMESPACE_TEXT, XML_DESCRIPTION, XML_TOK_TEXT_DOCUMENT_DESCRIPTION},
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
    { XML_NAMESPACE_TEXT, XML_CHAPTER,  XML_TOK_TEXT_CHAPTER },
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
    { XML_NAMESPACE_TEXT, XML_NOTE_REF, XML_TOK_TEXT_NOTE_REF },
    { XML_NAMESPACE_TEXT, XML_BIBLIOGRAPHY_MARK,
      XML_TOK_TEXT_BIBLIOGRAPHY_MARK },
    { XML_NAMESPACE_OFFICE, XML_ANNOTATION, XML_TOK_TEXT_ANNOTATION },
    { XML_NAMESPACE_OFFICE, XML_ANNOTATION_END, XML_TOK_TEXT_ANNOTATION_END },
    { XML_NAMESPACE_TEXT, XML_SCRIPT, XML_TOK_TEXT_SCRIPT },
    { XML_NAMESPACE_TEXT, XML_TABLE_FORMULA, XML_TOK_TEXT_TABLE_FORMULA },
    { XML_NAMESPACE_TEXT, XML_DROP_DOWN, XML_TOK_TEXT_DROP_DOWN },

    // Calc fields
    { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_TEXT_SHEET_NAME },

    // draw fields
    { XML_NAMESPACE_TEXT, XML_MEASURE,  XML_TOK_TEXT_MEASURE },
    { XML_NAMESPACE_LO_EXT, XML_PAGE_NAME, XML_TOK_TEXT_PAGE_NAME },
    { XML_NAMESPACE_TEXT, XML_PAGE_NAME, XML_TOK_TEXT_PAGE_NAME },

    // RDF metadata
    { XML_NAMESPACE_TEXT, XML_META,         XML_TOK_TEXT_META },
    { XML_NAMESPACE_TEXT, XML_META_FIELD,   XML_TOK_TEXT_META_FIELD },

    // redlining (aka change tracking)
    { XML_NAMESPACE_TEXT, XML_CHANGE_START, XML_TOK_TEXTP_CHANGE_START },
    { XML_NAMESPACE_TEXT, XML_CHANGE_END  , XML_TOK_TEXTP_CHANGE_END },
    { XML_NAMESPACE_TEXT, XML_CHANGE, XML_TOK_TEXTP_CHANGE },

    { XML_NAMESPACE_PRESENTATION, XML_HEADER, XML_TOK_DRAW_HEADER },
    { XML_NAMESPACE_PRESENTATION, XML_FOOTER, XML_TOK_DRAW_FOOTER },
    { XML_NAMESPACE_PRESENTATION, XML_DATE_TIME, XML_TOK_DRAW_DATE_TIME },
    { XML_NAMESPACE_TEXT, XML_PAGE_CONTINUATION, XML_TOK_TEXT_PAGE_CONTINUATION },

    { XML_NAMESPACE_FIELD, XML_FIELDMARK, XML_TOK_TEXT_FIELDMARK },
    { XML_NAMESPACE_FIELD, XML_FIELDMARK_START, XML_TOK_TEXT_FIELDMARK_START },
    { XML_NAMESPACE_FIELD, XML_FIELDMARK_END, XML_TOK_TEXT_FIELDMARK_END },


    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextPAttrTokenMap[] =
{
    { XML_NAMESPACE_XML  , XML_ID,          XML_TOK_TEXT_P_XMLID },
    { XML_NAMESPACE_XHTML, XML_ABOUT,       XML_TOK_TEXT_P_ABOUT },
    { XML_NAMESPACE_XHTML, XML_PROPERTY,    XML_TOK_TEXT_P_PROPERTY },
    { XML_NAMESPACE_XHTML, XML_CONTENT,     XML_TOK_TEXT_P_CONTENT },
    { XML_NAMESPACE_XHTML, XML_DATATYPE,    XML_TOK_TEXT_P_DATATYPE },
    { XML_NAMESPACE_TEXT, XML_ID,           XML_TOK_TEXT_P_TEXTID },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,   XML_TOK_TEXT_P_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_COND_STYLE_NAME,
                                            XML_TOK_TEXT_P_COND_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL,XML_TOK_TEXT_P_LEVEL },
    { XML_NAMESPACE_TEXT, XML_IS_LIST_HEADER,XML_TOK_TEXT_P_IS_LIST_HEADER },
    { XML_NAMESPACE_TEXT, XML_RESTART_NUMBERING,XML_TOK_TEXT_P_RESTART_NUMBERING },
    { XML_NAMESPACE_TEXT, XML_START_VALUE,XML_TOK_TEXT_P_START_VALUE },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextNumberedParagraphAttrTokenMap[] =
{
    { XML_NAMESPACE_XML , XML_ID,    XML_TOK_TEXT_NUMBERED_PARAGRAPH_XMLID },
    { XML_NAMESPACE_TEXT, XML_LIST_ID,
        XML_TOK_TEXT_NUMBERED_PARAGRAPH_LIST_ID },
    { XML_NAMESPACE_TEXT, XML_LEVEL, XML_TOK_TEXT_NUMBERED_PARAGRAPH_LEVEL },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,
        XML_TOK_TEXT_NUMBERED_PARAGRAPH_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_CONTINUE_NUMBERING,
        XML_TOK_TEXT_NUMBERED_PARAGRAPH_CONTINUE_NUMBERING },
    { XML_NAMESPACE_TEXT, XML_START_VALUE,
        XML_TOK_TEXT_NUMBERED_PARAGRAPH_START_VALUE },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextListBlockAttrTokenMap[] =
{
    { XML_NAMESPACE_XML , XML_ID,           XML_TOK_TEXT_LIST_BLOCK_XMLID },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,
            XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_CONTINUE_NUMBERING,
            XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING },
    { XML_NAMESPACE_TEXT, XML_CONTINUE_LIST,
            XML_TOK_TEXT_LIST_BLOCK_CONTINUE_LIST },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextListBlockElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_LIST_HEADER, XML_TOK_TEXT_LIST_HEADER },
    { XML_NAMESPACE_TEXT, XML_LIST_ITEM,    XML_TOK_TEXT_LIST_ITEM   },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextFrameAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_STYLE_NAME, XML_TOK_TEXT_FRAME_STYLE_NAME },
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_TEXT_FRAME_NAME },
    { XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_TOK_TEXT_FRAME_ANCHOR_TYPE },
    { XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER, XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER },
    { XML_NAMESPACE_SVG, XML_X, XML_TOK_TEXT_FRAME_X },
    { XML_NAMESPACE_SVG, XML_Y, XML_TOK_TEXT_FRAME_Y },
    { XML_NAMESPACE_SVG, XML_WIDTH, XML_TOK_TEXT_FRAME_WIDTH },
    { XML_NAMESPACE_FO, XML_MIN_WIDTH, XML_TOK_TEXT_FRAME_MIN_WIDTH },
    { XML_NAMESPACE_STYLE, XML_REL_WIDTH, XML_TOK_TEXT_FRAME_REL_WIDTH },
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_TOK_TEXT_FRAME_HEIGHT },
    { XML_NAMESPACE_FO, XML_MIN_HEIGHT, XML_TOK_TEXT_FRAME_MIN_HEIGHT },
    { XML_NAMESPACE_STYLE, XML_REL_HEIGHT, XML_TOK_TEXT_FRAME_REL_HEIGHT },
    { XML_NAMESPACE_DRAW, XML_CHAIN_NEXT_NAME, XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME },
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_TEXT_FRAME_HREF },
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_TEXT_FRAME_FILTER_NAME },
    { XML_NAMESPACE_DRAW, XML_ZINDEX, XML_TOK_TEXT_FRAME_Z_INDEX },
    { XML_NAMESPACE_DRAW, XML_TRANSFORM, XML_TOK_TEXT_FRAME_TRANSFORM },
    { XML_NAMESPACE_DRAW, XML_CLASS_ID, XML_TOK_TEXT_FRAME_CLASS_ID },
    { XML_NAMESPACE_DRAW,   XML_CODE,           XML_TOK_TEXT_FRAME_CODE },
    { XML_NAMESPACE_DRAW,   XML_OBJECT,         XML_TOK_TEXT_FRAME_OBJECT },
    { XML_NAMESPACE_DRAW,   XML_ARCHIVE,        XML_TOK_TEXT_FRAME_ARCHIVE },
    { XML_NAMESPACE_DRAW,   XML_MAY_SCRIPT,     XML_TOK_TEXT_FRAME_MAY_SCRIPT },
    { XML_NAMESPACE_DRAW,   XML_MIME_TYPE,  XML_TOK_TEXT_FRAME_MIME_TYPE },
    { XML_NAMESPACE_DRAW, XML_APPLET_NAME, XML_TOK_TEXT_FRAME_APPLET_NAME },
    { XML_NAMESPACE_DRAW, XML_FRAME_NAME, XML_TOK_TEXT_FRAME_FRAME_NAME },
    { XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE },
    { XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_TABLE, XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextContourAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG, XML_WIDTH,     XML_TOK_TEXT_CONTOUR_WIDTH      },
    { XML_NAMESPACE_SVG, XML_HEIGHT,    XML_TOK_TEXT_CONTOUR_HEIGHT     },
    { XML_NAMESPACE_SVG, XML_VIEWBOX,   XML_TOK_TEXT_CONTOUR_VIEWBOX    },
    { XML_NAMESPACE_SVG, XML_D,         XML_TOK_TEXT_CONTOUR_D          },
    { XML_NAMESPACE_DRAW,XML_POINTS,    XML_TOK_TEXT_CONTOUR_POINTS     },
    { XML_NAMESPACE_DRAW,XML_RECREATE_ON_EDIT,  XML_TOK_TEXT_CONTOUR_AUTO   },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextHyperlinkAttrTokenMap[] =
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

static const SvXMLTokenMapEntry aTextMasterPageElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_HEADER, XML_TOK_TEXT_MP_HEADER },
    { XML_NAMESPACE_STYLE, XML_FOOTER, XML_TOK_TEXT_MP_FOOTER },
    { XML_NAMESPACE_STYLE, XML_HEADER_LEFT, XML_TOK_TEXT_MP_HEADER_LEFT },
    { XML_NAMESPACE_STYLE, XML_FOOTER_LEFT, XML_TOK_TEXT_MP_FOOTER_LEFT },
    { XML_NAMESPACE_LO_EXT, XML_HEADER_FIRST, XML_TOK_TEXT_MP_HEADER_FIRST },
    { XML_NAMESPACE_LO_EXT, XML_FOOTER_FIRST, XML_TOK_TEXT_MP_FOOTER_FIRST },
    { XML_NAMESPACE_STYLE, XML_HEADER_FIRST, XML_TOK_TEXT_MP_HEADER_FIRST },
    { XML_NAMESPACE_STYLE, XML_FOOTER_FIRST, XML_TOK_TEXT_MP_FOOTER_FIRST },

    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTextFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_FIXED, XML_TOK_TEXTFIELD_FIXED },
    { XML_NAMESPACE_TEXT, XML_DESCRIPTION,  XML_TOK_TEXTFIELD_DESCRIPTION },
    { XML_NAMESPACE_TEXT, XML_HELP, XML_TOK_TEXTFIELD_HELP },
    { XML_NAMESPACE_TEXT, XML_HINT, XML_TOK_TEXTFIELD_HINT },
    { XML_NAMESPACE_TEXT, XML_PLACEHOLDER_TYPE,
                XML_TOK_TEXTFIELD_PLACEHOLDER_TYPE },
    { XML_NAMESPACE_TEXT, XML_NAME, XML_TOK_TEXTFIELD_NAME },
    { XML_NAMESPACE_TEXT, XML_FORMULA, XML_TOK_TEXTFIELD_FORMULA },
    { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_TEXTFIELD_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                XML_TOK_TEXTFIELD_NUM_LETTER_SYNC },
    { XML_NAMESPACE_TEXT, XML_DISPLAY_FORMULA,
                XML_TOK_TEXTFIELD_DISPLAY_FORMULA },
    { XML_NAMESPACE_TEXT, XML_VALUE_TYPE, XML_TOK_TEXTFIELD_VALUE_TYPE }, // #i32362#: src680m48++ saves text:value-type
    { XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TOK_TEXTFIELD_VALUE_TYPE },
    { XML_NAMESPACE_TEXT, XML_VALUE, XML_TOK_TEXTFIELD_VALUE },
    { XML_NAMESPACE_OFFICE, XML_VALUE, XML_TOK_TEXTFIELD_VALUE },
    { XML_NAMESPACE_TEXT, XML_STRING_VALUE, XML_TOK_TEXTFIELD_STRING_VALUE },
    { XML_NAMESPACE_OFFICE, XML_STRING_VALUE, XML_TOK_TEXTFIELD_STRING_VALUE },
    { XML_NAMESPACE_TEXT, XML_DATE_VALUE, XML_TOK_TEXTFIELD_DATE_VALUE },
    { XML_NAMESPACE_OFFICE, XML_DATE_VALUE, XML_TOK_TEXTFIELD_DATE_VALUE },
    { XML_NAMESPACE_TEXT, XML_TIME_VALUE, XML_TOK_TEXTFIELD_TIME_VALUE },
    { XML_NAMESPACE_OFFICE, XML_TIME_VALUE, XML_TOK_TEXTFIELD_TIME_VALUE },
    { XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_TOK_TEXTFIELD_BOOL_VALUE},
    { XML_NAMESPACE_OFFICE, XML_CURRENCY, XML_TOK_TEXTFIELD_CURRENCY},
    { XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME,
                XML_TOK_TEXTFIELD_DATA_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_DISPLAY_OUTLINE_LEVEL,
                XML_TOK_TEXTFIELD_NUMBERING_LEVEL },
    { XML_NAMESPACE_TEXT, XML_SEPARATION_CHARACTER,
                XML_TOK_TEXTFIELD_NUMBERING_SEPARATOR },
    { XML_NAMESPACE_TEXT, XML_DISPLAY, XML_TOK_TEXTFIELD_DISPLAY },
    { XML_NAMESPACE_TEXT, XML_TIME_ADJUST, XML_TOK_TEXTFIELD_TIME_ADJUST },
    { XML_NAMESPACE_TEXT, XML_DATE_ADJUST, XML_TOK_TEXTFIELD_DATE_ADJUST },
    { XML_NAMESPACE_TEXT, XML_PAGE_ADJUST, XML_TOK_TEXTFIELD_PAGE_ADJUST },
    { XML_NAMESPACE_TEXT, XML_SELECT_PAGE, XML_TOK_TEXTFIELD_SELECT_PAGE },
    { XML_NAMESPACE_TEXT, XML_DATABASE_NAME, XML_TOK_TEXTFIELD_DATABASE_NAME},
    { XML_NAMESPACE_TEXT, XML_TABLE_NAME, XML_TOK_TEXTFIELD_TABLE_NAME },
    { XML_NAMESPACE_TEXT, XML_COLUMN_NAME, XML_TOK_TEXTFIELD_COLUMN_NAME },
    { XML_NAMESPACE_TEXT, XML_ROW_NUMBER, XML_TOK_TEXTFIELD_ROW_NUMBER },
    { XML_NAMESPACE_TEXT, XML_CONDITION, XML_TOK_TEXTFIELD_CONDITION },
    { XML_NAMESPACE_TEXT, XML_STRING_VALUE_IF_TRUE,
                XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE },
    { XML_NAMESPACE_TEXT, XML_STRING_VALUE_IF_FALSE,
                XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE },
    { XML_NAMESPACE_TEXT, XML_EDITING_CYCLES, XML_TOK_TEXTFIELD_REVISION },
    { XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL, XML_TOK_TEXTFIELD_OUTLINE_LEVEL},
    { XML_NAMESPACE_TEXT, XML_ACTIVE, XML_TOK_TEXTFIELD_ACTIVE },
    { XML_NAMESPACE_TEXT, XML_NOTE_CLASS, XML_TOK_TEXTFIELD_NOTE_CLASS },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_FORMAT,
                XML_TOK_TEXTFIELD_REFERENCE_FORMAT },
    { XML_NAMESPACE_TEXT, XML_REF_NAME, XML_TOK_TEXTFIELD_REF_NAME },
    { XML_NAMESPACE_TEXT, XML_CONNECTION_NAME,
      XML_TOK_TEXTFIELD_CONNECTION_NAME },
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_TEXTFIELD_HREF },
    { XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME,
      XML_TOK_TEXTFIELD_TARGET_FRAME },
    { XML_NAMESPACE_TEXT, XML_ANNOTATION, XML_TOK_TEXTFIELD_ANNOTATION },
    { XML_NAMESPACE_SCRIPT, XML_LANGUAGE, XML_TOK_TEXTFIELD_LANGUAGE },
    { XML_NAMESPACE_TEXT, XML_KIND, XML_TOK_TEXTFIELD_MEASURE_KIND },
    { XML_NAMESPACE_TEXT, XML_IS_HIDDEN, XML_TOK_TEXTFIELD_IS_HIDDEN },
    { XML_NAMESPACE_TEXT, XML_CURRENT_VALUE,
                XML_TOK_TEXTFIELD_CURRENT_VALUE },
    { XML_NAMESPACE_TEXT, XML_TABLE_TYPE, XML_TOK_TEXTFIELD_TABLE_TYPE },
    { XML_NAMESPACE_OFFICE, XML_NAME, XML_TOK_TEXT_NAME },
    { XML_NAMESPACE_LO_EXT, XML_REFERENCE_LANGUAGE,
                XML_TOK_TEXTFIELD_REFERENCE_LANGUAGE },
    { XML_NAMESPACE_TEXT, XML_REFERENCE_LANGUAGE,
                XML_TOK_TEXTFIELD_REFERENCE_LANGUAGE },

    XML_TOKEN_MAP_END
};


// maximum allowed length of combined characters field
#define MAX_COMBINED_CHARACTERS 6

struct XMLTextImportHelper::Impl
{
    std::unique_ptr<SvXMLTokenMap> m_xTextElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextPElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextPAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextFieldAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextNumberedParagraphAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextListBlockAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextListBlockElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextFrameAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextContourAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextHyperlinkAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_xTextMasterPageElemTokenMap;
    std::unique_ptr< std::vector<OUString> > m_xPrevFrmNames;
    std::unique_ptr< std::vector<OUString> > m_xNextFrmNames;
    std::unique_ptr<XMLTextListsHelper> m_xTextListsHelper;

    SvXMLImportContextRef m_xAutoStyles;

    rtl::Reference< SvXMLImportPropertyMapper > m_xParaImpPrMap;
    rtl::Reference< SvXMLImportPropertyMapper > m_xTextImpPrMap;
    rtl::Reference< SvXMLImportPropertyMapper > m_xFrameImpPrMap;
    rtl::Reference< SvXMLImportPropertyMapper > m_xSectionImpPrMap;
    rtl::Reference< SvXMLImportPropertyMapper > m_xRubyImpPrMap;

    std::unique_ptr<SvI18NMap> m_xRenameMap;

    /* Change and extend data structure:
       - data structure contains candidates of paragraph styles, which
         will be assigned to the outline style
       - data structure contains more than one candidate for each list level
         of the outline style (#i69629#)
    */
    std::unique_ptr< std::vector< OUString > []>
        m_xOutlineStylesCandidates;

    // start range, xml:id, RDFa stuff
    typedef std::tuple<
        uno::Reference<text::XTextRange>, OUString,
        std::shared_ptr< ::xmloff::ParsedRDFaAttributes > >
            BookmarkMapEntry_t;
    /// start ranges for open bookmarks
    std::map< OUString, BookmarkMapEntry_t > m_BookmarkStartRanges;

    std::vector< OUString > m_BookmarkVector;

    /// name of the last 'open' redline that started between paragraphs
    OUString m_sOpenRedlineIdentifier;

    // Used for frame deduplication, the name of the last frame imported directly before the current one
    OUString msLastImportedFrameName;

    bool m_bBookmarkHidden;
    OUString m_sBookmarkCondition;

    uno::Reference<text::XText> m_xText;
    uno::Reference<text::XTextCursor> m_xCursor;
    uno::Reference<text::XTextRange> m_xCursorAsRange;
    uno::Reference<container::XNameContainer> m_xParaStyles;
    uno::Reference<container::XNameContainer> m_xTextStyles;
    uno::Reference<container::XNameContainer> m_xNumStyles;
    uno::Reference<container::XNameContainer> m_xFrameStyles;
    uno::Reference<container::XNameContainer> m_xPageStyles;
    uno::Reference<container::XNameContainer> m_xCellStyles;
    uno::Reference<container::XIndexReplace> m_xChapterNumbering;
    uno::Reference<container::XNameAccess> m_xTextFrames;
    uno::Reference<container::XNameAccess> m_xGraphics;
    uno::Reference<container::XNameAccess> m_xObjects;
    uno::Reference<lang::XMultiServiceFactory> m_xServiceFactory;

    SvXMLImport & m_rSvXMLImport;

    bool m_bInsertMode : 1;
    bool m_bStylesOnlyMode : 1;
    bool m_bBlockMode : 1;
    bool m_bProgress : 1;
    bool m_bOrganizerMode : 1;
    bool m_bBodyContentStarted : 1;

    /// Are we inside a <text:deletion> element (deleted redline section)
    bool m_bInsideDeleteContext : 1;

    typedef ::std::pair< OUString, OUString> field_name_type_t;
    typedef ::std::pair< OUString, OUString > field_param_t;
    typedef ::std::vector< field_param_t > field_params_t;
    typedef ::std::pair< field_name_type_t, field_params_t > field_stack_item_t;
    typedef ::std::stack< field_stack_item_t > field_stack_t;

    field_stack_t m_FieldStack;

    OUString m_sCellParaStyleDefault;

    std::unique_ptr<std::map<OUString, OUString>> m_pCrossRefHeadingBookmarkMap;

    Impl(       uno::Reference<frame::XModel> const& rModel,
                SvXMLImport & rImport,
                bool const bInsertMode, bool const bStylesOnlyMode,
                bool const bProgress, bool const bBlockMode,
                bool const bOrganizerMode)
        :   m_xTextListsHelper( new XMLTextListsHelper() )
        ,   m_bBookmarkHidden( false )
        // XML import: reconstrution of assignment of paragraph style to outline levels (#i69629#)
        ,   m_xServiceFactory( rModel, UNO_QUERY )
        ,   m_rSvXMLImport( rImport )
        ,   m_bInsertMode( bInsertMode )
        ,   m_bStylesOnlyMode( bStylesOnlyMode )
        ,   m_bBlockMode( bBlockMode )
        ,   m_bProgress( bProgress )
        ,   m_bOrganizerMode( bOrganizerMode )
        ,   m_bBodyContentStarted( true )
        ,   m_bInsideDeleteContext( false )
    {
    }
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    void InitOutlineStylesCandidates()
    {
        if (!m_xOutlineStylesCandidates)
        {
            size_t const size(m_xChapterNumbering->getCount());
            m_xOutlineStylesCandidates.reset(
                new ::std::vector< OUString >[size] );
        }
    }

};


uno::Reference< text::XText > & XMLTextImportHelper::GetText()
{
    return m_xImpl->m_xText;
}

uno::Reference< text::XTextCursor > & XMLTextImportHelper::GetCursor()
{
    return m_xImpl->m_xCursor;
}

uno::Reference< text::XTextRange > & XMLTextImportHelper::GetCursorAsRange()
{
    return m_xImpl->m_xCursorAsRange;
}

bool XMLTextImportHelper::IsInsertMode() const
{
    return m_xImpl->m_bInsertMode;
}

bool XMLTextImportHelper::IsStylesOnlyMode() const
{
    return m_xImpl->m_bStylesOnlyMode;
}

bool XMLTextImportHelper::IsBlockMode() const
{
    return m_xImpl->m_bBlockMode;
}

bool XMLTextImportHelper::IsOrganizerMode() const
{
    return m_xImpl->m_bOrganizerMode;
}

bool XMLTextImportHelper::IsProgress() const
{
    return m_xImpl->m_bProgress;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetParaStyles() const
{
    return m_xImpl->m_xParaStyles;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetTextStyles() const
{
    return m_xImpl->m_xTextStyles;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetNumberingStyles() const
{
    return m_xImpl->m_xNumStyles;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetFrameStyles() const
{
    return m_xImpl->m_xFrameStyles;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetPageStyles() const
{
    return m_xImpl->m_xPageStyles;
}

uno::Reference<container::XNameContainer> const&
XMLTextImportHelper::GetCellStyles() const
{
    return m_xImpl->m_xCellStyles;
}

uno::Reference<container::XIndexReplace> const&
XMLTextImportHelper::GetChapterNumbering() const
{
    return m_xImpl->m_xChapterNumbering;
}

rtl::Reference< SvXMLImportPropertyMapper > const&
XMLTextImportHelper::GetParaImportPropertySetMapper() const
{
    return m_xImpl->m_xParaImpPrMap;
}

rtl::Reference< SvXMLImportPropertyMapper > const&
XMLTextImportHelper::GetTextImportPropertySetMapper() const
{
    return m_xImpl->m_xTextImpPrMap;
}

rtl::Reference< SvXMLImportPropertyMapper > const&
XMLTextImportHelper::GetSectionImportPropertySetMapper() const
{
    return m_xImpl->m_xSectionImpPrMap;
}

rtl::Reference< SvXMLImportPropertyMapper > const&
XMLTextImportHelper::GetRubyImportPropertySetMapper() const
{
    return m_xImpl->m_xRubyImpPrMap;
}

void XMLTextImportHelper::SetInsideDeleteContext(bool const bNew)
{
    m_xImpl->m_bInsideDeleteContext = bNew;
}

bool XMLTextImportHelper::IsInsideDeleteContext() const
{
    return m_xImpl->m_bInsideDeleteContext;
}

SvXMLImport & XMLTextImportHelper::GetXMLImport()
{
    return m_xImpl->m_rSvXMLImport;
}

XMLTextListsHelper & XMLTextImportHelper::GetTextListHelper()
{
    return *m_xImpl->m_xTextListsHelper;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextElemTokenMap()
{
    if (!m_xImpl->m_xTextElemTokenMap.get())
    {
        m_xImpl->m_xTextElemTokenMap.reset(
            new SvXMLTokenMap( aTextElemTokenMap ));
    }
    return *m_xImpl->m_xTextElemTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextPElemTokenMap()
{
    if (!m_xImpl->m_xTextPElemTokenMap.get())
    {
        m_xImpl->m_xTextPElemTokenMap.reset(
            new SvXMLTokenMap( aTextPElemTokenMap ));
    }
    return *m_xImpl->m_xTextPElemTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextPAttrTokenMap()
{
    if (!m_xImpl->m_xTextPAttrTokenMap.get())
    {
        m_xImpl->m_xTextPAttrTokenMap.reset(
            new SvXMLTokenMap( aTextPAttrTokenMap ));
    }
    return *m_xImpl->m_xTextPAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextFrameAttrTokenMap()
{
    if (!m_xImpl->m_xTextFrameAttrTokenMap.get())
    {
        m_xImpl->m_xTextFrameAttrTokenMap.reset(
            new SvXMLTokenMap( aTextFrameAttrTokenMap ));
    }
    return *m_xImpl->m_xTextFrameAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextContourAttrTokenMap()
{
    if (!m_xImpl->m_xTextContourAttrTokenMap.get())
    {
        m_xImpl->m_xTextContourAttrTokenMap.reset(
            new SvXMLTokenMap( aTextContourAttrTokenMap ));
    }
    return *m_xImpl->m_xTextContourAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextHyperlinkAttrTokenMap()
{
    if (!m_xImpl->m_xTextHyperlinkAttrTokenMap.get())
    {
        m_xImpl->m_xTextHyperlinkAttrTokenMap.reset(
            new SvXMLTokenMap( aTextHyperlinkAttrTokenMap ));
    }
    return *m_xImpl->m_xTextHyperlinkAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextMasterPageElemTokenMap()
{
    if (!m_xImpl->m_xTextMasterPageElemTokenMap.get())
    {
        m_xImpl->m_xTextMasterPageElemTokenMap.reset(
            new SvXMLTokenMap( aTextMasterPageElemTokenMap ));
    }
    return *m_xImpl->m_xTextMasterPageElemTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextFieldAttrTokenMap()
{
    if (!m_xImpl->m_xTextFieldAttrTokenMap.get())
    {
        m_xImpl->m_xTextFieldAttrTokenMap.reset(
            new SvXMLTokenMap( aTextFieldAttrTokenMap ));
    }
    return *m_xImpl->m_xTextFieldAttrTokenMap;
}


namespace
{
    class FieldParamImporter
    {
        public:
            typedef pair<OUString,OUString> field_param_t;
            typedef vector<field_param_t> field_params_t;
            FieldParamImporter(const field_params_t* const pInParams, Reference<XNameContainer> const & xOutParams)
                : m_pInParams(pInParams)
                , m_xOutParams(xOutParams)
            { };
            void Import();

        private:
            const field_params_t* const m_pInParams;
            Reference<XNameContainer> m_xOutParams;
    };

    void FieldParamImporter::Import()
    {
        ::std::vector<OUString> vListEntries;
        ::std::map<OUString, Any> vOutParams;
        for(field_params_t::const_iterator pCurrent = m_pInParams->begin();
            pCurrent != m_pInParams->end();
            ++pCurrent)
        {
            if(pCurrent->first == ODF_FORMDROPDOWN_RESULT)
            {
                // sal_Int32
                vOutParams[pCurrent->first] <<= pCurrent->second.toInt32();
            }
            else if(pCurrent->first == ODF_FORMCHECKBOX_RESULT)
            {
                // bool
                vOutParams[pCurrent->first] <<= pCurrent->second.toBoolean();
            }
            else if(pCurrent->first == ODF_FORMDROPDOWN_LISTENTRY)
            {
                // sequence
                vListEntries.push_back(pCurrent->second);
            }
            else
                vOutParams[pCurrent->first] <<= pCurrent->second;
        }
        if(!vListEntries.empty())
        {
            Sequence<OUString> vListEntriesSeq(vListEntries.size());
            copy(vListEntries.begin(), vListEntries.end(), vListEntriesSeq.begin());
            vOutParams[OUString(ODF_FORMDROPDOWN_LISTENTRY)] <<= vListEntriesSeq;
        }
        for(::std::map<OUString, Any>::const_iterator pCurrent = vOutParams.begin();
            pCurrent != vOutParams.end();
            ++pCurrent)
        {
            try
            {
                m_xOutParams->insertByName(pCurrent->first, pCurrent->second);
            }
            catch(const ElementExistException&)
            {
            }
        }
    }
}

XMLTextImportHelper::XMLTextImportHelper(
        uno::Reference<frame::XModel> const& rModel,
        SvXMLImport& rImport,
        bool const bInsertMode, bool const bStylesOnlyMode,
        bool const bProgress, bool const bBlockMode,
        bool const bOrganizerMode)
    : m_xImpl( new Impl(rModel, rImport, bInsertMode, bStylesOnlyMode,
                    bProgress, bBlockMode, bOrganizerMode) )
    , m_xBackpatcherImpl( MakeBackpatcherImpl() )
{
    static const char s_PropNameDefaultListId[] = "DefaultListId";

    Reference< XChapterNumberingSupplier > xCNSupplier( rModel, UNO_QUERY );

    if (xCNSupplier.is())
    {
        // note: m_xChapterNumbering is accessed to import some fields
        m_xImpl->m_xChapterNumbering = xCNSupplier->getChapterNumberingRules();
        // the AutoCorrect document doesn't have a proper outline numbering
        if (!IsBlockMode() && m_xImpl->m_xChapterNumbering.is())
        {
            Reference< XPropertySet > const xNumRuleProps(
                m_xImpl->m_xChapterNumbering, UNO_QUERY);
            if ( xNumRuleProps.is() )
            {
                Reference< XPropertySetInfo > xNumRulePropSetInfo(
                                            xNumRuleProps->getPropertySetInfo());
                if (xNumRulePropSetInfo.is() &&
                    xNumRulePropSetInfo->hasPropertyByName(
                         s_PropNameDefaultListId))
                {
                    OUString sListId;
                    xNumRuleProps->getPropertyValue(s_PropNameDefaultListId)
                        >>= sListId;
                    assert( !sListId.isEmpty() &&
                                "no default list id found at chapter numbering rules instance. Serious defect." );
                    if ( !sListId.isEmpty() )
                    {
                        Reference< XNamed > const xChapterNumNamed(
                            m_xImpl->m_xChapterNumbering, UNO_QUERY);
                        if ( xChapterNumNamed.is() )
                        {
                            m_xImpl->m_xTextListsHelper->KeepListAsProcessed(
                                                    sListId,
                                                    xChapterNumNamed->getName(),
                                                    OUString() );
                        }
                    }
                }
            }
        }
    }

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( rModel, UNO_QUERY );
//  SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff", "no chapter numbering supplier" ); for clipboard there may be documents without styles

    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies(xFamiliesSupp->getStyleFamilies());

        const OUString aParaStyles("ParagraphStyles");
        if( xFamilies->hasByName( aParaStyles ) )
        {
            m_xImpl->m_xParaStyles.set(xFamilies->getByName(aParaStyles),
                UNO_QUERY);
        }

        const OUString aCharStyles("CharacterStyles");
        if( xFamilies->hasByName( aCharStyles ) )
        {
            m_xImpl->m_xTextStyles.set(xFamilies->getByName(aCharStyles),
                UNO_QUERY);
        }

        const OUString aNumStyles("NumberingStyles");
        if( xFamilies->hasByName( aNumStyles ) )
        {
            m_xImpl->m_xNumStyles.set(xFamilies->getByName(aNumStyles),
                UNO_QUERY);
        }

        const OUString aFrameStyles("FrameStyles");
        if( xFamilies->hasByName( aFrameStyles ) )
        {
            m_xImpl->m_xFrameStyles.set(xFamilies->getByName(aFrameStyles),
                UNO_QUERY);
        }

        const OUString aPageStyles("PageStyles");
        if( xFamilies->hasByName( aPageStyles ) )
        {
            m_xImpl->m_xPageStyles.set(xFamilies->getByName(aPageStyles),
                UNO_QUERY);
        }

        const OUString aCellStyles("CellStyles");
        if( xFamilies->hasByName( aCellStyles ) )
        {
            m_xImpl->m_xCellStyles.set(xFamilies->getByName(aCellStyles),
                UNO_QUERY);
        }
    }

    Reference < XTextFramesSupplier > xTFS( rModel, UNO_QUERY );
    if( xTFS.is() )
    {
        m_xImpl->m_xTextFrames.set(xTFS->getTextFrames());
    }

    Reference < XTextGraphicObjectsSupplier > xTGOS( rModel, UNO_QUERY );
    if( xTGOS.is() )
    {
        m_xImpl->m_xGraphics.set(xTGOS->getGraphicObjects());
    }

    Reference < XTextEmbeddedObjectsSupplier > xTEOS( rModel, UNO_QUERY );
    if( xTEOS.is() )
    {
        m_xImpl->m_xObjects.set(xTEOS->getEmbeddedObjects());
    }

    XMLPropertySetMapper *pPropMapper =
            new XMLTextPropertySetMapper( TextPropMap::PARA, false );
    m_xImpl->m_xParaImpPrMap =
        new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TextPropMap::TEXT, false );
    m_xImpl->m_xTextImpPrMap =
        new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TextPropMap::FRAME, false );
    m_xImpl->m_xFrameImpPrMap =
        new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TextPropMap::SECTION, false );
    m_xImpl->m_xSectionImpPrMap =
        new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper = new XMLTextPropertySetMapper( TextPropMap::RUBY, false );
    m_xImpl->m_xRubyImpPrMap =
        new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

XMLTextImportHelper::~XMLTextImportHelper()
{
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateShapeExtPropMapper(SvXMLImport& rImport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::FRAME, false );
    return new XMLTextImportPropertyMapper( pPropMapper, rImport );
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateParaExtPropMapper(SvXMLImport& rImport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::SHAPE_PARA, false );
    return new XMLTextImportPropertyMapper( pPropMapper, rImport );
}

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateParaDefaultExtPropMapper(SvXMLImport& rImport)
{
    XMLPropertySetMapper* pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::SHAPE_PARA, false );
    SvXMLImportPropertyMapper* pImportMapper = new XMLTextImportPropertyMapper( pPropMapper, rImport );

    pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::TEXT_ADDITIONAL_DEFAULTS, false );
    pImportMapper->ChainImportMapper( new XMLTextImportPropertyMapper( pPropMapper, rImport ) );

    return pImportMapper;
}

SvXMLImportPropertyMapper*
    XMLTextImportHelper::CreateTableDefaultExtPropMapper(
        SvXMLImport& rImport )
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::TABLE_DEFAULTS, false );
    return new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

SvXMLImportPropertyMapper*
    XMLTextImportHelper::CreateTableRowDefaultExtPropMapper(
        SvXMLImport& rImport )
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::TABLE_ROW_DEFAULTS, false );
    return new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

SvXMLImportPropertyMapper*
    XMLTextImportHelper::CreateTableCellExtPropMapper(
        SvXMLImport& rImport )
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::CELL, false );
    return new XMLTextImportPropertyMapper( pPropMapper, rImport );
}

void XMLTextImportHelper::SetCursor( const Reference < XTextCursor > & rCursor )
{
    m_xImpl->m_xCursor.set(rCursor);
    m_xImpl->m_xText.set(rCursor->getText());
    m_xImpl->m_xCursorAsRange.set( rCursor, UNO_QUERY );
}

void XMLTextImportHelper::ResetCursor()
{
    m_xImpl->m_xCursor.set(nullptr);
    m_xImpl->m_xText.set(nullptr);
    m_xImpl->m_xCursorAsRange.set(nullptr);
}


bool XMLTextImportHelper::HasFrameByName( const OUString& rName ) const
{
    return (m_xImpl->m_xTextFrames.is() &&
            m_xImpl->m_xTextFrames->hasByName(rName))
        || (m_xImpl->m_xGraphics.is() &&
            m_xImpl->m_xGraphics->hasByName(rName))
        || (m_xImpl->m_xObjects.is() &&
            m_xImpl->m_xObjects->hasByName(rName));
}

bool XMLTextImportHelper::IsDuplicateFrame(const OUString& sName, sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight) const
{
    if (HasFrameByName(sName))
    {
        uno::Reference<beans::XPropertySet> xOtherFrame;
        if(m_xImpl->m_xTextFrames.is() && m_xImpl->m_xTextFrames->hasByName(sName))
            xOtherFrame.set(m_xImpl->m_xTextFrames->getByName(sName), uno::UNO_QUERY);
        else if(m_xImpl->m_xGraphics.is() && m_xImpl->m_xGraphics->hasByName(sName))
            xOtherFrame.set(m_xImpl->m_xGraphics->getByName(sName), uno::UNO_QUERY);
        else if (m_xImpl->m_xObjects.is() && m_xImpl->m_xObjects->hasByName(sName))
            xOtherFrame.set(m_xImpl->m_xObjects->getByName(sName), uno::UNO_QUERY);

        Reference< XPropertySetInfo > xPropSetInfo = xOtherFrame->getPropertySetInfo();
        if(xPropSetInfo->hasPropertyByName("Width"))
        {
            sal_Int32 nOtherWidth = 0;
            xOtherFrame->getPropertyValue("Width") >>= nOtherWidth;
            if(nWidth != nOtherWidth)
                return false;
        }

        if (xPropSetInfo->hasPropertyByName("Height"))
        {
            sal_Int32 nOtherHeight = 0;
            xOtherFrame->getPropertyValue("Height") >>= nOtherHeight;
            if (nHeight != nOtherHeight)
                return false;
        }

        if (xPropSetInfo->hasPropertyByName("HoriOrientPosition"))
        {
            sal_Int32 nOtherX = 0;
            xOtherFrame->getPropertyValue("HoriOrientPosition") >>= nOtherX;
            if (nX != nOtherX)
                return false;
        }

        if (xPropSetInfo->hasPropertyByName("VertOrientPosition"))
        {
            sal_Int32 nOtherY = 0;
            xOtherFrame->getPropertyValue("VertOrientPosition") >>= nOtherY;
            if (nY != nOtherY)
                return false;
        }

        // In some case, position is not defined for frames, so check whether the two frames follow each other (are anchored to the same position)
        if (m_xImpl->msLastImportedFrameName != sName)
        {
            return false;
        }

        return true;
    }
    return false;
}

void XMLTextImportHelper::StoreLastImportedFrameName(const OUString& rName)
{
    m_xImpl->msLastImportedFrameName = rName;
}

void XMLTextImportHelper::ClearLastImportedTextFrameName()
{
    m_xImpl->msLastImportedFrameName.clear();
}

void XMLTextImportHelper::InsertString( const OUString& rChars )
{
    assert(m_xImpl->m_xText.is());
    assert(m_xImpl->m_xCursorAsRange.is());
    if (m_xImpl->m_xText.is())
    {
        m_xImpl->m_xText->insertString(m_xImpl->m_xCursorAsRange,
            rChars, false);
    }
}

void XMLTextImportHelper::InsertString( const OUString& rChars,
                                        bool& rIgnoreLeadingSpace )
{
    assert(m_xImpl->m_xText.is());
    assert(m_xImpl->m_xCursorAsRange.is());
    if (m_xImpl->m_xText.is())
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
                        sChars.append( u' ' );
                    rIgnoreLeadingSpace = true;
                    break;
                default:
                    rIgnoreLeadingSpace = false;
                    sChars.append( c );
                    break;
            }
        }
        m_xImpl->m_xText->insertString(m_xImpl->m_xCursorAsRange,
                                       sChars.makeStringAndClear(), false);
    }
}

void XMLTextImportHelper::InsertControlCharacter( sal_Int16 nControl )
{
    assert(m_xImpl->m_xText.is());
    assert(m_xImpl->m_xCursorAsRange.is());
    if (m_xImpl->m_xText.is())
    {
        m_xImpl->m_xText->insertControlCharacter(
            m_xImpl->m_xCursorAsRange, nControl, false);
    }
}

void XMLTextImportHelper::InsertTextContent(
    Reference < XTextContent > const & xContent )
{
    assert(m_xImpl->m_xText.is());
    assert(m_xImpl->m_xCursorAsRange.is());
    if (m_xImpl->m_xText.is())
    {
        // note: this may throw IllegalArgumentException and callers handle it
        m_xImpl->m_xText->insertTextContent( m_xImpl->m_xCursorAsRange, xContent, false);
    }
}

void XMLTextImportHelper::DeleteParagraph()
{
    assert(m_xImpl->m_xText.is());
    assert(m_xImpl->m_xCursor.is());
    assert(m_xImpl->m_xCursorAsRange.is());

    bool bDelete = true;
    Reference < XEnumerationAccess > const xEnumAccess(
        m_xImpl->m_xCursor, UNO_QUERY);
    if( xEnumAccess.is() )
    {
        Reference < XEnumeration > xEnum(xEnumAccess->createEnumeration());
        SAL_WARN_IF(!xEnum->hasMoreElements(), "xmloff.text",
                "empty text enumeration");
        if( xEnum->hasMoreElements() )
        {
            Reference < XComponent > xComp( xEnum->nextElement(), UNO_QUERY );
            assert(xComp.is());
            if( xComp.is() )
            {
                xComp->dispose();
                bDelete = false;
            }
        }
    }
    if( bDelete )
    {
        if (m_xImpl->m_xCursor->goLeft( 1, true ))
        {
            m_xImpl->m_xText->insertString(m_xImpl->m_xCursorAsRange,
                                           "", true);
        }
    }
}

OUString XMLTextImportHelper::ConvertStarFonts( const OUString& rChars,
                                                const OUString& rStyleName,
                                                sal_uInt8& rFlags,
                                                bool bPara,
                                                SvXMLImport& rImport ) const
{
    OUStringBuffer sChars( rChars );
    bool bConverted = false;
    for( sal_Int32 j=0; j<rChars.getLength(); j++ )
    {
        sal_Unicode c = rChars[j];
        if( c >= 0xf000 && c <= 0xf0ff )
        {
            if( (rFlags & CONV_STAR_FONT_FLAGS_VALID) == 0 )
            {
                XMLTextStyleContext *pStyle = nullptr;
                sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                           : XML_STYLE_FAMILY_TEXT_TEXT;
                if (!rStyleName.isEmpty() && m_xImpl->m_xAutoStyles.is())
                {
                    const SvXMLStyleContext* pTempStyle =
                        static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                                FindStyleChildContext( nFamily, rStyleName,
                                                       true );
                    pStyle = const_cast<XMLTextStyleContext*>( dynamic_cast< const XMLTextStyleContext* >(pTempStyle));
                }

                if( pStyle )
                {
                    sal_Int32 nCount = pStyle->GetProperties_().size();
                    if( nCount )
                    {
                        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())
                                ->GetImportPropertyMapper(nFamily);
                        if( xImpPrMap.is() )
                        {
                            rtl::Reference<XMLPropertySetMapper> rPropMapper =
                                xImpPrMap->getPropertySetMapper();
                            for( sal_Int32 i=0; i < nCount; i++ )
                            {
                                const XMLPropertyState& rProp = pStyle->GetProperties_()[i];
                                sal_Int32 nIdx = rProp.mnIndex;
                                sal_uInt32 nContextId = rPropMapper->GetEntryContextId(nIdx);
                                if( CTF_FONTFAMILYNAME == nContextId )
                                {
                                    rFlags &= ~(CONV_FROM_STAR_BATS|CONV_FROM_STAR_MATH);
                                    OUString sFontName;
                                    rProp.maValue >>= sFontName;
                                    if( sFontName.equalsIgnoreAsciiCase( "StarBats"  ) )
                                        rFlags |= CONV_FROM_STAR_BATS;
                                    else if( sFontName.equalsIgnoreAsciiCase( "StarMath" ) )
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
                sChars[j] = rImport.ConvStarBatsCharToStarSymbol( c );
                bConverted = true;
            }
            else if( (rFlags & CONV_FROM_STAR_MATH ) != 0 )
            {
                sChars[j] = rImport.ConvStarMathCharToStarSymbol( c );
                bConverted = true;
            }
        }
    }

    return bConverted ? sChars.makeStringAndClear() : rChars;
}

/* Helper method to determine, if a paragraph style has a list style (inclusive
   an empty one) inherits a list style (inclusive an empty one) from one of its parents (#i69629#)
*/
/* Apply special case, that found list style equals the chapter numbering, also
   to the found list styles of the parent styles. (#i73973#)
*/
static bool lcl_HasListStyle( const OUString& sStyleName,
                              const Reference < XNameContainer >& xParaStyles,
                              SvXMLImport const & rImport,
                              const OUString& sNumberingStyleName,
                              const OUString& sOutlineStyleName )
{
    bool bRet( false );

    if ( !xParaStyles->hasByName( sStyleName ) )
    {
        // error case
        return true;
    }

    Reference< XPropertyState > xPropState( xParaStyles->getByName( sStyleName ),
                                            UNO_QUERY );
    if ( !xPropState.is() )
    {
        // error case
        return false;
    }

    if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
    {
        // list style found
        bRet = true;
        // special case: the set list style equals the chapter numbering
        Reference< XPropertySet > xPropSet( xPropState, UNO_QUERY );
        if ( xPropSet.is() )
        {
            OUString sListStyle;
            xPropSet->getPropertyValue( sNumberingStyleName ) >>= sListStyle;
            if ( !sListStyle.isEmpty() &&
                 sListStyle == sOutlineStyleName )
            {
                bRet = false;
            }
        }
    }
    else
    {
        // Tools.Outline settings lost on Save (#i77708#)
        sal_Int32 nUPD( 0 );
        sal_Int32 nBuild( 0 );
        // Don't use UPD for versioning: xmloff/source/text/txtstyli.cxx and txtimp.cxx (#i86058#)
        const bool bBuildIdFound = rImport.getBuildIds( nUPD, nBuild );
        // search list style at parent
        Reference<XStyle> xStyle( xPropState, UNO_QUERY );
        while ( xStyle.is() )
        {
            OUString aParentStyle( xStyle->getParentStyle() );
            if ( !aParentStyle.isEmpty() )
            {
                aParentStyle =
                    rImport.GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                                 aParentStyle );
            }
            if ( aParentStyle.isEmpty() || !xParaStyles->hasByName( aParentStyle ) )
            {
                // no list style found
                break;
            }
            else
            {
                xPropState.set( xParaStyles->getByName( aParentStyle ),
                                UNO_QUERY );
                if ( !xPropState.is() )
                {
                    // error case
                    return true;
                }
                if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
                {
                    // list style found
                    bRet = true;
                    // Special case: the found list style equals the chapter numbering (#i73973#)
                    Reference< XPropertySet > xPropSet( xPropState, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        OUString sListStyle;
                        xPropSet->getPropertyValue( sNumberingStyleName ) >>= sListStyle;
                        if ( !sListStyle.isEmpty() &&
                             sListStyle == sOutlineStyleName )
                        {
                            bRet = false;
                        }
                        // Special handling for text documents from OOo version prior OOo 2.4 (#i77708#)
                        /* Check explicitly on certain versions and on import of
                           text documents in OpenOffice.org file format (#i86058#)
                        */
                        else if ( sListStyle.isEmpty() &&
                                  ( rImport.IsTextDocInOOoFileFormat() ||
                                    ( bBuildIdFound &&
                                      ( ( nUPD == 641 ) || ( nUPD == 645 ) || // prior OOo 2.0
                                        ( nUPD == 680 && nBuild <= 9238 ) ) ) ) ) // OOo 2.0 - OOo 2.3.1
                        {
                            bRet = false;
                        }
                    }
                    break;
                }
                else
                {
                    // search list style at parent
                    Reference<XStyle> xParentStyle(xPropState, UNO_QUERY);
                    if (xStyle == xParentStyle)
                    {
                        // error case
                        return true;
                    }
                    xStyle = xParentStyle;
                }
            }
        }
    }

    return bRet;
}
OUString XMLTextImportHelper::SetStyleAndAttrs(
        SvXMLImport const & rImport,
        const Reference < XTextCursor >& rCursor,
        const OUString& rStyleName,
        bool bPara,
        bool bOutlineLevelAttrFound,
        sal_Int8 nOutlineLevel,
        // Numberings/Bullets in table not visible after save/reload (#i80724#)
        bool bSetListAttrs )
{
    static const char s_NumberingRules[] = "NumberingRules";
    static const char s_NumberingIsNumber[] = "NumberingIsNumber";
    static const char s_NumberingLevel[] = "NumberingLevel";
    static const char s_ParaIsNumberingRestart[] = "ParaIsNumberingRestart";
    static const char s_NumberingStartValue[] = "NumberingStartValue";
    static const char s_PropNameListId[] = "ListId";
    static const char s_PageDescName[] = "PageDescName";
    static const char s_OutlineLevel[] = "OutlineLevel";

    const sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                     : XML_STYLE_FAMILY_TEXT_TEXT;
    XMLTextStyleContext *pStyle = nullptr;
    OUString sStyleName( rStyleName );
    if (!sStyleName.isEmpty() && m_xImpl->m_xAutoStyles.is())
    {
        const SvXMLStyleContext* pTempStyle =
            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                    FindStyleChildContext( nFamily, sStyleName, true );
        pStyle = const_cast<XMLTextStyleContext*>(dynamic_cast< const XMLTextStyleContext* >(pTempStyle));
    }
    if( pStyle )
        sStyleName = pStyle->GetParentName();

    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo(
        xPropSet->getPropertySetInfo());

    // style
    if( !sStyleName.isEmpty() )
    {
        sStyleName = rImport.GetStyleDisplayName( nFamily, sStyleName );
        const OUString rPropName = bPara ? OUString("ParaStyleName") : OUString("CharStyleName");
        const Reference < XNameContainer > & rStyles = bPara
            ? m_xImpl->m_xParaStyles
            : m_xImpl->m_xTextStyles;
        if( rStyles.is() &&
            xPropSetInfo->hasPropertyByName( rPropName ) &&
            rStyles->hasByName( sStyleName ) )
        {
            xPropSet->setPropertyValue( rPropName, makeAny(sStyleName) );
        }
        else
            sStyleName.clear();
    }

    /* The outline level needs to be only applied as list level, if the heading
       is not inside a list and if it by default applies the outline style. (#i70748#)
    */
    bool bApplyOutlineLevelAsListLevel( false );
    // Numberings/Bullets in table not visible after save/reload (#i80724#)
    if (bSetListAttrs && bPara
        && xPropSetInfo->hasPropertyByName( s_NumberingRules))
    {
        // Set numbering rules
        Reference< XIndexReplace > const xNumRules(
                xPropSet->getPropertyValue(s_NumberingRules), UNO_QUERY);

        XMLTextListBlockContext * pListBlock(nullptr);
        XMLTextListItemContext  * pListItem(nullptr);
        XMLNumberedParaContext  * pNumberedParagraph(nullptr);
        GetTextListHelper().ListContextTop(
            pListBlock, pListItem, pNumberedParagraph);

        assert(!(pListBlock && pNumberedParagraph) && "XMLTextImportHelper::"
            "SetStyleAndAttrs: both list and numbered-paragraph???");

        Reference < XIndexReplace > xNewNumRules;
        sal_Int8 nLevel(-1);
        OUString sListId;
        sal_Int16 nStartValue(-1);
        bool bNumberingIsNumber(true);

        if (pListBlock) {

            if (!pListItem) {
                bNumberingIsNumber = false; // list-header
            }
            // consider text:style-override property of <text:list-item>
            xNewNumRules.set(
                (pListItem != nullptr && pListItem->HasNumRulesOverride())
                    ? pListItem->GetNumRulesOverride()
                    : pListBlock->GetNumRules() );
            nLevel = static_cast<sal_Int8>(pListBlock->GetLevel());

            if ( pListItem && pListItem->HasStartValue() ) {
               nStartValue = pListItem->GetStartValue();
            }

            // Inconsistent behavior regarding lists (#i92811#)
            sListId = m_xImpl->m_xTextListsHelper->GetListIdForListBlock(
                            *pListBlock);
        }
        else if (pNumberedParagraph)
        {
            xNewNumRules.set(pNumberedParagraph->GetNumRules());
            nLevel = static_cast<sal_Int8>(pNumberedParagraph->GetLevel());
            sListId = pNumberedParagraph->GetListId();
            nStartValue = pNumberedParagraph->GetStartValue();
        }


        if (pListBlock || pNumberedParagraph)
        {
            // Assure that list style of automatic paragraph style is applied at paragraph. (#i101349#)
            bool bApplyNumRules = pStyle && pStyle->IsListStyleSet();
            if ( !bApplyNumRules )
            {
                bool bSameNumRules = xNewNumRules == xNumRules;
                if( !bSameNumRules && xNewNumRules.is() && xNumRules.is() )
                {
                    // If the interface pointers are different, then this does
                    // not mean that the num rules are different. Further tests
                    // are required then. However, if only one num rule is
                    // set, no tests are required of course.
                    Reference< XNamed > xNewNamed( xNewNumRules, UNO_QUERY );
                    Reference< XNamed > xNamed( xNumRules, UNO_QUERY );
                    if( xNewNamed.is() && xNamed.is() )
                    {
                        bSameNumRules = xNewNamed->getName() == xNamed->getName();
                    }
                    else
                    {
                        Reference< XAnyCompare > xNumRuleCompare( xNumRules, UNO_QUERY );
                        if( xNumRuleCompare.is() )
                        {
                            bSameNumRules = (xNumRuleCompare->compare( Any(xNumRules), Any(xNewNumRules) ) == 0);
                        }
                    }
                }
                bApplyNumRules = !bSameNumRules;
            }

            if ( bApplyNumRules )
            {
                // #102607# This may except when xNewNumRules contains
                // a Writer-NumRule-Implementation bug gets applied to
                // a shape. Since this may occur inside a document
                // (e.g. when edited), this must be handled
                // gracefully.
                try
                {
                    xPropSet->setPropertyValue(
                        s_NumberingRules, makeAny(xNewNumRules) );
                }
                catch(const Exception&)
                {
                    ; // I would really like to use a warning here,
                      // but I can't access the XMLErrorHandler from
                      // here.
                }
            }

            if (!bNumberingIsNumber &&
                xPropSetInfo->hasPropertyByName(s_NumberingIsNumber))
            {
                xPropSet->setPropertyValue(s_NumberingIsNumber, Any(false));
            }

            xPropSet->setPropertyValue( s_NumberingLevel, Any(nLevel) );

            if( pListBlock && pListBlock->IsRestartNumbering() )
            {
                // TODO: property missing
                if (xPropSetInfo->hasPropertyByName(s_ParaIsNumberingRestart))
                {
                    xPropSet->setPropertyValue(s_ParaIsNumberingRestart,
                                               makeAny(true) );
                }
                pListBlock->ResetRestartNumbering();
            }

            if ( 0 <= nStartValue &&
                xPropSetInfo->hasPropertyByName(s_NumberingStartValue))
            {
                xPropSet->setPropertyValue(s_NumberingStartValue,
                                           makeAny(nStartValue));
            }

            if (xPropSetInfo->hasPropertyByName(s_PropNameListId))
            {
                if (!sListId.isEmpty()) {
                    xPropSet->setPropertyValue(s_PropNameListId,
                        makeAny(sListId) );
                }
            }

            GetTextListHelper().SetListItem( nullptr );
        }
        else
        {
            /* If the paragraph is not in a list but its style, remove it from
               the list. Do not remove it, if the list of the style is
               the chapter numbering rule.
            */
            if( xNumRules.is() )
            {
                bool bRemove( true );
                // Special handling for document from OOo 2.x (#i70748#)
                sal_Int32 nUPD( 0 );
                sal_Int32 nBuild( 0 );
                const bool bBuildIdFound = rImport.getBuildIds( nUPD, nBuild );
                if ( ( bBuildIdFound && nUPD == 680 ) ||
                     !pStyle || !pStyle->IsListStyleSet() )
                {
                    if (m_xImpl->m_xChapterNumbering.is())
                    {
                        Reference< XNamed > xNumNamed( xNumRules, UNO_QUERY );
                        Reference< XNamed > const xChapterNumNamed (
                            m_xImpl->m_xChapterNumbering, UNO_QUERY);
                        if ( xNumNamed.is() && xChapterNumNamed.is() &&
                             xNumNamed->getName() == xChapterNumNamed->getName() )
                        {
                            bRemove = false;
                            // RFE: inserting headings into text documents (#i70748#)
                            bApplyOutlineLevelAsListLevel = true;
                        }
                    }
                }
                else
                {
                    SAL_INFO_IF(!pStyle->GetListStyle().isEmpty(),
                        "xmloff.text",
                        "automatic paragraph style with list style name, but paragraph not in list???");
                }
                if ( bRemove )
                {
                    xPropSet->setPropertyValue( s_NumberingRules, Any() );
                }
            }
        }
    }

    // hard paragraph properties
    if( pStyle )
    {
        pStyle->FillPropertySet( xPropSet );
        if( bPara && pStyle->HasMasterPageName() &&
            xPropSetInfo->hasPropertyByName(s_PageDescName))
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                                XML_STYLE_FAMILY_MASTER_PAGE,
                                pStyle->GetMasterPageName()) );
            if( sDisplayName.isEmpty() ||
                (m_xImpl->m_xPageStyles.is() &&
                 m_xImpl->m_xPageStyles->hasByName( sDisplayName)))
            {
                xPropSet->setPropertyValue(s_PageDescName,
                        makeAny(sDisplayName));
            }
        }
        if( bPara && !pStyle->GetDropCapStyleName().isEmpty() &&
            m_xImpl->m_xTextStyles.is())
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                                XML_STYLE_FAMILY_TEXT_TEXT,
                                pStyle->GetDropCapStyleName()) );
            if (m_xImpl->m_xTextStyles->hasByName(sDisplayName) &&
                xPropSetInfo->hasPropertyByName("DropCapCharStyleName"))
            {
                xPropSet->setPropertyValue("DropCapCharStyleName", makeAny(sDisplayName));
            }
        }

        // combined characters special treatment
        if (!bPara && pStyle->HasCombinedCharactersLetter())
        {
            // insert combined characters text field
            if (m_xImpl->m_xServiceFactory.is())
            {
                uno::Reference<beans::XPropertySet> const xTmp(
                    m_xImpl->m_xServiceFactory->createInstance(
                        "com.sun.star.text.TextField.CombinedCharacters"), UNO_QUERY);
                if( xTmp.is() )
                {
                    // fix cursor if larger than possible for
                    // combined characters field
                    if (rCursor->getString().getLength() >
                            MAX_COMBINED_CHARACTERS)
                    {
                        rCursor->gotoRange(rCursor->getStart(), false);
                        rCursor->goRight(MAX_COMBINED_CHARACTERS, true);
                    }

                    // set field value (the combined character string)
                    xTmp->setPropertyValue("Content",
                        makeAny(rCursor->getString()));

                    // insert the field over it's original text
                    Reference<XTextContent> xTextContent(xTmp, UNO_QUERY);
                    if (m_xImpl->m_xText.is() && rCursor.is())
                    {
                        // #i107225# the combined characters need to be inserted first
                        // the selected text has to be removed afterwards
                        m_xImpl->m_xText->insertTextContent( rCursor->getStart(), xTextContent, true );

                        if( !rCursor->getString().isEmpty() )
                        {
                            try
                            {
                                uno::Reference< text::XTextCursor > xCrsr = rCursor->getText()->createTextCursorByRange( rCursor->getStart() );
                                xCrsr->goLeft( 1, true );
                                uno::Reference< beans::XPropertySet> xCrsrProperties( xCrsr, uno::UNO_QUERY_THROW );
                                //the hard properties of the removed text need to be applied to the combined characters field
                                pStyle->FillPropertySet( xCrsrProperties );
                                xCrsr->collapseToEnd();
                                xCrsr->gotoRange( rCursor->getEnd(), true );
                                xCrsr->setString( OUString() );
                            }
                            catch(const uno::Exception&)
                            {
                            }
                        }
                    }
                }
            }
        }
    }

    // outline level; set after list style has been set
    // Complete re-worked and corrected: (#i53198#)
    // - set outline level at paragraph
    // - set numbering level at paragraph, if none is already set
    // - assure that style is marked as an outline style for the corresponding
    //   outline level.
    // - DO NOT set type of numbering rule to outline.
    // - DO NOT set numbering rule directly at the paragraph.

    // Some minor rework and adjust access to paragraph styles (#i70748#)
    if ( bPara )
    {
        // Headings not numbered anymore in 3.1 (#i103817#)
        sal_Int16 nCurrentOutlineLevelInheritedFromParagraphStyle = 0;
        const bool bHasOutlineLevelProp(
            xPropSetInfo->hasPropertyByName(s_OutlineLevel));
        if ( bHasOutlineLevelProp )
        {
            xPropSet->getPropertyValue(s_OutlineLevel)
                >>= nCurrentOutlineLevelInheritedFromParagraphStyle;
        }
        if ( nOutlineLevel > 0 )
        {
            if ( bHasOutlineLevelProp )
            {
                // In case that the value equals the value of its paragraph style
                // attribute outline level, the paragraph attribute value is left unset
                if ( nCurrentOutlineLevelInheritedFromParagraphStyle != nOutlineLevel )
                {
                    xPropSet->setPropertyValue( s_OutlineLevel,
                        makeAny( static_cast<sal_Int16>(nOutlineLevel) ) );
                }
            }

            // RFE: inserting headings into text documents (#i70748#)
            if ( bApplyOutlineLevelAsListLevel )
            {
                sal_Int16 nNumLevel = -1;
                xPropSet->getPropertyValue( s_NumberingLevel ) >>= nNumLevel;
                if ( nNumLevel == -1 ||
                     nNumLevel != (nOutlineLevel - 1) )
                {
                    xPropSet->setPropertyValue( s_NumberingLevel,
                            makeAny( static_cast<sal_Int8>(nOutlineLevel - 1) ) );
                }
            }
            /* Correction: (#i69629#)
               - for text document from version OOo 2.0.4/SO 8 PU4 and earlier
                 the paragraph style of a heading should be assigned to the
                 corresponding list level of the outline style.
               - for other text documents the paragraph style of a heading is only
                 a candidate for an assignment to the list level of the outline
                 style, if it has no direct list style property and (if exists) the
                 automatic paragraph style has also no direct list style set.
            */
            if (m_xImpl->m_xParaStyles.is() && m_xImpl->m_xParaStyles->hasByName(sStyleName))
            {
                bool bOutlineStyleCandidate( false );

                sal_Int32 nUPD( 0 );
                sal_Int32 nBuild( 0 );
                const bool bBuildIdFound = rImport.getBuildIds( nUPD, nBuild );
                // Lost outline numbering in master document (#i73509#)
                // Check explicitly on certain versions (#i86058#)
                if ( rImport.IsTextDocInOOoFileFormat() ||
                     ( bBuildIdFound &&
                       ( nUPD == 645 || nUPD == 641 ) ) )
                {
                    bOutlineStyleCandidate = true;
                }
                else if ( nUPD == 680 && nBuild <= 9073 ) /* BuildId of OOo 2.0.4/SO8 PU4 */
                {
                    bOutlineStyleCandidate = bOutlineLevelAttrFound;
                }
                if ( bOutlineStyleCandidate )
                {
                    AddOutlineStyleCandidate( nOutlineLevel, sStyleName );
                }
                // Assure that heading applies the outline style (#i103817#)
                if ( ( !pStyle || !pStyle->IsListStyleSet() ) &&
                     !bOutlineStyleCandidate &&
                     m_xImpl->m_xChapterNumbering.is())
                {
                    if ( !lcl_HasListStyle( sStyleName,
                                    m_xImpl->m_xParaStyles, GetXMLImport(),
                                    "NumberingStyleName",
                                    "" ) )
                    {
                        // heading not in a list --> apply outline style
                        xPropSet->setPropertyValue( s_NumberingRules,
                            makeAny(m_xImpl->m_xChapterNumbering) );
                        xPropSet->setPropertyValue( s_NumberingLevel,
                            makeAny(static_cast<sal_Int8>(nOutlineLevel - 1)));
                    }
                }
            }
        }
        //handle for text:p,if the paragraphstyle outlinelevel is set to[1~10]
        else if( bHasOutlineLevelProp )
        {
            if ( nCurrentOutlineLevelInheritedFromParagraphStyle != 0 )
            {
                xPropSet->setPropertyValue(s_OutlineLevel,
                    makeAny( sal_Int16(0) ));
            }
        }
    }

    return sStyleName;
}

void XMLTextImportHelper::FindOutlineStyleName( OUString& rStyleName,
                                                sal_Int8 nOutlineLevel )
{
    // style name empty?
    if( rStyleName.isEmpty() )
    {
        // Empty? Then we need o do stuff. Let's do error checking first.
        if (m_xImpl->m_xChapterNumbering.is() &&
            ( nOutlineLevel > 0 ) &&
            (nOutlineLevel <= m_xImpl->m_xChapterNumbering->getCount()))
        {
            nOutlineLevel--;   // for the remainder, the level's are 0-based

            // empty style name: look-up previously used name

            // if we don't have a previously used name, we'll use the default
            m_xImpl->InitOutlineStylesCandidates();
            if (m_xImpl->m_xOutlineStylesCandidates[nOutlineLevel].empty())
            {
                // no other name used previously? Then use default

                // iterate over property value sequence to find the style name
                Sequence<PropertyValue> aProperties;
                m_xImpl->m_xChapterNumbering->getByIndex( nOutlineLevel )
                    >>= aProperties;
                for( sal_Int32 i = 0; i < aProperties.getLength(); i++ )
                {
                    if (aProperties[i].Name == "HeadingStyleName")
                    {
                        OUString aOutlineStyle;
                        aProperties[i].Value >>= aOutlineStyle;
                        m_xImpl->m_xOutlineStylesCandidates[nOutlineLevel]
                            .push_back( aOutlineStyle );
                        break;  // early out, if we found it!.
                    }
                }
            }

            // finally, we'll use the previously used style name for this
            // format (or the default we've just put into that style)
            // take last added one (#i71249#)
            rStyleName =
                m_xImpl->m_xOutlineStylesCandidates[nOutlineLevel].back();
        }
        // else: nothing we can do, so we'll leave it empty
    }
    // else: we already had a style name, so we let it pass.
}

void XMLTextImportHelper::AddOutlineStyleCandidate( const sal_Int8 nOutlineLevel,
                                                    const OUString& rStyleName )
{
    if (!rStyleName.isEmpty()
        && m_xImpl->m_xChapterNumbering.is()
        && (nOutlineLevel > 0)
        && (nOutlineLevel <= m_xImpl->m_xChapterNumbering->getCount()))
    {
        m_xImpl->InitOutlineStylesCandidates();
        m_xImpl->m_xOutlineStylesCandidates[nOutlineLevel-1].push_back(
            rStyleName);
    }
}

void XMLTextImportHelper::SetOutlineStyles( bool bSetEmptyLevels )
{
    if (!(m_xImpl->m_xOutlineStylesCandidates != nullptr || bSetEmptyLevels) ||
        !m_xImpl->m_xChapterNumbering.is() ||
        IsInsertMode())
        return;

    bool bChooseLastOne( false );
    {
        if ( GetXMLImport().IsTextDocInOOoFileFormat() )
        {
            bChooseLastOne = true;
        }
        else
        {
            sal_Int32 nUPD( 0 );
            sal_Int32 nBuild( 0 );
            if ( GetXMLImport().getBuildIds( nUPD, nBuild ) )
            {
                // check explicitly on certain versions
                bChooseLastOne = ( nUPD == 641 ) || ( nUPD == 645 ) ||  // prior OOo 2.0
                                 ( nUPD == 680 && nBuild <= 9073 ); // OOo 2.0 - OOo 2.0.4
            }
        }
    }

    OUString sOutlineStyleName;
    {
        Reference<XPropertySet> xChapterNumRule(
            m_xImpl->m_xChapterNumbering, UNO_QUERY);
        const OUString sName("Name");
        xChapterNumRule->getPropertyValue(sName) >>= sOutlineStyleName;
    }

    const sal_Int32 nCount = m_xImpl->m_xChapterNumbering->getCount();
    /* First collect all paragraph styles chosen for assignment to each
       list level of the outline style, then perform the intrinsic assignment.
       Reason: The assignment of a certain paragraph style to a list level
               of the outline style causes side effects on the children
               paragraph styles in Writer. (#i106218#)
    */
    ::std::vector<OUString> sChosenStyles(nCount);
    for( sal_Int32 i=0; i < nCount; ++i )
    {
        if ( bSetEmptyLevels ||
             (m_xImpl->m_xOutlineStylesCandidates &&
              !m_xImpl->m_xOutlineStylesCandidates[i].empty()))
        {
            // determine, which candidate is one to be assigned to the list
            // level of the outline style
            if (m_xImpl->m_xOutlineStylesCandidates &&
                !m_xImpl->m_xOutlineStylesCandidates[i].empty())
            {
                if ( bChooseLastOne )
                {
                    sChosenStyles[i] =
                    m_xImpl->m_xOutlineStylesCandidates[i].back();
                }
                else
                {
                    for (size_t j = 0;
                        j < m_xImpl->m_xOutlineStylesCandidates[i].size();
                        ++j)
                    {
                        if (!lcl_HasListStyle(
                                m_xImpl->m_xOutlineStylesCandidates[i][j],
                                m_xImpl->m_xParaStyles,
                                GetXMLImport(),
                                "NumberingStyleName",
                                sOutlineStyleName))
                        {
                            sChosenStyles[i] =
                                m_xImpl->m_xOutlineStylesCandidates[i][j];
                            break;
                        }
                    }
                }
            }
        }
    }
    // Trashed outline numbering in ODF 1.1 text document created by OOo 3.x (#i106218#)
    Sequence < PropertyValue > aProps( 1 );
    PropertyValue *pProps = aProps.getArray();
    pProps->Name = "HeadingStyleName";
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        // Paragraph style assignments in Outline of template lost from second level on (#i107610#)
        if ( bSetEmptyLevels || !sChosenStyles[i].isEmpty() )
        {
            pProps->Value <<= sChosenStyles[i];
            m_xImpl->m_xChapterNumbering->replaceByIndex(i,
                    makeAny( aProps ));
        }
    }

}

void XMLTextImportHelper::SetHyperlink(
    SvXMLImport const & rImport,
    const Reference < XTextCursor >& rCursor,
    const OUString& rHRef,
    const OUString& rName,
    const OUString& rTargetFrameName,
    const OUString& rStyleName,
    const OUString& rVisitedStyleName,
    XMLEventsImportContext* pEvents)
{
    static const char s_HyperLinkURL[] = "HyperLinkURL";
    static const char s_HyperLinkName[] = "HyperLinkName";
    static const char s_HyperLinkTarget[] = "HyperLinkTarget";
    static const char s_UnvisitedCharStyleName[] = "UnvisitedCharStyleName";
    static const char s_VisitedCharStyleName[] = "VisitedCharStyleName";
    static const char s_HyperLinkEvents[] = "HyperLinkEvents";

    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference < XPropertySetInfo > xPropSetInfo(
        xPropSet->getPropertySetInfo());
    if (!xPropSetInfo.is() || !xPropSetInfo->hasPropertyByName(s_HyperLinkURL))
        return;

    xPropSet->setPropertyValue(s_HyperLinkURL, makeAny(rHRef));

    if (xPropSetInfo->hasPropertyByName(s_HyperLinkName))
    {
        xPropSet->setPropertyValue(s_HyperLinkName, makeAny(rName));
    }

    if (xPropSetInfo->hasPropertyByName(s_HyperLinkTarget))
    {
        xPropSet->setPropertyValue(s_HyperLinkTarget,
            makeAny(rTargetFrameName));
    }

    if ( (pEvents != nullptr) &&
        xPropSetInfo->hasPropertyByName(s_HyperLinkEvents))
    {
        // The API treats events at hyperlinks differently from most
        // other properties: You have to set a name replace with the
        // events in it. The easiest way to do this is to 1) get
        // events, 2) set new ones, and 3) then put events back.
        uno::Reference<XNameReplace> const xReplace(
            xPropSet->getPropertyValue(s_HyperLinkEvents), UNO_QUERY);
        if (xReplace.is())
        {
            // set events
            pEvents->SetEvents(xReplace);

            // put events
            xPropSet->setPropertyValue(s_HyperLinkEvents, makeAny(xReplace));
        }
    }

    if (m_xImpl->m_xTextStyles.is())
    {
        OUString sDisplayName(
            rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rStyleName ) );
        if( !sDisplayName.isEmpty() &&
            xPropSetInfo->hasPropertyByName(s_UnvisitedCharStyleName) &&
            m_xImpl->m_xTextStyles->hasByName(sDisplayName))
        {
            xPropSet->setPropertyValue(s_UnvisitedCharStyleName,
                makeAny(sDisplayName));
        }

        sDisplayName =
            rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rVisitedStyleName );
        if( !sDisplayName.isEmpty() &&
            xPropSetInfo->hasPropertyByName(s_VisitedCharStyleName) &&
            m_xImpl->m_xTextStyles->hasByName(sDisplayName))
        {
            xPropSet->setPropertyValue(s_VisitedCharStyleName,
                makeAny(sDisplayName));
        }
    }
}

void XMLTextImportHelper::SetRuby(
    SvXMLImport const & rImport,
    const Reference < XTextCursor >& rCursor,
    const OUString& rStyleName,
    const OUString& rTextStyleName,
    const OUString& rText )
{
    Reference<XPropertySet> xPropSet(rCursor, UNO_QUERY);

    OUString sRubyText("RubyText");

    // if we have one Ruby property, we assume all of them are present
    if (xPropSet.is() &&
        xPropSet->getPropertySetInfo()->hasPropertyByName( sRubyText ))
    {
        // the ruby text
        xPropSet->setPropertyValue(sRubyText, makeAny(rText));

        // the ruby style (ruby-adjust)
        if (!rStyleName.isEmpty() && m_xImpl->m_xAutoStyles.is())
        {
            const SvXMLStyleContext* pTempStyle =
                static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                FindStyleChildContext( XML_STYLE_FAMILY_TEXT_RUBY,
                                       rStyleName, true );
            XMLPropStyleContext *pStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(pTempStyle));

            if (nullptr != pStyle)
                pStyle->FillPropertySet( xPropSet );
        }

        // the ruby text character style
        if (m_xImpl->m_xTextStyles.is())
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rTextStyleName ) );
            if( (!sDisplayName.isEmpty()) &&
                m_xImpl->m_xTextStyles->hasByName( sDisplayName ))
            {
                xPropSet->setPropertyValue("RubyCharStyleName", makeAny(sDisplayName));
            }
        }
    }
}

void XMLTextImportHelper::SetAutoStyles( SvXMLStylesContext *pStyles )
{
    m_xImpl->m_xAutoStyles = pStyles;
}

SvXMLImportContext *XMLTextImportHelper::CreateTextChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList,
        XMLTextType eType )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetTextElemTokenMap();
    bool bHeading = false;
    bool bContent = true;
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
    case XML_TOK_TEXT_H:
        bHeading = true;
        SAL_FALLTHROUGH;
    case XML_TOK_TEXT_P:
        pContext = new XMLParaContext( rImport,
                                       nPrefix, rLocalName,
                                       xAttrList, bHeading );
        if (m_xImpl->m_bProgress && XMLTextType::Shape != eType)
        {
            rImport.GetProgressBarHelper()->Increment();
        }
        break;
    case XML_TOK_TEXT_NUMBERED_PARAGRAPH:
        pContext = new XMLNumberedParaContext(
                        rImport, nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_TEXT_LIST:
        pContext = new XMLTextListBlockContext( rImport, *this,
                                                nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TABLE_TABLE:
        if( XMLTextType::Body == eType ||
            XMLTextType::TextBox == eType ||
             XMLTextType::Section == eType ||
            XMLTextType::HeaderFooter == eType ||
            XMLTextType::ChangedRegion == eType ||
            XMLTextType::Cell == eType )
            pContext = CreateTableChildContext( rImport, nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TEXT_SEQUENCE_DECLS:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::HeaderFooter == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSequence);
            bContent = false;
        }
        break;

    case XML_TOK_TEXT_VARFIELD_DECLS:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::HeaderFooter == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSimple);
            bContent = false;
        }
        break;

    case XML_TOK_TEXT_USERFIELD_DECLS:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted)||
            XMLTextType::HeaderFooter == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeUserField);
            bContent = false;
        }
        break;

    case XML_TOK_TEXT_DDE_DECLS:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::HeaderFooter == eType )
        {
            pContext = new XMLDdeFieldDeclsImportContext(
                rImport, nPrefix, rLocalName);
            bContent = false;
        }
        break;

    case XML_TOK_TEXT_FRAME_PAGE:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::TextBox == eType ||
            XMLTextType::ChangedRegion == eType )
        {
            TextContentAnchorType eAnchorType =
                XMLTextType::TextBox == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType );
            bContent = false;
        }
        break;

    case XML_TOK_DRAW_A_PAGE:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::TextBox == eType ||
             XMLTextType::ChangedRegion == eType)
        {
            TextContentAnchorType eAnchorType =
                XMLTextType::TextBox == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameHyperlinkContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType );
            bContent = false;
        }
        break;

    case XML_TOK_TEXT_INDEX_TITLE:
    case XML_TOK_TEXT_SECTION:
        pContext = new XMLSectionImportContext( rImport, nPrefix, rLocalName );
        break;

    case XML_TOK_TEXT_TOC:
    case XML_TOK_TEXT_OBJECT_INDEX:
    case XML_TOK_TEXT_TABLE_INDEX:
    case XML_TOK_TEXT_ILLUSTRATION_INDEX:
    case XML_TOK_TEXT_USER_INDEX:
    case XML_TOK_TEXT_ALPHABETICAL_INDEX:
    case XML_TOK_TEXT_BIBLIOGRAPHY_INDEX:
        if( XMLTextType::Shape != eType )
            pContext = new XMLIndexTOCContext( rImport, nPrefix, rLocalName );
        break;

    case XML_TOK_TEXT_TRACKED_CHANGES:
        pContext = new XMLTrackedChangesImportContext( rImport, nPrefix,
                                                       rLocalName);
        bContent = false;
        break;

    case XML_TOK_TEXT_CHANGE:
    case XML_TOK_TEXT_CHANGE_START:
    case XML_TOK_TEXT_CHANGE_END:
        pContext = new XMLChangeImportContext(
            rImport, nPrefix, rLocalName,
            ((nToken == XML_TOK_TEXT_CHANGE_END)
                ? XMLChangeImportContext::Element::END
                : (nToken == XML_TOK_TEXT_CHANGE_START)
                    ? XMLChangeImportContext::Element::START
                    : XMLChangeImportContext::Element::POINT),
            true);
        break;

    case XML_TOK_TEXT_FORMS:
        pContext = xmloff::OFormLayerXMLImport::createOfficeFormsContext(rImport, nPrefix, rLocalName);
        bContent = false;
        break;

    case XML_TOK_TEXT_AUTOMARK:
        if( XMLTextType::Body == eType )
        {
            pContext = new XMLAutoMarkFileContext(rImport, nPrefix,rLocalName);
        }
        bContent = false;
        break;

    case XML_TOK_TEXT_CALCULATION_SETTINGS:
        pContext = new XMLCalculationSettingsContext ( rImport, nPrefix, rLocalName, xAttrList);
        bContent = false;
    break;

    default:
        if ((XMLTextType::Body == eType && m_xImpl->m_bBodyContentStarted) ||
            XMLTextType::TextBox == eType ||
             XMLTextType::ChangedRegion == eType )
        {
            Reference < XShapes > xShapes;
            pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                    rImport, nPrefix, rLocalName, xAttrList, xShapes );
            bContent = false;
        }
    }

    // handle open redlines
    if ( (XML_TOK_TEXT_CHANGE != nToken) &&
         (XML_TOK_TEXT_CHANGE_END != nToken) &&
         (XML_TOK_TEXT_CHANGE_START != nToken) )
    {
//      ResetOpenRedlineId();
    }

    if( XMLTextType::Body == eType && bContent )
    {
        m_xImpl->m_bBodyContentStarted = false;
    }

    if( nToken != XML_TOK_TEXT_FRAME_PAGE )
        ClearLastImportedTextFrameName();
    return pContext;
}

SvXMLImportContext *XMLTextImportHelper::CreateTableChildContext(
        SvXMLImport&,
        sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/,
        const Reference< XAttributeList > & )
{
    return nullptr;
}

/// get data style key for use with NumberFormat property
sal_Int32 XMLTextImportHelper::GetDataStyleKey(const OUString& sStyleName,
                                               bool* pIsSystemLanguage )
{
    if (!m_xImpl->m_xAutoStyles.is())
        return -1;

    const SvXMLStyleContext* pStyle =
        static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                  FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE,
                                              sStyleName, true );

    // get appropriate context


    // first check if it's a impress and draw only number format
    // this is needed since its also a SvXMLNumFormatContext,
    // that was needed to support them for controls in impress/draw also
    const SdXMLNumberFormatImportContext* pSdNumStyle = dynamic_cast<const SdXMLNumberFormatImportContext*>( pStyle  );
    if( pSdNumStyle )
    {
        return pSdNumStyle->GetDrawKey();
    }
    else
    {
        SvXMLNumFormatContext* pNumStyle = const_cast<SvXMLNumFormatContext*>(dynamic_cast<const SvXMLNumFormatContext*>( pStyle ) );
        if( pNumStyle )
        {
            if( pIsSystemLanguage != nullptr )
                *pIsSystemLanguage = pNumStyle->IsSystemLanguage();

            // return key
            return pNumStyle->GetKey();
        }
    }
    return -1;
}

const SvxXMLListStyleContext *XMLTextImportHelper::FindAutoListStyle( const OUString& rName ) const
{
    const SvxXMLListStyleContext *pStyle = nullptr;
    if (m_xImpl->m_xAutoStyles.is())
    {
        const SvXMLStyleContext* pTempStyle =
            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                    FindStyleChildContext( XML_STYLE_FAMILY_TEXT_LIST, rName,
                                           true );
        pStyle = dynamic_cast< const SvxXMLListStyleContext* >(pTempStyle);
    }

    return pStyle;
}

XMLPropStyleContext *XMLTextImportHelper::FindAutoFrameStyle( const OUString& rName ) const
{
    XMLPropStyleContext *pStyle = nullptr;
    if (m_xImpl->m_xAutoStyles.is())
    {
        const SvXMLStyleContext* pTempStyle =
            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                    FindStyleChildContext( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rName,
                                           true );
        pStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(pTempStyle));
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindSectionStyle(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = nullptr;
    if (m_xImpl->m_xAutoStyles.is())
    {
        const SvXMLStyleContext* pTempStyle =
            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_TEXT_SECTION,
                               rName, true );
        pStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(pTempStyle));
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindPageMaster(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = nullptr;
    if (m_xImpl->m_xAutoStyles.is())
    {
        const SvXMLStyleContext* pTempStyle =
            static_cast<SvXMLStylesContext *>(m_xImpl->m_xAutoStyles.get())->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_PAGE_MASTER,
                               rName, true );
        pStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(pTempStyle));
    }

    return pStyle;
}


void XMLTextImportHelper::PushListContext()
{
    GetTextListHelper().PushListContext(static_cast<XMLTextListBlockContext*>(nullptr));
}

void XMLTextImportHelper::PopListContext()
{
    GetTextListHelper().PopListContext();
}


const SvXMLTokenMap& XMLTextImportHelper::GetTextNumberedParagraphAttrTokenMap()
{
    if (!m_xImpl->m_xTextNumberedParagraphAttrTokenMap.get())
    {
        m_xImpl->m_xTextNumberedParagraphAttrTokenMap.reset(
            new SvXMLTokenMap( aTextNumberedParagraphAttrTokenMap ) );
    }
    return *m_xImpl->m_xTextNumberedParagraphAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockAttrTokenMap()
{
    if (!m_xImpl->m_xTextListBlockAttrTokenMap.get())
    {
        m_xImpl->m_xTextListBlockAttrTokenMap.reset(
            new SvXMLTokenMap( aTextListBlockAttrTokenMap ) );
    }
    return *m_xImpl->m_xTextListBlockAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockElemTokenMap()
{
    if (!m_xImpl->m_xTextListBlockElemTokenMap.get())
    {
        m_xImpl->m_xTextListBlockElemTokenMap.reset(
            new SvXMLTokenMap( aTextListBlockElemTokenMap ) );
    }
    return *m_xImpl->m_xTextListBlockElemTokenMap;
}

SvI18NMap& XMLTextImportHelper::GetRenameMap()
{
    if (!m_xImpl->m_xRenameMap.get())
    {
        m_xImpl->m_xRenameMap.reset( new SvI18NMap );
    }
    return *m_xImpl->m_xRenameMap;
}

void XMLTextImportHelper::InsertBookmarkStartRange(
    const OUString & sName,
    const Reference<XTextRange> & rRange,
    OUString const& i_rXmlId,
    std::shared_ptr< ::xmloff::ParsedRDFaAttributes > & i_rpRDFaAttributes)
{
    m_xImpl->m_BookmarkStartRanges[sName] =
        std::make_tuple(rRange, i_rXmlId, i_rpRDFaAttributes);
    m_xImpl->m_BookmarkVector.push_back(sName);
}

bool XMLTextImportHelper::FindAndRemoveBookmarkStartRange(
    const OUString & sName,
    Reference<XTextRange> & o_rRange,
    OUString & o_rXmlId,
    std::shared_ptr< ::xmloff::ParsedRDFaAttributes > & o_rpRDFaAttributes)
{
    if (m_xImpl->m_BookmarkStartRanges.count(sName))
    {
        Impl::BookmarkMapEntry_t & rEntry =
            (*m_xImpl->m_BookmarkStartRanges.find(sName)).second;
        o_rRange.set(std::get<0>(rEntry));
        o_rXmlId = std::get<1>(rEntry);
        o_rpRDFaAttributes = std::get<2>(rEntry);
        m_xImpl->m_BookmarkStartRanges.erase(sName);
        auto it(m_xImpl->m_BookmarkVector.begin());
        while (it != m_xImpl->m_BookmarkVector.end() && *it != sName)
        {
            ++it;
        }
        if (it!=m_xImpl->m_BookmarkVector.end())
        {
            m_xImpl->m_BookmarkVector.erase(it);
        }
        return true;
    }
    else
    {
        return false;
    }
}

OUString XMLTextImportHelper::FindActiveBookmarkName()
{
    if (!m_xImpl->m_BookmarkVector.empty())
    {
        return m_xImpl->m_BookmarkVector.back();
    }
    else
    {
        return OUString(); // return the empty string on error...
    }
}

void XMLTextImportHelper::pushFieldCtx( const OUString& name, const OUString& type )
{
    m_xImpl->m_FieldStack.push(Impl::field_stack_item_t(
        Impl::field_name_type_t(name, type), Impl::field_params_t()));
}

void XMLTextImportHelper::popFieldCtx()
{
    if ( !m_xImpl->m_FieldStack.empty() )
        m_xImpl->m_FieldStack.pop();
}

void XMLTextImportHelper::addFieldParam( const OUString& name, const OUString& value )
{
    assert(!m_xImpl->m_FieldStack.empty());
    if (!m_xImpl->m_FieldStack.empty()) {
        Impl::field_stack_item_t & FieldStackItem(m_xImpl->m_FieldStack.top());
        FieldStackItem.second.emplace_back( name, value );
    }
}

OUString XMLTextImportHelper::getCurrentFieldType()
{
    assert(!m_xImpl->m_FieldStack.empty());
    if (!m_xImpl->m_FieldStack.empty())
    {
        return m_xImpl->m_FieldStack.top().first.second;
    }
    else
    {
        return OUString();
    }
}

bool XMLTextImportHelper::hasCurrentFieldCtx()
{
    return !m_xImpl->m_FieldStack.empty();
}

void XMLTextImportHelper::setCurrentFieldParamsTo(css::uno::Reference< css::text::XFormField> const &xFormField)
{
    assert(!m_xImpl->m_FieldStack.empty());
    if (!m_xImpl->m_FieldStack.empty() && xFormField.is())
    {
        FieldParamImporter(&m_xImpl->m_FieldStack.top().second,
            xFormField->getParameters()).Import();
    }
}


void XMLTextImportHelper::ConnectFrameChains(
        const OUString& rFrmName,
        const OUString& rNextFrmName,
        const Reference < XPropertySet >& rFrmPropSet )
{
    if( rFrmName.isEmpty() )
        return;

    if( !rNextFrmName.isEmpty() )
    {
        OUString sNextFrmName(GetRenameMap().Get( XML_TEXT_RENAME_TYPE_FRAME,
                                                    rNextFrmName ));
        if (m_xImpl->m_xTextFrames.is()
            && m_xImpl->m_xTextFrames->hasByName(sNextFrmName))
        {
            rFrmPropSet->setPropertyValue("ChainNextName",
                makeAny(sNextFrmName));
        }
        else
        {
            if (!m_xImpl->m_xPrevFrmNames.get())
            {
                m_xImpl->m_xPrevFrmNames.reset( new std::vector<OUString> );
                m_xImpl->m_xNextFrmNames.reset( new std::vector<OUString> );
            }
            m_xImpl->m_xPrevFrmNames->push_back(rFrmName);
            m_xImpl->m_xNextFrmNames->push_back(sNextFrmName);
        }
    }
    if (m_xImpl->m_xPrevFrmNames.get() && !m_xImpl->m_xPrevFrmNames->empty())
    {
        for(std::vector<OUString>::iterator i = m_xImpl->m_xPrevFrmNames->begin(), j = m_xImpl->m_xNextFrmNames->begin(); i != m_xImpl->m_xPrevFrmNames->end() && j != m_xImpl->m_xNextFrmNames->end(); ++i, ++j)
        {
            if((*j) == rFrmName)
            {
                // The previous frame must exist, because it existing than
                // inserting the entry
                rFrmPropSet->setPropertyValue("ChainPrevName", makeAny(*i));

                i = m_xImpl->m_xPrevFrmNames->erase(i);
                j = m_xImpl->m_xNextFrmNames->erase(j);

                // There cannot be more than one previous frames
                break;
            }
        }
    }
}

bool XMLTextImportHelper::IsInFrame() const
{
    static const char s_TextFrame[] = "TextFrame";

    bool bIsInFrame = false;

    // are we currently in a text frame? yes, if the cursor has a
    // TextFrame property and it's non-NULL
    Reference<XPropertySet> xPropSet(const_cast<XMLTextImportHelper*>(this)->GetCursor(), UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(s_TextFrame))
        {
            uno::Reference<XTextFrame> const xFrame(
                xPropSet->getPropertyValue(s_TextFrame), UNO_QUERY);

            if (xFrame.is())
            {
                bIsInFrame = true;
            }
        }
    }

    return bIsInFrame;
}

bool XMLTextImportHelper::IsInHeaderFooter() const
{
    return false;
}

Reference< XPropertySet> XMLTextImportHelper::createAndInsertOLEObject(
                                        SvXMLImport&,
                                        const OUString& /*rHRef*/,
                                        const OUString& /*rStyleName*/,
                                        const OUString& /*rTblName*/,
                                        sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/ )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}

Reference< XPropertySet> XMLTextImportHelper::createAndInsertOOoLink(
                                        SvXMLImport&,
                                        const OUString& /*rHRef*/,
                                        const OUString& /*rStyleName*/,
                                        const OUString& /*rTblName*/,
                                        sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/ )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}

Reference< XPropertySet> XMLTextImportHelper::createAndInsertApplet(
                                        const OUString& /*rCode*/,
                                        const OUString& /*rName*/,
                                        bool /*bMayScript*/,
                                        const OUString& /*rHRef*/,
                                        sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/ )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}
Reference< XPropertySet> XMLTextImportHelper::createAndInsertPlugin(
                                        const OUString& /*rMimeType*/,
                                        const OUString& /*rHRef*/,
                                        sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/ )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}
Reference< XPropertySet> XMLTextImportHelper::createAndInsertFloatingFrame(
                                        const OUString& /*rName*/,
                                        const OUString& /*rHRef*/,
                                        const OUString& /*rStyleName*/,
                                        sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/ )
{
    Reference< XPropertySet> xPropSet;
    return xPropSet;
}

void XMLTextImportHelper::endAppletOrPlugin(
        const Reference < XPropertySet> &,
        std::map < const OUString, OUString > &)
{
}
// redline helper: dummy implementation to be overridden in sw/filter/xml
void XMLTextImportHelper::RedlineAdd( const OUString& /*rType*/,
                                      const OUString& /*rId*/,
                                      const OUString& /*rAuthor*/,
                                      const OUString& /*rComment*/,
                                      const util::DateTime& /*rDateTime*/,
                                      bool /*bMergeLastPara*/)
{
    // dummy implementation: do nothing
}

Reference<XTextCursor> XMLTextImportHelper::RedlineCreateText(
    Reference<XTextCursor> & /*rOldCursor*/,
    const OUString& /*rId*/)
{
    // dummy implementation: do nothing
    Reference<XTextCursor> xRet;
    return xRet;
}

void XMLTextImportHelper::RedlineSetCursor(
    const OUString& /*rId*/,
    bool /*bStart*/,
    bool /*bIsOutsideOfParagraph*/)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::RedlineAdjustStartNodeCursor(bool)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetShowChanges( bool )
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetRecordChanges( bool )
{
    // dummy implementation: do nothing
}
void XMLTextImportHelper::SetChangesProtectionKey(const Sequence<sal_Int8> &)
{
    // dummy implementation: do nothing
}


OUString const & XMLTextImportHelper::GetOpenRedlineId()
{
    return m_xImpl->m_sOpenRedlineIdentifier;
}

void XMLTextImportHelper::SetOpenRedlineId( OUString const & rId)
{
    m_xImpl->m_sOpenRedlineIdentifier = rId;
}

void XMLTextImportHelper::ResetOpenRedlineId()
{
    SetOpenRedlineId("");
}

void
XMLTextImportHelper::SetCellParaStyleDefault(OUString const& rNewValue)
{
    m_xImpl->m_sCellParaStyleDefault = rNewValue;
}

OUString const& XMLTextImportHelper::GetCellParaStyleDefault()
{
    return m_xImpl->m_sCellParaStyleDefault;
}

void XMLTextImportHelper::AddCrossRefHeadingMapping(OUString const& rFrom, OUString const& rTo)
{
    if (!m_xImpl->m_pCrossRefHeadingBookmarkMap)
    {
        m_xImpl->m_pCrossRefHeadingBookmarkMap.reset(new std::map<OUString, OUString>);
    }
    m_xImpl->m_pCrossRefHeadingBookmarkMap->insert(std::make_pair(rFrom, rTo));
}

// tdf#94804: hack to map cross reference fields that reference duplicate marks
// note that we can't really check meta:generator for this since the file might
// be round-tripped by different versions preserving duplicates => always map
void XMLTextImportHelper::MapCrossRefHeadingFieldsHorribly()
{
    if (!m_xImpl->m_pCrossRefHeadingBookmarkMap)
    {
        return;
    }

    uno::Reference<text::XTextFieldsSupplier> const xFieldsSupplier(
            m_xImpl->m_rSvXMLImport.GetModel(), uno::UNO_QUERY);
    if (!xFieldsSupplier.is())
    {
        return;
    }
    uno::Reference<container::XEnumerationAccess> const xFieldsEA(
            xFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> const xFields(
            xFieldsEA->createEnumeration());
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> const xFieldInfo(
                xFields->nextElement(), uno::UNO_QUERY);
        if (!xFieldInfo->supportsService("com.sun.star.text.textfield.GetReference"))
        {
            continue;
        }
        uno::Reference<beans::XPropertySet> const xField(
                xFieldInfo, uno::UNO_QUERY);
        sal_uInt16 nType(0);
        xField->getPropertyValue("ReferenceFieldSource") >>= nType;
        if (text::ReferenceFieldSource::BOOKMARK != nType)
        {
            continue;
        }
        OUString name;
        xField->getPropertyValue("SourceName") >>= name;
        auto const iter(m_xImpl->m_pCrossRefHeadingBookmarkMap->find(name));
        if (iter == m_xImpl->m_pCrossRefHeadingBookmarkMap->end())
        {
            continue;
        }
        xField->setPropertyValue("SourceName", uno::makeAny(iter->second));
    }
}

void XMLTextImportHelper::setBookmarkAttributes(bool hidden, OUString const& condition)
{
    m_xImpl->m_bBookmarkHidden = hidden;
    m_xImpl->m_sBookmarkCondition = condition;
}

bool XMLTextImportHelper::getBookmarkHidden()
{
    return m_xImpl->m_bBookmarkHidden;
}

const OUString& XMLTextImportHelper::getBookmarkCondition()
{
    return m_xImpl->m_sBookmarkCondition;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
