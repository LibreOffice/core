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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPER_IMPL_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPER_IMPL_HXX

#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <queue>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <boost/optional.hpp>

#include <ooxml/resourceids.hxx>

#include <DomainMapper.hxx>
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

#include <string.h>

namespace com{ namespace sun{ namespace star{
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
}}}

namespace writerfilter {
namespace dmapper {

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

/*-------------------------------------------------------------------------
    property stack element
  -----------------------------------------------------------------------*/
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
 * Storage for state that is relevant outside a header/footer, but not inside it.
 *
 * In case some state of DomainMapper_Impl should be reset before handling the
 * header/footer and should be restored once handling of header/footer is done,
 * then you can use this class to do so.
 */
class HeaderFooterContext
{
    bool m_bTextInserted;
public:
    HeaderFooterContext(bool bTextInserted);
    bool getTextInserted();
};

/*--------------------------------------------------
   field stack element
 * --------------------------------------------------*/
class FieldContext
{
    bool                                                                            m_bFieldCommandCompleted;
    css::uno::Reference<css::text::XTextRange> m_xStartRange;

    OUString                                                                 m_sCommand;
    OUString m_sResult;
    bool m_bFieldLocked;

    css::uno::Reference<css::text::XTextField> m_xTextField;
    css::uno::Reference<css::text::XFormField>  m_xFormField;
    css::uno::Reference<css::beans::XPropertySet> m_xTOC;
    css::uno::Reference<css::beans::XPropertySet> m_xTC; // TOX entry
    css::uno::Reference<css::beans::XPropertySet> m_xCustomField;
    OUString                                                                 m_sHyperlinkURL;
    FFDataHandler::Pointer_t                                                        m_pFFDataHandler;
    FormControlHelper::Pointer_t                                                    m_pFormControlHelper;
    /// (Character) properties of the field itself.
    PropertyMapPtr m_pProperties;

public:
    FieldContext(css::uno::Reference<css::text::XTextRange> const& xStart);
    ~FieldContext();

    const css::uno::Reference<css::text::XTextRange>& GetStartRange() const { return m_xStartRange; }

    void                    AppendCommand(const OUString& rPart);
    const OUString&  GetCommand() const {return m_sCommand; }

    void AppendResult(OUString const& rResult) { m_sResult += rResult; }
    const OUString&  GetResult() const { return m_sResult; }

    void                    SetCommandCompleted() { m_bFieldCommandCompleted = true; }
    bool                    IsCommandCompleted() const { return m_bFieldCommandCompleted;    }

    void                    SetFieldLocked() { m_bFieldLocked = true; }
    bool                    IsFieldLocked() { return m_bFieldLocked; }

    const css::uno::Reference<css::beans::XPropertySet>& GetCustomField() const { return m_xCustomField; }
    void SetCustomField(css::uno::Reference<css::beans::XPropertySet> const& xCustomField) { m_xCustomField = xCustomField; }
    const css::uno::Reference<css::text::XTextField>& GetTextField() const { return m_xTextField;}
    void SetTextField(css::uno::Reference<css::text::XTextField> const& xTextField) { m_xTextField = xTextField;}
    const css::uno::Reference<css::text::XFormField>& GetFormField() const { return m_xFormField;}
    void SetFormField(css::uno::Reference<css::text::XFormField> const& xFormField) { m_xFormField = xFormField;}

    void SetTOC(css::uno::Reference<css::beans::XPropertySet> const& xTOC) { m_xTOC = xTOC; }
    const css::uno::Reference<css::beans::XPropertySet>& GetTOC() { return m_xTOC; }

    void SetTC(css::uno::Reference<css::beans::XPropertySet> const& xTC) { m_xTC = xTC; }
    const css::uno::Reference<css::beans::XPropertySet>& GetTC() { return m_xTC; }

