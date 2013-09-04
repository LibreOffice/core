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
#ifndef INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX
#define INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX

#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vector>
#include <stack>
#include <boost/optional.hpp>

#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#include <dmapper/DomainMapper.hxx>
#include <DomainMapperTableManager.hxx>
#include <PropertyMap.hxx>
#include <FontTable.hxx>
#include <NumberingManager.hxx>
#include <StyleSheetTable.hxx>
#include <SettingsTable.hxx>
#include <ThemeTable.hxx>
#include <GraphicImport.hxx>
#include <OLEHandler.hxx>
#include <FFDataHandler.hxx>
#include <FormControlHelper.hxx>
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

using namespace com::sun::star;

class SdtHelper;

struct _PageMar
{
    sal_Int32 top;
    sal_Int32 right;
    sal_Int32 bottom;
    sal_Int32 left;
    sal_Int32 header;
    sal_Int32 footer;
    sal_Int32 gutter;
    public:
        _PageMar();
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
    CONTEXT_LIST,
    NUMBER_OF_CONTEXTS
};

enum BreakType
{
    PAGE_BREAK,
    COLUMN_BREAK
};
/*--------------------------------------------------
   field stack element
 * --------------------------------------------------*/
class FieldContext
{
    bool                                                                            m_bFieldCommandCompleted;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >          m_xStartRange;

    OUString                                                                 m_sCommand;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >          m_xTextField;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFormField >          m_xFormField;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTOC;//TOX
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTC;//TOX entry
    OUString                                                                 m_sHyperlinkURL;
    FFDataHandler::Pointer_t                                                        m_pFFDataHandler;
    FormControlHelper::Pointer_t                                                    m_pFormControlHelper;

public:
    FieldContext(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xStart);
    ~FieldContext();

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetStartRange() const { return m_xStartRange; }

    void                    AppendCommand(const OUString& rPart);
    const OUString&  GetCommand() const {return m_sCommand; }

    void                    SetCommandCompleted() { m_bFieldCommandCompleted = true; }
    bool                    IsCommandCompleted() const { return m_bFieldCommandCompleted;    }

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >      GetTextField() const { return m_xTextField;}
    void    SetTextField(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField > xTextField) { m_xTextField = xTextField;}
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFormField >      GetFormField() const { return m_xFormField;}
    void    SetFormField(::com::sun::star::uno::Reference< ::com::sun::star::text::XFormField > xFormField) { m_xFormField = xFormField;}

    void    SetTOC( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xTOC ) { m_xTOC = xTOC; }
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   GetTOC() { return m_xTOC; }

    void    SetTC( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xTC ) { m_xTC = xTC; }
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   GetTC( ) { return m_xTC; }

    void    SetHyperlinkURL( const OUString& rURL ) { m_sHyperlinkURL = rURL; }
    const OUString&                                                      GetHyperlinkURL() { return m_sHyperlinkURL; }

    void setFFDataHandler(FFDataHandler::Pointer_t pFFDataHandler) { m_pFFDataHandler = pFFDataHandler; }
    FFDataHandler::Pointer_t getFFDataHandler() const { return m_pFFDataHandler; }

    void setFormControlHelper(FormControlHelper::Pointer_t pFormControlHelper) { m_pFormControlHelper = pFormControlHelper; }
    FormControlHelper::Pointer_t getFormControlHelper() const { return m_pFormControlHelper; }

    ::std::vector<OUString> GetCommandParts() const;
};

struct TextAppendContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextAppend >       xTextAppend;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >        xInsertPosition;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XParagraphCursor >  xCursor;
    ParagraphPropertiesPtr                                                        pLastParagraphProperties;

    TextAppendContext( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextAppend >& xAppend,
           const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >& xCur ) :
        xTextAppend( xAppend )
    {
        xCursor.set(xCur, uno::UNO_QUERY);
        xInsertPosition.set(xCursor, uno::UNO_QUERY);
    }
};

struct AnchoredContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >       xTextContent;
    bool                                                                           bToRemove;

    AnchoredContext( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) :
        xTextContent( xContent ), bToRemove( false ) {}
};

typedef boost::shared_ptr<FieldContext>  FieldContextPtr;

typedef std::stack<ContextType>                 ContextStack;
typedef std::stack<PropertyMapPtr>              PropertyStack;
typedef std::stack< TextAppendContext >         TextAppendStack;
typedef std::stack<FieldContextPtr>                FieldStack;
typedef std::stack< AnchoredContext >           TextContentStack;



