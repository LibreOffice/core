/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtimp.cxx,v $
 *
 *  $Revision: 1.138 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:09:52 $
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
#include "precompiled_xmloff.hxx"

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTDARR_STRINGSDTOR_DECL
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XCHAPTERNUMBERINGSUPPLIER_HPP_
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_TXTSTYLI_HXX_
#include <xmloff/txtstyli.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif

#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _XMLOFF_XMLNUMI_HXX
#include <xmloff/xmlnumi.hxx>
#endif

#ifndef _XMLOFF_TXTPARAI_HXX
#include "txtparai.hxx"
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_TXTIMPPR_HXX
#include "txtimppr.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_TXTVFLDI_HXX
#include "txtvfldi.hxx"
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif
#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif
#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif
#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif
#ifndef _XMLOFF_XMLPROPERTYBACKPATCHTER_HXX
#include "XMLPropertyBackpatcher.hxx"
#endif
#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#include "XMLTextFrameHyperlinkContext.hxx"
#endif
#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#include "XMLSectionImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLINDEXTOCCONTEXT_HXX_
#include "XMLIndexTOCContext.hxx"
#endif
#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX
#include <xmloff/XMLFontStylesContext.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include <xmloff/XMLEventsImportContext.hxx>
#endif
#ifndef _XMLOFF_XMLTRACKEDCHANGESIMPORTCONTEXT_HXX
#include "XMLTrackedChangesImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#include "XMLChangeImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLAUTOMARKFILECONTEXT_HXX_
#include "XMLAutoMarkFileContext.hxx"
#endif

#ifndef _XMLOFF_XMLCALCULATION_SETTINGS_CONTEXT_HXX
#include "XMLCalculationSettingsContext.hxx"
#endif
#ifndef _XMLOFF_FORMSIMP_HXX
#include <xmloff/formsimp.hxx>
#endif
#ifndef _XMLOFF_NUMBERSTYLESIMPORT_HXX
#include "XMLNumberStylesImport.hxx"
#endif
// --> OD 2006-10-12 #i69629#
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
// <--

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
using namespace ::xmloff::token;
using ::com::sun::star::util::DateTime;
using namespace ::com::sun::star::ucb;

using ::comphelper::UStringLess;

static __FAR_DATA SvXMLTokenMapEntry aTextElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_P,                XML_TOK_TEXT_P              },
    { XML_NAMESPACE_TEXT, XML_H,                XML_TOK_TEXT_H              },
    { XML_NAMESPACE_TEXT, XML_LIST,             XML_TOK_TEXT_LIST           },
    { XML_NAMESPACE_DRAW, XML_FRAME,            XML_TOK_TEXT_FRAME_PAGE     },
    { XML_NAMESPACE_DRAW, XML_A,                XML_TOK_DRAW_A_PAGE },
    { XML_NAMESPACE_TABLE,XML_TABLE,            XML_TOK_TABLE_TABLE         },
//  { XML_NAMESPACE_TABLE,XML_SUB_TABLE,        XML_TOK_TABLE_SUBTABLE      },
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
    // --> FLR #i52127#
    { XML_NAMESPACE_TEXT, XML_NUMBERED_PARAGRAPH, XML_TOK_TEXT_NUMBERED_PARAGRAPH   },
    // <--

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPElemTokenMap[] =
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
    { XML_NAMESPACE_TEXT, XML_SCRIPT, XML_TOK_TEXT_SCRIPT },
    { XML_NAMESPACE_TEXT, XML_TABLE_FORMULA, XML_TOK_TEXT_TABLE_FORMULA },
    { XML_NAMESPACE_TEXT, XML_DROPDOWN, XML_TOK_TEXT_DROPDOWN },

    // Calc fields
    { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_TEXT_SHEET_NAME },

    // draw fields
    { XML_NAMESPACE_TEXT, XML_MEASURE,  XML_TOK_TEXT_MEASURE },

    // redlining (aka change tracking)
    { XML_NAMESPACE_TEXT, XML_CHANGE_START, XML_TOK_TEXTP_CHANGE_START },
    { XML_NAMESPACE_TEXT, XML_CHANGE_END  , XML_TOK_TEXTP_CHANGE_END },
    { XML_NAMESPACE_TEXT, XML_CHANGE, XML_TOK_TEXTP_CHANGE },

    { XML_NAMESPACE_PRESENTATION, XML_HEADER, XML_TOK_DRAW_HEADER },
    { XML_NAMESPACE_PRESENTATION, XML_FOOTER, XML_TOK_DRAW_FOOTER },
    { XML_NAMESPACE_PRESENTATION, XML_DATE_TIME, XML_TOK_DRAW_DATE_TIME },
    { XML_NAMESPACE_TEXT, XML_PAGE_CONTINUATION, XML_TOK_TEXT_PAGE_CONTINUATION },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME,   XML_TOK_TEXT_P_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_COND_STYLE_NAME,
                                            XML_TOK_TEXT_P_COND_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL,XML_TOK_TEXT_P_LEVEL },
    { XML_NAMESPACE_TEXT, XML_ID,           XML_TOK_TEXT_P_ID },
    { XML_NAMESPACE_TEXT, XML_IS_LIST_HEADER,XML_TOK_TEXT_P_IS_LIST_HEADER },
    { XML_NAMESPACE_TEXT, XML_RESTART_NUMBERING,XML_TOK_TEXT_P_RESTART_NUMBERING },
    { XML_NAMESPACE_TEXT, XML_START_VALUE,XML_TOK_TEXT_P_START_VALUE },
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
    { XML_NAMESPACE_TEXT, XML_LIST_ITEM,    XML_TOK_TEXT_LIST_ITEM   },
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
    { XML_NAMESPACE_FO, XML_MIN_WIDTH, XML_TOK_TEXT_FRAME_MIN_WIDTH },
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

