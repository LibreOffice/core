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
#ifndef INCLUDED_XMLOFF_TXTIMP_HXX
#define INCLUDED_XMLOFF_TXTIMP_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>

#include <map>
#include <memory>

#include <xmloff/xmltkmap.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

class XMLTextListsHelper;
class SvXMLImportContext;
class SvXMLTokenMap;
class SvXMLImportPropertyMapper;
class SvXMLNamespaceMap;
class SvXMLImport;
class SvXMLStylesContext;
class XMLTextListBlockContext;
class SvxXMLListStyleContext;
class XMLPropStyleContext;
class SvI18NMap;
class XMLSectionImportContext;
class XMLFontStylesContext;
template<class A> class XMLPropertyBackpatcher;
class XMLEventsImportContext;

namespace xmloff {
    struct ParsedRDFaAttributes;
}

namespace com { namespace sun { namespace star {
namespace text {
    class XText;
    class XTextCursor;
    class XTextRange;
    class XTextContent;
    class XFormField;
}
namespace frame { class XModel; }
namespace container { class XNameContainer; class XIndexReplace; class XNameAccess; }
namespace beans { class XPropertySet; }
namespace xml { namespace sax { class XAttributeList; } }
namespace util { struct DateTime; }
namespace lang { class XMultiServiceFactory; }
} } }

enum SwXMLTextElemTokens
{
    XML_TOK_TEXT_P,
    XML_TOK_TEXT_H,
    XML_TOK_TEXT_LIST,
    XML_TOK_TEXT_FRAME_PAGE,
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
    XML_TOK_TEXT_NUMBERED_PARAGRAPH
};

enum XMLTextPElemTokens
{
    XML_TOK_TEXT_SPAN,
    XML_TOK_TEXT_TAB_STOP,
    XML_TOK_TEXT_LINE_BREAK,
    XML_TOK_TEXT_SOFT_PAGE_BREAK,
    XML_TOK_TEXT_S,
    XML_TOK_TEXT_FRAME,
    XML_TOK_TEXT_HYPERLINK,
    XML_TOK_TEXT_RUBY,

    XML_TOK_TEXT_NOTE,
    XML_TOK_TEXT_BOOKMARK,
    XML_TOK_TEXT_BOOKMARK_START,
    XML_TOK_TEXT_BOOKMARK_END,
    XML_TOK_TEXT_REFERENCE,
    XML_TOK_TEXT_REFERENCE_START,
    XML_TOK_TEXT_REFERENCE_END,

    XML_TOK_TEXT_TOC_MARK,          // index marks
    XML_TOK_TEXT_TOC_MARK_START,
    XML_TOK_TEXT_TOC_MARK_END,
    XML_TOK_TEXT_USER_INDEX_MARK,
    XML_TOK_TEXT_USER_INDEX_MARK_START,
    XML_TOK_TEXT_USER_INDEX_MARK_END,
    XML_TOK_TEXT_ALPHA_INDEX_MARK,
    XML_TOK_TEXT_ALPHA_INDEX_MARK_START,
    XML_TOK_TEXT_ALPHA_INDEX_MARK_END,

    XML_TOK_TEXT_SENDER_FIRSTNAME,  // sender fields
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

    XML_TOK_TEXT_AUTHOR_NAME,   // author fields
    XML_TOK_TEXT_AUTHOR_INITIALS,

    XML_TOK_TEXT_DATE,          // date & time fields
    XML_TOK_TEXT_TIME,

    XML_TOK_TEXT_PAGE_NUMBER,   // page number fields
    XML_TOK_TEXT_PAGE_CONTINUATION_STRING,

    XML_TOK_TEXT_VARIABLE_SET,  // variable fields
    XML_TOK_TEXT_VARIABLE_GET,
    XML_TOK_TEXT_VARIABLE_INPUT,
    XML_TOK_TEXT_USER_FIELD_GET,
    XML_TOK_TEXT_USER_FIELD_INPUT,
    XML_TOK_TEXT_SEQUENCE,
    XML_TOK_TEXT_EXPRESSION,
    XML_TOK_TEXT_TEXT_INPUT,

