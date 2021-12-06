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
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <queue>
#include <stack>
#include <string_view>
#include <o3tl/sorted_vector.hxx>
#include <unordered_map>
#include <vector>
#include <optional>

#include <dmapper/CommentProperties.hxx>

#include "DomainMapper.hxx"
#include "DomainMapperTableManager.hxx"
#include "DomainMapperTableHandler.hxx"
#include "PropertyMap.hxx"
#include "FontTable.hxx"
#include "NumberingManager.hxx"
#include "StyleSheetTable.hxx"
#include "SettingsTable.hxx"
#include "ThemeTable.hxx"
#include "GraphicImport.hxx"
#include "OLEHandler.hxx"
#include "FFDataHandler.hxx"
#include "SmartTagHandler.hxx"
#include "FormControlHelper.hxx"
#include <map>

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
                class XFormField;
        }
        namespace beans{ class XPropertySet;}
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
    COLUMN_BREAK
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

/**
 * Storage for state that is relevant outside a header/footer, but not inside it.
 *
 * In case some state of DomainMapper_Impl should be reset before handling the
 * header/footer and should be restored once handling of header/footer is done,
 * then you can use this class to do so.
 */
class HeaderFooterContext
{
    bool      m_bTextInserted;
    sal_Int32 m_nTableDepth;

public:
    explicit HeaderFooterContext(bool bTextInserted, sal_Int32 nTableDepth);
    bool getTextInserted() const;
    sal_Int32 getTableDepth() const;
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

    OUString m_sCommand;
    OUString m_sResult;
    std::optional<FieldId> m_eFieldId;
    bool m_bFieldLocked;

    css::uno::Reference<css::text::XTextField> m_xTextField;
    css::uno::Reference<css::text::XFormField> m_xFormField;
    css::uno::Reference<css::beans::XPropertySet> m_xTOC;
    css::uno::Reference<css::beans::XPropertySet> m_xTC; // TOX entry
    css::uno::Reference<css::beans::XPropertySet> m_xCustomField;

    OUString m_sHyperlinkURL;
    /// A frame for the hyperlink when one exists.
    OUString m_sHyperlinkTarget;

    FFDataHandler::Pointer_t m_pFFDataHandler;
    FormControlHelper::Pointer_t m_pFormControlHelper;
    /// (Character) properties of the field itself.
    PropertyMapPtr m_pProperties;

    std::vector<FieldParagraph> m_aParagraphsToFinish;

public:
    explicit FieldContext(css::uno::Reference<css::text::XTextRange> const& xStart);
    ~FieldContext() override;

    const css::uno::Reference<css::text::XTextRange>& GetStartRange() const { return m_xStartRange; }

    void                    AppendCommand(std::u16string_view rPart);
    const OUString&  GetCommand() const {return m_sCommand; }

    void SetFieldId(FieldId eFieldId ) { m_eFieldId = eFieldId; }
    std::optional<FieldId> const & GetFieldId() const { return m_eFieldId; }

    void AppendResult(std::u16string_view rResult) { m_sResult += rResult; }
    const OUString&  GetResult() const { return m_sResult; }

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

    void setFFDataHandler(FFDataHandler::Pointer_t pFFDataHandler) { m_pFFDataHandler = pFFDataHandler; }
    const FFDataHandler::Pointer_t& getFFDataHandler() const { return m_pFFDataHandler; }

    void setFormControlHelper(FormControlHelper::Pointer_t pFormControlHelper) { m_pFormControlHelper = pFormControlHelper; }
    const FormControlHelper::Pointer_t& getFormControlHelper() const { return m_pFormControlHelper; }
    const PropertyMapPtr& getProperties() const { return m_pProperties; }

    ::std::vector<OUString> GetCommandParts() const;

    std::vector<FieldParagraph>& GetParagraphsToFinish() { return m_aParagraphsToFinish; }
};

struct TextAppendContext
{
    css::uno::Reference<css::text::XTextAppend> xTextAppend;
    css::uno::Reference<css::text::XTextRange> xInsertPosition;
    css::uno::Reference<css::text::XParagraphCursor> xCursor;
    ParagraphPropertiesPtr pLastParagraphProperties;

