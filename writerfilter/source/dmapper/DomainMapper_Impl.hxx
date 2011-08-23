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
#ifndef INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX
#define INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vector>
#include <stack>

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
#include <SettingsTable.hxx>
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
        }
        namespace beans{ class XPropertySet;}
}}}

namespace writerfilter {
namespace dmapper {

using namespace com::sun::star;

//#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
//sal_Int32 lcl_convertToMM100(sal_Int32 _t);

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

/*-- 14.06.2006 07:42:52---------------------------------------------------
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
/*-----------------29.01.2007 11:47-----------------
   field stack element
 * --------------------------------------------------*/
class FieldContext
{
    bool                                                                            m_bFieldCommandCompleted;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >          m_xStartRange;

    ::rtl::OUString                                                                 m_sCommand;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >          m_xTextField;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTOC;//TOX
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTC;//TOX entry
    ::rtl::OUString                                                                 m_sHyperlinkURL;
    FFDataHandler::Pointer_t                                                        m_pFFDataHandler;
    FormControlHelper::Pointer_t                                                    m_pFormControlHelper;

public:
    FieldContext(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xStart);
    ~FieldContext();

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetStartRange() const { return m_xStartRange; }

    void                    AppendCommand(const ::rtl::OUString& rPart);
    const ::rtl::OUString&  GetCommand() const {return m_sCommand; }

    void                    SetCommandCompleted() { m_bFieldCommandCompleted = true; }
    bool                    IsCommandCompleted() const { return m_bFieldCommandCompleted;    }

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >      GetTextField() const { return m_xTextField;}
    void    SetTextField(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField > xTextField) { m_xTextField = xTextField;}

    void    SetTOC( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xTOC ) { m_xTOC = xTOC; }
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   GetTOC() { return m_xTOC; }

    void    SetTC( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xTC ) { m_xTC = xTC; }
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   GetTC( ) { return m_xTC; }

    void    SetHyperlinkURL( const ::rtl::OUString& rURL ) { m_sHyperlinkURL = rURL; }
    const ::rtl::OUString&                                                      GetHyperlinkURL() { return m_sHyperlinkURL; }
    
    void setFFDataHandler(FFDataHandler::Pointer_t pFFDataHandler) { m_pFFDataHandler = pFFDataHandler; }
    FFDataHandler::Pointer_t getFFDataHandler() const { return m_pFFDataHandler; }

    void setFormControlHelper(FormControlHelper::Pointer_t pFormControlHelper) { m_pFormControlHelper = pFormControlHelper; }
    FormControlHelper::Pointer_t getFormControlHelper() const { return m_pFormControlHelper; }

    ::std::vector<rtl::OUString> GetCommandParts() const;
};

struct TextAppendContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextAppend >       xTextAppend;
    ParagraphPropertiesPtr                                                        pLastParagraphProperties;

