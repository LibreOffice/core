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
#ifndef _XMLOFF_TEXTIMP_HXX_
#define _XMLOFF_TEXTIMP_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef __SGI_STL_MAP
#include <map>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <bf_xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <bf_xmloff/xmltkmap.hxx>
#endif

// functional.hxx is obsolete and should be replaced by its comphelper
// counterpart
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <bf_xmloff/uniref.hxx>
#endif
namespace com { namespace sun { namespace star {
namespace text { class XText; class XTextCursor; class XTextRange; class XTextContent; }
namespace frame { class XModel; }
namespace container { class XNameContainer; class XIndexReplace; class XNameAccess; }
namespace beans { class XPropertySet; }
namespace xml { namespace sax { class XAttributeList; } }
namespace util { struct DateTime; }
namespace lang { class XMultiServiceFactory; }
namespace ucb { class XAnyCompare; }
} } }
namespace binfilter {
class SvStringsDtor;

class SvXMLImport;
class SvXMLStylesContext;
class XMLTextListBlockContext;
class XMLTextListItemContext;
class SvxXMLListStyleContext;
class XMLPropStyleContext;
class SvI18NMap;
class XMLSectionImportContext;
class XMLFontStylesContext;
template<class A> class XMLPropertyBackpatcher;
class XMLEventsImportContext;


enum SwXMLTextElemTokens
{
    XML_TOK_TEXT_P,
    XML_TOK_TEXT_H,
    XML_TOK_TEXT_ORDERED_LIST,
    XML_TOK_TEXT_UNORDERED_LIST,
    XML_TOK_TEXT_TEXTBOX_PAGE,
    XML_TOK_TEXT_IMAGE_PAGE,
    XML_TOK_TEXT_OBJECT_PAGE,
    XML_TOK_TEXT_OBJECT_OLE_PAGE,
    XML_TOK_TABLE_TABLE,
    XML_TOK_TEXT_VARFIELD_DECLS,
    XML_TOK_TEXT_USERFIELD_DECLS,
    XML_TOK_TEXT_SEQUENCE_DECLS,
    XML_TOK_TEXT_DDE_DECLS,
    XML_TOK_DRAW_A_PAGE,
    XML_TOK_TEXT_SECTION,
    XML_TOK_TEXT_TOC,
    XML_TOK_TEXT_OBJECT_INDEX,
    XML_TOK_TEXT_TABLE_INDEX,
    XML_TOK_TEXT_ILLUSTRATION_INDEX,
    XML_TOK_TEXT_USER_INDEX,
    XML_TOK_TEXT_ALPHABETICAL_INDEX,
    XML_TOK_TEXT_BIBLIOGRAPHY_INDEX,
    XML_TOK_TEXT_INDEX_TITLE,
    XML_TOK_TEXT_TRACKED_CHANGES,
    XML_TOK_TEXT_CHANGE_START,
    XML_TOK_TEXT_CHANGE_END,
    XML_TOK_TEXT_CHANGE,
    XML_TOK_TEXT_FORMS,
    XML_TOK_TEXT_CALCULATION_SETTINGS,
    XML_TOK_TEXT_AUTOMARK,
    XML_TOK_TEXT_APPLET_PAGE,
    XML_TOK_TEXT_PLUGIN_PAGE,
    XML_TOK_TEXT_FLOATING_FRAME_PAGE,
    XML_TOK_TEXT_ELEM_END=XML_TOK_UNKNOWN
};

enum XMLTextPElemTokens
{
    XML_TOK_TEXT_SPAN,
    XML_TOK_TEXT_TAB_STOP,
    XML_TOK_TEXT_LINE_BREAK,
    XML_TOK_TEXT_S,
    XML_TOK_TEXT_TEXTBOX,
    XML_TOK_TEXT_IMAGE,
    XML_TOK_TEXT_OBJECT,
    XML_TOK_TEXT_OBJECT_OLE,
    XML_TOK_TEXT_FLOATING_FRAME,
    XML_TOK_TEXT_APPLET,
    XML_TOK_TEXT_PLUGIN,
    XML_TOK_TEXT_HYPERLINK,
    XML_TOK_TEXT_RUBY,

