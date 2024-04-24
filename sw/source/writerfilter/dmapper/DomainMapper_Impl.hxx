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
#pragma once

#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <queue>
#include <stack>
#include <string_view>
#include <o3tl/sorted_vector.hxx>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>

#include <ooxml/OOXMLDocument.hxx>

#include <dmapper/CommentProperties.hxx>

#include "DomainMapper.hxx"
#include "DomainMapperTableManager.hxx"
#include "DomainMapperTableHandler.hxx"
#include "PropertyMap.hxx"
#include "FontTable.hxx"
#include "NumberingManager.hxx"
#include "StyleSheetTable.hxx"
#include "SettingsTable.hxx"
#include "ThemeHandler.hxx"
#include "GraphicImport.hxx"
#include "OLEHandler.hxx"
#include "FFDataHandler.hxx"
#include "SmartTagHandler.hxx"
#include "FormControlHelper.hxx"
#include <map>

class SwXTextDocument;
class SwXDocumentSettings;
class SwXTextSection;
class SwXTextField;
class SwXTextFrame;
namespace com::sun::star{
        namespace awt{
            struct Size;
        }
        namespace lang{
            class XMultiServiceFactory;
            struct Locale;
        }
        namespace text
        {
                class XTextField;
                class XTextFrame;
                class XFormField;
        }
        namespace beans{ class XPropertySet;}
}

namespace writerfilter::ooxml {
    class OOXMLDocument;
}

namespace writerfilter::dmapper {

class SdtHelper;

struct PageMar
{
    sal_Int32 top;
    sal_Int32 right;
    sal_Int32 bottom;
    sal_Int32 left;
    sal_Int32 header;
    sal_Int32 footer;
    sal_Int32 gutter;
    public:
        PageMar();
};
enum PageMarElement
{
    PAGE_MAR_TOP,
    PAGE_MAR_RIGHT,
    PAGE_MAR_BOTTOM,
    PAGE_MAR_LEFT,
    PAGE_MAR_HEADER,
    PAGE_MAR_FOOTER,
    PAGE_MAR_GUTTER
};

struct PaperSource
{
    sal_Int32 first;
    sal_Int32 other;
    public:
        PaperSource() :
            first(0),
            other(0)
            {}
};

enum PaperSourceElement
{
    PAPER_SOURCE_FIRST,
    PAPER_SOURCE_OTHER,
};

/// property stack element
enum ContextType
{
    CONTEXT_SECTION,
    CONTEXT_PARAGRAPH,
    CONTEXT_CHARACTER,
    CONTEXT_STYLESHEET,
    CONTEXT_LIST
};
enum { NUMBER_OF_CONTEXTS = CONTEXT_LIST + 1 };

enum BreakType
{
    PAGE_BREAK,
    COLUMN_BREAK,
    LINE_BREAK
};

/**
 * Two special footnotes are a separator line, and a continuation line.
 * In MSOffice, these can contain text as well, but LO doesn't implement this
 * rarely used feature, so the separator text needs to be skipped. (tdf#123262)
 * Three-way logic is needed because there is no guaranteed on-off event.
 * OFF == not in footnote separator
 * ON == in footnote separator
 * SKIPPING == ON status has been recognized.
 */
enum class SkipFootnoteSeparator
{
    OFF,
    ON,
    SKIPPING
};

// type of stored redlines
enum StoredRedlines
{
    FRAME = 0,
    FOOTNOTE,
    ENDNOTE,
    NONE
};

struct RubyInfo
{
    OUString    sRubyText;
    OUString    sRubyStyle;
    sal_uInt32  nSprmId;
    sal_uInt32  nRubyAlign;
    sal_uInt32  nHps;
    sal_uInt32  nHpsBaseText;

    RubyInfo():
        nSprmId(0),
        nRubyAlign(0),
        nHps(0),
        nHpsBaseText(0)
    {
    }
};

enum class SubstreamType
{
    Body,
    Header,
    Footer,
    Footnote,
    Endnote,
    Annotation,
};

/**
 * Storage for state that is relevant outside a header/footer, but not inside it.
 *
 * In case some state of DomainMapper_Impl should be reset before handling the
 * header/footer and should be restored once handling of header/footer is done,
 * then you can use this class to do so.
 *
 * note: presumably more state should be moved here.
 */
struct SubstreamContext
{
    SubstreamType eSubstreamType = SubstreamType::Body;
    bool      bTextInserted = false;
    /**
     * This contains the raw table depth. nTableDepth > 0 is the same as
     * getTableManager().isInTable(), unless we're in the first paragraph of a
     * table, or first paragraph after a table, as the table manager is only
     * updated once we ended the paragraph (and know if the para has the
     * inTbl SPRM or not).
     */
    sal_Int32 nTableDepth = 0;
    // deferred breaks need to be saved for RTF, also for DOCX annotations
    bool bIsColumnBreakDeferred = false;
    bool bIsPageBreakDeferred = false;
    sal_Int32 nLineBreaksDeferred = 0;
    /// Current paragraph had at least one field in it.
    bool bParaHadField = false;
    /// Current paragraph in a table is first paragraph of a cell
    bool bFirstParagraphInCell = true;
    /// If the current paragraph has any runs.
    bool bParaChanged = false;
    bool bIsFirstParaInSectionAfterRedline = true;
    bool bIsFirstParaInSection = true;
    bool bIsLastParaInSection = false;
    /// If the current paragraph contains section property definitions.
    bool bParaSectpr = false;
    bool bIsPreviousParagraphFramed = false;
    /// Current paragraph had at least one inline object in it.
    bool bParaWithInlineObject = false;
    /// This is a continuation of already finished paragraph - e.g., first in an index section
    bool bRemoveThisParagraph = false;
    bool bIsFirstParaInShape = false;
    /// If the current section has footnotes.
    bool bHasFtn = false;
    css::uno::Reference<css::beans::XPropertySet> xPreviousParagraph;
    /// Current paragraph has automatic before spacing.
    bool bParaAutoBefore = false;
    /// Raw table cell depth.
    sal_Int32 nTableCellDepth = 0;
    /// If the next tab should be ignored, used for footnotes.
    bool bCheckFirstFootnoteTab = false;
    std::optional<sal_Int16> oLineBreakClear;
    bool bIsInTextBox = false;
    css::uno::Reference<css::text::XTextContent> xEmbedded;
    /// If we want to set "sdt end" on the next character context.
    bool bSdtEndDeferred = false;
    /// If we want to set "paragraph sdt end" on the next paragraph context.
    bool bParaSdtEndDeferred = false;
    /// If the current paragraph is inside a structured document element.
    bool bSdt = false;
    css::uno::Reference<css::text::XTextRange> xSdtEntryStart;
    OUString sCurrentParaStyleName; ///< highly inaccurate. Overwritten by "overlapping" paragraphs like flys.
    bool bHasFootnoteStyle = false;
    bool bCheckFootnoteStyle = false;
    bool bIsInFootnoteProperties = false;
    RubyInfo aRubyInfo;
    bool bTextFrameInserted = false;
    bool bIsFirstRun = false;
    bool bIsOutsideAParagraph = true;
    std::map<sal_Int32, css::uno::Any> deferredCharacterProperties;
};

/// Information about a paragraph to be finished after a field end.
struct FieldParagraph
{
    PropertyMapPtr m_pPropertyMap;
    bool m_bRemove = false;
};

/// field stack element
class FieldContext : public virtual SvRefBase
{
    bool m_bFieldCommandCompleted;
    css::uno::Reference<css::text::XTextRange> m_xStartRange;