    TextAppendContext( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextAppend >& xAppend ) :
        xTextAppend( xAppend ){}
};

typedef boost::shared_ptr<FieldContext>  FieldContextPtr;

typedef std::stack<ContextType>                 ContextStack;
typedef std::stack<PropertyMapPtr>              PropertyStack;
typedef std::stack< TextAppendContext >         TextAppendStack;
typedef std::stack<FieldContextPtr>                FieldStack;
typedef std::stack< com::sun::star::uno::Reference< com::sun::star::text::XTextContent > >  TextContentStack;

/*-- 18.07.2006 08:49:08---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        sal_Int32 GetData( Id nName );
        void      SetData( Id nName, sal_Int32 nValue );
};

/*-- 17.07.2006 09:14:13---------------------------------------------------
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
/*-- 12.06.2007 07:15:31---------------------------------------------------
    /// helper to remember bookmark start position
  -----------------------------------------------------------------------*/
struct BookmarkInsertPosition
{
    bool                                                                    m_bIsStartOfText;
    ::rtl::OUString                                                         m_sBookmarkName;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  m_xTextRange;
    BookmarkInsertPosition(bool bIsStartOfText, const ::rtl::OUString& rName, ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange):
        m_bIsStartOfText( bIsStartOfText ),
        m_sBookmarkName( rName ),
        m_xTextRange( xTextRange )
     {}
};

struct RedlineParams
{
    ::rtl::OUString m_sAuthor;
    ::rtl::OUString m_sDate;
    sal_Int32       m_nId;
    sal_Int32       m_nToken;
};
typedef boost::shared_ptr< RedlineParams > RedlineParamsPtr;

/*-- 03.03.2008 11:01:38---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
class DomainMapper;
class DomainMapper_Impl
{
public:
    typedef TableManager< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >, PropertyMapPtr > TableManager_t;
    typedef TableDataHandler< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >, TablePropertyMapPtr > TableDataHandler_t;
    typedef std::map < ::rtl::OUString, BookmarkInsertPosition > BookmarkMap_t;

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

    TextContentStack
              m_aAnchoredStack; 

    FieldStack                                                                      m_aFieldStack;
    bool                                                                            m_bFieldMode;
    bool                                                                            m_bSetUserFieldContent;
    bool                                                                            m_bIsFirstSection;
    bool                                                                            m_bIsColumnBreakDeferred;
    bool                                                                            m_bIsPageBreakDeferred;
    bool                                                                            m_bIsInShape;
    bool                                                                            m_bShapeContextAdded;

    LineNumberSettings                                                              m_aLineNumberSettings;

    BookmarkMap_t                                                                   m_aBookmarkMap;

    _PageMar                                                                        m_aPageMargins;


    // TableManagers are stacked: one for each stream to avoid any confusion
    std::stack< boost::shared_ptr< DomainMapperTableManager > > m_aTableManagers;

    //each context needs a stack of currently used attributes
    FIB                     m_aFIB;
    PropertyStack           m_aPropertyStacks[NUMBER_OF_CONTEXTS];
    ContextStack            m_aContextStack;
    FontTablePtr            m_pFontTable;
    ListsManager::Pointer   m_pListTable;
    StyleSheetTablePtr      m_pStyleSheetTable;
    ThemeTablePtr           m_pThemeTable;
    SettingsTablePtr        m_pSettingsTable;
    GraphicImportPtr        m_pGraphicImport;


    PropertyMapPtr                  m_pTopContext;
    PropertyMapPtr           m_pLastSectionContext;

    ::std::vector<DeletableTabStop> m_aCurrentTabStops;
    sal_uInt32                      m_nCurrentTabStopIndex;
    ::rtl::OUString                 m_sCurrentParaStyleId;
    bool                            m_bInStyleSheetImport; //in import of fonts, styles, lists or lfos
    bool                            m_bInAnyTableImport; //in import of fonts, styles, lists or lfos

    bool                            m_bLineNumberingSet;
    bool                            m_bIsInFootnoteProperties;
    bool                            m_bIsCustomFtnMark;

    //registered frame properties
    ::com::sun::star::uno::Sequence< beans::PropertyValue >   m_aFrameProperties;
    ::com::sun::star::uno::Reference< text::XTextRange >      m_xFrameStartRange;
    ::com::sun::star::uno::Reference< text::XTextRange >      m_xFrameEndRange;

    // Redline stack
    std::vector< RedlineParamsPtr > m_aRedlines;
    RedlineParamsPtr                m_pParaRedline;
    bool                            m_bIsParaChange;

    bool                            m_bParaChanged;
    bool                            m_bIsLastParaInSection;

    //annotation import
    uno::Reference< beans::XPropertySet >                                      m_xAnnotationField;

    void                            GetCurrentLocale(::com::sun::star::lang::Locale& rLocale);
    void                            SetNumberFormat( const ::rtl::OUString& rCommand,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropertySet );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    FindOrCreateFieldMaster( const sal_Char* pFieldMasterService,
                                                            const ::rtl::OUString& rFieldMasterName )
                                                                throw(::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       GetDocumentSettings();

public:
    DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference < uno::XComponentContext >  xContext,
            uno::Reference< lang::XComponent >  xModel,
            SourceDocumentType eDocumentType );
    DomainMapper_Impl();
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
    void SetDocumentSettingsProperty( const ::rtl::OUString& rPropName, const uno::Any& rValue );

    void CreateRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange, RedlineParamsPtr& pRedline  );

    void CheckParaRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange );

    void CheckRedline( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xRange );

    void StartParaChange( );
    void EndParaChange( );

    void RemoveLastParagraph( );
    void SetIsLastParagraphInSection( bool bIsLast );

    void deferBreak( BreakType deferredBreakType );
    bool isBreakDeferred( BreakType deferredBreakType );
    void clearDeferredBreaks();
    void finishParagraph( PropertyMapPtr pPropertyMap );
    void appendTextPortion( const ::rtl::OUString& rString, PropertyMapPtr pPropertyMap );
    void appendTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >,
                                const uno::Sequence< beans::PropertyValue >  );
    void appendOLE( const ::rtl::OUString& rStreamName, OLEHandlerPtr pOleHandler );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > appendTextSectionAfter(
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xBefore );

//    void appendTextSection();

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
            m_pStyleSheetTable.reset(new StyleSheetTable( m_rDMapper, m_xTextDocument ));
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

    void        SetCurrentParaStyleId(::rtl::OUString sStringValue) {m_sCurrentParaStyleId = sStringValue;}
    ::rtl::OUString   GetCurrentParaStyleId() const {return m_sCurrentParaStyleId;}

    ::com::sun::star::uno::Any    GetPropertyFromStyleSheet(PropertyIds eId);
    void        SetStyleSheetImport( bool bSet ) { m_bInStyleSheetImport = bSet;}
    bool        IsStyleSheetImport()const { return m_bInStyleSheetImport;}
    void        SetAnyTableImport( bool bSet ) { m_bInAnyTableImport = bSet;}
    bool        IsAnyTableImport()const { return m_bInAnyTableImport;}

    void PushShapeContext( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    void PopShapeContext();

    void PushPageHeader(SectionPropertyMap::PageType eType);
    void PushPageFooter(SectionPropertyMap::PageType eType);

    void PopPageHeaderFooter();

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
    void AppendFieldCommand(::rtl::OUString& rPartOfCommand);
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
        uno::Reference< beans::XPropertySet > xFieldProperties);
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
        const ::rtl::OUString & sTOCServiceName);
    //the field command has to be closed (0x14 appeared)
    void CloseFieldCommand();
    //the _current_ fields require a string type result while TOCs accept richt results
    bool IsFieldResultAsString();
    //apply the result text to the related field
    void SetFieldResult( ::rtl::OUString& rResult );
    // set FFData of top field context
    void SetFieldFFData( FFDataHandler::Pointer_t pFFDataHandler );
    //the end of field is reached (0x15 appeared) - the command might still be open
    void PopFieldContext();

    void AddBookmark( const ::rtl::OUString& rBookmarkName, const ::rtl::OUString& rId );

    DomainMapperTableManager& getTableManager()
    {
        boost::shared_ptr< DomainMapperTableManager > pMngr = m_aTableManagers.top();
        return *pMngr.get( );
    }

    void appendTableManager( )
    {
        boost::shared_ptr< DomainMapperTableManager > pMngr(
                new DomainMapperTableManager( m_eDocumentType == DOCUMENT_OOXML ) );
        m_aTableManagers.push( pMngr );
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

    void InitPageMargins() { m_aPageMargins = _PageMar(); }
    void SetPageMarginTwip( PageMarElement eElement, sal_Int32 nValue );
    const _PageMar& GetPageMargins() const {return m_aPageMargins;}

    const LineNumberSettings& GetLineNumberSettings() const { return m_aLineNumberSettings;}
    void SetLineNumberSettings(const LineNumberSettings& rSet) { m_aLineNumberSettings = rSet;}

    void SetInFootnoteProperties(bool bSet) { m_bIsInFootnoteProperties = bSet;}
    bool IsInFootnoteProperties() const { return m_bIsInFootnoteProperties;}

    void SetCustomFtnMark(bool bSet) { m_bIsCustomFtnMark = bSet; }
    bool IsCustomFtnMark() const { return m_bIsCustomFtnMark;  }

    void RegisterFrameConversion(
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xFrameStartRange,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xFrameEndRange,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFrameProperties
        );
    bool ExecuteFrameConversion();

    void AddNewRedline( );

    RedlineParamsPtr GetTopRedline( );
    
    sal_Int32 GetCurrentRedlineToken( ); 
    void SetCurrentRedlineAuthor( rtl::OUString sAuthor );
    void SetCurrentRedlineDate( rtl::OUString sDate );
    void SetCurrentRedlineId( sal_Int32 nId );
    void SetCurrentRedlineToken( sal_Int32 nToken );
    void RemoveCurrentRedline( );
    void ResetParaRedline( );
    
    void ApplySettingsTable();
    SectionPropertyMap * GetSectionContext();
};
} //namespace dmapper
} //namespace writerfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