    XML_TOK_TEXT_DATABASE_DISPLAY,  // database fields
    XML_TOK_TEXT_DATABASE_NEXT,
    XML_TOK_TEXT_DATABASE_SELECT,
    XML_TOK_TEXT_DATABASE_ROW_NUMBER,
    XML_TOK_TEXT_DATABASE_NAME,

    XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR, //document info fields
    XML_TOK_TEXT_DOCUMENT_CREATION_DATE,
    XML_TOK_TEXT_DOCUMENT_CREATION_TIME,
    XML_TOK_TEXT_DOCUMENT_DESCRIPTION,
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

    XML_TOK_TEXT_PLACEHOLDER,   // misc. fields
    XML_TOK_TEXT_CONDITIONAL_TEXT,
    XML_TOK_TEXT_HIDDEN_TEXT,
    XML_TOK_TEXT_HIDDEN_PARAGRAPH,
    XML_TOK_TEXT_FILENAME,
    XML_TOK_TEXT_CHAPTER,
    XML_TOK_TEXT_TEMPLATENAME,
    XML_TOK_TEXT_WORD_COUNT,    // statistics/count fields
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
    XML_TOK_TEXT_NOTE_REF,
    XML_TOK_TEXT_SHEET_NAME,
    XML_TOK_TEXT_PAGE_NAME,
    XML_TOK_TEXT_BIBLIOGRAPHY_MARK,
    XML_TOK_TEXT_ANNOTATION,
    XML_TOK_TEXT_ANNOTATION_END,
    XML_TOK_TEXT_NAME,
    XML_TOK_TEXT_SCRIPT,
    XML_TOK_TEXT_TABLE_FORMULA,
    XML_TOK_TEXT_DROP_DOWN,

    // RDF metadata
    XML_TOK_TEXT_META,
    XML_TOK_TEXT_META_FIELD,

    // misc
    XML_TOK_TEXTP_CHANGE_START, // TEXTP avoids clash with SwXMLTextElemTokens
    XML_TOK_TEXTP_CHANGE_END,
    XML_TOK_TEXTP_CHANGE,
    XML_TOK_DRAW_A,
    XML_TOK_TEXT_MEASURE,

    XML_TOK_DRAW_HEADER,
    XML_TOK_DRAW_FOOTER,
    XML_TOK_DRAW_DATE_TIME,
    XML_TOK_TEXT_PAGE_CONTINUATION,

    XML_TOK_TEXT_FIELDMARK,
    XML_TOK_TEXT_FIELDMARK_START,
    XML_TOK_TEXT_FIELDMARK_END
};

enum XMLTextPAttrTokens
{
    XML_TOK_TEXT_P_XMLID,
    XML_TOK_TEXT_P_ABOUT,
    XML_TOK_TEXT_P_PROPERTY,
    XML_TOK_TEXT_P_CONTENT,
    XML_TOK_TEXT_P_DATATYPE,
    XML_TOK_TEXT_P_TEXTID,
    XML_TOK_TEXT_P_STYLE_NAME,
    XML_TOK_TEXT_P_COND_STYLE_NAME,
    XML_TOK_TEXT_P_LEVEL,
    XML_TOK_TEXT_P_CLASS_NAMES,
    XML_TOK_TEXT_P_IS_LIST_HEADER,
    XML_TOK_TEXT_P_RESTART_NUMBERING,
    XML_TOK_TEXT_P_START_VALUE
};

enum XMLTextNumberedParagraphAttrTokens
{
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_XMLID,
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_LIST_ID,
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_LEVEL,
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_STYLE_NAME,
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_CONTINUE_NUMBERING,
    XML_TOK_TEXT_NUMBERED_PARAGRAPH_START_VALUE
};