    // Two command string:
    // 0: Normal, inserted command line
    // 1: Deleted command line
    OUString m_sCommand[2];
    OUString m_sResult;
    OUString m_sVariableValue;
    std::optional<FieldId> m_eFieldId;
    bool m_bFieldLocked;
    // Current command line type: normal or deleted
    bool m_bCommandType;
public:
    bool m_bSetUserFieldContent = false;
    bool m_bSetCitation = false;
    bool m_bSetDateValue = false;
private:

    css::uno::Reference<css::text::XTextField> m_xTextField;
    css::uno::Reference<css::text::XFormField> m_xFormField;
    css::uno::Reference<css::beans::XPropertySet> m_xTOC;
    css::uno::Reference<css::beans::XPropertySet> m_xTC; // TOX entry
    css::uno::Reference<css::beans::XPropertySet> m_xCustomField;

    OUString m_sHyperlinkURL;
    /// A frame for the hyperlink when one exists.
    OUString m_sHyperlinkTarget;
    OUString m_sHyperlinkStyle;

    FFDataHandler::Pointer_t m_pFFDataHandler;
    FormControlHelper::Pointer_t m_pFormControlHelper;
    /// (Character) properties of the field itself.
    PropertyMapPtr m_pProperties;

    std::vector<FieldParagraph> m_aParagraphsToFinish;

public:
    explicit FieldContext(css::uno::Reference<css::text::XTextRange> xStart);
    ~FieldContext() override;

    const css::uno::Reference<css::text::XTextRange>& GetStartRange() const { return m_xStartRange; }

    void                    AppendCommand(std::u16string_view rPart);
    const OUString&  GetCommand() const {return m_sCommand[m_bCommandType]; }
    bool GetCommandIsEmpty(bool bType) const { return m_sCommand[bType].isEmpty(); }
    void SetCommandType(bool cType) { m_bCommandType = cType; }

    void SetFieldId(FieldId eFieldId ) { m_eFieldId = eFieldId; }
    std::optional<FieldId> const & GetFieldId() const { return m_eFieldId; }

    void AppendResult(std::u16string_view rResult) { m_sResult += rResult; }
    const OUString&  GetResult() const { return m_sResult; }

    void CacheVariableValue(const css::uno::Any& rAny);
    const OUString& GetVariableValue() { return m_sVariableValue; }

    void                    SetCommandCompleted() { m_bFieldCommandCompleted = true; }
    bool                    IsCommandCompleted() const { return m_bFieldCommandCompleted;    }

    void                    SetFieldLocked() { m_bFieldLocked = true; }
    bool                    IsFieldLocked() const { return m_bFieldLocked; }

    const css::uno::Reference<css::beans::XPropertySet>& GetCustomField() const { return m_xCustomField; }
    void SetCustomField(css::uno::Reference<css::beans::XPropertySet> const& xCustomField) { m_xCustomField = xCustomField; }
    const css::uno::Reference<css::text::XTextField>& GetTextField() const { return m_xTextField;}
    void SetTextField(css::uno::Reference<css::text::XTextField> const& xTextField);
    const css::uno::Reference<css::text::XFormField>& GetFormField() const { return m_xFormField;}
    void SetFormField(css::uno::Reference<css::text::XFormField> const& xFormField) { m_xFormField = xFormField;}

    void SetTOC(css::uno::Reference<css::beans::XPropertySet> const& xTOC) { m_xTOC = xTOC; }
    const css::uno::Reference<css::beans::XPropertySet>& GetTOC() const { return m_xTOC; }

    void SetTC(css::uno::Reference<css::beans::XPropertySet> const& xTC) { m_xTC = xTC; }
    const css::uno::Reference<css::beans::XPropertySet>& GetTC() const { return m_xTC; }

    void  SetHyperlinkURL( const OUString& rURL ) { m_sHyperlinkURL = rURL; }
    const OUString& GetHyperlinkURL() const { return m_sHyperlinkURL; }
    void SetHyperlinkTarget(const OUString& rTarget) { m_sHyperlinkTarget = rTarget; }
    const OUString& GetHyperlinkTarget() const { return m_sHyperlinkTarget; }
    void  SetHyperlinkStyle(const OUString& rStyle) { m_sHyperlinkStyle = rStyle; }
    const OUString& GetHyperlinkStyle() const { return m_sHyperlinkStyle; }

    void setFFDataHandler(const FFDataHandler::Pointer_t& pFFDataHandler) { m_pFFDataHandler = pFFDataHandler; }
    const FFDataHandler::Pointer_t& getFFDataHandler() const { return m_pFFDataHandler; }

    void setFormControlHelper(const FormControlHelper::Pointer_t& pFormControlHelper) { m_pFormControlHelper = pFormControlHelper; }
    const FormControlHelper::Pointer_t& getFormControlHelper() const { return m_pFormControlHelper; }
    const PropertyMapPtr& getProperties() const { return m_pProperties; }

    ::std::vector<OUString> GetCommandParts() const;

    std::vector<FieldParagraph>& GetParagraphsToFinish() { return m_aParagraphsToFinish; }
};

struct AnchoredContext
{
    css::uno::Reference<css::text::XTextContent> xTextContent;
    bool bToRemove;