    /**
     * Objects anchored to the current paragraph, may affect the paragraph
     * spacing.
     */
    std::vector<AnchoredObjectInfo> m_aAnchoredObjects;

    inline TextAppendContext(const css::uno::Reference<css::text::XTextAppend>& xAppend, const css::uno::Reference<css::text::XTextCursor>& xCur);
};

struct AnchoredContext
{
    css::uno::Reference<css::text::XTextContent> xTextContent;
    bool bToRemove;

    explicit AnchoredContext(const css::uno::Reference<css::text::XTextContent>& xContent)
        : xTextContent(xContent), bToRemove(false)
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
        // same defaults as SvxXMLTabStopContext_Impl
        FillChar = ' ';
        DecimalChar = ',';
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
    BookmarkInsertPosition(bool bIsStartOfText, const OUString& rName, css::uno::Reference<css::text::XTextRange> const& xTextRange):
        m_bIsStartOfText( bIsStartOfText ),
        m_sBookmarkName( rName ),
        m_xTextRange( xTextRange )
     {}
};

struct PermInsertPosition
{
    bool        m_bIsStartOfText;
    sal_Int32   m_Id;
    OUString    m_Ed;
    OUString    m_EdGrp;

    css::uno::Reference<css::text::XTextRange> m_xTextRange;

    PermInsertPosition(bool bIsStartOfText, sal_Int32 id, const OUString& ed, const OUString& edGrp, css::uno::Reference<css::text::XTextRange> const& xTextRange)
        : m_bIsStartOfText(bIsStartOfText)
        , m_Id(id)
        , m_Ed(ed)
        , m_EdGrp(edGrp)
        , m_xTextRange(xTextRange)
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

/// Contains information about a table that will be potentially converted to a floating one at the section end.
struct FloatingTableInfo
{
    css::uno::Reference<css::text::XTextRange> m_xStart;
    css::uno::Reference<css::text::XTextRange> m_xEnd;
    css::uno::Sequence<css::beans::PropertyValue> m_aFrameProperties;
    sal_Int32 m_nTableWidth;
    sal_Int32 m_nTableWidthType;
    /// Break type of the section that contains this table.
    sal_Int32 m_nBreakType = -1;
    /// Tables in footnotes and endnotes are always floating
    bool m_bConvertToFloatingInFootnote = false;

    FloatingTableInfo(css::uno::Reference<css::text::XTextRange> const& xStart,
            css::uno::Reference<css::text::XTextRange> const& xEnd,
            const css::uno::Sequence<css::beans::PropertyValue>& aFrameProperties,
            sal_Int32 nTableWidth, sal_Int32 nTableWidthType, bool bConvertToFloatingInFootnote)
        : m_xStart(xStart),
        m_xEnd(xEnd),
        m_aFrameProperties(aFrameProperties),
        m_nTableWidth(nTableWidth),
        m_nTableWidthType(nTableWidthType),
        m_bConvertToFloatingInFootnote(bConvertToFloatingInFootnote)
    {
    }
    css::uno::Any getPropertyValue(std::u16string_view propertyName);
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
    OUString m_aBaseUrl;
    css::uno::Reference<css::text::XTextDocument> m_xTextDocument;
    css::uno::Reference<css::beans::XPropertySet> m_xDocumentSettings;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::container::XNameContainer> m_xPageStyles1;
    // cache next available number, expensive to repeatedly compute
    std::optional<int> m_xNextUnusedPageStyleNo;
    css::uno::Reference<css::container::XNameContainer> m_xCharacterStyles;
    // cache next available number, expensive to repeatedly compute
    std::optional<int> m_xNextUnusedCharacterStyleNo;
    css::uno::Reference<css::text::XText> m_xBodyText;
    css::uno::Reference<css::text::XTextContent> m_xEmbedded;