    XML_TOK_TEXT_FOOTNOTE,
    XML_TOK_TEXT_ENDNOTE,
    XML_TOK_TEXT_BOOKMARK,
    XML_TOK_TEXT_BOOKMARK_START,
    XML_TOK_TEXT_BOOKMARK_END,
    XML_TOK_TEXT_REFERENCE,
    XML_TOK_TEXT_REFERENCE_START,
    XML_TOK_TEXT_REFERENCE_END,

    XML_TOK_TEXT_TOC_MARK,			// index marks
    XML_TOK_TEXT_TOC_MARK_START,
    XML_TOK_TEXT_TOC_MARK_END,
    XML_TOK_TEXT_USER_INDEX_MARK,
    XML_TOK_TEXT_USER_INDEX_MARK_START,
    XML_TOK_TEXT_USER_INDEX_MARK_END,
    XML_TOK_TEXT_ALPHA_INDEX_MARK,
    XML_TOK_TEXT_ALPHA_INDEX_MARK_START,
    XML_TOK_TEXT_ALPHA_INDEX_MARK_END,

    XML_TOK_TEXT_SENDER_FIRSTNAME,	// sender fields
    XML_TOK_TEXT_SENDER_LASTNAME,
    XML_TOK_TEXT_SENDER_INITIALS,
    XML_TOK_TEXT_SENDER_TITLE,
    XML_TOK_TEXT_SENDER_POSITION,
    XML_TOK_TEXT_SENDER_EMAIL,
    XML_TOK_TEXT_SENDER_PHONE_PRIVATE,
    XML_TOK_TEXT_SENDER_FAX,
    XML_TOK_TEXT_SENDER_COMPANY,
    XML_TOK_TEXT_SENDER_PHONE_WORK,
    XML_TOK_TEXT_SENDER_STREET,
    XML_TOK_TEXT_SENDER_CITY,
    XML_TOK_TEXT_SENDER_POSTAL_CODE,
    XML_TOK_TEXT_SENDER_COUNTRY,
    XML_TOK_TEXT_SENDER_STATE_OR_PROVINCE,

    XML_TOK_TEXT_AUTHOR_NAME,	// author fields
    XML_TOK_TEXT_AUTHOR_INITIALS,

    XML_TOK_TEXT_DATE,			// date & time fields
    XML_TOK_TEXT_TIME,

    XML_TOK_TEXT_PAGE_NUMBER,	// page number fields
    XML_TOK_TEXT_PAGE_CONTINUATION_STRING,

    XML_TOK_TEXT_VARIABLE_SET,	// variable fields
    XML_TOK_TEXT_VARIABLE_GET,
    XML_TOK_TEXT_VARIABLE_INPUT,
    XML_TOK_TEXT_USER_FIELD_GET,
    XML_TOK_TEXT_USER_FIELD_INPUT,
    XML_TOK_TEXT_SEQUENCE,
    XML_TOK_TEXT_EXPRESSION,
    XML_TOK_TEXT_TEXT_INPUT,

    XML_TOK_TEXT_DATABASE_DISPLAY,	// database fields
    XML_TOK_TEXT_DATABASE_NEXT,
    XML_TOK_TEXT_DATABASE_SELECT,
    XML_TOK_TEXT_DATABASE_ROW_NUMBER,
    XML_TOK_TEXT_DATABASE_NAME,

    XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR, //document info fields
    XML_TOK_TEXT_DOCUMENT_CREATION_DATE,
    XML_TOK_TEXT_DOCUMENT_CREATION_TIME,
    XML_TOK_TEXT_DOCUMENT_DESCRIPTION,
    XML_TOK_TEXT_DOCUMENT_INFORMATION_0,
    XML_TOK_TEXT_DOCUMENT_INFORMATION_1,
    XML_TOK_TEXT_DOCUMENT_INFORMATION_2,
    XML_TOK_TEXT_DOCUMENT_INFORMATION_3,
    XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR,
    XML_TOK_TEXT_DOCUMENT_PRINT_DATE,
    XML_TOK_TEXT_DOCUMENT_PRINT_TIME,
    XML_TOK_TEXT_DOCUMENT_TITLE,
    XML_TOK_TEXT_DOCUMENT_SUBJECT,
    XML_TOK_TEXT_DOCUMENT_KEYWORDS,
    XML_TOK_TEXT_DOCUMENT_REVISION,
    XML_TOK_TEXT_DOCUMENT_EDIT_DURATION,
    XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR,
    XML_TOK_TEXT_DOCUMENT_SAVE_DATE,
    XML_TOK_TEXT_DOCUMENT_SAVE_TIME,
    XML_TOK_TEXT_DOCUMENT_USER_DEFINED,