    void    SetHyperlinkURL( const OUString& rURL ) { m_sHyperlinkURL = rURL; }
    const OUString&                                                      GetHyperlinkURL() { return m_sHyperlinkURL; }

    void setFFDataHandler(FFDataHandler::Pointer_t pFFDataHandler) { m_pFFDataHandler = pFFDataHandler; }
    const FFDataHandler::Pointer_t& getFFDataHandler() const { return m_pFFDataHandler; }

    void setFormControlHelper(FormControlHelper::Pointer_t pFormControlHelper) { m_pFormControlHelper = pFormControlHelper; }
    const FormControlHelper::Pointer_t& getFormControlHelper() const { return m_pFormControlHelper; }
    const PropertyMapPtr& getProperties() { return m_pProperties; }

    ::std::vector<OUString> GetCommandParts() const;
};

struct TextAppendContext
{
    css::uno::Reference<css::text::XTextAppend> xTextAppend;
    css::uno::Reference<css::text::XTextRange> xInsertPosition;
    css::uno::Reference<css::text::XParagraphCursor> xCursor;
    ParagraphPropertiesPtr                                                        pLastParagraphProperties;

    TextAppendContext(const css::uno::Reference<css::text::XTextAppend>& xAppend, const css::uno::Reference<css::text::XTextCursor>& xCur)
        : xTextAppend(xAppend)
    {
        xCursor.set(xCur, css::uno::UNO_QUERY);
        xInsertPosition.set(xCursor, css::uno::UNO_QUERY);
    }
};

struct AnchoredContext
{
    css::uno::Reference<css::text::XTextContent> xTextContent;
    bool bToRemove;

    AnchoredContext(const css::uno::Reference<css::text::XTextContent>& xContent)
        : xTextContent(xContent), bToRemove(false)
    {
    }
};

typedef std::shared_ptr<FieldContext>  FieldContextPtr;

/*-------------------------------------------------------------------------
    extended tab stop struct
  -----------------------------------------------------------------------*/
struct DeletableTabStop : public css::style::TabStop
{
    bool bDeleted;
    DeletableTabStop()
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
/*-------------------------------------------------------------------------
    /// helper to remember bookmark start position
  -----------------------------------------------------------------------*/
struct BookmarkInsertPosition
{
    bool                                                                    m_bIsStartOfText;
    OUString                                                         m_sBookmarkName;
    css::uno::Reference<css::text::XTextRange> m_xTextRange;
    BookmarkInsertPosition(bool bIsStartOfText, const OUString& rName, css::uno::Reference<css::text::XTextRange> const& xTextRange):
        m_bIsStartOfText( bIsStartOfText ),
        m_sBookmarkName( rName ),
        m_xTextRange( xTextRange )
     {}
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
    bool        bIsOn;
    sal_Int32   nDistance;
    sal_Int32   nInterval;
    bool        bRestartAtEachPage;
    sal_Int32   nStartValue;
    LineNumberSettings() :
        bIsOn(false)
        ,nDistance(0)
        ,nInterval(0)
        ,bRestartAtEachPage(true)
        ,nStartValue(1)
    {}

};

/// Contains information about a table that will be potentially converted to a floating one at the section end.
struct FloatingTableInfo
{
    css::uno::Reference<css::text::XTextRange> m_xStart;
    css::uno::Reference<css::text::XTextRange> m_xEnd;
    css::uno::Sequence<css::beans::PropertyValue> m_aFrameProperties;
    sal_Int32 m_nTableWidth;