    std::stack<TextAppendContext>                                                   m_aTextAppendStack;
    std::stack<AnchoredContext>                                                     m_aAnchoredStack;
    std::stack<HeaderFooterContext>                                                 m_aHeaderFooterStack;
    std::stack<std::pair<TextAppendContext, bool>>                                  m_aHeaderFooterTextAppendStack;
    std::deque<FieldContextPtr> m_aFieldStack;
    bool m_bForceGenericFields;
    bool                                                                            m_bSetUserFieldContent;
    bool                                                                            m_bSetCitation;
    bool                                                                            m_bSetDateValue;
    bool                                                                            m_bIsFirstSection;
    bool                                                                            m_bIsColumnBreakDeferred;
    bool                                                                            m_bIsPageBreakDeferred;
    /// If we want to set "sdt end" on the next character context.
    bool                                                                            m_bSdtEndDeferred;
    /// If we want to set "paragraph sdt end" on the next paragraph context.
    bool                                                                            m_bParaSdtEndDeferred;
    bool                                                                            m_bStartTOC;
    bool                                                                            m_bStartTOCHeaderFooter;
    /// If we got any text that is the pre-rendered result of the TOC field.
    bool                                                                            m_bStartedTOC;
    bool                                                                            m_bStartIndex;
    bool                                                                            m_bStartBibliography;
    unsigned int                                                                    m_nStartGenericField;
    bool                                                                            m_bTextInserted;
    LineNumberSettings                                                              m_aLineNumberSettings;

    BookmarkMap_t                                                                   m_aBookmarkMap;
    OUString                                                                        m_sCurrentBkmkId;
    OUString                                                                        m_sCurrentBkmkName;
    OUString                                                                        m_sCurrentBkmkPrefix;

    PermMap_t                                                                       m_aPermMap;
    sal_Int32                                                                       m_sCurrentPermId;
    OUString                                                                        m_sCurrentPermEd;
    OUString                                                                        m_sCurrentPermEdGrp;

    PageMar                                                                        m_aPageMargins;
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
    ThemeTablePtr           m_pThemeTable;
    SettingsTablePtr        m_pSettingsTable;
    GraphicImportPtr        m_pGraphicImport;


    PropertyMapPtr                  m_pTopContext;
    PropertyMapPtr           m_pLastSectionContext;
    PropertyMapPtr           m_pLastCharacterContext;

    ::std::vector<DeletableTabStop> m_aCurrentTabStops;
    OUString                        m_sCurrentParaStyleName; //highly inaccurate. Overwritten by "overlapping" paragraphs like comments, flys.
    OUString                        m_sDefaultParaStyleName; //caches the ConvertedStyleName of the default paragraph style
    bool                            m_bInDocDefaultsImport;
    bool                            m_bInStyleSheetImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInNumberingImport; //in import of numbering (i.e. numbering.xml)
    bool                            m_bInAnyTableImport; //in import of fonts, styles, lists or lfos
    enum class HeaderFooterImportState
    {
        none,
        header,
        footer,
    }                               m_eInHeaderFooterImport;
    bool                            m_bDiscardHeaderFooter;
    bool                            m_bInFootOrEndnote;
    bool                            m_bInFootnote;
    PropertyMapPtr m_pFootnoteContext;
    bool m_bHasFootnoteStyle;
    bool m_bCheckFootnoteStyle;
    /// Skip paragraphs from the <w:separator/> footnote
    SkipFootnoteSeparator           m_eSkipFootnoteState;
    /// preload footnotes and endnotes
    sal_Int32                       m_nFootnotes;
    sal_Int32                       m_nEndnotes;

    bool                            m_bLineNumberingSet;
    bool                            m_bIsInFootnoteProperties;

    RubyInfo                        m_aRubyInfo;
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
    // redline data of the terminating run, if it's a moveFrom deletion
    RedlineParamsPtr                m_pParaMarkerRedlineMoveFrom;
    // This is for removing workaround (double ZWSPs around the anchoring point) for track
    // changed images anchored *to* character, if it's followed by a redline text run immediately.
    // (In that case, the image is part of a tracked text range, no need for the dummy
    // text ZWSPs to keep the change tracking of the image in Writer.)
    bool                            m_bRedlineImageInPreviousRun;