    XML_TOK_TEXT_PLACEHOLDER,	// misc. fields
    XML_TOK_TEXT_CONDITIONAL_TEXT,
    XML_TOK_TEXT_HIDDEN_TEXT,
    XML_TOK_TEXT_HIDDEN_PARAGRAPH,
    XML_TOK_TEXT_FILENAME,
    XML_TOK_TEXT_CHAPTER,
    XML_TOK_TEXT_TEMPLATENAME,
    XML_TOK_TEXT_WORD_COUNT,	// statistics/count fields
    XML_TOK_TEXT_PARAGRAPH_COUNT,
    XML_TOK_TEXT_TABLE_COUNT,
    XML_TOK_TEXT_CHARACTER_COUNT,
    XML_TOK_TEXT_IMAGE_COUNT,
    XML_TOK_TEXT_OBJECT_COUNT,
    XML_TOK_TEXT_PAGE_COUNT,
    XML_TOK_TEXT_GET_PAGE_VAR,
    XML_TOK_TEXT_SET_PAGE_VAR,
    XML_TOK_TEXT_MACRO,
    XML_TOK_TEXT_DDE,
    XML_TOK_TEXT_REFERENCE_REF,
    XML_TOK_TEXT_BOOKMARK_REF,
    XML_TOK_TEXT_SEQUENCE_REF,
    XML_TOK_TEXT_FOOTNOTE_REF,
    XML_TOK_TEXT_ENDNOTE_REF,
    XML_TOK_TEXT_SHEET_NAME,
    XML_TOK_TEXT_BIBLIOGRAPHY_MARK,
    XML_TOK_TEXT_ANNOTATION,
    XML_TOK_TEXT_SCRIPT,
    XML_TOK_TEXT_TABLE_FORMULA,
    XML_TOK_TEXT_DROPDOWN,

    // misc
    XML_TOK_TEXTP_CHANGE_START,	// avoid ambigiouty with SwXMLTextElemTokens
    XML_TOK_TEXTP_CHANGE_END,	// avoid ambigiouty with SwXMLTextElemTokens
    XML_TOK_TEXTP_CHANGE,		// avoid ambigiouty with SwXMLTextElemTokens
    XML_TOK_DRAW_A,
    XML_TOK_TEXT_MEASURE,