static __FAR_DATA SvXMLTokenMapEntry aTextContourAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG, XML_WIDTH,     XML_TOK_TEXT_CONTOUR_WIDTH      },
    { XML_NAMESPACE_SVG, XML_HEIGHT,    XML_TOK_TEXT_CONTOUR_HEIGHT     },
    { XML_NAMESPACE_SVG, XML_VIEWBOX,   XML_TOK_TEXT_CONTOUR_VIEWBOX    },
    { XML_NAMESPACE_SVG, XML_D,         XML_TOK_TEXT_CONTOUR_D          },
    { XML_NAMESPACE_DRAW,XML_POINTS,    XML_TOK_TEXT_CONTOUR_POINTS     },
    { XML_NAMESPACE_DRAW,XML_RECREATE_ON_EDIT,  XML_TOK_TEXT_CONTOUR_AUTO   },
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
        sal_Bool bOrganizerM )
:   pTextElemTokenMap( 0 )
,   pTextPElemTokenMap( 0 )
,   pTextPAttrTokenMap( 0 )
,   pTextFieldAttrTokenMap( 0 )
,   pTextListBlockAttrTokenMap( 0 )
,   pTextListBlockElemTokenMap( 0 )
,   pTextFrameAttrTokenMap( 0 )
,   pTextContourAttrTokenMap( 0 )
,   pTextHyperlinkAttrTokenMap( 0 )
,   pTextMasterPageElemTokenMap( 0 )
,   pPrevFrmNames( 0 )
,   pNextFrmNames( 0 )

,   pRenameMap( 0 )
// --> OD 2006-10-12 #i69629#
,   mpOutlineStylesCandidates( 0 )
// <--

,   pFootnoteBackpatcher( NULL )
,   pSequenceIdBackpatcher( NULL )
,   pSequenceNameBackpatcher( NULL )

,   xServiceFactory( rModel, UNO_QUERY )

,   rSvXMLImport( rImport )

,   bInsertMode( bInsertM )
,   bStylesOnlyMode( bStylesOnlyM )
,   bBlockMode( bBlockM )
,   bProgress( bPrg )
,   bOrganizerMode( bOrganizerM )
,   bBodyContentStarted( sal_True )

    // #107848#
    // Initialize inside_deleted_section flag correctly
,   bInsideDeleteContext( sal_False )

,   sParaStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName"))
,   sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName"))
,   sHeadingStyleName(RTL_CONSTASCII_USTRINGPARAM("HeadingStyleName"))
,   sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel"))
,   sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue"))
,   sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules"))
,   sParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart"))
,   sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"))
,   sCurrentPresentation(RTL_CONSTASCII_USTRINGPARAM("CurrentPresentation"))
,   sSequenceNumber(RTL_CONSTASCII_USTRINGPARAM("SequenceNumber"))
,   sSourceName(RTL_CONSTASCII_USTRINGPARAM("SourceName"))
,   sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName"))
,   sChainPrevName(RTL_CONSTASCII_USTRINGPARAM("ChainPrevName"))
,   sHyperLinkURL(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL"))
,   sHyperLinkName(RTL_CONSTASCII_USTRINGPARAM("HyperLinkName"))
,   sHyperLinkTarget(RTL_CONSTASCII_USTRINGPARAM("HyperLinkTarget"))
,   sUnvisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("UnvisitedCharStyleName"))
,   sVisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("VisitedCharStyleName"))
,   sTextFrame(RTL_CONSTASCII_USTRINGPARAM("TextFrame"))
,   sPageDescName(RTL_CONSTASCII_USTRINGPARAM("PageDescName"))
,   sServerMap(RTL_CONSTASCII_USTRINGPARAM("ServerMap"))
,   sHyperLinkEvents(RTL_CONSTASCII_USTRINGPARAM("HyperLinkEvents"))
,   sContent(RTL_CONSTASCII_USTRINGPARAM("Content"))
,   sServiceCombinedCharacters(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField.CombinedCharacters"))
,   sNumberingStyleName(RTL_CONSTASCII_USTRINGPARAM("NumberingStyleName"))
{
    Reference< XChapterNumberingSupplier > xCNSupplier( rModel, UNO_QUERY );

    if( xCNSupplier.is() )
        xChapterNumbering = xCNSupplier->getChapterNumberingRules();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( rModel, UNO_QUERY );
//  DBG_ASSERT( xFamiliesSupp.is(), "no chapter numbering supplier" ); for clipboard there may be documents without styles

    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies(xFamiliesSupp->getStyleFamilies());

        const OUString aParaStyles(RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles"));
        if( xFamilies->hasByName( aParaStyles ) )
        {
            xParaStyles.set(xFamilies->getByName( aParaStyles ), UNO_QUERY);
        }

        const OUString aCharStyles(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles"));
        if( xFamilies->hasByName( aCharStyles ) )
        {
            xTextStyles.set(xFamilies->getByName( aCharStyles ), UNO_QUERY);
        }

        const OUString aNumStyles(RTL_CONSTASCII_USTRINGPARAM("NumberingStyles"));
        if( xFamilies->hasByName( aNumStyles ) )
        {
            xNumStyles.set(xFamilies->getByName( aNumStyles ), UNO_QUERY);
        }

        const OUString aFrameStyles(RTL_CONSTASCII_USTRINGPARAM("FrameStyles"));
        if( xFamilies->hasByName( aFrameStyles ) )
        {
            xFrameStyles.set(xFamilies->getByName( aFrameStyles ), UNO_QUERY);
        }

        const OUString aPageStyles(RTL_CONSTASCII_USTRINGPARAM("PageStyles"));
        if( xFamilies->hasByName( aPageStyles ) )
        {
            xPageStyles.set(xFamilies->getByName( aPageStyles ), UNO_QUERY);
        }
    }

    Reference < XTextFramesSupplier > xTFS( rModel, UNO_QUERY );
    if( xTFS.is() )
        xTextFrames.set(xTFS->getTextFrames());

    Reference < XTextGraphicObjectsSupplier > xTGOS( rModel, UNO_QUERY );
    if( xTGOS.is() )
        xGraphics.set(xTGOS->getGraphicObjects());

    Reference < XTextEmbeddedObjectsSupplier > xTEOS( rModel, UNO_QUERY );
    if( xTEOS.is() )
        xObjects.set(xTEOS->getEmbeddedObjects());

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
    // --> OD 2006-10-12 #i69629#
    delete [] mpOutlineStylesCandidates;
    // <--

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

SvXMLImportPropertyMapper *XMLTextImportHelper::CreateParaDefaultExtPropMapper(SvXMLImport& rImport, XMLFontStylesContext* pFontDecls)
{
    if (!pFontDecls)
        pFontDecls = const_cast<XMLFontStylesContext*>(rImport.GetFontDecls());

    XMLPropertySetMapper* pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE_PARA );
    SvXMLImportPropertyMapper* pImportMapper = new XMLTextImportPropertyMapper( pPropMapper, rImport, pFontDecls );

    pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT_ADDITIONAL_DEFAULTS );
    pImportMapper->ChainImportMapper( new XMLTextImportPropertyMapper( pPropMapper, rImport, pFontDecls ) );

    return pImportMapper;
}

SvXMLImportPropertyMapper*
    XMLTextImportHelper::CreateTableDefaultExtPropMapper(
        SvXMLImport& rImport,
        XMLFontStylesContext* )
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TABLE_DEFAULTS );
    return new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