    explicit AnchoredContext(css::uno::Reference<css::text::XTextContent> xContent)
        : xTextContent(std::move(xContent)), bToRemove(false)
    {
    }
};

typedef tools::SvRef<FieldContext>  FieldContextPtr;

/*-------------------------------------------------------------------------
    extended tab stop struct
  -----------------------------------------------------------------------*/
struct DeletableTabStop : public css::style::TabStop
{
    bool bDeleted;
    explicit DeletableTabStop()
        : bDeleted(false)
    {
        FillChar = ' ';    // same default as SvxXMLTabStopContext_Impl
    }
    DeletableTabStop(const css::style::TabStop& rTabStop)
        : TabStop(rTabStop),
        bDeleted(false)
    {
    }
};
/// helper to remember bookmark start position
struct BookmarkInsertPosition
{
    bool                                                             m_bIsStartOfText;
    OUString                                                         m_sBookmarkName;
    css::uno::Reference<css::text::XTextRange> m_xTextRange;
    BookmarkInsertPosition(bool bIsStartOfText, OUString  rName, css::uno::Reference<css::text::XTextRange> xTextRange):
        m_bIsStartOfText( bIsStartOfText ),
        m_sBookmarkName(std::move( rName )),
        m_xTextRange(std::move( xTextRange ))
     {}
};

struct PermInsertPosition
{
    bool        m_bIsStartOfText;
    sal_Int32   m_Id;
    OUString    m_Ed;
    OUString    m_EdGrp;

    css::uno::Reference<css::text::XTextRange> m_xTextRange;

    PermInsertPosition(bool bIsStartOfText, sal_Int32 id, OUString  ed, OUString edGrp, css::uno::Reference<css::text::XTextRange> xTextRange)
        : m_bIsStartOfText(bIsStartOfText)
        , m_Id(id)
        , m_Ed(std::move(ed))
        , m_EdGrp(std::move(edGrp))
        , m_xTextRange(std::move(xTextRange))
    {}

    OUString createBookmarkName() const
    {
        OUString bookmarkName;

        assert((!m_Ed.isEmpty()) || (!m_EdGrp.isEmpty()));

        if (m_Ed.isEmpty())
        {
            bookmarkName += "permission-for-group:" +
                OUString::number(m_Id) +
                ":" +
                m_EdGrp;
        }
        else
        {
            bookmarkName += "permission-for-user:" +
                OUString::number(m_Id) +
                ":" +
                m_Ed;
        }

        //todo: make sure the name is not used already!
        return bookmarkName;
    }
};

/// Stores the start/end positions of an annotation before its insertion.
struct AnnotationPosition
{
    css::uno::Reference<css::text::XTextRange> m_xStart;
    css::uno::Reference<css::text::XTextRange> m_xEnd;
};

struct LineNumberSettings
{
    sal_Int32   nDistance;
    sal_Int32   nInterval;
    bool        bRestartAtEachPage;
    LineNumberSettings() :
         nDistance(-1)
        ,nInterval(0)
        ,bRestartAtEachPage(true)
    {}

};

/// Stores original/in-file-format info about a single anchored object.
struct AnchoredObjectInfo
{
    css::uno::Reference<css::text::XTextContent> m_xAnchoredObject;
    sal_Int32 m_nLeftMargin = 0;
    RedlineParamsPtr m_xRedlineForInline;
};

/// Stores info about objects anchored to a given paragraph.
struct AnchoredObjectsInfo
{
    css::uno::Reference<css::text::XTextRange> m_xParagraph;
    std::vector<AnchoredObjectInfo> m_aAnchoredObjects;
};

struct TextAppendContext
{
    css::uno::Reference<css::text::XTextAppend> xTextAppend;
    css::uno::Reference<css::text::XParagraphCursor> xCursor;
    css::uno::Reference<css::text::XTextRange> xInsertPosition;
    ParagraphPropertiesPtr pLastParagraphProperties;

    /**
     * Objects anchored to the current paragraph, may affect the paragraph
     * spacing.
     */
    std::vector<AnchoredObjectInfo> m_aAnchoredObjects;

    TextAppendContext(css::uno::Reference<css::text::XTextAppend> const& i_xAppend,
                      css::uno::Reference<css::text::XTextCursor> const& i_xCursor)
        : xTextAppend(i_xAppend)
        , xCursor(i_xCursor, css::uno::UNO_QUERY)
        , xInsertPosition(xCursor)
    {}
};

struct SymbolData
{
    sal_Unicode cSymbol;
    OUString    sFont;
    SymbolData():
        cSymbol(),
        sFont()
    { }
};

class DomainMapper;
class DomainMapper_Impl final
{
public:
    typedef std::map < OUString, BookmarkInsertPosition > BookmarkMap_t;
    typedef std::map < sal_Int32, PermInsertPosition >    PermMap_t;

private:
    SourceDocumentType                                                              m_eDocumentType;
    DomainMapper&                                                                   m_rDMapper;
    writerfilter::ooxml::OOXMLDocument* m_pOOXMLDocument;
    OUString m_aBaseUrl;
    rtl::Reference<SwXTextDocument> m_xTextDocument;
    rtl::Reference<SwXDocumentSettings> m_xDocumentSettings;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::container::XNameContainer> m_xPageStyles1;
    // cache next available number, expensive to repeatedly compute
    std::optional<int> m_xNextUnusedPageStyleNo;
    css::uno::Reference<css::container::XNameContainer> m_xCharacterStyles;
    css::uno::Reference<css::container::XNameContainer> m_xParagraphStyles;

    // cache next available number, expensive to repeatedly compute
    std::optional<int> m_xNextUnusedCharacterStyleNo;
    css::uno::Reference<css::text::XText> m_xBodyText;

    std::stack<TextAppendContext>                                                   m_aTextAppendStack;
    std::stack<AnchoredContext>                                                     m_aAnchoredStack;
public: // DomainMapper needs it
    std::stack<SubstreamContext> m_StreamStateStack;
private:
    std::stack<std::pair<TextAppendContext, PagePartType>> m_aHeaderFooterTextAppendStack;

    std::deque<FieldContextPtr> m_aFieldStack;
    bool m_bForceGenericFields;
    /// Type of decimal symbol associated to the document language in Writer locale definition
    bool                                                                            m_bIsDecimalComma;
    bool                                                                            m_bIsFirstSection;
    bool                                                                            m_bStartTOC;
    bool                                                                            m_bStartTOCHeaderFooter;
    /// If we got any text that is the pre-rendered result of the TOC field.
    bool                                                                            m_bStartedTOC;
    bool                                                                            m_bStartIndex;
    bool                                                                            m_bStartBibliography;
    unsigned int                                                                    m_nStartGenericField;
    bool                                                                            m_bTextDeleted;
    LineNumberSettings                                                              m_aLineNumberSettings;

    std::vector<OUString>                                                           m_aRedlineMoveIDs;
    // Remember the last used redline MoveID. To avoid regression, because of wrong docx export
    sal_uInt32                                                                      m_nLastRedlineMovedID;