    XML_TOK_TEXT_P_ELEM_END=XML_TOK_UNKNOWN
};

enum XMLTextPAttrTokens
{
    XML_TOK_TEXT_P_STYLE_NAME,
    XML_TOK_TEXT_P_COND_STYLE_NAME,
    XML_TOK_TEXT_P_LEVEL,
    XML_TOK_TEXT_P_END=XML_TOK_UNKNOWN
};

enum XMLTextListBlockAttrTokens
{
    XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME,
    XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING,
    XML_TOK_TEXT_LIST_BLOCK_END=XML_TOK_UNKNOWN
};

enum XMLTextListBlockElemTokens
{
    XML_TOK_TEXT_LIST_HEADER,
    XML_TOK_TEXT_LIST_ITEM,
    XML_TOK_TEXT_LIST_BLOCK_ELEM_END=XML_TOK_UNKNOWN
};

enum XMLTextFrameAttrTokens
{
    XML_TOK_TEXT_FRAME_STYLE_NAME,
    XML_TOK_TEXT_FRAME_NAME,
    XML_TOK_TEXT_FRAME_ANCHOR_TYPE,
    XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER,
    XML_TOK_TEXT_FRAME_X,
    XML_TOK_TEXT_FRAME_Y,
    XML_TOK_TEXT_FRAME_WIDTH,
    XML_TOK_TEXT_FRAME_REL_WIDTH,
    XML_TOK_TEXT_FRAME_HEIGHT,
    XML_TOK_TEXT_FRAME_MIN_HEIGHT,
    XML_TOK_TEXT_FRAME_REL_HEIGHT,
    XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME,
    XML_TOK_TEXT_FRAME_HREF,
    XML_TOK_TEXT_FRAME_FILTER_NAME,
    XML_TOK_TEXT_FRAME_Z_INDEX,
    XML_TOK_TEXT_FRAME_TRANSFORM,
    XML_TOK_TEXT_FRAME_CLASS_ID,
    XML_TOK_TEXT_FRAME_CODE,
    XML_TOK_TEXT_FRAME_OBJECT,
    XML_TOK_TEXT_FRAME_ARCHIVE,
    XML_TOK_TEXT_FRAME_MAY_SCRIPT,
    XML_TOK_TEXT_FRAME_MIME_TYPE,
    XML_TOK_TEXT_FRAME_APPLET_NAME,
    XML_TOK_TEXT_FRAME_FRAME_NAME,
    XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE,
    XML_TOK_TEXT_FRAME_END=XML_TOK_UNKNOWN
};

enum XMLTextHyperlinkAttrTokens
{
    XML_TOK_TEXT_HYPERLINK_HREF,
    XML_TOK_TEXT_HYPERLINK_NAME,
    XML_TOK_TEXT_HYPERLINK_TARGET_FRAME,
    XML_TOK_TEXT_HYPERLINK_SHOW,
    XML_TOK_TEXT_HYPERLINK_STYLE_NAME,
    XML_TOK_TEXT_HYPERLINK_VIS_STYLE_NAME,
    XML_TOK_TEXT_HYPERLINK_SERVER_MAP,
    XML_TOK_TEXT_HYPERLINK_END=XML_TOK_UNKNOWN
};

enum XMLTextPageMasterElemTokens
{
    XML_TOK_TEXT_MP_HEADER,
    XML_TOK_TEXT_MP_FOOTER,
    XML_TOK_TEXT_MP_HEADER_LEFT,
    XML_TOK_TEXT_MP_FOOTER_LEFT,
    XML_TOK_TEXT_MP_END=XML_TOK_UNKNOWN
};

enum XMLTextContourAttrTokens
{
    XML_TOK_TEXT_CONTOUR_WIDTH,
    XML_TOK_TEXT_CONTOUR_HEIGHT,
    XML_TOK_TEXT_CONTOUR_VIEWBOX,
    XML_TOK_TEXT_CONTOUR_POINTS,
    XML_TOK_TEXT_CONTOUR_D,
    XML_TOK_TEXT_CONTOUR_AUTO,
    XML_TOK_TEXT_CONTOUR_END=XML_TOK_UNKNOWN
};
enum XMLTextType
{
    XML_TEXT_TYPE_BODY,
    XML_TEXT_TYPE_CELL,
    XML_TEXT_TYPE_SHAPE,
    XML_TEXT_TYPE_TEXTBOX,
    XML_TEXT_TYPE_HEADER_FOOTER,
    XML_TEXT_TYPE_SECTION,
    XML_TEXT_TYPE_FOOTNOTE,
    XML_TEXT_TYPE_CHANGED_REGION
};

#define XML_TEXT_RENAME_TYPE_FRAME 10
#define XML_TEXT_RENAME_TYPE_TABLE 20

// create type for section list, XMLSectionList_Impl
DECLARE_LIST( XMLSectionList_Impl, XMLSectionImportContext* )

class XMLTextImportHelper : public UniRefBase
{
    SvXMLTokenMap *pTextElemTokenMap;
    SvXMLTokenMap *pTextPElemTokenMap;
    SvXMLTokenMap *pTextPAttrTokenMap;
    SvXMLTokenMap *pTextFieldAttrTokenMap;
    SvXMLTokenMap *pTextListBlockAttrTokenMap;
    SvXMLTokenMap *pTextListBlockElemTokenMap;
    SvXMLTokenMap *pTextFrameAttrTokenMap;
    SvXMLTokenMap *pTextContourAttrTokenMap;
    SvXMLTokenMap *pTextHyperlinkAttrTokenMap;
    SvXMLTokenMap *pTextMasterPageElemTokenMap;
    SvStringsDtor *pPrevFrmNames;
    SvStringsDtor *pNextFrmNames;

    SvXMLImportContextRef xAutoStyles;
    SvXMLImportContextRef xFontDecls;
    SvXMLImportContextRef xListBlock;
    SvXMLImportContextRef xListItem;

    XMLSectionList_Impl aSectionList;

    UniReference < SvXMLImportPropertyMapper > xParaImpPrMap;
    UniReference < SvXMLImportPropertyMapper > xTextImpPrMap;
    UniReference < SvXMLImportPropertyMapper > xFrameImpPrMap;
    UniReference < SvXMLImportPropertyMapper > xSectionImpPrMap;
    UniReference < SvXMLImportPropertyMapper > xRubyImpPrMap;