enum XMLTextListBlockAttrTokens
{
    XML_TOK_TEXT_LIST_BLOCK_XMLID,
    XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME,
    XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING,
    XML_TOK_TEXT_LIST_BLOCK_CONTINUE_LIST
};

enum XMLTextListBlockElemTokens
{
    XML_TOK_TEXT_LIST_HEADER,
    XML_TOK_TEXT_LIST_ITEM
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
    XML_TOK_TEXT_FRAME_MIN_WIDTH
};

enum XMLTextHyperlinkAttrTokens
{
    XML_TOK_TEXT_HYPERLINK_HREF,
    XML_TOK_TEXT_HYPERLINK_NAME,
    XML_TOK_TEXT_HYPERLINK_TARGET_FRAME,
    XML_TOK_TEXT_HYPERLINK_SHOW,
    XML_TOK_TEXT_HYPERLINK_STYLE_NAME,
    XML_TOK_TEXT_HYPERLINK_VIS_STYLE_NAME,
    XML_TOK_TEXT_HYPERLINK_SERVER_MAP
};

enum XMLTextPageMasterElemTokens
{
    XML_TOK_TEXT_MP_HEADER,
    XML_TOK_TEXT_MP_FOOTER,
    XML_TOK_TEXT_MP_HEADER_LEFT,
    XML_TOK_TEXT_MP_FOOTER_LEFT,
    XML_TOK_TEXT_MP_HEADER_FIRST,
    XML_TOK_TEXT_MP_FOOTER_FIRST
};

enum XMLTextContourAttrTokens
{
    XML_TOK_TEXT_CONTOUR_WIDTH,
    XML_TOK_TEXT_CONTOUR_HEIGHT,
    XML_TOK_TEXT_CONTOUR_VIEWBOX,
    XML_TOK_TEXT_CONTOUR_POINTS,
    XML_TOK_TEXT_CONTOUR_D,
    XML_TOK_TEXT_CONTOUR_AUTO
};
enum XMLTextType
{
    Body,
    Cell,
    Shape,
    TextBox,
    HeaderFooter,
    Section,
    Footnote,
    ChangedRegion
};

/// variable type (for XMLSetVarFieldImportContext)
enum VarType
{
    VarTypeSimple,
    VarTypeUserField,
    VarTypeSequence
    // see following NOTE!
};

// NOTE: these values must be distinct from any in "enum VarType" because they
// are used as keys to SvI18NMap::Add()
#define XML_TEXT_RENAME_TYPE_FRAME 10
#define XML_TEXT_RENAME_TYPE_TABLE 20

class XMLOFF_DLLPUBLIC XMLTextImportHelper : public salhelper::SimpleReferenceObject
{
private:
    XMLTextImportHelper(const XMLTextImportHelper&) = delete;
    XMLTextImportHelper& operator=(const XMLTextImportHelper&) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_xImpl;
    /// ugly, but implementation of this is in XMLPropertyBackpatcher.cxx
    struct BackpatcherImpl;
    std::shared_ptr<BackpatcherImpl> m_xBackpatcherImpl;
    static std::shared_ptr<BackpatcherImpl> MakeBackpatcherImpl();

protected:
    virtual SvXMLImportContext *CreateTableChildContext(
                SvXMLImport& rImport,
                sal_uInt16 nPrefix, const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );

    // access, lazy initialization and destruction of backpatchers
    // Code is implemented in XMLPropertyBackpatcher.cxx
    inline XMLPropertyBackpatcher<sal_Int16>& GetFootnoteBP();
    inline XMLPropertyBackpatcher<sal_Int16>& GetSequenceIdBP();
    inline XMLPropertyBackpatcher< OUString> & GetSequenceNameBP();

public:

    XMLTextImportHelper(
            css::uno::Reference< css::frame::XModel > const& rModel,
            SvXMLImport& rImport,
            bool const bInsertMode = false, bool const bStylesOnlyMode = false,
            bool const bProgress = false, bool const bBlockMode = false,
            bool const bOrganizerMode = false);