    FloatingTableInfo(css::uno::Reference<css::text::XTextRange> const& xStart,
            css::uno::Reference<css::text::XTextRange> const& xEnd,
            const css::uno::Sequence<css::beans::PropertyValue>& aFrameProperties,
            sal_Int32 nTableWidth)
        : m_xStart(xStart),
        m_xEnd(xEnd),
        m_aFrameProperties(aFrameProperties),
        m_nTableWidth(nTableWidth)
    {
    }
    css::uno::Any getPropertyValue(const OUString &propertyName);
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
class DomainMapper_Impl
{
public:
    typedef std::map < OUString, BookmarkInsertPosition > BookmarkMap_t;

private:
    SourceDocumentType                                                              m_eDocumentType;
    DomainMapper&                                                                   m_rDMapper;
    css::uno::Reference<css::text::XTextDocument> m_xTextDocument;
    css::uno::Reference<css::beans::XPropertySet> m_xDocumentSettings;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::container::XNameContainer> m_xPageStyles;
    css::uno::Reference<css::text::XText> m_xBodyText;
    css::uno::Reference<css::text::XTextContent> m_xEmbedded;

    std::stack<TextAppendContext>                                                   m_aTextAppendStack;
    std::stack<AnchoredContext>                                                     m_aAnchoredStack;
    std::stack<HeaderFooterContext>                                                 m_aHeaderFooterStack;
    std::stack<FieldContextPtr>                                                     m_aFieldStack;
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
    bool                                                                            m_bTOCPageRef;
    bool                                                                            m_bStartGenericField;
    bool                                                                            m_bTextInserted;
    LineNumberSettings                                                              m_aLineNumberSettings;

    BookmarkMap_t                                                                   m_aBookmarkMap;
    OUString                                                                        m_sCurrentBkmkId;
    OUString                                                                        m_sCurrentBkmkName;

    PageMar                                                                        m_aPageMargins;
    SymbolData                                                                      m_aSymbolData;

    // TableManagers are stacked: one for each stream to avoid any confusion
    std::stack< std::shared_ptr< DomainMapperTableManager > > m_aTableManagers;
    std::shared_ptr<DomainMapperTableHandler> m_pTableHandler;

    //each context needs a stack of currently used attributes
    std::stack<PropertyMapPtr>  m_aPropertyStacks[NUMBER_OF_CONTEXTS];
    std::stack<ContextType> m_aContextStack;
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
    OUString                 m_sCurrentParaStyleId;
    bool                            m_bInStyleSheetImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInAnyTableImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInHeaderFooterImport;
    bool                            m_bDiscardHeaderFooter;
    bool                            m_bInFootOrEndnote;
    /// Did we get a <w:separator/> for this footnote already?
    bool                            m_bSeenFootOrEndnoteSeparator;

    bool                            m_bLineNumberingSet;
    bool                            m_bIsInFootnoteProperties;
    bool                            m_bIsCustomFtnMark;

    RubyInfo                        m_aRubyInfo;
    //registered frame properties
    std::vector<css::beans::PropertyValue> m_aFrameProperties;
    css::uno::Reference<css::text::XTextRange> m_xFrameStartRange;
    css::uno::Reference<css::text::XTextRange> m_xFrameEndRange;

    // Redline stack
    std::stack< std::vector< RedlineParamsPtr > > m_aRedlines;
    // The redline currently read, may be also stored by a context instead of m_aRedlines.
    RedlineParamsPtr                m_currentRedline;
    RedlineParamsPtr                m_pParaMarkerRedline;
    bool                            m_bIsParaMarkerChange;

    /// If the current paragraph has any runs.
    bool                            m_bParaChanged;
    bool                            m_bIsFirstParaInSection;
    bool                            m_bDummyParaAddedForTableInSection;
    bool                            m_bTextFrameInserted;
    bool                            m_bIsLastParagraphFramed;
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

    css::uno::Reference< css::text::XTextCursor > xTOCMarkerCursor;
    css::uno::Reference< css::text::XTextCursor > mxTOCTextCursor;

    //annotation import
    css::uno::Reference< css::beans::XPropertySet > m_xAnnotationField;
    sal_Int32 m_nAnnotationId;
    std::unordered_map< sal_Int32, AnnotationPosition > m_aAnnotationPositions;