    SvI18NMap *pRenameMap;
    ::rtl::OUString *pOutlineStyles;

    /// start ranges for open bookmarks
    ::std::map< ::rtl::OUString,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextRange>,
                ::comphelper::UStringLess> aBookmarkStartRanges;

    /// backpatcher for references to footnotes and endnotes
    XMLPropertyBackpatcher<sal_Int16> * pFootnoteBackpatcher;

    /// backpatchers for references to sequences
    XMLPropertyBackpatcher<sal_Int16> * pSequenceIdBackpatcher;

    XMLPropertyBackpatcher< ::rtl::OUString> * pSequenceNameBackpatcher;

    /// name of the last 'open' redline that started between paragraphs
    ::rtl::OUString sOpenRedlineIdentifier;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XText > xText;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextRange > xCursorAsRange;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer > xParaStyles;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer > xTextStyles;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer > xNumStyles;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer > xFrameStyles;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer > xPageStyles;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexReplace > xChapterNumbering;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > xTextFrames;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > xGraphics;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > xObjects;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XAnyCompare > xNumRuleCompare;

    sal_Bool bInsertMode : 1;
    sal_Bool bStylesOnlyMode : 1;
    sal_Bool bBlockMode : 1;
    sal_Bool bProgress : 1;
    sal_Bool bOrganizerMode : 1;
    sal_Bool bBodyContentStarted : 1;

    // #107848#
    // One more flag to remember if we are inside a deleted redline section
    sal_Bool bInsideDeleteContext : 1;

    SvXMLTokenMap *_GetTextElemTokenMap();
    SvXMLTokenMap *_GetTextPElemTokenMap();
    SvXMLTokenMap *_GetTextPAttrTokenMap();
    SvXMLTokenMap *_GetTextFrameAttrTokenMap();
    SvXMLTokenMap *_GetTextContourAttrTokenMap();
    SvXMLTokenMap *_GetTextHyperlinkAttrTokenMap();
    SvXMLTokenMap *_GetTextMasterPageElemTokenMap();

    // clean up backpatchers; to be called only by destructor
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void _FinitBackpatcher();

protected:
    virtual SvXMLImportContext *CreateTableChildContext(
                SvXMLImport& rImport,
                sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // access, lazy initialization and destruction of backpatchers
    // Code is implemented in XMLPropertyBackpatcher.cxx
#if     !(defined(MACOSX) && (__GNUC__ < 3))
    inline XMLPropertyBackpatcher<sal_Int16>& GetFootnoteBP();
    inline XMLPropertyBackpatcher<sal_Int16>& GetSequenceIdBP();
    inline XMLPropertyBackpatcher< ::rtl::OUString> & GetSequenceNameBP();
#else
    XMLPropertyBackpatcher<sal_Int16>& GetFootnoteBP();
    XMLPropertyBackpatcher<sal_Int16>& GetSequenceIdBP();
    XMLPropertyBackpatcher< ::rtl::OUString> & GetSequenceNameBP();
#endif

public:

    const ::rtl::OUString sParaStyleName;
    const ::rtl::OUString sCharStyleName;
    const ::rtl::OUString sHeadingStyleName;
    const ::rtl::OUString sNumberingLevel;
    const ::rtl::OUString sNumberingStartValue;
    const ::rtl::OUString sNumberingRules;
    const ::rtl::OUString sParaIsNumberingRestart;
    const ::rtl::OUString sNumberingIsNumber;
    const ::rtl::OUString sCurrentPresentation;
    const ::rtl::OUString sSequenceNumber;
    const ::rtl::OUString sSourceName;
    const ::rtl::OUString sChainNextName;
    const ::rtl::OUString sChainPrevName;
    const ::rtl::OUString sHyperLinkURL;
    const ::rtl::OUString sHyperLinkName;
    const ::rtl::OUString sHyperLinkTarget;
    const ::rtl::OUString sUnvisitedCharStyleName;
    const ::rtl::OUString sVisitedCharStyleName;
    const ::rtl::OUString sTextFrame;
    const ::rtl::OUString sPageDescName;
    const ::rtl::OUString sServerMap;
    const ::rtl::OUString sHyperLinkEvents;
    const ::rtl::OUString sContent;
    const ::rtl::OUString sServiceCombinedCharacters;

    XMLTextImportHelper(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::frame::XModel>& rModel,
            SvXMLImport& rImport,
            sal_Bool bInsertM = sal_False, sal_Bool bStylesOnlyM = sal_False,
            sal_Bool bProgress = sal_False,
            sal_Bool bBlockMode = sal_False,
            sal_Bool bOrganizerMode = sal_False );

    ~XMLTextImportHelper();

    void SetCursor(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::text::XTextCursor >& rCursor );
    void ResetCursor();

    void SetAutoStyles( SvXMLStylesContext *pStyles );

    void SetFontDecls( XMLFontStylesContext *pFontDecls );
    const XMLFontStylesContext *GetFontDecls() const;

    SvXMLImportContext *CreateTextChildContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            XMLTextType eType = XML_TEXT_TYPE_SHAPE );