    virtual ~XMLTextImportHelper() override;

    void SetCursor(
            const css::uno::Reference< css::text::XTextCursor >& rCursor );
    void ResetCursor();

    void SetAutoStyles( SvXMLStylesContext *pStyles );

    SvXMLImportContext *CreateTextChildContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            XMLTextType eType = XMLTextType::Shape );

    SvXMLTokenMap const& GetTextElemTokenMap();
    SvXMLTokenMap const& GetTextPElemTokenMap();
    SvXMLTokenMap const& GetTextPAttrTokenMap();
    SvXMLTokenMap const& GetTextFrameAttrTokenMap();
    SvXMLTokenMap const& GetTextContourAttrTokenMap();
    SvXMLTokenMap const& GetTextHyperlinkAttrTokenMap();
    SvXMLTokenMap const& GetTextMasterPageElemTokenMap();

    const SvXMLTokenMap& GetTextNumberedParagraphAttrTokenMap();
    const SvXMLTokenMap& GetTextListBlockAttrTokenMap();
    const SvXMLTokenMap& GetTextListBlockElemTokenMap();
    const SvXMLTokenMap& GetTextFieldAttrTokenMap(); // impl: txtfldi.cxx

    css::uno::Reference< css::text::XText > & GetText();
    css::uno::Reference< css::text::XTextCursor > & GetCursor();
    css::uno::Reference< css::text::XTextRange > & GetCursorAsRange();

    bool IsInsertMode() const;
    bool IsStylesOnlyMode() const;
    bool IsBlockMode() const;
    bool IsOrganizerMode() const;
    bool IsProgress() const;

    OUString ConvertStarFonts( const OUString& rChars,
                               const OUString& rStyleName,
                                     sal_uInt8& rFlags,
                                     bool bPara,
                                     SvXMLImport& rImport ) const;
    // insert a string without special whitespace processing enabled
    void InsertString( const OUString& rChars );
    // insert a string with special whitespace processing enabled
    void InsertString( const OUString& rChars,
                       bool& rIgnoreLeadingSpace );
    // Delete current paragraph
    void DeleteParagraph();

    void InsertControlCharacter( sal_Int16 nControl );
    void InsertTextContent( css::uno::Reference< css::text::XTextContent > & xContent);

    // Add parameter <bOutlineLevelAttrFound> (#i73509#)
    // Add parameter <bSetListAttrs> in order to suppress the handling of the list attributes (#i80724#)
    OUString SetStyleAndAttrs(
            SvXMLImport& rImport,
            const css::uno::Reference< css::text::XTextCursor >& rCursor,
            const OUString& rStyleName,
            bool bPara,
            bool bOutlineLevelAttrFound = false,
            sal_Int8 nOutlineLevel = -1,
            bool bSetListAttrs = true );

    /** Find a suitable name for the given outline level.
     *  If rStyleName is empty, change it to a previously used or default style
     *  name for that level. Otherwise, leave rStyleName unmodified.
     */
    // Adjust 2nd parameter (#i69629#)
    void FindOutlineStyleName( OUString& rStyleName,
                               sal_Int8 nOutlineLevel );

    // Change method name to reflect change of data structure (#i69629#)
    void AddOutlineStyleCandidate( const sal_Int8 nOutlineLevel,
                                   const OUString& rStyleName );

    void SetOutlineStyles( bool bSetEmpty );

    void SetHyperlink(
            SvXMLImport& rImport,
            const css::uno::Reference< css::text::XTextCursor >& rCursor,
            const OUString& rHRef,
            const OUString& rName,
            const OUString& rTargetFrameName,
            const OUString& rStyleName,
            const OUString& rVisitedStyleName,
            XMLEventsImportContext* pEvents);
    void SetRuby(
            SvXMLImport& rImport,
            const css::uno::Reference< css::text::XTextCursor >& rCursor,
            const OUString& rStyleName,
            const OUString& rTextStyleName,
            const OUString& rText );

    /// return key appropriate for use with NumberFormat property
    /// return -1 if style is not found
    /// (may return whether data style uses the system language)
    sal_Int32 GetDataStyleKey( const OUString& sStyleName,
                               bool* pIsSystemLanguage = nullptr );

    const SvxXMLListStyleContext *FindAutoListStyle(
            const OUString& rName ) const;
    XMLPropStyleContext *FindAutoFrameStyle(
            const OUString& rName ) const;
    XMLPropStyleContext* FindSectionStyle(
            const OUString& rName ) const;
    XMLPropStyleContext* FindPageMaster(
            const OUString& rName ) const;

    const css::uno::Reference< css::container::XNameContainer> & GetParaStyles() const;

    const css::uno::Reference< css::container::XNameContainer> & GetTextStyles() const;

    const css::uno::Reference< css::container::XNameContainer> &
        GetNumberingStyles() const;

    const css::uno::Reference< css::container::XNameContainer> & GetFrameStyles() const;

    const css::uno::Reference< css::container::XNameContainer> & GetPageStyles() const;

    const css::uno::Reference< css::container::XNameContainer> & GetCellStyles() const;

    const css::uno::Reference< css::container::XIndexReplace > &
        GetChapterNumbering() const;

    bool HasFrameByName( const OUString& rName ) const;
    void ConnectFrameChains( const OUString& rFrmName,
        const OUString& rNextFrmName,
        const css::uno::Reference< css::beans::XPropertySet >& rFrmPropSet );

    rtl::Reference< SvXMLImportPropertyMapper > const&
        GetParaImportPropertySetMapper() const;
    rtl::Reference< SvXMLImportPropertyMapper > const&
        GetTextImportPropertySetMapper() const;
    rtl::Reference< SvXMLImportPropertyMapper > const&
        GetSectionImportPropertySetMapper() const;
    rtl::Reference< SvXMLImportPropertyMapper > const&
        GetRubyImportPropertySetMapper() const;

    static SvXMLImportPropertyMapper *CreateShapeExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper *CreateParaExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper* CreateParaDefaultExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper* CreateTableDefaultExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper* CreateTableRowDefaultExtPropMapper(SvXMLImport&);
    static SvXMLImportPropertyMapper* CreateTableCellExtPropMapper(SvXMLImport&);

    SvI18NMap& GetRenameMap();

    /// save the start of a range reference
    void InsertBookmarkStartRange(
        const OUString & sName,
        const css::uno::Reference< css::text::XTextRange> & rRange,
        OUString const& i_rXmlId,
        std::shared_ptr< ::xmloff::ParsedRDFaAttributes > &
            i_rpRDFaAttributes);

    /// process the start of a range reference
    bool FindAndRemoveBookmarkStartRange(
        const OUString & sName,
        css::uno::Reference< css::text::XTextRange> & o_rRange,
        OUString & o_rXmlId,
        std::shared_ptr< ::xmloff::ParsedRDFaAttributes > & o_rpRDFaAttributes);

    OUString FindActiveBookmarkName();

    void pushFieldCtx( const OUString& name, const OUString& type );
    void popFieldCtx();
    void addFieldParam( const OUString& name, const OUString& value );
    void setCurrentFieldParamsTo(css::uno::Reference< css::text::XFormField> &xFormField);
    OUString getCurrentFieldType();
    bool hasCurrentFieldCtx();


    /// insert new footnote ID.
    /// Also fixup open references from the backpatch list to this ID.
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void InsertFootnoteID(const OUString& sXMLId, sal_Int16 nAPIId);

    /// set the proper footnote reference ID,
    /// or put into backpatch list if ID is unknown
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void ProcessFootnoteReference(
        const OUString& sXMLId,
        const css::uno::Reference< css::beans::XPropertySet> & xPropSet);

    /// insert new sequence ID
    /// Also fixup open references from backpatch list to this ID.
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void InsertSequenceID(const OUString& sXMLId,
                          const OUString& sName,
                          sal_Int16 nAPIId);

    /// set sequence ID or insert into backpatch list
    // Code is implemented in XMLPropertyBackpatcher.cxx
    void ProcessSequenceReference(
        const OUString& sXMLId,
        const css::uno::Reference< css::beans::XPropertySet> & xPropSet);

    bool IsInFrame() const;
    virtual bool IsInHeaderFooter() const;

    virtual css::uno::Reference< css::beans::XPropertySet>
            createAndInsertOLEObject( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString &rStyleName,
                                      const OUString &rTblName,
                                         sal_Int32 nWidth, sal_Int32 nHeight );

    virtual css::uno::Reference< css::beans::XPropertySet>
            createAndInsertOOoLink( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString &rStyleName,
                                      const OUString &rTblName,
                                         sal_Int32 nWidth, sal_Int32 nHeight );

    virtual css::uno::Reference< css::beans::XPropertySet>
        createAndInsertApplet(
            const OUString &rName,
            const OUString &rCode,
            bool bMayScript,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual css::uno::Reference< css::beans::XPropertySet>
        createAndInsertPlugin(
            const OUString &rMimeType,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual css::uno::Reference< css::beans::XPropertySet>
        createAndInsertFloatingFrame(
            const OUString &rName,
            const OUString &rHRef,
            const OUString &rStyleName,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual void endAppletOrPlugin(
        const css::uno::Reference< css::beans::XPropertySet> &rPropSet,
        ::std::map < const OUString, OUString > &rParamMap );

    // applet helper methods
    // implemented in sw/filter/xml/txtparai.hxx

    // redlining helper methods
    // (to be implemented in sw/filter/xml/txtparai.hxx)

    virtual void RedlineAdd(
            /// redline type (insert, del,... )
            const OUString& rType,
            /// use to identify this redline
            const OUString& rId,
            /// name of the author
            const OUString& rAuthor,
            /// redline comment
            const OUString& rComment,
            /// date+time
            const css::util::DateTime& rDateTime,
            /// merge last paras
            bool bMergeLastParagraph);

    virtual css::uno::Reference< css::text::XTextCursor> RedlineCreateText(
            /// needed to get the document
            css::uno::Reference< css::text::XTextCursor > & rOldCursor,
            /// ID used to RedlineAdd() call
            const OUString& rId);

    virtual void RedlineSetCursor(
        /// ID used to RedlineAdd() call
        const OUString& rId,
        /// start or end Cursor
        bool bStart,
        /// range is not within <text:p>
        bool bIsOutsideOfParagraph);

    virtual void RedlineAdjustStartNodeCursor( bool bStart );
    virtual void SetShowChanges( bool bShowChanges );
    virtual void SetRecordChanges( bool bRecordChanges );
    virtual void SetChangesProtectionKey(
        const css::uno::Sequence<sal_Int8> & rProtectionKey );

    /// get the last open redline ID
    OUString GetOpenRedlineId();
    /// modify the last open redline ID
    void SetOpenRedlineId( OUString const & rId);
    /// reset the last open redline ID
    void ResetOpenRedlineId();

    /** redlining : Setter to remember the fact we are inside/outside
     * a <text:deletion> element (deleted redline section) */
    void SetInsideDeleteContext(bool const bNew);
    /** redlining : Getter to know if we are inside
     * a <text:deletion> element (deleted redline section) */
    bool IsInsideDeleteContext() const;

    SvXMLImport & GetXMLImport();

    XMLTextListsHelper & GetTextListHelper();

    // forwards to TextListHelper; these are used in many places
    /// push a list context on the list context stack
    void PushListContext();
    /// pop the list context stack
    void PopListContext();

    void SetCellParaStyleDefault(OUString const& rNewValue);
    OUString const& GetCellParaStyleDefault();

    void AddCrossRefHeadingMapping(OUString const& rFrom, OUString const& rTo);
    void MapCrossRefHeadingFieldsHorribly();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