class FIB
{
    sal_Int32   aFIBData[ NS_rtf::LN_LCBSTTBFUSSR - NS_rtf::LN_WIDENT + 1];
    sal_Int32   nLNCHS;
    public:
        FIB() :
            nLNCHS( 0 )
            {
                memset(&aFIBData, 0x00, sizeof(aFIBData));
            }

        sal_Int32 GetLNCHS() const {return nLNCHS;}
        void      SetLNCHS(sal_Int32 nValue) {nLNCHS = nValue;}
        void      SetData( Id nName, sal_Int32 nValue );
};

/*-------------------------------------------------------------------------
    extended tab stop struct
  -----------------------------------------------------------------------*/
struct DeletableTabStop : public ::com::sun::star::style::TabStop
{
    bool bDeleted;
    DeletableTabStop() :
        bDeleted( false ){}
    DeletableTabStop( const ::com::sun::star::style::TabStop& rTabStop ) :
        TabStop( rTabStop ),
            bDeleted( false ){}
};
/*-------------------------------------------------------------------------
    /// helper to remember bookmark start position
  -----------------------------------------------------------------------*/
struct BookmarkInsertPosition
{
    bool                                                                    m_bIsStartOfText;
    OUString                                                         m_sBookmarkName;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  m_xTextRange;
    BookmarkInsertPosition(bool bIsStartOfText, const OUString& rName, ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange):
        m_bIsStartOfText( bIsStartOfText ),
        m_sBookmarkName( rName ),
        m_xTextRange( xTextRange )
     {}
};

/// Stores the start/end positions of an annotation before its insertion.
struct AnnotationPosition
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  m_xStart;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  m_xEnd;
};

struct RedlineParams
{
    OUString m_sAuthor;
    OUString m_sDate;
    sal_Int32       m_nId;
    sal_Int32       m_nToken;
};
typedef boost::shared_ptr< RedlineParams > RedlineParamsPtr;



struct LineNumberSettings
{
    bool        bIsOn;
    sal_Int32   nDistance;
    sal_Int32   nInterval;
    sal_Int32   bRestartAtEachPage;
    sal_Int32   nStartValue;
    LineNumberSettings() :
        bIsOn(false)
        ,nDistance(0)
        ,nInterval(0)
        ,bRestartAtEachPage(true)
        ,nStartValue(1)
    {}

};


class DomainMapper;
class WRITERFILTER_DLLPRIVATE DomainMapper_Impl
{
public:
    typedef TableManager< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >, PropertyMapPtr > TableManager_t;
    typedef TableDataHandler< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >, TablePropertyMapPtr > TableDataHandler_t;
    typedef std::map < OUString, BookmarkInsertPosition > BookmarkMap_t;

private:
    SourceDocumentType                                                              m_eDocumentType;
    DomainMapper&                                                                   m_rDMapper;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument >       m_xTextDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDocumentSettings;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > m_xTextFactory;
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >     m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xPageStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >               m_xBodyText;

    TextAppendStack                                                                 m_aTextAppendStack;

    TextContentStack                                                                m_aAnchoredStack;

    FieldStack                                                                      m_aFieldStack;
    bool                                                                            m_bSetUserFieldContent;
    bool                                                                            m_bIsFirstSection;
    bool                                                                            m_bIsColumnBreakDeferred;
    bool                                                                            m_bIsPageBreakDeferred;

    LineNumberSettings                                                              m_aLineNumberSettings;

    BookmarkMap_t                                                                   m_aBookmarkMap;

    _PageMar                                                                        m_aPageMargins;


    // TableManagers are stacked: one for each stream to avoid any confusion
    std::stack< boost::shared_ptr< DomainMapperTableManager > > m_aTableManagers;
    TableDataHandler_t::Pointer_t m_pTableHandler;

    //each context needs a stack of currently used attributes
    FIB                     m_aFIB;
    PropertyStack           m_aPropertyStacks[NUMBER_OF_CONTEXTS];
    ContextStack            m_aContextStack;
    FontTablePtr            m_pFontTable;
    ListsManager::Pointer   m_pListTable;
    std::deque< com::sun::star::uno::Reference< com::sun::star::drawing::XShape > > m_aPendingShapes;
    StyleSheetTablePtr      m_pStyleSheetTable;
    ThemeTablePtr           m_pThemeTable;
    SettingsTablePtr        m_pSettingsTable;
    GraphicImportPtr        m_pGraphicImport;


    PropertyMapPtr                  m_pTopContext;
    PropertyMapPtr           m_pLastSectionContext;
    PropertyMapPtr           m_pLastCharacterContext;