    BookmarkMap_t                                                                   m_aBookmarkMap;
    OUString                                                                        m_sCurrentBkmkId;
    OUString                                                                        m_sCurrentBkmkName;
    OUString                                                                        m_sCurrentBkmkPrefix;

    PermMap_t                                                                       m_aPermMap;
    sal_Int32                                                                       m_sCurrentPermId;
    OUString                                                                        m_sCurrentPermEd;
    OUString                                                                        m_sCurrentPermEdGrp;

    PageMar                                                                        m_aPageMargins;
    PaperSource                                                                     m_aPaperSource;
    SymbolData                                                                      m_aSymbolData;

    // TableManagers are stacked: one for each stream to avoid any confusion
    std::stack< tools::SvRef< DomainMapperTableManager > > m_aTableManagers;
    tools::SvRef<DomainMapperTableHandler> m_pTableHandler;
    // List of document lists overrides. They are applied only once on first occurrence in document
    o3tl::sorted_vector<sal_Int32> m_aListOverrideApplied;

    //each context needs a stack of currently used attributes
    std::stack<PropertyMapPtr>  m_aPropertyStacks[NUMBER_OF_CONTEXTS];
    std::stack<ContextType> m_aContextStack;
    std::queue<std::optional<sal_Int16>> m_aFrameDirectionQueue;
    bool                    m_bFrameDirectionSet;
    FontTablePtr            m_pFontTable;
    ListsManager::Pointer   m_pListTable;
    std::deque< css::uno::Reference<css::drawing::XShape> > m_aPendingShapes;
    StyleSheetTablePtr      m_pStyleSheetTable;
    SettingsTablePtr        m_pSettingsTable;
    GraphicImportPtr        m_pGraphicImport;

    std::unique_ptr<ThemeHandler> m_pThemeHandler;

    PropertyMapPtr                  m_pTopContext;
    tools::SvRef<SectionPropertyMap> m_pLastSectionContext;
    PropertyMapPtr           m_pLastCharacterContext;

    ::std::vector<DeletableTabStop> m_aCurrentTabStops;
    OUString                        m_sDefaultParaStyleName; //caches the ConvertedStyleName of the default paragraph style
    bool                            m_bInDocDefaultsImport;
    bool                            m_bInStyleSheetImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInNumberingImport; //in import of numbering (i.e. numbering.xml)
    bool                            m_bInAnyTableImport; //in import of fonts, styles, lists or lfos
    bool                            m_bDiscardHeaderFooter;
    PropertyMapPtr m_pFootnoteContext;
    /// Skip paragraphs from the <w:separator/> footnote
    SkipFootnoteSeparator           m_eSkipFootnoteState;
    /// preload footnotes and endnotes
    sal_Int32                       m_nFootnotes; // footnote count
    sal_Int32                       m_nEndnotes;  // endnote count
    // these are the real first notes, use their content in the first notes
    sal_Int32                       m_nFirstFootnoteIndex;
    sal_Int32                       m_nFirstEndnoteIndex;

    bool                            m_bLineNumberingSet;

    //registered frame properties
    std::vector<css::beans::PropertyValue> m_aFrameProperties;
    css::uno::Reference<css::text::XTextRange> m_xFrameStartRange;
    css::uno::Reference<css::text::XTextRange> m_xFrameEndRange;

    // Redline stack
    std::stack< std::vector< RedlineParamsPtr > > m_aRedlines;
    // The redline currently read, may be also stored by a context instead of m_aRedlines.
    RedlineParamsPtr                m_currentRedline;
    RedlineParamsPtr                m_previousRedline;
    RedlineParamsPtr                m_pParaMarkerRedline;
    bool                            m_bIsParaMarkerChange;
    bool                            m_bIsParaMarkerMove;
    // redline data of the terminating run, if it's a moveFrom deletion or a moveTo insertion
    RedlineParamsPtr                m_pParaMarkerRedlineMove;
    // This is for removing workaround (double ZWSPs around the anchoring point) for track
    // changed images anchored *to* character, if it's followed by a redline text run immediately.
    // (In that case, the image is part of a tracked text range, no need for the dummy
    // text ZWSPs to keep the change tracking of the image in Writer.)
    bool                            m_bRedlineImageInPreviousRun;

    bool                            m_bDummyParaAddedForTableInSection;
    bool                            m_bIsLastSectionGroup;
    bool                            m_bUsingEnhancedFields;

    css::uno::Reference< css::text::XTextCursor > m_xTOCMarkerCursor;

    ::std::set<::std::pair<PagePartType, PageType>> m_HeaderFooterSeen;

    //annotation import
    rtl::Reference< SwXTextField > m_xAnnotationField;
    sal_Int32 m_nAnnotationId;
    bool m_bAnnotationResolved = false;
    OUString m_sAnnotationParent;
    OUString m_sAnnotationImportedParaId;
    std::unordered_map< sal_Int32, AnnotationPosition > m_aAnnotationPositions;

    void SetNumberFormat(const OUString& rCommand, css::uno::Reference<css::beans::XPropertySet> const& xPropertySet, bool bDetectFormat = false);
    /// @throws css::uno::Exception
    css::uno::Reference<css::beans::XPropertySet> FindOrCreateFieldMaster(const char* pFieldMasterService, const OUString& rFieldMasterName);
    rtl::Reference<SwXDocumentSettings> const & GetDocumentSettings();

    SmartTagHandler m_aSmartTagHandler;

    css::uno::Reference<css::text::XTextRange> m_xGlossaryEntryStart;
    std::stack<BookmarkInsertPosition> m_xSdtStarts;

    std::queue< rtl::Reference< SwXTextFrame > > m_xPendingTextBoxFrames;

public:
    css::uno::Reference<css::text::XTextRange> m_xInsertTextRange;
    css::uno::Reference<css::text::XTextRange> m_xAltChunkStartingRange;
    std::deque<sal_Int32> m_aFootnoteIds;
    std::deque<sal_Int32> m_aEndnoteIds;

private:
    bool m_bIsNewDoc;
    bool m_bIsAltChunk = false;
    bool m_bIsReadGlossaries;

public:
    DomainMapper_Impl(
            DomainMapper& rDMapper,
            css::uno::Reference < css::uno::XComponentContext > xContext,
            rtl::Reference< SwXTextDocument > const& xModel,
            SourceDocumentType eDocumentType,
            utl::MediaDescriptor const & rMediaDesc);
    ~DomainMapper_Impl();

    void setDocumentReference(writerfilter::ooxml::OOXMLDocument* pDocument) { if (!m_pOOXMLDocument) m_pOOXMLDocument = pDocument; };
    writerfilter::ooxml::OOXMLDocument* getDocumentReference() const;

    SectionPropertyMap* GetLastSectionContext( )
    {
        return m_pLastSectionContext.get( );
    }