    /// If the current paragraph has any runs.
    bool                            m_bParaChanged;
    bool                            m_bIsFirstParaInSection;
    bool                            m_bIsFirstParaInSectionAfterRedline;
    bool                            m_bIsFirstParaInShape = false;
    bool                            m_bDummyParaAddedForTableInSection;
    bool                            m_bDummyParaAddedForTableInSectionPage;
    bool                            m_bTextFrameInserted;
    bool                            m_bIsPreviousParagraphFramed;
    bool                            m_bIsLastParaInSection;
    bool                            m_bIsLastSectionGroup;
    bool                            m_bIsInComments;
    /// If the current paragraph contains section property definitions.
    bool                            m_bParaSectpr;
    bool                            m_bUsingEnhancedFields;
    /// If the current paragraph is inside a structured document element.
    bool                            m_bSdt;
    bool                            m_bIsFirstRun;
    bool                            m_bIsOutsideAParagraph;
    /// This is a continuation of already finished paragraph - e.g., first in an index section
    bool                            m_bRemoveThisParagraph = false;

    css::uno::Reference< css::text::XTextCursor > xTOCMarkerCursor;

    //annotation import
    css::uno::Reference< css::beans::XPropertySet > m_xAnnotationField;
    sal_Int32 m_nAnnotationId;
    bool m_bAnnotationResolved = false;
    std::unordered_map< sal_Int32, AnnotationPosition > m_aAnnotationPositions;

    void GetCurrentLocale(css::lang::Locale& rLocale);
    void SetNumberFormat(const OUString& rCommand, css::uno::Reference<css::beans::XPropertySet> const& xPropertySet, bool bDetectFormat = false);
    /// @throws css::uno::Exception
    css::uno::Reference<css::beans::XPropertySet> FindOrCreateFieldMaster(const char* pFieldMasterService, const OUString& rFieldMasterName);
    css::uno::Reference<css::beans::XPropertySet> const & GetDocumentSettings();

    std::map<sal_Int32, css::uno::Any> deferredCharacterProperties;
    SmartTagHandler m_aSmartTagHandler;

    css::uno::Reference<css::text::XTextRange> m_xGlossaryEntryStart;
    css::uno::Reference<css::text::XTextRange> m_xSdtEntryStart;

public:
    css::uno::Reference<css::text::XTextRange> m_xInsertTextRange;
    css::uno::Reference<css::text::XTextRange> m_xAltChunkStartingRange;
private:
    bool m_bIsNewDoc;
    bool m_bIsAltChunk = false;
    bool m_bIsReadGlossaries;
public:
    DomainMapper_Impl(
            DomainMapper& rDMapper,
            css::uno::Reference < css::uno::XComponentContext > const& xContext,
            css::uno::Reference< css::lang::XComponent > const& xModel,
            SourceDocumentType eDocumentType,
            utl::MediaDescriptor const & rMediaDesc);
    ~DomainMapper_Impl();

    SectionPropertyMap* GetLastSectionContext( )
    {
        return dynamic_cast< SectionPropertyMap* >( m_pLastSectionContext.get( ) );
    }

    css::uno::Reference<css::container::XNameContainer> const & GetPageStyles();
    OUString GetUnusedPageStyleName();
    css::uno::Reference<css::container::XNameContainer> const & GetCharacterStyles();
    OUString GetUnusedCharacterStyleName();
    css::uno::Reference<css::text::XText> const & GetBodyText();
    const css::uno::Reference<css::lang::XMultiServiceFactory>& GetTextFactory() const
    {
        return m_xTextFactory;
    }
    const css::uno::Reference<css::text::XTextDocument>& GetTextDocument() const
    {
        return m_xTextDocument;
    }
    void SetDocumentSettingsProperty( const OUString& rPropName, const css::uno::Any& rValue );

    void CreateRedline(css::uno::Reference<css::text::XTextRange> const& xRange, const RedlineParamsPtr& pRedline);

    void CheckParaMarkerRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void CheckRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void StartParaMarkerChange( );
    void EndParaMarkerChange( );
    void ChainTextFrames();

