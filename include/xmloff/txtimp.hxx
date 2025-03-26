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
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>

#include <map>
#include <memory>

#include <salhelper/simplereferenceobject.hxx>

class XMLTextListsHelper;
class SvXMLImportContext;
class SvXMLTokenMap;
class SvXMLImportPropertyMapper;
class SvXMLImport;
class SvXMLStylesContext;
class SvxXMLListStyleContext;
class XMLPropStyleContext;
class SvI18NMap;
template<class A> class XMLPropertyBackpatcher;
class XMLEventsImportContext;

namespace xmloff {
    struct ParsedRDFaAttributes;
}

namespace com::sun::star {
namespace text {
    class XText;
    class XTextCursor;
    class XTextRange;
    class XTextContent;
    class XFormField;
}
namespace frame { class XModel; }
namespace container { class XNameContainer; class XIndexReplace; }
namespace beans { class XPropertySet; }
namespace xml::sax { class XAttributeList; }
namespace xml::sax { class XFastAttributeList; }
namespace util { struct DateTime; }
}

namespace rtl { template <class reference_type> class Reference; }

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
                sal_Int32 nElement,
                const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

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

    void dispose();

    void SetCursor(
            const css::uno::Reference< css::text::XTextCursor >& rCursor );
    void ResetCursor();

    void SetAutoStyles( SvXMLStylesContext *pStyles );

    SvXMLImportContext *CreateTextChildContext(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            XMLTextType eType = XMLTextType::Shape );

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
    void InsertString( std::u16string_view rChars,
                       bool& rIgnoreLeadingSpace );
    // Delete current paragraph
    void DeleteParagraph(bool dontCorrectBookmarks = false);

    void InsertControlCharacter( sal_Int16 nControl );
    void InsertTextContent( css::uno::Reference< css::text::XTextContent > const & xContent);

    // Add parameter <bOutlineLevelAttrFound> (#i73509#)
    // Add parameter <bSetListAttrs> in order to suppress the handling of the list attributes (#i80724#)
    OUString SetStyleAndAttrs(
            SvXMLImport & rImport,
            const css::uno::Reference< css::text::XTextCursor >& rCursor,
            const OUString& rStyleName,
            bool bPara,
            bool bOutlineLevelAttrFound = false,
            sal_Int8 nOutlineLevel = -1,
            bool bSetListAttrs = true,
            bool bOutlineContentVisible = true);

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
            SvXMLImport const & rImport,
            const css::uno::Reference< css::text::XTextCursor >& rCursor,
            const OUString& rHRef,
            const OUString& rName,
            const OUString& rTargetFrameName,
            const OUString& rStyleName,
            const OUString& rVisitedStyleName,
            XMLEventsImportContext* pEvents);
    void SetRuby(
            SvXMLImport const & rImport,
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
    XMLPropStyleContext* FindAutoCharStyle(const OUString& rName) const;

    XMLPropStyleContext* FindDrawingPage(OUString const& rName) const;

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

    bool IsDuplicateFrame(const OUString& sName, sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight) const;
    void StoreLastImportedFrameName(const OUString& rName);
    void ClearLastImportedTextFrameName();

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
    static SvXMLImportPropertyMapper* CreateDrawingPageExtPropMapper(SvXMLImport&);

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

    void pushFieldCtx( const OUString& name, const OUString& type );
    css::uno::Reference<css::text::XFormField> popFieldCtx();
    void addFieldParam( const OUString& name, const OUString& value );
    void setCurrentFieldParamsTo(css::uno::Reference< css::text::XFormField> const &xFormField);
    ::std::pair<OUString, OUString> getCurrentFieldType() const;
    css::uno::Reference<css::text::XTextRange> getCurrentFieldStart() const;
    bool hasCurrentFieldSeparator() const;
    bool hasCurrentFieldCtx() const;


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
            /// move id, to find other parts (moveFrom/MoveTo)
            const OUString& rMoveId,
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

    virtual void RedlineAdjustStartNodeCursor();
    virtual void SetShowChanges( bool bShowChanges );
    virtual void SetRecordChanges( bool bRecordChanges );
    virtual void SetChangesProtectionKey(
        const css::uno::Sequence<sal_Int8> & rProtectionKey );

    /// get the last open redline ID
    OUString const & GetOpenRedlineId() const;
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
    OUString const& GetCellParaStyleDefault() const;

    void AddCrossRefHeadingMapping(OUString const& rFrom, OUString const& rTo);
    void MapCrossRefHeadingFieldsHorribly();

    void setBookmarkAttributes(OUString const& bookmark, bool hidden, OUString const& condition);
    bool getBookmarkHidden(OUString const& bookmark) const;
    const OUString& getBookmarkCondition(OUString const& bookmark) const;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