    ::std::vector<DeletableTabStop> m_aCurrentTabStops;
    sal_uInt32                      m_nCurrentTabStopIndex;
    OUString                 m_sCurrentParaStyleId;
    bool                            m_bInStyleSheetImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInAnyTableImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInHeaderFooterImport;
    bool                            m_bDiscardHeaderFooter;

    bool                            m_bLineNumberingSet;
    bool                            m_bIsInFootnoteProperties;
    bool                            m_bIsCustomFtnMark;

    //registered frame properties
    ::com::sun::star::uno::Sequence< beans::PropertyValue >   m_aFrameProperties;
    ::com::sun::star::uno::Reference< text::XTextRange >      m_xFrameStartRange;
    ::com::sun::star::uno::Reference< text::XTextRange >      m_xFrameEndRange;

    // Redline stack
    std::stack< std::vector< RedlineParamsPtr > > m_aRedlines;
    RedlineParamsPtr                m_pParaRedline;
    bool                            m_bIsParaChange;

    /// If the current paragraph has any runs.
    bool                            m_bParaChanged;
    bool                            m_bIsFirstParaInSection;
    bool                            m_bIsLastParaInSection;
    bool                            m_bIsInComments;
    /// If the current paragraph contains section property definitions.
    bool                            m_bParaSectpr;
    bool                            m_bUsingEnhancedFields;
    /// If the current paragraph is inside a structured document element.
    bool                            m_bSdt;

    //annotation import
    uno::Reference< beans::XPropertySet >                                      m_xAnnotationField;
    AnnotationPosition                                                         m_aAnnotationPosition;

    void                            GetCurrentLocale(::com::sun::star::lang::Locale& rLocale);
    void                            SetNumberFormat( const OUString& rCommand,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropertySet );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    FindOrCreateFieldMaster( const sal_Char* pFieldMasterService,
                                                            const OUString& rFieldMasterName )
                                                                throw(::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       GetDocumentSettings();

    std::map< sal_Int32, com::sun::star::uno::Any > deferredCharacterProperties;

public:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > m_xInsertTextRange;
private:
    bool m_bIsNewDoc;

public:
    DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference < uno::XComponentContext >  xContext,
            uno::Reference< lang::XComponent >  xModel,
            SourceDocumentType eDocumentType,
            uno::Reference< text::XTextRange > xInsertTextRange,
            bool bIsNewDoc );
    virtual ~DomainMapper_Impl();

    SectionPropertyMap* GetLastSectionContext( )
    {
        return dynamic_cast< SectionPropertyMap* >( m_pLastSectionContext.get( ) );
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetPageStyles();
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >               GetBodyText();
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > GetTextFactory() const
    {
        return m_xTextFactory;
    }
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >     GetComponentContext() const
    {
        return m_xComponentContext;
    }
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument >       GetTextDocument() const
    {
        return m_xTextDocument;
    }
    void SetDocumentSettingsProperty( const OUString& rPropName, const uno::Any& rValue );

    void CreateRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange, RedlineParamsPtr& pRedline  );