    void GetCurrentLocale(css::lang::Locale& rLocale);
    void SetNumberFormat(const OUString& rCommand, css::uno::Reference<css::beans::XPropertySet> const& xPropertySet, bool bDetectFormat = false);
    css::uno::Reference<css::beans::XPropertySet> FindOrCreateFieldMaster(const sal_Char* pFieldMasterService, const OUString& rFieldMasterName) throw(css::uno::Exception);
    css::uno::Reference<css::beans::XPropertySet> const & GetDocumentSettings();

    std::map<sal_Int32, css::uno::Any> deferredCharacterProperties;
    SmartTagHandler m_aSmartTagHandler;

public:
    css::uno::Reference<css::text::XTextRange> m_xInsertTextRange;
private:
    bool m_bIsNewDoc;
public:
    DomainMapper_Impl(
            DomainMapper& rDMapper,
            css::uno::Reference < css::uno::XComponentContext > const& xContext,
            css::uno::Reference< css::lang::XComponent > const& xModel,
            SourceDocumentType eDocumentType,
            utl::MediaDescriptor& rMediaDesc);
    virtual ~DomainMapper_Impl();

    SectionPropertyMap* GetLastSectionContext( )
    {
        return dynamic_cast< SectionPropertyMap* >( m_pLastSectionContext.get( ) );
    }

    css::uno::Reference<css::container::XNameContainer> const & GetPageStyles();
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

    static void CreateRedline(css::uno::Reference<css::text::XTextRange> const& xRange, const RedlineParamsPtr& pRedline);

    void CheckParaMarkerRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void CheckRedline(css::uno::Reference<css::text::XTextRange> const& xRange);

    void StartParaMarkerChange( );
    void EndParaMarkerChange( );
    void ChainTextFrames();

    void RemoveDummyParaForTableInSection();
    void AddDummyParaForTableInSection();
    void RemoveLastParagraph( );
    void SetIsLastParagraphInSection( bool bIsLast );
    bool GetIsLastParagraphInSection() { return m_bIsLastParaInSection;}
    void SetRubySprmId( sal_uInt32 nSprmId) { m_aRubyInfo.nSprmId = nSprmId ; }
    void SetRubyText( OUString &sText,OUString &sStyle) {
        m_aRubyInfo.sRubyText = sText;
        m_aRubyInfo.sRubyStyle = sStyle;
    }
    const RubyInfo & GetRubyInfo() const { return m_aRubyInfo;}
    void SetRubyInfo(const RubyInfo & rInfo) { m_aRubyInfo = rInfo;}

    void SetIsLastSectionGroup( bool bIsLast );
    bool GetIsLastSectionGroup() { return m_bIsLastSectionGroup;}
    void SetIsFirstParagraphInSection( bool bIsFirst );
    bool GetIsFirstParagraphInSection() { return m_bIsFirstParaInSection;}
    void SetIsDummyParaAddedForTableInSection( bool bIsAdded );
    bool GetIsDummyParaAddedForTableInSection() { return m_bDummyParaAddedForTableInSection;}
    void SetIsTextFrameInserted( bool bIsInserted );
    bool GetIsTextFrameInserted() { return m_bTextFrameInserted;}
    void SetIsLastParagraphFramed( bool bIsFramed ) { m_bIsLastParagraphFramed = bIsFramed; }
    bool GetIsLastParagraphFramed() { return m_bIsLastParagraphFramed; }
    void SetParaSectpr(bool bParaSectpr);
    bool GetParaSectpr() { return m_bParaSectpr;}

    void SetSymbolChar( sal_Int32 nSymbol) { m_aSymbolData.cSymbol = sal_Unicode(nSymbol); }
    void SetSymbolFont( OUString &rName ) { m_aSymbolData.sFont = rName; }
    const SymbolData & GetSymbolData() { return m_aSymbolData;}

    /// Setter method for m_bSdt.
    void SetSdt(bool bSdt);
    /// Getter method for m_bSdt.
    bool GetSdt() { return m_bSdt;}
    bool GetParaChanged() { return m_bParaChanged;}