    inline const SvXMLTokenMap& GetTextElemTokenMap();
    inline const SvXMLTokenMap& GetTextPElemTokenMap();
    inline const SvXMLTokenMap& GetTextPAttrTokenMap();
    inline const SvXMLTokenMap& GetTextFrameAttrTokenMap();
    inline const SvXMLTokenMap& GetTextContourAttrTokenMap();
    inline const SvXMLTokenMap& GetTextHyperlinkAttrTokenMap();
    inline const SvXMLTokenMap& GetTextMasterPageElemTokenMap();

    const SvXMLTokenMap& GetTextListBlockAttrTokenMap();
    const SvXMLTokenMap& GetTextListBlockElemTokenMap();
    const SvXMLTokenMap& GetTextFieldAttrTokenMap(); // impl: txtfldi.cxx

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XText > & GetText() { return xText; }
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > & GetCursor() { return xCursor; }
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextRange > & GetCursorAsRange()
    {
        return xCursorAsRange;
    }

    sal_Bool IsInsertMode() { return bInsertMode; }
    sal_Bool IsStylesOnlyMode() { return bStylesOnlyMode; }
    sal_Bool IsBlockMode() { return bBlockMode; }
    sal_Bool IsOrganizerMode() { return bOrganizerMode; }
    sal_Bool IsProgress() { return bProgress; }

    XMLTextListBlockContext *GetListBlock();
    SvXMLImportContext *_GetListBlock() { return &xListBlock; }
    sal_Bool IsInList() const { return xListBlock.Is(); }
    void SetListBlock( XMLTextListBlockContext *pListBlock );
    void _SetListBlock( SvXMLImportContext *pListBlock );

    XMLTextListItemContext *GetListItem();
    SvXMLImportContext *_GetListItem() { return &xListItem; }
    sal_Bool HasListItem() const { return xListItem.Is(); }
    void SetListItem( XMLTextListItemContext *pListItem );
    void _SetListItem( SvXMLImportContext *pListItem );

    XMLSectionList_Impl& GetSectionList() { return aSectionList; }

#ifdef CONV_STAR_FONTS
    ::rtl::OUString ConvertStarFonts( const ::rtl::OUString& rChars,
                                      const ::rtl::OUString& rStyleName,
                                      sal_uInt8& rFlags,
                                      sal_Bool bPara,
                                         SvXMLImport& rImport ) const;
#endif
    // insert a string without special whitespace processing enabled
    void InsertString( const ::rtl::OUString& rChars );
    // insert a string with special whitespace processing enabled
    void InsertString( const ::rtl::OUString& rChars,
                       sal_Bool& rIgnoreLeadingSpace );
    // Delete current paragraph
    void DeleteParagraph();

    void InsertControlCharacter( sal_Int16 nControl );
    void InsertTextContent( ::com::sun::star::uno::Reference <
                            ::com::sun::star::text::XTextContent > & xContent);

    ::rtl::OUString SetStyleAndAttrs(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::text::XTextCursor >& rCursor,
            const ::rtl::OUString& rStyleName,
            sal_Bool bPara );

    /** Find a suitable name for the given outline level.
     *  If rStyleName is empty, change it to a previously used or default style
     *  name for that level. Otherwise, leave rStyleName unmodified.
     */
    void FindOutlineStyleName( ::rtl::OUString& rStyleName, sal_Int8 nLevel );