    css::uno::Reference<css::container::XNameContainer> const & GetPageStyles();
    OUString GetUnusedPageStyleName();
    css::uno::Reference<css::container::XNameContainer> const & GetCharacterStyles();
    css::uno::Reference<css::container::XNameContainer> const& GetParagraphStyles();
    OUString GetUnusedCharacterStyleName();
    css::uno::Reference<css::text::XText> const & GetBodyText();
    const rtl::Reference<SwXTextDocument>& GetTextDocument() const
    {
        return m_xTextDocument;
    }
    void SetDocumentSettingsProperty( const OUString& rPropName, const css::uno::Any& rValue );

    void CreateRedline(css::uno::Reference<css::text::XTextRange> const& xRange, const RedlineParamsPtr& pRedline);

    void CheckParaMarkerRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void CheckRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void StartParaMarkerChange( );
    void EndParaMarkerChange( );
    void StartParaMarkerMove( );
    void EndParaMarkerMove( );
    void ChainTextFrames();

    void PushTextBoxContent();
    void PopTextBoxContent();
    void AttachTextBoxContentToShape(css::uno::Reference<css::drawing::XShape> xShape);

    void RemoveDummyParaForTableInSection();
    void AddDummyParaForTableInSection();
    void RemoveLastParagraph();

    void checkIfHeaderFooterIsEmpty(PagePartType ePagePartType, PageType eType);
    void prepareHeaderFooterContent(css::uno::Reference<css::beans::XPropertySet> const& xPageStyle,
                                    PagePartType ePagePartType, PropertyIds eID,
                                    bool bAppendToHeaderAndFooterTextStack);

    void SetIsDecimalComma() { m_bIsDecimalComma = true; };
    void SetIsLastParagraphInSection( bool bIsLast );
    bool GetIsLastParagraphInSection() const { return m_StreamStateStack.top().bIsLastParaInSection; }
    void SetRubySprmId(sal_uInt32 const nSprmId) { m_StreamStateStack.top().aRubyInfo.nSprmId = nSprmId; }
    void SetRubyText( OUString const &sText, OUString const &sStyle) {
        m_StreamStateStack.top().aRubyInfo.sRubyText = sText;
        m_StreamStateStack.top().aRubyInfo.sRubyStyle = sStyle;
    }
    const RubyInfo & GetRubyInfo() const { return m_StreamStateStack.top().aRubyInfo; }
    void SetRubyInfo(const RubyInfo & rInfo) { m_StreamStateStack.top().aRubyInfo = rInfo; }

    void SetIsLastSectionGroup( bool bIsLast );
    bool GetIsLastSectionGroup() const { return m_bIsLastSectionGroup;}
    void SetIsFirstParagraphInSection( bool bIsFirst );
    void SetIsFirstParagraphInSectionAfterRedline( bool bIsFirstAfterRedline );
    bool GetIsFirstParagraphInSection( bool bAfterRedline = false ) const;
    void SetIsFirstParagraphInShape(bool bIsFirst);
    bool GetIsFirstParagraphInShape() const { return m_StreamStateStack.top().bIsFirstParaInShape; }
    void SetIsDummyParaAddedForTableInSection( bool bIsAdded );
    bool GetIsDummyParaAddedForTableInSection() const { return m_bDummyParaAddedForTableInSection;}

    /// Track if a textframe has been inserted into this section
    void SetIsTextFrameInserted( bool bIsInserted );
    bool GetIsTextFrameInserted() const { return m_StreamStateStack.top().bTextFrameInserted; }
    void SetIsTextDeleted(bool bIsTextDeleted) { m_bTextDeleted = bIsTextDeleted; }

    void SetIsPreviousParagraphFramed(bool const bIsFramed)
    { m_StreamStateStack.top().bIsPreviousParagraphFramed = bIsFramed; }
    bool GetIsPreviousParagraphFramed() const { return m_StreamStateStack.top().bIsPreviousParagraphFramed; }
    void SetParaSectpr(bool bParaSectpr);
    bool GetParaSectpr() const { return m_StreamStateStack.top().bParaSectpr; }

    void SetSymbolChar( sal_Int32 nSymbol) { m_aSymbolData.cSymbol = sal_Unicode(nSymbol); }
    void SetSymbolFont( OUString const &rName ) { m_aSymbolData.sFont = rName; }
    const SymbolData & GetSymbolData() const { return m_aSymbolData;}

    void SetSdt(bool bSdt);

    void PushSdt();
    void PopSdt();
    /// Gives access to the currently open run/inline SDTs.
    const std::stack<BookmarkInsertPosition>& GetSdtStarts() const;


    bool GetParaChanged() const { return m_StreamStateStack.top().bParaChanged; }
    bool GetParaHadField() const { return m_StreamStateStack.top().bParaHadField; }
    bool GetRemoveThisPara() const { return m_StreamStateStack.top().bRemoveThisParagraph; }

    void deferBreak( BreakType deferredBreakType );
    bool isBreakDeferred( BreakType deferredBreakType );
    void clearDeferredBreaks();
    void clearDeferredBreak(BreakType deferredBreakType);

    void setSdtEndDeferred(bool bSdtEndDeferred);
    bool isSdtEndDeferred() const;
    void setParaSdtEndDeferred(bool bParaSdtEndDeferred);
    bool isParaSdtEndDeferred() const;

    void finishParagraph( const PropertyMapPtr& pPropertyMap, const bool bRemove = false, const bool bNoNumbering = false);
    void applyToggleAttributes( const PropertyMapPtr& pPropertyMap );
    void MergeAtContentImageRedlineWithNext(const css::uno::Reference<css::text::XTextAppend>& xTextAppend);
    void appendTextPortion( const OUString& rString, const PropertyMapPtr& pPropertyMap );
    void appendTextContent(const css::uno::Reference<css::text::XTextContent>&, const css::uno::Sequence<css::beans::PropertyValue>&);
    void appendOLE( const OUString& rStreamName, const std::shared_ptr<OLEHandler>& pOleHandler );
    void appendStarMath( const Value& v);
    void adjustLastPara(sal_Int8 nAlign);
    rtl::Reference<SwXTextSection> appendTextSectionAfter(css::uno::Reference<css::text::XTextRange> const & xBefore);

    /// AutoText import: each entry is placed in the separate section
    void appendGlossaryEntry();
    /// Remember where entry was started
    void setGlossaryEntryStart( css::uno::Reference<css::text::XTextRange> const & xStart )
    {
        m_xGlossaryEntryStart = xStart;
    }