    void deferBreak( BreakType deferredBreakType );
    bool isBreakDeferred( BreakType deferredBreakType );
    void clearDeferredBreaks();
    void clearDeferredBreak(BreakType deferredBreakType);

    void setSdtEndDeferred(bool bSdtEndDeferred);
    bool isSdtEndDeferred();
    void setParaSdtEndDeferred(bool bParaSdtEndDeferred);
    bool isParaSdtEndDeferred();

    void finishParagraph( const PropertyMapPtr& pPropertyMap );
    void appendTextPortion( const OUString& rString, const PropertyMapPtr& pPropertyMap );
    void appendTextContent(const css::uno::Reference<css::text::XTextContent>&, const css::uno::Sequence<css::beans::PropertyValue>&);
    void appendOLE( const OUString& rStreamName, const OLEHandlerPtr& pOleHandler );
    void appendStarMath( const Value& v );
    css::uno::Reference<css::beans::XPropertySet> appendTextSectionAfter(css::uno::Reference<css::text::XTextRange>& xBefore);

    // push the new properties onto the stack and make it the 'current' property map
    void    PushProperties(ContextType eId);
    void    PushStyleProperties(const PropertyMapPtr& pStyleProperties);
    void    PushListProperties(const PropertyMapPtr& pListProperties);
    void    PopProperties(ContextType eId);

    ContextType GetTopContextType() const { return m_aContextStack.top(); }
    const PropertyMapPtr& GetTopContext()
    {
        return m_pTopContext;
    }
    PropertyMapPtr GetTopContextOfType(ContextType eId);

    css::uno::Reference<css::text::XTextAppend> GetTopTextAppend();
    FieldContextPtr GetTopFieldContext();

    FontTablePtr const & GetFontTable()
    {
        if(!m_pFontTable)
            m_pFontTable.reset(new FontTable());
         return m_pFontTable;
    }
    StyleSheetTablePtr const & GetStyleSheetTable()
    {
        if(!m_pStyleSheetTable)
            m_pStyleSheetTable.reset(new StyleSheetTable( m_rDMapper, m_xTextDocument, m_bIsNewDoc ));
        return m_pStyleSheetTable;
    }
    ListsManager::Pointer const & GetListTable();
    ThemeTablePtr const & GetThemeTable()
    {
        if(!m_pThemeTable)
            m_pThemeTable.reset( new ThemeTable );
        return m_pThemeTable;
    }

    SettingsTablePtr const & GetSettingsTable()
    {
        if( !m_pSettingsTable )
            m_pSettingsTable.reset( new SettingsTable );
        return m_pSettingsTable;
    }

    GraphicImportPtr const & GetGraphicImport( GraphicImportType eGraphicImportType );
    void            ResetGraphicImport();
    // this method deletes the current m_pGraphicImport after import
    void    ImportGraphic(const writerfilter::Reference< Properties>::Pointer_t&, GraphicImportType eGraphicImportType );

    void InitTabStopFromStyle(const css::uno::Sequence<css::style::TabStop>& rInitTabStops);
    void    IncorporateTabStop( const DeletableTabStop &aTabStop );
    css::uno::Sequence<css::style::TabStop> GetCurrentTabStopAndClear();

    void        SetCurrentParaStyleId(const OUString& sStringValue) {m_sCurrentParaStyleId = sStringValue;}
    const OUString& GetCurrentParaStyleId() const {return m_sCurrentParaStyleId;}

    css::uno::Any GetPropertyFromStyleSheet(PropertyIds eId);
    void        SetStyleSheetImport( bool bSet ) { m_bInStyleSheetImport = bSet;}
    bool        IsStyleSheetImport()const { return m_bInStyleSheetImport;}
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
    bool IsInHeaderFooter() const { return m_bInHeaderFooterImport; }