SvXMLImportPropertyMapper*
    XMLTextImportHelper::CreateTableRowDefaultExtPropMapper(
        SvXMLImport& rImport,
        XMLFontStylesContext* )
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TABLE_ROW_DEFAULTS );
    return new SvXMLImportPropertyMapper( pPropMapper, rImport );
}

void XMLTextImportHelper::SetCursor( const Reference < XTextCursor > & rCursor )
{
    xCursor.set(rCursor);
    xText.set(rCursor->getText());
    xCursorAsRange.set( rCursor, UNO_QUERY );
}

void XMLTextImportHelper::ResetCursor()
{
    xCursor.set(0);
    xText.set(0);
    xCursorAsRange.set(0);
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
        Reference < XEnumeration > xEnum(xEnumAccess->createEnumeration());
        DBG_ASSERT( xEnum->hasMoreElements(), "empty text enumeration" );
        if( xEnum->hasMoreElements() )
        {
            Reference < XComponent > xComp( xEnum->nextElement(), UNO_QUERY );
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

OUString XMLTextImportHelper::ConvertStarFonts( const OUString& rChars,
                                                const OUString& rStyleName,
                                                sal_uInt8& rFlags,
                                                 sal_Bool bPara,
                                                SvXMLImport& rImport ) const
{
    OUStringBuffer sChars( rChars );
    sal_Bool bConverted = sal_False;
    for( sal_Int32 j=0; j<rChars.getLength(); j++ )
    {
        sal_Unicode c = rChars[j];
        if( c >= 0xf000 && c <= 0xf0ff )
        {
            if( (rFlags & CONV_STAR_FONT_FLAGS_VALID) == 0 )
            {
                XMLTextStyleContext *pStyle = 0;
                sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                           : XML_STYLE_FAMILY_TEXT_TEXT;
                if( rStyleName.getLength() && xAutoStyles.Is() )
                {
                    const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                                FindStyleChildContext( nFamily, rStyleName,
                                                       sal_True );
                    pStyle = PTR_CAST( XMLTextStyleContext,pTempStyle);
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
                sChars.setCharAt( j, rImport.ConvStarBatsCharToStarSymbol( c ) );
                bConverted = sal_True;
            }
            else if( (rFlags & CONV_FROM_STAR_MATH ) != 0 )
            {
                sChars.setCharAt( j, rImport.ConvStarMathCharToStarSymbol( c ) );
                bConverted = sal_True;
            }
        }
    }

    return bConverted ? sChars.makeStringAndClear() : rChars;
}

OUString XMLTextImportHelper::SetStyleAndAttrs(
        SvXMLImport& rImport,
        const Reference < XTextCursor >& rCursor,
        const OUString& rStyleName,
        sal_Bool bPara,
        sal_Bool bOutlineLevelAttrFound,
        sal_Int8 nOutlineLevel,
        // --> OD 2007-08-17 #i80724#
        sal_Bool bSetListAttrs )
        // <--
{
    const sal_uInt16 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                     : XML_STYLE_FAMILY_TEXT_TEXT;
    XMLTextStyleContext *pStyle = 0;
    OUString sStyleName( rStyleName );
    if( sStyleName.getLength() && xAutoStyles.Is() )
    {
        const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( nFamily, sStyleName, sal_True );
        pStyle = PTR_CAST( XMLTextStyleContext,pTempStyle);
    }
    if( pStyle )
        sStyleName = pStyle->GetParentName();

    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo(
        xPropSet->getPropertySetInfo());

    // style
    if( sStyleName.getLength() )
    {
        sStyleName = rImport.GetStyleDisplayName( nFamily, sStyleName );
        const String& rPropName = bPara ? sParaStyleName : sCharStyleName;
        const Reference < XNameContainer > & rStyles = bPara ? xParaStyles
                                                          : xTextStyles;
        if( rStyles.is() &&
            xPropSetInfo->hasPropertyByName( rPropName ) &&
            rStyles->hasByName( sStyleName ) )
        {
            xPropSet->setPropertyValue( rPropName, makeAny(sStyleName) );
        }
        else
            sStyleName = OUString();
    }

    // --> OD 2007-08-17 #i80724#
    if ( bSetListAttrs && bPara && xPropSetInfo->hasPropertyByName( sNumberingRules )  )
    // <--
    {
        // Set numbering rules
        Reference < XIndexReplace > xNumRules(xPropSet->getPropertyValue( sNumberingRules ), UNO_QUERY);

        if( IsInList() )
        {
            XMLTextListBlockContext *pListBlock = GetListBlock();
            Reference < XIndexReplace > xNewNumRules(
                pListBlock->GetNumRules());

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
                            xNumRuleCompare.set(
                                xCompareFac->createAnyCompareByName(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            "NumberingRules" ) ) ));
                            OSL_ENSURE( xNumRuleCompare .is(),
                                    "got no Numbering Rules comparison" );
                        }
                    }
                    if( xNumRuleCompare.is() )
                    {
                        bSameNumRules = (xNumRuleCompare->compare( makeAny(xNumRules),
                            makeAny(xNewNumRules) ) == 0);
                    }
                }
            }

            if( !bSameNumRules )
            {
                // #102607# This may except when xNewNumRules contains
                // a Writer-NumRule-Implementation bug gets applied to
                // a shape. Since this may occur inside a document
                // (e.g. when edited), this must be handled
                // gracefully.
                try
                {
                    xPropSet->setPropertyValue( sNumberingRules, makeAny(xNewNumRules) );
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
                xPropSet->setPropertyValue( sNumberingIsNumber, makeAny(bTmp) );
            }

            xPropSet->setPropertyValue( sNumberingLevel, makeAny(nLevel) );

            // #i36217# for impress/draw hard set the IsNumbering property
            // since we may not export it in future versions
/* remove this fix as we also export paragraphs with numbering level > 0 withoud IsNumbering
            {
                static const OUString sIsNumbering( RTL_CONSTASCII_USTRINGPARAM( "IsNumbering" ) );
                if( xPropSetInfo->hasPropertyByName( sIsNumbering ) )
                {
                    sal_Bool bTmp = pListItem ? sal_True : sal_False;
                    xPropSet->setPropertyValue( sIsNumbering, Any( bTmp ) );
                }
            }
*/
            if( pListBlock->IsRestartNumbering() )
            {
                // TODO: property missing
                if( xPropSetInfo->hasPropertyByName( sParaIsNumberingRestart ) )
                {
                    sal_Bool bTmp = sal_True;
                    xPropSet->setPropertyValue(sParaIsNumberingRestart,
                                               makeAny(bTmp) );
                }
                pListBlock->ResetRestartNumbering();
            }
            if( pListItem && pListItem->HasStartValue() &&
                xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                xPropSet->setPropertyValue(sNumberingStartValue,
                                           makeAny(pListItem->GetStartValue()));
            }
            SetListItem( (XMLTextListItemContext *)0 );
        }
        else
        {
            // If the paragraph is not in a list but its style, remove it from
            // the list.
            // --> OD 2005-10-25 #126347# - do not remove it, if the list
            // of the style is the chapter numbering rule.
            if( xNumRules.is() )
            {
                bool bRemove( true );
                if ( xChapterNumbering.is() )
                {
                    Reference< XNamed > xNumNamed( xNumRules, UNO_QUERY );
                    Reference< XNamed > xChapterNumNamed( xChapterNumbering, UNO_QUERY );
                    if ( xNumNamed.is() && xChapterNumNamed.is() &&
                         xNumNamed->getName() == xChapterNumNamed->getName() )
                    {
                        bRemove = false;
                    }
                }
                if ( bRemove )
                {
                    xPropSet->setPropertyValue( sNumberingRules, Any() );
                }
            }
            // <--
        }
    }

    // hard paragraph properties
    if( pStyle )
    {
        pStyle->FillPropertySet( xPropSet );
        if( bPara && pStyle->HasMasterPageName() &&
            xPropSetInfo->hasPropertyByName( sPageDescName ) )
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                                XML_STYLE_FAMILY_MASTER_PAGE,
                                pStyle->GetMasterPageName()) );
            if( !sDisplayName.getLength() ||
                (xPageStyles.is() &&
                 xPageStyles->hasByName( sDisplayName )) )
            {
                xPropSet->setPropertyValue( sPageDescName, makeAny(sDisplayName) );
            }
        }
        if( bPara && pStyle->GetDropCapStyleName().getLength() &&
            xTextStyles.is() )
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                                XML_STYLE_FAMILY_TEXT_TEXT,
                                pStyle->GetDropCapStyleName()) );
            if( xTextStyles->hasByName( sDisplayName  ) &&
                xPropSetInfo->hasPropertyByName( sDisplayName ) )
            {
                xPropSet->setPropertyValue( pStyle->sDropCapCharStyleName, makeAny(sDisplayName) );
            }
        }

        // combined characters special treatment
        if (!bPara && pStyle->HasCombinedCharactersLetter())
        {
            // insert combined characters text field
            if( xServiceFactory.is() )
            {
                Reference<XPropertySet> xTmp( xServiceFactory->createInstance(sServiceCombinedCharacters), UNO_QUERY );
                if( xTmp.is() )
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
                    xTmp->setPropertyValue(sContent, makeAny(rCursor->getString()));

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

    // outline level; set after list style has been set
    // --> OD 2005-08-25 #i53198#
    // Complete re-worked and corrected:
    // - set outline level at paragraph
    // - set numbering level at paragraph, if none is already set
    // - assure that style is marked as an outline style for the corresponding
    //   outline level.
    // - DO NOT set type of numbering rule to outline.
    // - DO NOT set numbering rule directly at the paragraph.
    if ( bPara && nOutlineLevel != -1 )
    {
        // --> OD 2005-08-25 #i53198# - set the outline level at the paragraph.
        {
            const OUString sParaChapterNumberingLevel(
                        RTL_CONSTASCII_USTRINGPARAM("ParaChapterNumberingLevel") );
            xPropSet->setPropertyValue( sParaChapterNumberingLevel,
                        makeAny( static_cast<sal_Int8>(nOutlineLevel - 1) ) );
        }
        // <--
        // --> OD 2005-09-01 #i53198# - set numbering level of the paragraph to
        // its outline level, if no numbering level is set.
        {
            sal_Int16 nNumLevel = -1;
            xPropSet->getPropertyValue( sNumberingLevel ) >>= nNumLevel;
            if ( nNumLevel == -1 )
            {
                xPropSet->setPropertyValue( sNumberingLevel,
                        makeAny( static_cast<sal_Int8>(nOutlineLevel - 1) ) );
            }
        }
        // <--
        // --> OD 2006-10-13 #i69629# - correction:
        // - for text document from version OOo 2.0.4/SO 8 PU4 and earlier
        //   the paragraph style of a heading should be assigned to the
        //   corresponding list level of the outline style.
        // - for other text documents the paragraph style of a heading is only
        //   a candidate for an assignment to the list level of the outline
        //   style, if it has no direct list style property and (if exists) the
        //   automatic paragraph style has also no direct list style set.
        if( xParaStyles->hasByName( sStyleName ) )
        {
            bool bOutlineStyleCandidate( false );

            sal_Int32 nUPD( 0 );
            sal_Int32 nBuild( 0 );
            // --> OD 2007-12-19 #152540#
            const bool bBuildIdFound = rImport.getBuildIds( nUPD, nBuild );
            // --> OD 2007-07-25 #i73509#
            if ( rImport.IsTextDocInOOoFileFormat() ||
                 ( bBuildIdFound && nUPD < 680 ) )
            {
                bOutlineStyleCandidate = true;
            }
            else if ( nUPD == 680 && nBuild <= 9073 ) /* BuildId of OOo 2.0.4/SO8 PU4 */
            {
                bOutlineStyleCandidate = bOutlineLevelAttrFound;
            }
            // <--
            else
            {
                Reference< XPropertyState > xStylePropState(
                                xParaStyles->getByName( sStyleName ), UNO_QUERY );
                if ( xStylePropState.is() &&
                     xStylePropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
                {
                    bOutlineStyleCandidate = false;
                }
                // --> OD 2007-01-11 #i73361#
                // The automatic paragraph style doesn't have to be considered.
//                else if ( pStyle && /* automatic paragraph style */
//                          pStyle->IsListStyleSet() )
//                {
//                    bOutlineStyleCandidate = false;
//                }
                // <--
                else
                {
                    bOutlineStyleCandidate = true;
                }
            }

            if ( bOutlineStyleCandidate )
            {
                AddOutlineStyleCandidate( nOutlineLevel, sStyleName );
            }
        }
        // <--
    }
    // <--

    return sStyleName;
}

// --> OD 2006-10-12 #i69629#
// adjustments to reflect change of internal data structure <mpOutlineStylesCandidates>
void XMLTextImportHelper::FindOutlineStyleName( ::rtl::OUString& rStyleName,
                                                sal_Int8 nOutlineLevel )
{
    // style name empty?
    if( rStyleName.getLength() == 0 )
    {
        // Empty? Then we need o do stuff. Let's do error checking first.
        if( xChapterNumbering.is() &&
            ( nOutlineLevel > 0 ) &&
            ( nOutlineLevel <= xChapterNumbering->getCount() ) )
        {
            nOutlineLevel--;   // for the remainder, the level's are 0-based

            // empty style name: look-up previously used name

            // if we don't have a previously used name, we'll use the default
            if ( mpOutlineStylesCandidates == NULL )
            {
                mpOutlineStylesCandidates = new ::std::vector<OUString>[xChapterNumbering->getCount()];
            }

            if ( mpOutlineStylesCandidates[nOutlineLevel].size() == 0 )
            {
                // no other name used previously? Then use default

                // iterate over property value sequence to find the style name
                Sequence<PropertyValue> aProperties;
                xChapterNumbering->getByIndex( nOutlineLevel ) >>= aProperties;
                for( sal_Int32 i = 0; i < aProperties.getLength(); i++ )
                {
                    if( aProperties[i].Name == sHeadingStyleName )
                    {
                        rtl::OUString aOutlineStyle;
                        aProperties[i].Value >>= aOutlineStyle;
                        mpOutlineStylesCandidates[nOutlineLevel].push_back( aOutlineStyle );
                        break;  // early out, if we found it!.
                    }
                }
            }

            // finally, we'll use the previously used style name for this
            // format (or the default we've just put into that style)
            // --> OD 2006-11-06 #i71249# - take last added one
            rStyleName = mpOutlineStylesCandidates[nOutlineLevel].back();
            // <--
        }
        // else: nothing we can do, so we'll leave it empty
    }
    // else: we already had a style name, so we let it pass.
}
// <--

// --> OD 2006-10-12 #i69629#
void XMLTextImportHelper::AddOutlineStyleCandidate( const sal_Int8 nOutlineLevel,
                                                    const OUString& rStyleName )
{
    if ( rStyleName.getLength() &&
         xChapterNumbering.is() &&
         nOutlineLevel > 0 && nOutlineLevel <= xChapterNumbering->getCount() )
    {
        if( !mpOutlineStylesCandidates )
        {
#ifdef IRIX
            /* GCC 2 bug when member function is called as part of an array
             * initialiser
             */
            sal_Int8 count = xChapterNumbering->getCount();
            mpOutlineStylesCandidates = new ::std::vector<OUString>[count];
#else
            mpOutlineStylesCandidates = new ::std::vector<OUString>[xChapterNumbering->getCount()];
#endif
        }
        mpOutlineStylesCandidates[nOutlineLevel-1].push_back( rStyleName );
    }
}
// <--

// --> OD 2006-10-12 #i69629#
// helper method to determine, if a paragraph style has a list style (inclusive
// an empty one) inherits a list style (inclusive an empty one) from one of its parents
// --> OD 2007-01-29 #i73973#
// apply special case, that found list style equals the chapter numbering, also
// to the found list styles of the parent styles.
sal_Bool lcl_HasListStyle( OUString sStyleName,
                           const Reference < XNameContainer >& xParaStyles,
                           SvXMLImport& rImport,
                           const OUString& sNumberingStyleName,
                           const OUString& sOutlineStyleName )
{
    sal_Bool bRet( sal_False );

    if ( !xParaStyles->hasByName( sStyleName ) )
    {
        // error case
        return sal_True;
    }

    Reference< XPropertyState > xPropState( xParaStyles->getByName( sStyleName ),
                                            UNO_QUERY );
    if ( !xPropState.is() )
    {
        // error case
        return sal_False;
    }

    if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
    {
        // list style found
        bRet = sal_True;
        // special case: the set list style equals the chapter numbering
        Reference< XPropertySet > xPropSet( xPropState, UNO_QUERY );
        if ( xPropSet.is() )
        {
            OUString sListStyle;
            xPropSet->getPropertyValue( sNumberingStyleName ) >>= sListStyle;
            if ( sListStyle == sOutlineStyleName )
            {
                bRet = sal_False;
            }
        }
    }
    else
    {
        // --> OD 2007-12-07 #i77708#
        sal_Int32 nUPD( 0 );
        sal_Int32 nBuild( 0 );
        rImport.getBuildIds( nUPD, nBuild );
        // <--
        // search list style at parent
        Reference<XStyle> xStyle( xPropState, UNO_QUERY );
        while ( xStyle.is() )
        {
            OUString aParentStyle( xStyle->getParentStyle() );
            if ( aParentStyle.getLength() > 0 )
            {
                aParentStyle =
                    rImport.GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                                 aParentStyle );
            }
            if ( aParentStyle.getLength() == 0 ||
                 !xParaStyles->hasByName( aParentStyle ) )
            {
                // no list style found
                break;
            }
            else
            {
                xPropState = Reference< XPropertyState >(
                                    xParaStyles->getByName( aParentStyle ),
                                    UNO_QUERY );
                if ( !xPropState.is() )
                {
                    // error case
                    return sal_True;
                }
                if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
                {
                    // list style found
                    bRet = sal_True;
                    // --> OD 2007-01-29 #i73973#
                    // special case: the found list style equals the chapter numbering
                    Reference< XPropertySet > xPropSet( xPropState, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        OUString sListStyle;
                        xPropSet->getPropertyValue( sNumberingStyleName ) >>= sListStyle;
                        if ( sListStyle == sOutlineStyleName )
                        {
                            bRet = sal_False;
                        }
                        // --> OD 2007-12-07 #i77708#
                        // special handling for text documents from OOo version prior OOo 2.3.1
                        else if ( sListStyle.getLength() == 0 &&
                                  ( ( nUPD < 680 ) ||  // prior OOo 2.0
                                    ( nUPD == 680 && nBuild <= 9238 ) ) ) // OOo 2.0 - OOo 2.3.1
                        {
                            bRet = sal_False;
                        }
                        // <--
                    }
                    // <--
                    break;
                }
                else
                {
                    // search list style at parent
                    xStyle = Reference<XStyle>( xPropState, UNO_QUERY );
                }
            }
        }
    }

    return bRet;
}
// <--
// --> OD 2006-10-12 #i69629#
void XMLTextImportHelper::SetOutlineStyles( sal_Bool bSetEmptyLevels )
{
    if ( ( mpOutlineStylesCandidates != NULL || bSetEmptyLevels ) &&
         xChapterNumbering.is() &&
         !( IsInsertMode() || IsStylesOnlyMode() ) )
    {
        // --> OD 2007-12-19 #152540#
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
                    bChooseLastOne = nUPD < 680 ||
                                     ( nUPD == 680 && nBuild <= 9073 ) /* BuildId of OOo 2.0.4/SO8 PU4 */;
                }
            }
        }
        // <--

        OUString sOutlineStyleName;
        {
            Reference<XPropertySet> xChapterNumRule( xChapterNumbering, UNO_QUERY );
            const OUString sName(RTL_CONSTASCII_USTRINGPARAM("Name"));
            xChapterNumRule->getPropertyValue(sName) >>= sOutlineStyleName;
        }

        OUString sEmpty;
        sal_Int32 nCount = xChapterNumbering->getCount();
        for( sal_Int32 i=0; i < nCount; ++i )
        {
            if ( bSetEmptyLevels ||
                 ( mpOutlineStylesCandidates &&
                   mpOutlineStylesCandidates[i].size() > 0 ) )
            {
                // determine, which candidate is one to be assigned to the list
                // level of the outline style
                OUString sChoosenStyle( sEmpty );
                if ( mpOutlineStylesCandidates &&
                     mpOutlineStylesCandidates[i].size() > 0 )
                {
                    // --> OD 2007-12-19 #152540#
                    if ( bChooseLastOne )
                    // <--
                    {
                        // --> OD 2006-11-06 #i71249# - take last added one
                        sChoosenStyle = mpOutlineStylesCandidates[i].back();
                        // <--
                    }
                    else
                    {
                        for ( sal_uInt32 j = 0; j < mpOutlineStylesCandidates[i].size(); ++j )
                        {
                            if ( !lcl_HasListStyle( mpOutlineStylesCandidates[i][j],
                                                    xParaStyles, GetXMLImport(),
                                                    sNumberingStyleName,
                                                    sOutlineStyleName ) )
                            {
                                sChoosenStyle = mpOutlineStylesCandidates[i][j];
                                break;
                            }
                        }
                    }
                }

                Sequence < PropertyValue > aProps( 1 );
                PropertyValue *pProps = aProps.getArray();
                pProps->Name = sHeadingStyleName;
                pProps->Value <<= sChoosenStyle;

                xChapterNumbering->replaceByIndex( i, makeAny( aProps ) );
            }
        }
    }
}
// <--