    void SetOutlineStyle( sal_Int8 nLevel, const ::rtl::OUString& rStyleName );
    void SetOutlineStyles();
    void SetHyperlink(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::text::XTextCursor >& rCursor,
            const ::rtl::OUString& rHRef,
            const ::rtl::OUString& rName,
            const ::rtl::OUString& rTargetFrameName,
            const ::rtl::OUString& rStyleName,
            const ::rtl::OUString& rVisitedStyleName,
            XMLEventsImportContext* pEvents = NULL);
    void SetRuby(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::text::XTextCursor >& rCursor,
            const ::rtl::OUString& rStyleName,
            const ::rtl::OUString& rTextStyleName,
            const ::rtl::OUString& rText );

    /// return key appropriate for use with NumberFormat property
    /// return -1 if style is not found
    /// (may return whether data style uses the system language)
    sal_Int32 GetDataStyleKey( const ::rtl::OUString& sStyleName,
                               sal_Bool* pIsSystemLanguage = NULL );

    const SvxXMLListStyleContext *FindAutoListStyle(
            const ::rtl::OUString& rName ) const;
    XMLPropStyleContext *FindAutoFrameStyle(
            const ::rtl::OUString& rName ) const;
    XMLPropStyleContext* FindSectionStyle(
            const ::rtl::OUString& rName ) const;
    XMLPropStyleContext* FindPageMaster(
            const ::rtl::OUString& rName ) const;

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer>& GetParaStyles() const
    {
           return xParaStyles;
    }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer>& GetTextStyles() const
    {
           return xTextStyles;
    }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer>& GetNumberingStyles() const
    {
           return xNumStyles;
    }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer>& GetFrameStyles() const
    {
           return xFrameStyles;
    }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XNameContainer>& GetPageStyles() const
    {
           return xPageStyles;
    }

    const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexReplace >& GetChapterNumbering() const
    {
        return xChapterNumbering;
    }

    sal_Bool HasFrameByName( const ::rtl::OUString& rName ) const;
    void ConnectFrameChains( const ::rtl::OUString& rFrmName,
        const ::rtl::OUString& rNextFrmName,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet >& rFrmPropSet );

    const UniReference < SvXMLImportPropertyMapper >&
            GetParaImportPropertySetMapper() const { return xParaImpPrMap; }
    const UniReference < SvXMLImportPropertyMapper >&
            GetTextImportPropertySetMapper() const { return xTextImpPrMap; }
    const UniReference < SvXMLImportPropertyMapper >&
            GetFrameImportPropertySetMapper() const { return xFrameImpPrMap; }
    const UniReference < SvXMLImportPropertyMapper >&
        GetSectionImportPropertySetMapper() const { return xSectionImpPrMap; }
    const UniReference < SvXMLImportPropertyMapper >&
        GetRubyImportPropertySetMapper() const { return xRubyImpPrMap; }
    static SvXMLImportPropertyMapper *CreateShapeExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper *CreateCharExtPropMapper(SvXMLImport&, XMLFontStylesContext *pFontDecls = NULL);
    static SvXMLImportPropertyMapper *CreateParaExtPropMapper(SvXMLImport&, XMLFontStylesContext *pFontDecls = NULL);
    static sal_Bool HasDrawNameAttribute(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        SvXMLNamespaceMap& rNamespaceMap );

    SvI18NMap& GetRenameMap();

    /// save the start of a range reference
    void InsertBookmarkStartRange(
        const ::rtl::OUString sName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange> & rRange);

    /// process the start of a range reference
    sal_Bool FindAndRemoveBookmarkStartRange(
        ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange> & rRange,
        const ::rtl::OUString sName);

    /// insert new footnote ID.
    /// Also fixup open references from the backpatch list to this ID.
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void InsertFootnoteID(const ::rtl::OUString& sXMLId, sal_Int16 nAPIId);

    /// set the proper footnote reference ID,
    /// or put into backpatch list if ID is unknown
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void ProcessFootnoteReference(
        const ::rtl::OUString& sXMLId,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & xPropSet);

    /// insert new sequence ID
    /// Also fixup open references from backpatch list to this ID.
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void InsertSequenceID(const ::rtl::OUString& sXMLId,
                          const ::rtl::OUString& sName,
                          sal_Int16 nAPIId);

    /// set sequence ID or insert into backpatch list
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void ProcessSequenceReference(
        const ::rtl::OUString& sXMLId,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & xPropSet);