    void PushFootOrEndnote( bool bIsFootnote );
    void PopFootOrEndnote();
    bool IsInFootOrEndnote() const { return m_bInFootOrEndnote; }
    /// Got a <w:separator/>.
    void SeenFootOrEndnoteSeparator();

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
    void AppendFieldCommand(OUString& rPartOfCommand);
    void handleRubyEQField( const FieldContextPtr& pContext);
    void handleFieldAsk
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    void handleAutoNum
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    static void handleAuthor
        (OUString const& rFirstParam,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties,
        FieldId eFieldId);
    void handleDocProperty
        (const FieldContextPtr& pContext,
        OUString const& rFirstParam,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties);
    void handleToc
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties,
        const OUString & sTOCServiceName);
    void handleIndex
        (const FieldContextPtr& pContext,
        css::uno::Reference< css::uno::XInterface > & xFieldInterface,
        css::uno::Reference< css::beans::XPropertySet > const& xFieldProperties,
        const OUString & sTOCServiceName);

    void handleBibliography
        (const FieldContextPtr& pContext,
        const OUString & sTOCServiceName);
    /// The field command has to be closed (cFieldSep appeared).
    void CloseFieldCommand();
    //the _current_ fields require a string type result while TOCs accept richt results
    bool IsFieldResultAsString();
    void AppendFieldResult(OUString const& rResult);
    //apply the result text to the related field
    void SetFieldResult(OUString const& rResult);
    // set FFData of top field context
    void SetFieldFFData( const FFDataHandler::Pointer_t& pFFDataHandler );
    /// The end of field is reached (cFieldEnd appeared) - the command might still be open.
    void PopFieldContext();

    void SetBookmarkName( const OUString& rBookmarkName );
    void StartOrEndBookmark( const OUString& rId );

    void AddAnnotationPosition(
        const bool bStart,
        const sal_Int32 nAnnotationId );

    bool hasTableManager() const
    {
        return !m_aTableManagers.empty();
    }

    DomainMapperTableManager& getTableManager()
    {
        std::shared_ptr< DomainMapperTableManager > pMngr = m_aTableManagers.top();
        return *pMngr.get( );
    }

    void appendTableManager( )
    {
        std::shared_ptr<DomainMapperTableManager> pMngr(new DomainMapperTableManager());
        m_aTableManagers.push( pMngr );
    }