    void RemoveDummyParaForTableInSection();
    void AddDummyParaForTableInSection();
    void RemoveLastParagraph( );
    void SetIsLastParagraphInSection( bool bIsLast );
    bool GetIsLastParagraphInSection() const { return m_bIsLastParaInSection;}
    void SetRubySprmId( sal_uInt32 nSprmId) { m_aRubyInfo.nSprmId = nSprmId ; }
    void SetRubyText( OUString const &sText, OUString const &sStyle) {
        m_aRubyInfo.sRubyText = sText;
        m_aRubyInfo.sRubyStyle = sStyle;
    }
    const RubyInfo & GetRubyInfo() const { return m_aRubyInfo;}
    void SetRubyInfo(const RubyInfo & rInfo) { m_aRubyInfo = rInfo;}

    void SetIsLastSectionGroup( bool bIsLast );
    bool GetIsLastSectionGroup() const { return m_bIsLastSectionGroup;}
    void SetIsFirstParagraphInSection( bool bIsFirst );
    void SetIsFirstParagraphInSectionAfterRedline( bool bIsFirstAfterRedline );
    bool GetIsFirstParagraphInSection( bool bAfterRedline = false ) const;
    void SetIsFirstParagraphInShape(bool bIsFirst);
    bool GetIsFirstParagraphInShape() const { return m_bIsFirstParaInShape; }
    void SetIsDummyParaAddedForTableInSection( bool bIsAdded );
    bool GetIsDummyParaAddedForTableInSection() const { return m_bDummyParaAddedForTableInSection;}
    void SetIsDummyParaAddedForTableInSectionPage(bool bIsAdded);
    bool GetIsDummyParaAddedForTableInSectionPage() const { return m_bDummyParaAddedForTableInSectionPage; }

    /// Track if a textframe has been inserted into this section
    void SetIsTextFrameInserted( bool bIsInserted );
    bool GetIsTextFrameInserted() const { return m_bTextFrameInserted;}

    void SetIsPreviousParagraphFramed( bool bIsFramed ) { m_bIsPreviousParagraphFramed = bIsFramed; }
    bool GetIsPreviousParagraphFramed() const { return m_bIsPreviousParagraphFramed; }
    void SetParaSectpr(bool bParaSectpr);
    bool GetParaSectpr() const { return m_bParaSectpr;}

    void SetSymbolChar( sal_Int32 nSymbol) { m_aSymbolData.cSymbol = sal_Unicode(nSymbol); }
    void SetSymbolFont( OUString const &rName ) { m_aSymbolData.sFont = rName; }
    const SymbolData & GetSymbolData() const { return m_aSymbolData;}

    /// Setter method for m_bSdt.
    void SetSdt(bool bSdt);
    /// Getter method for m_bSdt.
    bool GetSdt() const { return m_bSdt;}
    bool GetParaChanged() const { return m_bParaChanged;}
    bool GetParaHadField() const { return m_bParaHadField; }
    bool GetRemoveThisPara() const { return m_bRemoveThisParagraph; }

    void deferBreak( BreakType deferredBreakType );
    bool isBreakDeferred( BreakType deferredBreakType );
    void clearDeferredBreaks();
    void clearDeferredBreak(BreakType deferredBreakType);

    void setSdtEndDeferred(bool bSdtEndDeferred);
    bool isSdtEndDeferred() const;
    void setParaSdtEndDeferred(bool bParaSdtEndDeferred);
    bool isParaSdtEndDeferred() const;

    void finishParagraph( const PropertyMapPtr& pPropertyMap, const bool bRemove = false, const bool bNoNumbering = false);
    void appendTextPortion( const OUString& rString, const PropertyMapPtr& pPropertyMap );
    void appendTextContent(const css::uno::Reference<css::text::XTextContent>&, const css::uno::Sequence<css::beans::PropertyValue>&);
    void appendOLE( const OUString& rStreamName, const std::shared_ptr<OLEHandler>& pOleHandler );
    void appendStarMath( const Value& v);
    void adjustLastPara(sal_Int8 nAlign);
    css::uno::Reference<css::beans::XPropertySet> appendTextSectionAfter(css::uno::Reference<css::text::XTextRange> const & xBefore);

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
    ThemeTablePtr const & GetThemeTable()
    {
        if(!m_pThemeTable)
            m_pThemeTable = new ThemeTable;
        return m_pThemeTable;
    }