    // push the new properties onto the stack and make it the 'current' property map
    void    PushProperties(ContextType eId);
    void    PushStyleProperties(const PropertyMapPtr& pStyleProperties);
    void    PushListProperties(const PropertyMapPtr& pListProperties);
    void    PopProperties(ContextType eId);

    ContextType GetTopContextType() const { return m_aContextStack.top(); }
    const PropertyMapPtr& GetTopContext() const
    {
        return m_pTopContext;
    }
    PropertyMapPtr GetTopContextOfType(ContextType eId);

    bool HasTopText() const;
    css::uno::Reference<css::text::XTextAppend> const & GetTopTextAppend();
    FieldContextPtr const & GetTopFieldContext();

    bool HasTopAnchoredObjects() const;

    FontTablePtr const & GetFontTable()
    {
        if(!m_pFontTable)
            m_pFontTable = new FontTable();
        return m_pFontTable;
    }
    StyleSheetTablePtr const & GetStyleSheetTable()
    {
        if(!m_pStyleSheetTable)
            m_pStyleSheetTable = new StyleSheetTable( m_rDMapper, m_xTextDocument, m_bIsNewDoc );
        return m_pStyleSheetTable;
    }
    OUString GetListStyleName(sal_Int32 nListId);
    ListsManager::Pointer const & GetListTable();

    std::unique_ptr<ThemeHandler> const& getThemeHandler()
    {
        if (!m_pThemeHandler && m_pOOXMLDocument && m_pOOXMLDocument->getTheme())
        {
            m_pThemeHandler = std::make_unique<ThemeHandler>(m_pOOXMLDocument->getTheme(), GetSettingsTable()->GetThemeFontLangProperties());
        }
        return m_pThemeHandler;
    }

    SettingsTablePtr const & GetSettingsTable()
    {
        if( !m_pSettingsTable )
            m_pSettingsTable = new SettingsTable(m_rDMapper);
        return m_pSettingsTable;
    }

    GraphicImportPtr const & GetGraphicImport();
    void            ResetGraphicImport();
    // this method deletes the current m_pGraphicImport after import
    void    ImportGraphic(const writerfilter::Reference<Properties>::Pointer_t&);

    void InitTabStopFromStyle(const css::uno::Sequence<css::style::TabStop>& rInitTabStops);
    void    IncorporateTabStop( const DeletableTabStop &aTabStop );
    css::uno::Sequence<css::style::TabStop> GetCurrentTabStopAndClear();

    void SetCurrentParaStyleName(const OUString& rString) { m_StreamStateStack.top().sCurrentParaStyleName = rString; }
    OUString  GetCurrentParaStyleName();
    OUString  GetDefaultParaStyleName();

    // specified style - including inherited properties. Indicate whether paragraph defaults should be checked.
    css::uno::Any GetPropertyFromStyleSheet(PropertyIds eId, StyleSheetEntryPtr pEntry, const bool bDocDefaults, const bool bPara, bool* bIsDocDefault = nullptr);
    // current paragraph style - including inherited properties
    css::uno::Any GetPropertyFromParaStyleSheet(PropertyIds eId);
    // context's character style - including inherited properties
    css::uno::Any GetPropertyFromCharStyleSheet(PropertyIds eId, const PropertyMapPtr& rContext);
    // get property first from the given context, or secondly via inheritance from styles/docDefaults
    css::uno::Any GetAnyProperty(PropertyIds eId, const PropertyMapPtr& rContext);
    void        SetDocDefaultsImport( bool bSet ) { m_bInDocDefaultsImport = bSet;}
    bool        IsDocDefaultsImport()const { return m_bInDocDefaultsImport;}
    void        SetStyleSheetImport( bool bSet ) { m_bInStyleSheetImport = bSet;}
    bool        IsStyleSheetImport()const { return m_bInStyleSheetImport;}
    void        SetNumberingImport( bool bSet ) { m_bInNumberingImport = bSet;}
    bool        IsNumberingImport() const { return m_bInNumberingImport;}
    void        SetAnyTableImport( bool bSet ) { m_bInAnyTableImport = bSet;}
    bool        IsAnyTableImport()const { return m_bInAnyTableImport;}
    bool        IsInShape()const { return m_aAnchoredStack.size() > 0;}

    void PushShapeContext(const css::uno::Reference<css::drawing::XShape>& xShape);
    void PopShapeContext();
    void UpdateEmbeddedShapeProps(const css::uno::Reference<css::drawing::XShape>& xShape);
    /// Add a pending shape: it's currently inserted into the document, but it should be removed before the import finishes.
    void PushPendingShape(const css::uno::Reference<css::drawing::XShape>& xShape);
    /// Get the first pending shape, if there are any.
    css::uno::Reference<css::drawing::XShape> PopPendingShape();

    void PopPageHeaderFooter(PagePartType ePagePartType, PageType eType);
    bool IsInHeaderFooter() const { auto const type(m_StreamStateStack.top().eSubstreamType); return type == SubstreamType::Header || type == SubstreamType::Footer; }
    void ConvertHeaderFooterToTextFrame(bool, bool);
    static void fillEmptyFrameProperties(std::vector<css::beans::PropertyValue>& rFrameProperties, bool bSetAnchorToChar);

    bool IsInTOC() const;

    void PushFootOrEndnote( bool bIsFootnote );
    void PopFootOrEndnote();
    bool IsInFootOrEndnote() const { auto const type(m_StreamStateStack.top().eSubstreamType); return type == SubstreamType::Footnote || type == SubstreamType::Endnote; }
    bool IsInFootnote() const { return m_StreamStateStack.top().eSubstreamType == SubstreamType::Footnote; }

    void StartCustomFootnote(const PropertyMapPtr pContext);
    void EndCustomFootnote();
    bool IsInCustomFootnote() const { return m_StreamStateStack.top().bHasFootnoteStyle; }
    bool CheckFootnoteStyle() const { return m_StreamStateStack.top().bCheckFootnoteStyle; }
    void SetHasFootnoteStyle(bool const bVal) { m_StreamStateStack.top().bHasFootnoteStyle = bVal; }
    void SetCheckFootnoteStyle(bool const bVal) { m_StreamStateStack.top().bCheckFootnoteStyle = bVal; }

    const PropertyMapPtr& GetFootnoteContext() const { return m_pFootnoteContext; }

    SkipFootnoteSeparator GetSkipFootnoteState() const { return m_eSkipFootnoteState; }
    void SetSkipFootnoteState(SkipFootnoteSeparator eId) { m_eSkipFootnoteState =  eId; }
    sal_Int32 GetFootnoteCount() const { return m_nFootnotes; }
    void IncrementFootnoteCount() { ++m_nFootnotes; }
    sal_Int32 GetEndnoteCount() const { return m_nEndnotes; }
    void IncrementEndnoteCount() { ++m_nEndnotes; }
    bool CopyTemporaryNotes(
        css::uno::Reference< css::text::XFootnote > xNoteSrc,
        css::uno::Reference< css::text::XFootnote > xNoteDest );
    void RemoveTemporaryFootOrEndnotes();