    void appendTableHandler( )
    {
        if (m_pTableHandler.get())
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

    const LineNumberSettings& GetLineNumberSettings() const { return m_aLineNumberSettings;}
    void SetLineNumberSettings(const LineNumberSettings& rSet) { m_aLineNumberSettings = rSet;}

    void SetInFootnoteProperties(bool bSet) { m_bIsInFootnoteProperties = bSet;}
    bool IsInFootnoteProperties() const { return m_bIsInFootnoteProperties;}

    void SetCustomFtnMark(bool bSet) { m_bIsCustomFtnMark = bSet; }
    bool IsCustomFtnMark() const { return m_bIsCustomFtnMark;  }

    bool IsInComments() const { return m_bIsInComments; };

    void CheckUnregisteredFrameConversion( );

    void RegisterFrameConversion(css::uno::Reference<css::text::XTextRange> const& xFrameStartRange,
                                 css::uno::Reference<css::text::XTextRange> const& xFrameEndRange,
                                 const std::vector<css::beans::PropertyValue>& aFrameProperties);
    void ExecuteFrameConversion();

    void AddNewRedline( sal_uInt32 sprmId );

    sal_Int32 GetCurrentRedlineToken( );
    void SetCurrentRedlineAuthor( const OUString& sAuthor );
    void SetCurrentRedlineDate( const OUString& sDate );
    void SetCurrentRedlineId( sal_Int32 nId );
    void SetCurrentRedlineToken( sal_Int32 nToken );
    void SetCurrentRedlineRevertProperties( const css::uno::Sequence<css::beans::PropertyValue>& aProperties );
    void SetCurrentRedlineIsRead();
    void RemoveTopRedline( );
    void ResetParaMarkerRedline( );
    void SetCurrentRedlineInitials( const OUString& sInitials );
    bool IsFirstRun() { return m_bIsFirstRun;}
    void SetIsFirstRun(bool bval) { m_bIsFirstRun = bval;}
    bool IsOutsideAParagraph() { return m_bIsOutsideAParagraph;}
    void SetIsOutsideAParagraph(bool bval) { m_bIsOutsideAParagraph = bval;}

    void ApplySettingsTable();
    SectionPropertyMap * GetSectionContext();
    /// If the current paragraph has a numbering style associated, this method returns its character style (part of the numbering rules)
    css::uno::Reference<css::beans::XPropertySet> GetCurrentNumberingCharStyle();
    /// If the current paragraph has a numbering style associated, this method returns its numbering rules
    css::uno::Reference<css::container::XIndexAccess> GetCurrentNumberingRules(sal_Int32* pListLevel = nullptr);

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

    /// Get a property of the current numbering style's current level.
    sal_Int32 getCurrentNumberingProperty(const OUString& aProp);

    /// If we're importing into a new document, or just pasting to an existing one.
    bool IsNewDoc() { return m_bIsNewDoc;}

    /// If we're inside <w:rPr>, inside <w:style w:type="table">
    bool m_bInTableStyleRunProps;

    std::shared_ptr<SdtHelper> m_pSdtHelper;

    /// Document background color, applied to every page style.
    boost::optional<sal_Int32> m_oBackgroundColor;

    /**
     * This contains the raw table depth. m_nTableDepth > 0 is the same as
     * getTableManager().isInTable(), unless we're in the first paragraph of a
     * table, or first paragraph after a table, as the table manager is only
     * updated once we ended the paragraph (and know if the para has the
     * inTbl SPRM or not).
     */
    sal_Int32 m_nTableDepth;

    /// If the document has a footnote separator.
    bool m_bHasFtnSep;

    /// If the next newline should be ignored, used by the special footnote separator paragraph.
    bool m_bIgnoreNextPara;
    /// If the next tab should be ignored, used for footnotes.
    bool m_bIgnoreNextTab;
    bool m_bFrameBtLr; ///< Bottom to top, left to right text frame direction is requested for the current text frame.
    /// Pending floating tables: they may be converted to text frames at the section end.
    std::vector<FloatingTableInfo> m_aPendingFloatingTables;

    /// Append a property to a sub-grabbag if necessary (e.g. 'lineRule', 'auto')
    void appendGrabBag(std::vector<css::beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, const OUString& aValue);
    void appendGrabBag(std::vector<css::beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, std::vector<css::beans::PropertyValue>& rValue);

    /// Enable, disable an check status of grabbags
    void enableInteropGrabBag(const OUString& aName);
    void disableInteropGrabBag();
    bool isInteropGrabBagEnabled();

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
    bool isInIndexContext() { return m_bStartIndex;}
    bool isInBibliographyContext() { return m_bStartBibliography;}
    SmartTagHandler& getSmartTagHandler() { return m_aSmartTagHandler; }

    void substream(Id rName, ::writerfilter::Reference<Stream>::Pointer_t const& ref);

    /// If the document needs to split paragraph.
    bool m_bIsSplitPara;

    /// Check if "SdtEndBefore" property is set
    bool IsSdtEndBefore();

    bool IsDiscardHeaderFooter();

private:
    void PushPageHeaderFooter(bool bHeader, SectionPropertyMap::PageType eType);
    std::vector<css::uno::Reference< css::drawing::XShape > > m_vTextFramesForChaining ;
    /// Current paragraph had at least one field in it.
    bool m_bParaHadField;
};

} //namespace dmapper
} //namespace writerfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