    void CheckParaRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange );

    void CheckRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange );

    void StartParaChange( );
    void EndParaChange( );

    void RemoveLastParagraph( );
    void SetIsLastParagraphInSection( bool bIsLast );
    bool GetIsLastParagraphInSection();
    void SetIsFirstParagraphInSection( bool bIsFirst );
    bool GetIsFirstParagraphInSection();
    void SetParaSectpr(bool bParaSectpr);
    bool GetParaSectpr();
    /// Setter method for m_bSdt.
    void SetSdt(bool bSdt);
    /// Getter method for m_bSdt.
    bool GetSdt();
    bool GetParaChanged();

    void deferBreak( BreakType deferredBreakType );
    bool isBreakDeferred( BreakType deferredBreakType );
    void clearDeferredBreaks();
    void clearDeferredBreak(BreakType deferredBreakType);
    void finishParagraph( PropertyMapPtr pPropertyMap );
    void appendTextPortion( const OUString& rString, PropertyMapPtr pPropertyMap );
    void appendTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >,
                                const uno::Sequence< beans::PropertyValue >  );
    void appendOLE( const OUString& rStreamName, OLEHandlerPtr pOleHandler );
    void appendStarMath( const Value& v );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > appendTextSectionAfter(
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xBefore );

    FIB&    GetFIB() {return m_aFIB;}
    // push the new properties onto the stack and make it the 'current' property map
    void    PushProperties(ContextType eId);
    void    PushStyleProperties(PropertyMapPtr pStyleProperties);
    void    PushListProperties(PropertyMapPtr pListProperties);
    void    PopProperties(ContextType eId);

    ContextType GetTopContextType() const { return m_aContextStack.top(); }
    PropertyMapPtr GetTopContext()
    {
        return m_pTopContext;
    }
    PropertyMapPtr GetTopContextOfType(ContextType eId);

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextAppend >  GetTopTextAppend();

    FontTablePtr GetFontTable()
    {
        if(!m_pFontTable)
            m_pFontTable.reset(new FontTable());
         return m_pFontTable;
    }
    StyleSheetTablePtr GetStyleSheetTable()
    {
        if(!m_pStyleSheetTable)
            m_pStyleSheetTable.reset(new StyleSheetTable( m_rDMapper, m_xTextDocument, m_bIsNewDoc ));
        return m_pStyleSheetTable;
    }
    ListsManager::Pointer GetListTable();
    ThemeTablePtr GetThemeTable()
    {
        if(!m_pThemeTable)
            m_pThemeTable.reset( new ThemeTable );
        return m_pThemeTable;
    }

    SettingsTablePtr GetSettingsTable()
    {
        if( !m_pSettingsTable )
            m_pSettingsTable.reset( new SettingsTable( m_rDMapper, m_xTextFactory ) );
        return m_pSettingsTable;
    }

    GraphicImportPtr GetGraphicImport( GraphicImportType eGraphicImportType );
    void            ResetGraphicImport();
    // this method deletes the current m_pGraphicImport after import
    void    ImportGraphic(writerfilter::Reference< Properties>::Pointer_t, GraphicImportType eGraphicImportType );

    void    InitTabStopFromStyle( const ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >& rInitTabStops );
    void    ModifyCurrentTabStop( Id nId, sal_Int32 nValue);
    void    IncorporateTabStop( const DeletableTabStop &aTabStop );
    ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >     GetCurrentTabStopAndClear();
    void                                NextTabStop() {++m_nCurrentTabStopIndex;}

    void        SetCurrentParaStyleId(OUString sStringValue) {m_sCurrentParaStyleId = sStringValue;}
    OUString   GetCurrentParaStyleId() const {return m_sCurrentParaStyleId;}

    ::com::sun::star::uno::Any    GetPropertyFromStyleSheet(PropertyIds eId);
    void        SetStyleSheetImport( bool bSet ) { m_bInStyleSheetImport = bSet;}
    bool        IsStyleSheetImport()const { return m_bInStyleSheetImport;}
    void        SetAnyTableImport( bool bSet ) { m_bInAnyTableImport = bSet;}
    bool        IsAnyTableImport()const { return m_bInAnyTableImport;}
    bool        IsInShape()const { return m_aAnchoredStack.size() > 0;}

    void PushShapeContext( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    void PopShapeContext();
    /// Add a pending shape: it's currently inserted into the document, but it should be removed before the import finishes.
    void PushPendingShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    /// Get the first pending shape, if there are any.
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > PopPendingShape();

    void PushPageHeader(SectionPropertyMap::PageType eType);
    void PushPageFooter(SectionPropertyMap::PageType eType);

    void PopPageHeaderFooter();
    bool IsInHeaderFooter() const { return m_bInHeaderFooterImport; }

    void PushFootOrEndnote( bool bIsFootnote );
    void PopFootOrEndnote();

    void PushAnnotation();
    void PopAnnotation();

    //field context starts with a 0x13
    void PushFieldContext();
    //the current field context waits for the completion of the command
    bool IsOpenFieldCommand() const;
    bool IsOpenField() const;
    //collect the pieces of the command
    void AppendFieldCommand(OUString& rPartOfCommand);
    void handleFieldAsk
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties);
    void handleAutoNum
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties);
    void handleAuthor
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties,
        FieldId eFieldId);
    void handleDocProperty
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties);
    void handleToc
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties,
        const OUString & sTOCServiceName);
    //the field command has to be closed (0x14 appeared)
    void CloseFieldCommand();
    //the _current_ fields require a string type result while TOCs accept richt results
    bool IsFieldResultAsString();
    //apply the result text to the related field
    void SetFieldResult( OUString& rResult );
    // set FFData of top field context
    void SetFieldFFData( FFDataHandler::Pointer_t pFFDataHandler );
    //the end of field is reached (0x15 appeared) - the command might still be open
    void PopFieldContext();

    void AddBookmark( const OUString& rBookmarkName, const OUString& rId );

    void AddAnnotationPosition(const bool bStart);

    DomainMapperTableManager& getTableManager()
    {
        boost::shared_ptr< DomainMapperTableManager > pMngr = m_aTableManagers.top();
        return *pMngr.get( );
    }

    void appendTableManager( )
    {
        boost::shared_ptr< DomainMapperTableManager > pMngr(
                new DomainMapperTableManager( m_eDocumentType == DOCUMENT_OOXML || m_eDocumentType == DOCUMENT_RTF ) );
        m_aTableManagers.push( pMngr );
    }

    void appendTableHandler( )
    {
        if (m_pTableHandler.get())
            m_aTableManagers.top()->setHandler(m_pTableHandler);
    }

    void popTableManager( )
    {
        if ( m_aTableManagers.size( ) > 0 )
            m_aTableManagers.pop( );
    }

    void SetLineNumbering( sal_Int32 nLnnMod, sal_Int32 nLnc, sal_Int32 ndxaLnn );
    bool IsLineNumberingSet() const {return m_bLineNumberingSet;}

    DeletableTabStop                m_aCurrentTabStop;

    bool IsOOXMLImport() const { return m_eDocumentType == DOCUMENT_OOXML; }

    bool IsRTFImport() const { return m_eDocumentType == DOCUMENT_RTF; }

    void InitPageMargins() { m_aPageMargins = _PageMar(); }
    void SetPageMarginTwip( PageMarElement eElement, sal_Int32 nValue );
    const _PageMar& GetPageMargins() const {return m_aPageMargins;}

    const LineNumberSettings& GetLineNumberSettings() const { return m_aLineNumberSettings;}
    void SetLineNumberSettings(const LineNumberSettings& rSet) { m_aLineNumberSettings = rSet;}

    void SetInFootnoteProperties(bool bSet) { m_bIsInFootnoteProperties = bSet;}
    bool IsInFootnoteProperties() const { return m_bIsInFootnoteProperties;}

    void SetCustomFtnMark(bool bSet) { m_bIsCustomFtnMark = bSet; }
    bool IsCustomFtnMark() const { return m_bIsCustomFtnMark;  }

    bool IsInComments() const { return m_bIsInComments; };

    void CheckUnregisteredFrameConversion( );

    void RegisterFrameConversion(
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xFrameStartRange,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xFrameEndRange,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFrameProperties
        );
    bool ExecuteFrameConversion();

    void AddNewRedline( );

    RedlineParamsPtr GetTopRedline( );

    sal_Int32 GetCurrentRedlineToken( );
    void SetCurrentRedlineAuthor( OUString sAuthor );
    void SetCurrentRedlineDate( OUString sDate );
    void SetCurrentRedlineId( sal_Int32 nId );
    void SetCurrentRedlineToken( sal_Int32 nToken );
    void RemoveCurrentRedline( );
    void ResetParaRedline( );
    void SetCurrentRedlineInitials( OUString sInitials );

    void ApplySettingsTable();
    SectionPropertyMap * GetSectionContext();
    /// If the current paragraph has a numbering style associated, this method returns its character style (part of the numbering rules)
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> GetCurrentNumberingCharStyle();
    /// If the current paragraph has a numbering style associated, this method returns its numbering rules
    com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> GetCurrentNumberingRules(sal_Int32* pListLevel = 0);

    /**
     Used for attributes/sprms which cannot be evaluated immediatelly (e.g. they depend
     on another one that comes in the same CONTEXT_CHARACTER). The property will be processed
     again in DomainMapper::processDeferredCharacterProperties().
    */
    void deferCharacterProperty( sal_Int32 id, com::sun::star::uno::Any value );
    /**
     Processes properties deferred using deferCharacterProperty(). To be called whenever the top
     CONTEXT_CHARACTER is going to be used (e.g. by appendText()).
    */
    void processDeferredCharacterProperties();

    /// Get a property of the current numbering style's current level.
    sal_Int32 getCurrentNumberingProperty(OUString aProp);

    /// If we're importing into a new document, or just pasting to an existing one.
    bool IsNewDoc();

    /// If we're inside <w:rPr>, inside <w:style w:type="table">
    bool m_bInTableStyleRunProps;

    SdtHelper* m_pSdtHelper;

    /// Document background color, applied to every page style.
    boost::optional<sal_Int32> m_oBackgroundColor;

    /**
     * This contains the raw table depth. m_nTableDepth > 0 is the same as
     * getTableManager().isInTable(), unless we're in the first paragraph of a
     * table, or first paragraph after a table, as the table manager is only
     * updated once we ended the paragraph (and know if the para has the
     * PFInTable SPRM or not).
     */
    sal_Int32 m_nTableDepth;

    /// If the document has a footnote separator.
    bool m_bHasFtnSep;

    /// If the next newline should be ignored, used by the special footnote separator paragraph.
    bool m_bIgnoreNextPara;
};
} //namespace dmapper
} //namespace writerfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