    SettingsTablePtr const & GetSettingsTable()
    {
        if( !m_pSettingsTable )
            m_pSettingsTable = new SettingsTable(m_rDMapper);
        return m_pSettingsTable;
    }

    GraphicImportPtr const & GetGraphicImport( GraphicImportType eGraphicImportType );
    void            ResetGraphicImport();
    // this method deletes the current m_pGraphicImport after import
    void    ImportGraphic(const writerfilter::Reference< Properties>::Pointer_t&, GraphicImportType eGraphicImportType );

    void InitTabStopFromStyle(const css::uno::Sequence<css::style::TabStop>& rInitTabStops);
    void    IncorporateTabStop( const DeletableTabStop &aTabStop );
    css::uno::Sequence<css::style::TabStop> GetCurrentTabStopAndClear();

    void            SetCurrentParaStyleName(const OUString& sStringValue) {m_sCurrentParaStyleName = sStringValue;}
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

    void PushPageHeader(SectionPropertyMap::PageType eType);
    void PushPageFooter(SectionPropertyMap::PageType eType);

    void PopPageHeaderFooter();
    bool IsInHeaderFooter() const { return m_eInHeaderFooterImport != HeaderFooterImportState::none; }
    void ConvertHeaderFooterToTextFrame(bool, bool);
    static void fillEmptyFrameProperties(std::vector<css::beans::PropertyValue>& rFrameProperties, bool bSetAnchorToChar);

    bool IsInTOC() const { return m_bStartTOC; }

    void PushFootOrEndnote( bool bIsFootnote );
    void PopFootOrEndnote();
    bool IsInFootOrEndnote() const { return m_bInFootOrEndnote; }
    bool IsInFootnote() const { return m_bInFootnote; }

    void StartCustomFootnote(const PropertyMapPtr pContext);
    void EndCustomFootnote();
    bool IsInCustomFootnote() const { return m_bHasFootnoteStyle; }
    bool CheckFootnoteStyle() const { return m_bCheckFootnoteStyle; }
    void SetHasFootnoteStyle(bool bVal) { m_bHasFootnoteStyle = bVal; }
    void SetCheckFootnoteStyle(bool bVal) { m_bCheckFootnoteStyle = bVal; }

    const PropertyMapPtr& GetFootnoteContext() const { return m_pFootnoteContext; }

    SkipFootnoteSeparator GetSkipFootnoteState() const { return m_eSkipFootnoteState; }
    void SetSkipFootnoteState(SkipFootnoteSeparator eId) { m_eSkipFootnoteState =  eId; }
    sal_Int32 GetFootnoteCount() const { return m_nFootnotes; }
    void IncrementFootnoteCount() { ++m_nFootnotes; }
    sal_Int32 GetEndnoteCount() const { return m_nEndnotes; }
    void IncrementEndnoteCount() { ++m_nEndnotes; }

    void PushAnnotation();
    void PopAnnotation();

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

    /// If we're right after the end of a table.
    bool m_bConvertedTable = false;

    bool IsOOXMLImport() const { return m_eDocumentType == SourceDocumentType::OOXML; }

    bool IsRTFImport() const { return m_eDocumentType == SourceDocumentType::RTF; }

    void InitPageMargins() { m_aPageMargins = PageMar(); }
    void SetPageMarginTwip( PageMarElement eElement, sal_Int32 nValue );
    const PageMar& GetPageMargins() const {return m_aPageMargins;}

    const LineNumberSettings& GetLineNumberSettings() const { return m_aLineNumberSettings;}
    void SetLineNumberSettings(const LineNumberSettings& rSet) { m_aLineNumberSettings = rSet;}

    void SetInFootnoteProperties(bool bSet) { m_bIsInFootnoteProperties = bSet;}
    bool IsInFootnoteProperties() const { return m_bIsInFootnoteProperties;}

    bool IsInComments() const { return m_bIsInComments; };