    sal_Bool IsInFrame() const;
    virtual sal_Bool IsInHeaderFooter() const;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
            createAndInsertOLEObject( SvXMLImport& rImport,
                                      const ::rtl::OUString& rHRef,
                                      const ::rtl::OUString &rStyleName,
                                      const ::rtl::OUString &rTblName,
                                         sal_Int32 nWidth, sal_Int32 nHeight );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertApplet(
            const ::rtl::OUString &rName,
            const ::rtl::OUString &rCode,
            sal_Bool bMayScript,
            const ::rtl::OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertPlugin(
            const ::rtl::OUString &rMimeType,
            const ::rtl::OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertFloatingFrame(
            const ::rtl::OUString &rName,
            const ::rtl::OUString &rHRef,
            const ::rtl::OUString &rStyleName,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual void endAppletOrPlugin(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> &rPropSet, 
        ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess > &rParamMap );

    // applet helper methods
    // implemented in sw/filter/xml/txtparai.hxx

    // redlining helper methods
    // (to be implemented in sw/filter/xml/txtparai.hxx)

    virtual void RedlineAdd( 
        const ::rtl::OUString& rType,		/// redline type (insert, del,... )
        const ::rtl::OUString& rId,			/// use to identify this redline
        const ::rtl::OUString& rAuthor,		/// name of the author
        const ::rtl::OUString& rComment,	/// redline comment
        const ::com::sun::star::util::DateTime& rDateTime,	/// date+time
        sal_Bool bMergeLastParagraph);      /// merge last paras
    virtual ::com::sun::star::uno::Reference< 
        ::com::sun::star::text::XTextCursor> RedlineCreateText(
            ::com::sun::star::uno::Reference< 	/// needed to get the document
                    ::com::sun::star::text::XTextCursor> & rOldCursor,
            const ::rtl::OUString& rId);	/// ID used to RedlineAdd() call
    virtual void RedlineSetCursor(
        const ::rtl::OUString& rId,			/// ID used to RedlineAdd() call
        sal_Bool bStart,					/// start or end Cursor
        sal_Bool bIsOutsideOfParagraph);	/// range is not within <text:p>
    virtual void RedlineAdjustStartNodeCursor(
        sal_Bool bStart);
    virtual void SetShowChanges( sal_Bool bShowChanges );
    virtual void SetRecordChanges( sal_Bool bRecordChanges );
    virtual void SetChangesProtectionKey( 
        const ::com::sun::star::uno::Sequence<sal_Int8> & rProtectionKey );

    // access to the last open redline ID:
    ::rtl::OUString GetOpenRedlineId();
    void SetOpenRedlineId( ::rtl::OUString& rId);
    void ResetOpenRedlineId();

    // #107848#
    // Access methods to the inside_deleted_section flag (redlining)
    void SetInsideDeleteContext(sal_Bool bNew) { bInsideDeleteContext = bNew; }
    sal_Bool IsInsideDeleteContext() const { return bInsideDeleteContext; }
};

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextElemTokenMap()
{
    if( !pTextElemTokenMap )
        pTextElemTokenMap = _GetTextElemTokenMap();

    return *pTextElemTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextPElemTokenMap()
{
    if( !pTextPElemTokenMap )
        pTextPElemTokenMap = _GetTextPElemTokenMap();

    return *pTextPElemTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextPAttrTokenMap()
{
    if( !pTextPAttrTokenMap )
        pTextPAttrTokenMap = _GetTextPAttrTokenMap();

    return *pTextPAttrTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextFrameAttrTokenMap()
{
    if( !pTextFrameAttrTokenMap )
        pTextFrameAttrTokenMap = _GetTextFrameAttrTokenMap();

    return *pTextFrameAttrTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextContourAttrTokenMap()
{
    if( !pTextContourAttrTokenMap )
        pTextContourAttrTokenMap = _GetTextContourAttrTokenMap();

    return *pTextContourAttrTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextHyperlinkAttrTokenMap()
{
    if( !pTextHyperlinkAttrTokenMap )
        pTextHyperlinkAttrTokenMap = _GetTextHyperlinkAttrTokenMap();

    return *pTextHyperlinkAttrTokenMap;
}

inline const SvXMLTokenMap& XMLTextImportHelper::GetTextMasterPageElemTokenMap()
{
    if( !pTextMasterPageElemTokenMap )
        pTextMasterPageElemTokenMap = _GetTextMasterPageElemTokenMap();

    return *pTextMasterPageElemTokenMap;
}

}//end of namespace binfilter
#endif