void XMLTextImportHelper::SetHyperlink(
    SvXMLImport& rImport,
    const Reference < XTextCursor >& rCursor,
    const OUString& rHRef,
    const OUString& rName,
    const OUString& rTargetFrameName,
    const OUString& rStyleName,
    const OUString& rVisitedStyleName,
    XMLEventsImportContext* pEvents)
{
    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference < XPropertySetInfo > xPropSetInfo(
        xPropSet->getPropertySetInfo());
    if( !xPropSetInfo.is() || !xPropSetInfo->hasPropertyByName(sHyperLinkURL) )
        return;

    xPropSet->setPropertyValue( sHyperLinkURL, makeAny(rHRef) );

    if( xPropSetInfo->hasPropertyByName( sHyperLinkName ) )
    {
        xPropSet->setPropertyValue( sHyperLinkName, makeAny(rName) );
    }

    if( xPropSetInfo->hasPropertyByName( sHyperLinkTarget ) )
    {
        xPropSet->setPropertyValue( sHyperLinkTarget, makeAny(rTargetFrameName) );
    }

    if ( (pEvents != NULL) &&
         xPropSetInfo->hasPropertyByName( sHyperLinkEvents ))
    {
        // The API treats events at hyperlinks differently from most
        // other properties: You have to set a name replace with the
        // events in it. The easiest way to to this is to 1) get
        // events, 2) set new ones, and 3) then put events back.
        Reference<XNameReplace> xReplace(xPropSet->getPropertyValue( sHyperLinkEvents ), UNO_QUERY);
        if (xReplace.is())
        {
            // set events
            pEvents->SetEvents(xReplace);

            // put events
            xPropSet->setPropertyValue( sHyperLinkEvents, makeAny(xReplace) );
        }
    }

    if( xTextStyles.is() )
    {
        OUString sDisplayName(
            rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rStyleName ) );
        if( sDisplayName.getLength() &&
            xPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName ) &&
            xTextStyles->hasByName( sDisplayName ) )
        {
            xPropSet->setPropertyValue( sUnvisitedCharStyleName, makeAny(sDisplayName) );
        }

        sDisplayName =
            rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rVisitedStyleName );
        if( sDisplayName.getLength() &&
            xPropSetInfo->hasPropertyByName( sVisitedCharStyleName ) &&
            xTextStyles->hasByName( sDisplayName ) )
        {
            xPropSet->setPropertyValue( sVisitedCharStyleName, makeAny(sDisplayName) );
        }
    }
}