    void CheckUnregisteredFrameConversion( );

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
    bool IsFirstRun() const { return m_bIsFirstRun;}
    void SetIsFirstRun(bool bval) { m_bIsFirstRun = bval;}
    bool IsOutsideAParagraph() const { return m_bIsOutsideAParagraph;}
    void SetIsOutsideAParagraph(bool bval) { m_bIsOutsideAParagraph = bval;}

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
    /// If the current paragraph has a numbering style associated, this method returns its character style (part of the numbering rules)
    css::uno::Reference<css::beans::XPropertySet> GetCurrentNumberingCharStyle();
    /// If the current paragraph has a numbering style associated, this method returns its numbering rules
    css::uno::Reference<css::container::XIndexAccess> GetCurrentNumberingRules(sal_Int32* pListLevel);

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
    void processDeferredCharacterProperties();

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

    /**
     * This contains the raw table depth. m_nTableDepth > 0 is the same as
     * getTableManager().isInTable(), unless we're in the first paragraph of a
     * table, or first paragraph after a table, as the table manager is only
     * updated once we ended the paragraph (and know if the para has the
     * inTbl SPRM or not).
     */
    sal_Int32 m_nTableDepth;
    /// Raw table cell depth.
    sal_Int32 m_nTableCellDepth;
    /// Table cell depth of the last finished paragraph.
    sal_Int32 m_nLastTableCellParagraphDepth;

    /// If the current section has footnotes.
    bool m_bHasFtn;
    /// If the current section has a footnote separator.
    bool m_bHasFtnSep;

    /// If the next tab should be ignored, used for footnotes.
    bool m_bCheckFirstFootnoteTab;
    bool m_bIgnoreNextTab;
    /// Pending floating tables: they may be converted to text frames at the section end.
    std::vector<FloatingTableInfo> m_aPendingFloatingTables;

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

    void SetParaAutoBefore(bool bParaAutoBefore) { m_bParaAutoBefore = bParaAutoBefore; }

    /// Forget about the previous paragraph, as it's not inside the same
    /// start/end node.
    void ClearPreviousParagraph();

    /// Handle redline text portions in a frame, footnotes and redlines:
    /// store their data, and create them after frame creation or footnote/endnote copying
    bool m_bIsActualParagraphFramed;
    std::deque<css::uno::Any> m_aStoredRedlines[StoredRedlines::NONE];

    bool IsParaWithInlineObject() const { return m_bParaWithInlineObject; }

    css::uno::Reference< css::embed::XStorage > m_xDocumentStorage;

    /// Handles <w:altChunk>.
    void HandleAltChunk(const OUString& rStreamName);

    void commentProps(const OUString& sId, const CommentProperties& rProps);

private:
    void PushPageHeaderFooter(bool bHeader, SectionPropertyMap::PageType eType);
    // Start a new index section; if needed, finish current paragraph
    css::uno::Reference<css::beans::XPropertySet> StartIndexSectionChecked(const OUString& sServiceName);
    std::vector<css::uno::Reference< css::drawing::XShape > > m_vTextFramesForChaining ;
    /// Current paragraph had at least one field in it.
    bool m_bParaHadField;
    bool m_bSaveParaHadField;
    css::uno::Reference<css::beans::XPropertySet> m_xPreviousParagraph;
    /// Current paragraph has automatic before spacing.
    bool m_bParaAutoBefore;
    /// Current paragraph in a table is first paragraph of a cell
    bool m_bFirstParagraphInCell;
    bool m_bSaveFirstParagraphInCell;
    /// Current paragraph had at least one inline object in it.
    bool m_bParaWithInlineObject;
    /// SAXException was seen so document will be abandoned
    bool m_bSaxError;

    std::unordered_map<OUString, CommentProperties> m_aCommentProps;
};

TextAppendContext::TextAppendContext(const css::uno::Reference<css::text::XTextAppend>& xAppend, const css::uno::Reference<css::text::XTextCursor>& xCur)
    : xTextAppend(xAppend)
{
    xCursor.set(xCur, css::uno::UNO_QUERY);
    xInsertPosition = xCursor;
}

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