    void PushAnnotation();
    void PopAnnotation();
    sal_Int32 GetAnnotationId() { return m_nAnnotationId; }

    /// A field context starts with a cFieldStart.
    void PushFieldContext();
    //the current field context waits for the completion of the command
    bool IsOpenFieldCommand() const;
    bool IsOpenField() const;
    //mark field in current context as locked (fixed)
    void SetFieldLocked();
    //collect the pieces of the command
    void AppendFieldCommand(OUString const & rPartOfCommand);
    void handleRubyEQField( const FieldContextPtr& pContext);
    void handleFieldSet
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > const & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    void handleFieldAsk
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    OUString convertFieldFormula(const OUString& input);
    void handleFieldFormula
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    void handleAutoNum
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > const & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    static void handleAuthor
        (std::u16string_view rFirstParam,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties,
        FieldId eFieldId);
    void handleDocProperty
        (const FieldContextPtr& pContext,
        OUString const& rFirstParam,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface);
    void handleToc
        (const FieldContextPtr& pContext,
        const OUString & sTOCServiceName);
    void handleIndex
        (const FieldContextPtr& pContext,
        const OUString & sTOCServiceName);

    void handleBibliography
        (const FieldContextPtr& pContext,
        const OUString & sTOCServiceName);
    /// The field command has to be closed (cFieldSep appeared).
    void CloseFieldCommand();
    //the _current_ fields require a string type result while TOCs accept richt results
    bool IsFieldResultAsString();
    void AppendFieldResult(std::u16string_view rResult);
    //apply the result text to the related field
    void SetFieldResult(OUString const& rResult);
    // set FFData of top field context
    void SetFieldFFData( const FFDataHandler::Pointer_t& pFFDataHandler );
    /// The end of field is reached (cFieldEnd appeared) - the command might still be open.
    void PopFieldContext();

    /// Returns title of the TOC placed in paragraph(s) before TOC field inside STD-frame
    OUString extractTocTitle();
    css::uno::Reference<css::beans::XPropertySet> createSectionForRange(css::uno::Reference< css::text::XTextRange > xStart, css::uno::Reference< css::text::XTextRange > xEnd, const OUString & sObjectType, bool stepLeft);

    void SetBookmarkName( const OUString& rBookmarkName );
    void StartOrEndBookmark( const OUString& rId );

    void SetMoveBookmark( bool IsFrom );

    void setPermissionRangeEd(const OUString& user);
    void setPermissionRangeEdGrp(const OUString& group);
    void startOrEndPermissionRange(sal_Int32 permissinId);

    void AddAnnotationPosition(
        const bool bStart,
        const sal_Int32 nAnnotationId );

    bool hasTableManager() const
    {
        return !m_aTableManagers.empty();
    }

    DomainMapperTableManager& getTableManager()
    {
        return *m_aTableManagers.top();
    }

    void appendTableManager( )
    {
        tools::SvRef<DomainMapperTableManager> pMngr(new DomainMapperTableManager());
        m_aTableManagers.push( pMngr );
    }

    void appendTableHandler( )
    {
        if (m_pTableHandler)
            m_aTableManagers.top()->setHandler(m_pTableHandler);
    }

    void popTableManager( )
    {
        if (hasTableManager())
            m_aTableManagers.pop();
    }

    void SetLineNumbering( sal_Int32 nLnnMod, sal_uInt32 nLnc, sal_Int32 ndxaLnn );
    bool IsLineNumberingSet() const {return m_bLineNumberingSet;}

    DeletableTabStop                m_aCurrentTabStop;

    bool IsOOXMLImport() const { return m_eDocumentType == SourceDocumentType::OOXML; }

    bool IsRTFImport() const { return m_eDocumentType == SourceDocumentType::RTF; }

    void InitPageMargins() { m_aPageMargins = PageMar(); }
    void SetPageMarginTwip( PageMarElement eElement, sal_Int32 nValue );
    const PageMar& GetPageMargins() const {return m_aPageMargins;}

    void InitPaperSource() { m_aPaperSource = PaperSource(); }
    void SetPaperSource( PaperSourceElement eElement, sal_Int32 nValue );
    const PaperSource& GetPaperSource() {return m_aPaperSource;}

    const LineNumberSettings& GetLineNumberSettings() const { return m_aLineNumberSettings;}
    void SetLineNumberSettings(const LineNumberSettings& rSet) { m_aLineNumberSettings = rSet;}

    void SetInFootnoteProperties(bool const bSet) { m_StreamStateStack.top().bIsInFootnoteProperties = bSet; }
    bool IsInFootnoteProperties() const { return m_StreamStateStack.top().bIsInFootnoteProperties; }

    bool IsInComments() const { return m_StreamStateStack.top().eSubstreamType == SubstreamType::Annotation; };

    std::vector<css::beans::PropertyValue> MakeFrameProperties(const ParagraphProperties& rProps);
    void CheckUnregisteredFrameConversion(bool bPreventOverlap = false);

    void RegisterFrameConversion(css::uno::Reference<css::text::XTextRange> const& xFrameStartRange,
                                 css::uno::Reference<css::text::XTextRange> const& xFrameEndRange,
                                 std::vector<css::beans::PropertyValue>&& aFrameProperties);
    void ExecuteFrameConversion();

    void AddNewRedline( sal_uInt32 sprmId );

    sal_Int32 GetCurrentRedlineToken( ) const;
    void SetCurrentRedlineAuthor( const OUString& sAuthor );
    void SetCurrentRedlineDate( const OUString& sDate );
    void SetCurrentRedlineId( sal_Int32 nId );
    void SetCurrentRedlineToken( sal_Int32 nToken );
    void SetCurrentRedlineRevertProperties( const css::uno::Sequence<css::beans::PropertyValue>& aProperties );
    void SetCurrentRedlineIsRead();
    void RemoveTopRedline( );
    void SetCurrentRedlineInitials( const OUString& sInitials );
    bool IsFirstRun() const { return m_StreamStateStack.top().bIsFirstRun; }
    void SetIsFirstRun(bool const bval) { m_StreamStateStack.top().bIsFirstRun = bval; }
    bool IsOutsideAParagraph() const { return m_StreamStateStack.top().bIsOutsideAParagraph; }
    void SetIsOutsideAParagraph(bool const bval) { m_StreamStateStack.top().bIsOutsideAParagraph = bval; }