void XMLTextImportHelper::SetRuby(
    SvXMLImport& rImport,
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
        // the ruby text
        xPropSet->setPropertyValue(sRubyText, makeAny(rText));

        // the ruby style (ruby-adjust)
        XMLPropStyleContext *pStyle = 0;
        if( rStyleName.getLength() && xAutoStyles.Is() )
        {
            const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                FindStyleChildContext( XML_STYLE_FAMILY_TEXT_RUBY,
                                       rStyleName, sal_True );
            pStyle = PTR_CAST(XMLPropStyleContext,pTempStyle);

            if (NULL != pStyle)
                pStyle->FillPropertySet( xPropSet );
        }

        // the ruby text character style
        if( xTextStyles.is() )
        {
            OUString sDisplayName(
                rImport.GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, rTextStyleName ) );
            if( (sDisplayName.getLength() > 0) &&
//              xPropSetInfo->hasPropertyByName( sRubyCharStyleName ) &&
                xTextStyles->hasByName( sDisplayName ) )
            {
                xPropSet->setPropertyValue(sRubyCharStyleName, makeAny(sDisplayName));
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
    sal_Bool bHeading = sal_False;
    sal_Bool bContent = sal_True;
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
     case XML_TOK_TEXT_NUMBERED_PARAGRAPH:
         pContext = new XMLNumberedParaContext( rImport, nPrefix, rLocalName );
        break;
    case XML_TOK_TEXT_LIST:
        pContext = new XMLTextListBlockContext( rImport, *this,
                                                nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TABLE_TABLE:
        if( XML_TEXT_TYPE_BODY == eType ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
             XML_TEXT_TYPE_SECTION == eType ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType ||
            XML_TEXT_TYPE_CHANGED_REGION == eType ||
            XML_TEXT_TYPE_CELL == eType )
            pContext = CreateTableChildContext( rImport, nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TEXT_SEQUENCE_DECLS:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_HEADER_FOOTER == eType )
        {
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSequence);
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

    case XML_TOK_TEXT_FRAME_PAGE:
        if( (XML_TEXT_TYPE_BODY == eType && bBodyContentStarted) ||
            XML_TEXT_TYPE_TEXTBOX == eType ||
            XML_TEXT_TYPE_CHANGED_REGION == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType );
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

//  if( !pContext )
//      pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    // handle open redlines
    if ( (XML_TOK_TEXT_CHANGE != nToken) &&
         (XML_TOK_TEXT_CHANGE_END != nToken) &&
         (XML_TOK_TEXT_CHANGE_START != nToken) )
    {
//      ResetOpenRedlineId();
    }

    if( XML_TEXT_TYPE_BODY == eType && bContent )
        bBodyContentStarted = sal_False;

    return pContext;
}

SvXMLImportContext *XMLTextImportHelper::CreateTableChildContext(
        SvXMLImport&,
        sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/,
        const Reference< XAttributeList > & )
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
        const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_TEXT_LIST, rName,
                                           sal_True );
        pStyle = PTR_CAST( SvxXMLListStyleContext ,pTempStyle);
    }

    return pStyle;
}

XMLPropStyleContext *XMLTextImportHelper::FindAutoFrameStyle( const OUString& rName ) const
{
    XMLPropStyleContext *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rName,
                                           sal_True );
        pStyle = PTR_CAST( XMLPropStyleContext ,pTempStyle);
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindSectionStyle(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = NULL;
    if (xAutoStyles.Is() )
    {
        const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_TEXT_SECTION,
                               rName, sal_True );
        pStyle = PTR_CAST( XMLPropStyleContext,pTempStyle);
    }

    return pStyle;
}

XMLPropStyleContext* XMLTextImportHelper::FindPageMaster(
    const OUString& rName ) const
{
    XMLPropStyleContext* pStyle = NULL;
    if (xAutoStyles.Is() )
    {
        const SvXMLStyleContext* pTempStyle = ((SvXMLStylesContext *)&xAutoStyles)->
                           FindStyleChildContext(
                               XML_STYLE_FAMILY_PAGE_MASTER,
                               rName, sal_True );
        pStyle = PTR_CAST( XMLPropStyleContext,pTempStyle);
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
    aBookmarkStartRanges[sName].set(rRange);
}

sal_Bool XMLTextImportHelper::FindAndRemoveBookmarkStartRange(
    Reference<XTextRange> & rRange,
    const OUString sName)
{
    if (aBookmarkStartRanges.count(sName))
    {
        rRange.set(aBookmarkStartRanges[sName]);
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
        OUString sNextFrmName(GetRenameMap().Get( XML_TEXT_RENAME_TYPE_FRAME,
                                                    rNextFrmName ));
        if( xTextFrames.is() && xTextFrames->hasByName( sNextFrmName ) )
        {
            rFrmPropSet->setPropertyValue( sChainNextName, makeAny(sNextFrmName) );
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

                rFrmPropSet->setPropertyValue( sChainPrevName, makeAny(OUString( *pPrev )) );

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
            Reference<XTextFrame> xFrame(xPropSet->getPropertyValue(sTextFrame), UNO_QUERY);

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
                                          sal_Bool /*bMayScript*/,
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
        std::map < const rtl::OUString, rtl::OUString, UStringLess > &)
{
}
// redline helper: dummy implementation to be overridden in sw/filter/xml
void XMLTextImportHelper::RedlineAdd(
    const OUString& /*rType*/,
    const OUString& /*rId*/,
    const OUString& /*rAuthor*/,
    const OUString& /*rComment*/,
    const DateTime& /*rDateTime*/,
    sal_Bool /*bMergeLastPara*/)
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
    sal_Bool /*bStart*/,
    sal_Bool /*bIsOutsideOfParagraph*/)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::RedlineAdjustStartNodeCursor(sal_Bool)
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetShowChanges( sal_Bool )
{
    // dummy implementation: do nothing
}

void XMLTextImportHelper::SetRecordChanges( sal_Bool )
{
    // dummy implementation: do nothing
}
void XMLTextImportHelper::SetChangesProtectionKey(const Sequence<sal_Int8> &)
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