    void ApplySettingsTable();

    css::uno::Reference<css::text::XTextAppend> GetCurrentXText() {
        return m_aTextAppendStack.empty() ? nullptr : m_aTextAppendStack.top().xTextAppend;
    }

    void NewFrameDirection() {
        m_aFrameDirectionQueue.push(std::nullopt);
        m_bFrameDirectionSet = false;
    }
    void SetFrameDirection(sal_Int16 nDirection) {
        if (!m_bFrameDirectionSet && !m_aFrameDirectionQueue.empty()) {
            m_aFrameDirectionQueue.back() = nDirection;
            m_bFrameDirectionSet = true;
        }
    }
    std::optional<sal_Int16> PopFrameDirection() {
        if (m_aFrameDirectionQueue.empty())
            return {};
        const std::optional<sal_Int16> nDirection = m_aFrameDirectionQueue.front();
        m_aFrameDirectionQueue.pop();
        return nDirection;
    }

    SectionPropertyMap * GetSectionContext();

    sal_Int16 GetListLevel(const StyleSheetEntryPtr& pEntry, const PropertyMapPtr& pParaContext = nullptr);
    void ValidateListLevel(const OUString& sStyleIdentifierD);

    /**
     Used for attributes/sprms which cannot be evaluated immediately (e.g. they depend
     on another one that comes in the same CONTEXT_CHARACTER). The property will be processed
     again in DomainMapper::processDeferredCharacterProperties().
    */
    void deferCharacterProperty(sal_Int32 id, const css::uno::Any& value);
    /**
     Processes properties deferred using deferCharacterProperty(). To be called whenever the top
     CONTEXT_CHARACTER is going to be used (e.g. by appendText()).
    */
    void processDeferredCharacterProperties(bool bCharContext = true);

    sal_Int32 getNumberingProperty(const sal_Int32 nListId, sal_Int32 nListLevel, const OUString& aProp);
    /// Get a property of the current numbering style's current level.
    sal_Int32 getCurrentNumberingProperty(const OUString& aProp);

    /// If we're importing into a new document, or just pasting to an existing one.
    bool IsNewDoc() const { return m_bIsNewDoc;}

    bool IsAltChunk() const { return m_bIsAltChunk;}

    /// If we're importing autotext.
    bool IsReadGlossaries() const { return m_bIsReadGlossaries;}

    tools::SvRef<SdtHelper> m_pSdtHelper;

    /// Document background color, applied to every page style.
    std::optional<sal_Int32> m_oBackgroundColor;
    bool m_bCopyStandardPageStyleFill = false;

    /// If the current section has a footnote separator.
    bool m_bHasFtnSep;

    /// Paragraphs with anchored objects in the current section.
    std::vector<AnchoredObjectsInfo> m_aAnchoredObjectAnchors;

    /// Append a property to a sub-grabbag if necessary (e.g. 'lineRule', 'auto')
    void appendGrabBag(std::vector<css::beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, const OUString& aValue);
    void appendGrabBag(std::vector<css::beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, std::vector<css::beans::PropertyValue>& rValue);

    /// Enable, disable and check status of grabbags
    void enableInteropGrabBag(const OUString& aName);
    void disableInteropGrabBag();
    bool isInteropGrabBagEnabled() const;

    /// Name of m_aInteropGrabBag.
    OUString m_aInteropGrabBagName;

    /// A toplevel dmapper grabbag, like 'pPr'.
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;

    /// A sub-grabbag of m_aInteropGrabBag, like 'spacing'.
    std::vector<css::beans::PropertyValue> m_aSubInteropGrabBag;

    /// ST_PositionOffset values we received
    std::pair<OUString, OUString> m_aPositionOffsets;
    /// ST_AlignH/V values we received
    std::pair<OUString, OUString> m_aAligns;
    /// ST_PositivePercentage values we received
    std::queue<OUString> m_aPositivePercentages;
    enum GraphicImportType m_eGraphicImportType = {};
    bool isInIndexContext() const { return m_bStartIndex;}
    bool isInBibliographyContext() const { return m_bStartBibliography;}
    SmartTagHandler& getSmartTagHandler() { return m_aSmartTagHandler; }

    void substream(Id rName, ::writerfilter::Reference<Stream>::Pointer_t const& ref);

    /// If the document needs to split paragraph.
    bool m_bIsSplitPara;

    /// Check if "SdtEndBefore" property is set
    bool IsSdtEndBefore();

    bool IsDiscardHeaderFooter() const;

    bool IsForceGenericFields() const { return m_bForceGenericFields; }

    void SetParaAutoBefore(bool const bParaAutoBefore) { m_StreamStateStack.top().bParaAutoBefore = bParaAutoBefore; }

    /// Forget about the previous paragraph, as it's not inside the same
    /// start/end node.
    void ClearPreviousParagraph();

    /// Check if previous paragraph has borders in between and do the border magic to it if so
    bool handlePreviousParagraphBorderInBetween() const;

    /// Handle redline text portions in a frame, footnotes and redlines:
    /// store their data, and create them after frame creation or footnote/endnote copying
    bool m_bIsActualParagraphFramed;
    std::deque<css::uno::Any> m_aStoredRedlines[StoredRedlines::NONE];

    bool IsParaWithInlineObject() const { return m_StreamStateStack.top().bParaWithInlineObject; }

    bool SeenHeaderFooter(PagePartType, PageType) const;

    css::uno::Reference< css::embed::XStorage > m_xDocumentStorage;

    /// Handles <w:altChunk>.
    void HandleAltChunk(const OUString& rStreamName);

    /// Handles <w:ptab>.
    void HandlePTab(sal_Int32 nAlignment);

    /// Handles <w:br w:clear="...">.
    void HandleLineBreakClear(sal_Int32 nClear);

    /// Handles <w:br>.
    void HandleLineBreak(const PropertyMapPtr& pPropertyMap);

    void commentProps(const OUString& sId, const CommentProperties& rProps);

    OUString ConvertTOCStyleName(OUString const&);

    OUString getFontNameForTheme(const Id id);

private:
    void PushPageHeaderFooter(PagePartType ePagePartType, PageType eType);
    // Start a new index section; if needed, finish current paragraph
    css::uno::Reference<css::beans::XPropertySet> StartIndexSectionChecked(const OUString& sServiceName);
    std::vector<css::uno::Reference< css::drawing::XShape > > m_vTextFramesForChaining ;
    /// SAXException was seen so document will be abandoned
    bool m_bSaxError;

    std::unordered_map<OUString, CommentProperties> m_aCommentProps;
};

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
