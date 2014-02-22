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

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLTextShapeImportHelper.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <doc.hxx>
#include <TextCursorHelper.hxx>
#include <unotext.hxx>
#include <unotextrange.hxx>
#include "unocrsr.hxx"
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <editsh.hxx>
#include "xmlimp.hxx"
#include "xmltexti.hxx"
#include <xmloff/DocumentSettingsContext.hxx>
#include <docsh.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <sfx2/printer.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/saveopt.hxx>
#include <tools/diagnose_ex.h>
#include <boost/unordered_set.hpp>


#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotxdoc.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xformsimport.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xforms;
using namespace ::xmloff::token;
using namespace ::std;

enum SwXMLDocTokens
{
    XML_TOK_DOC_FONTDECLS,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_DOC_SCRIPT,
    XML_TOK_DOC_SETTINGS,
    XML_TOK_DOC_XFORMS,
    XML_TOK_OFFICE_END=XML_TOK_UNKNOWN
};

static SvXMLTokenMapEntry aDocTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,     XML_TOK_DOC_FONTDECLS  },
    { XML_NAMESPACE_OFFICE, XML_STYLES,         XML_TOK_DOC_STYLES      },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES, XML_TOK_DOC_AUTOSTYLES    },
    { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,   XML_TOK_DOC_MASTERSTYLES   },
    { XML_NAMESPACE_OFFICE, XML_META,           XML_TOK_DOC_META        },
    { XML_NAMESPACE_OFFICE, XML_BODY,           XML_TOK_DOC_BODY        },
    { XML_NAMESPACE_OFFICE, XML_SCRIPTS,        XML_TOK_DOC_SCRIPT      },
    { XML_NAMESPACE_OFFICE, XML_SETTINGS,       XML_TOK_DOC_SETTINGS    },
    { XML_NAMESPACE_XFORMS, XML_MODEL,          XML_TOK_DOC_XFORMS      },
    XML_TOKEN_MAP_END
};

class SwXMLBodyContext_Impl : public SvXMLImportContext
{
    const SwXMLImport& GetSwImport() const
        { return (const SwXMLImport&)GetImport(); }
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    SwXMLBodyContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLBodyContext_Impl();

    TYPEINFO();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
};

SwXMLBodyContext_Impl::SwXMLBodyContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & /*xAttrList*/ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLBodyContext_Impl::~SwXMLBodyContext_Impl()
{
}

TYPEINIT1( SwXMLBodyContext_Impl, SvXMLImportContext );

SvXMLImportContext *SwXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & /*xAttrList*/ )
{
    return GetSwImport().CreateBodyContentContext( rLocalName );
}





class SwXMLDocContext_Impl : public virtual SvXMLImportContext
{

protected: 
    const SwXMLImport& GetSwImport() const
        { return (const SwXMLImport&)GetImport(); }
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    SwXMLDocContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLDocContext_Impl();

    TYPEINFO();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
};

SwXMLDocContext_Impl::SwXMLDocContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & /*xAttrList*/ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLDocContext_Impl::~SwXMLDocContext_Impl()
{
}

TYPEINIT1( SwXMLDocContext_Impl, SvXMLImportContext );

SvXMLImportContext *SwXMLDocContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetDocElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_DOC_FONTDECLS:
        pContext = GetSwImport().CreateFontDeclsContext( rLocalName,
                                                             xAttrList );
        break;
    case XML_TOK_DOC_STYLES:
        GetSwImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
        pContext = GetSwImport().CreateStylesContext( rLocalName, xAttrList,
                                                      sal_False );
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        
        if ( ! IsXMLToken( GetLocalName(), XML_DOCUMENT_STYLES ) )
            GetSwImport().GetProgressBarHelper()->Increment
                ( PROGRESS_BAR_STEP );
        pContext = GetSwImport().CreateStylesContext( rLocalName, xAttrList,
                                                      sal_True );
        break;

    case XML_TOK_DOC_MASTERSTYLES:
        pContext = GetSwImport().CreateMasterStylesContext( rLocalName,
                                                            xAttrList );
        break;
    case XML_TOK_DOC_META:
        OSL_FAIL("XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
        break;
    case XML_TOK_DOC_SCRIPT:
        pContext = GetSwImport().CreateScriptContext( rLocalName );
        break;
    case XML_TOK_DOC_BODY:
        GetSwImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
        pContext = new SwXMLBodyContext_Impl( GetSwImport(), nPrefix,
                                              rLocalName, xAttrList );
        break;
    case XML_TOK_DOC_SETTINGS:
        pContext = new XMLDocumentSettingsContext( GetImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_DOC_XFORMS:
        pContext = createXFormsModelContext(GetImport(), nPrefix, rLocalName);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );


    return pContext;
}


class SwXMLOfficeDocContext_Impl :
         public SwXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:

    SwXMLOfficeDocContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList,
                const Reference< document::XDocumentProperties >& xDocProps);
    virtual ~SwXMLOfficeDocContext_Impl();

    TYPEINFO();

    virtual SvXMLImportContext *CreateChildContext(
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
};

SwXMLOfficeDocContext_Impl::SwXMLOfficeDocContext_Impl(
                SwXMLImport& rImport,
                sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList,
                const Reference< document::XDocumentProperties >& xDocProps) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    SwXMLDocContext_Impl( rImport, nPrfx, rLName, xAttrList ),
    SvXMLMetaDocumentContext( rImport, nPrfx, rLName, xDocProps)
{
}

SwXMLOfficeDocContext_Impl::~SwXMLOfficeDocContext_Impl()
{
}

TYPEINIT1( SwXMLOfficeDocContext_Impl, SwXMLDocContext_Impl );

SvXMLImportContext* SwXMLOfficeDocContext_Impl::CreateChildContext(
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetSwImport().GetDocElemTokenMap();

    
    
    
    {
        if ( rTokenMap.Get( nPrefix, rLocalName ) == XML_TOK_DOC_BODY )
        {
            GetImport().GetTextImport()->SetOutlineStyles( sal_True );
        }
    }

    
    if ( XML_TOK_DOC_META == rTokenMap.Get( nPrefix, rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    nPrefix, rLocalName, xAttrList );
    } else {
        return SwXMLDocContext_Impl::CreateChildContext(
                    nPrefix, rLocalName, xAttrList );
    }
}


class SwXMLDocStylesContext_Impl : public SwXMLDocContext_Impl
{
public:

    SwXMLDocStylesContext_Impl( SwXMLImport& rImport,
                                sal_uInt16 nPrfx,
                                const OUString& rLName,
                                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLDocStylesContext_Impl();

    TYPEINFO();

    virtual void EndElement();
};

SwXMLDocStylesContext_Impl::SwXMLDocStylesContext_Impl(
                    SwXMLImport& rImport,
                    sal_uInt16 nPrfx,
                    const OUString& rLName,
                    const Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    SwXMLDocContext_Impl( rImport, nPrfx, rLName, xAttrList )
{
}

SwXMLDocStylesContext_Impl::~SwXMLDocStylesContext_Impl()
{
}

TYPEINIT1( SwXMLDocStylesContext_Impl, SwXMLDocContext_Impl );

void SwXMLDocStylesContext_Impl::EndElement()
{
    
    
    SwXMLImport& rSwImport = dynamic_cast<SwXMLImport&>( GetImport());
    GetImport().GetTextImport()->SetOutlineStyles(
            (rSwImport.GetStyleFamilyMask() & SFX_STYLE_FAMILY_PARA ) ? sal_True : sal_False);
}

const SvXMLTokenMap& SwXMLImport::GetDocElemTokenMap()
{
    if( !pDocElemTokenMap )
        pDocElemTokenMap = new SvXMLTokenMap( aDocTokenMap );

    return *pDocElemTokenMap;
}

SvXMLImportContext *SwXMLImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    
    if( XML_NAMESPACE_OFFICE==nPrefix &&
        ( IsXMLToken( rLocalName, XML_DOCUMENT_SETTINGS ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_CONTENT ) ))
        pContext = new SwXMLDocContext_Impl( *this, nPrefix, rLocalName,
                                             xAttrList );
    else if ( XML_NAMESPACE_OFFICE==nPrefix &&
              IsXMLToken( rLocalName, XML_DOCUMENT_META ) )
    {
        pContext = CreateMetaContext(rLocalName);
    }
    else if ( XML_NAMESPACE_OFFICE==nPrefix &&
              IsXMLToken( rLocalName, XML_DOCUMENT_STYLES ) )
    {
        pContext = new SwXMLDocStylesContext_Impl( *this, nPrefix, rLocalName,
                                                   xAttrList );
    }
    else if ( XML_NAMESPACE_OFFICE==nPrefix &&
              IsXMLToken( rLocalName, XML_DOCUMENT ) )
    {
        uno::Reference<document::XDocumentProperties> const xDocProps(
            GetDocumentProperties());
        
        pContext = new SwXMLOfficeDocContext_Impl( *this, nPrefix, rLocalName,
                        xAttrList, xDocProps);
    }
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

SwXMLImport::SwXMLImport(
    const uno::Reference< uno::XComponentContext > xContext,
    OUString const & implementationName, sal_uInt16 nImportFlags)
:   SvXMLImport( xContext, implementationName, nImportFlags ),
    pSttNdIdx( 0 ),
    pTableItemMapper( 0 ),
    pDocElemTokenMap( 0 ),
    pTableElemTokenMap( 0 ),
    pTableCellAttrTokenMap( 0 ),
    pGraphicResolver( 0 ),
    pEmbeddedResolver( 0 ),
    nStyleFamilyMask( SFX_STYLE_FAMILY_ALL ),
    bLoadDoc( true ),
    bInsert( false ),
    bBlock( false ),
    bShowProgress( true ),
    bOrganizerMode( false ),
    bInititedXForms( false ),
    bPreserveRedlineMode( sal_True ),
    doc( NULL )
{
    _InitItemImport();

}

SwXMLImport::~SwXMLImport() throw ()
{
    delete pDocElemTokenMap;
    delete pTableElemTokenMap;
    delete pTableCellAttrTokenMap;
    _FinitItemImport();
}

void SwXMLImport::setTextInsertMode(
         const Reference< XTextRange > & rInsertPos )
{
    bInsert = true;

    Reference < XText > xText = rInsertPos->getText();
    Reference < XTextCursor > xTextCursor =
        xText->createTextCursorByRange( rInsertPos );
    GetTextImport()->SetCursor( xTextCursor );
}

void SwXMLImport::setStyleInsertMode( sal_uInt16 nFamilies,
                                      sal_Bool bOverwrite )
{
    bInsert = !bOverwrite;
    nStyleFamilyMask = nFamilies;
    bLoadDoc = false;
}

void SwXMLImport::setBlockMode( )
{
    bBlock = true;
}

void SwXMLImport::setOrganizerMode( )
{
    bOrganizerMode = true;
}

namespace
{
    class theSwXMLImportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXMLImportUnoTunnelId > {};
}

const Sequence< sal_Int8 > & SwXMLImport::getUnoTunnelId() throw()
{
    return theSwXMLImportUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXMLImport::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return SvXMLImport::getSomething( rId );
}

static OTextCursorHelper *lcl_xml_GetSwXTextCursor( const Reference < XTextCursor >& rTextCursor )
{
    Reference<XUnoTunnel> xCrsrTunnel( rTextCursor, UNO_QUERY );
    OSL_ENSURE( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xCrsrTunnel.is() )
        return 0;
    OTextCursorHelper *pTxtCrsr = reinterpret_cast< OTextCursorHelper *>(
            sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething(  OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTxtCrsr, "SwXTextCursor missing" );
    return pTxtCrsr;
}

void SwXMLImport::startDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    
    SvXMLImport::startDocument();

    OSL_ENSURE( GetModel().is(), "model is missing" );
    if( !GetModel().is() )
        return;

    
    SolarMutexGuard aGuard;


    Reference< XPropertySet > xImportInfo( getImportInfo() );
    Reference< XPropertySetInfo > xPropertySetInfo;
       if( xImportInfo.is() )
        xPropertySetInfo = xImportInfo->getPropertySetInfo();
    if( xPropertySetInfo.is() )
    {
        Any aAny;
        
        OUString sStyleInsertModeFamilies("StyleInsertModeFamilies");
        if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeFamilies) )
        {
            aAny = xImportInfo->getPropertyValue(sStyleInsertModeFamilies);
            Sequence< OUString> aFamiliesSeq;
            if( aAny >>= aFamiliesSeq )
            {
                OUString sFrameStyles( "FrameStyles" );
                OUString sPageStyles( "PageStyles" );
                OUString sCharacterStyles( "CharacterStyles" );
                OUString sParagraphStyles( "ParagraphStyles" );
                OUString sNumberingStyles( "NumberingStyles" );
                sal_uInt16 nFamilyMask = 0U;
                sal_Int32 nCount = aFamiliesSeq.getLength();
                const OUString *pSeq = aFamiliesSeq.getConstArray();
                for( sal_Int32 i=0; i < nCount; i++ )
                {
                    const OUString& rFamily = pSeq[i];
                    if( rFamily==sFrameStyles )
                        nFamilyMask |= SFX_STYLE_FAMILY_FRAME;
                    else if( rFamily==sPageStyles )
                        nFamilyMask |= SFX_STYLE_FAMILY_PAGE;
                    else if( rFamily==sCharacterStyles )
                        nFamilyMask |= SFX_STYLE_FAMILY_CHAR;
                    else if( rFamily==sParagraphStyles )
                        nFamilyMask |= SFX_STYLE_FAMILY_PARA;
                    else if( rFamily==sNumberingStyles )
                        nFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
                }

                sal_Bool bOverwrite = sal_False;
                OUString sStyleInsertModeOverwrite("StyleInsertModeOverwrite");
                if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeOverwrite) )
                {
                    aAny = xImportInfo->getPropertyValue(sStyleInsertModeOverwrite);
                    if( aAny.getValueType() == ::getBooleanCppuType() &&
                        *static_cast<const sal_Bool*>(aAny.getValue()) )
                        bOverwrite = sal_True;
                }

                setStyleInsertMode( nFamilyMask, bOverwrite );
            }
        }

        
        OUString sTextInsertModeRange("TextInsertModeRange");
        if( xPropertySetInfo->hasPropertyByName(sTextInsertModeRange) )
        {
            aAny = xImportInfo->getPropertyValue(sTextInsertModeRange);
            Reference<XTextRange> xInsertTextRange;
            if( aAny >>= xInsertTextRange )
                setTextInsertMode( xInsertTextRange );
        }

        
        OUString sAutoTextMode("AutoTextMode");
        if( xPropertySetInfo->hasPropertyByName(sAutoTextMode) )
        {
            aAny = xImportInfo->getPropertyValue(sAutoTextMode);
            if( aAny.getValueType() == ::getBooleanCppuType() &&
                *static_cast<const sal_Bool*>(aAny.getValue()) )
                    setBlockMode();
        }

        
        OUString sOrganizerMode("OrganizerMode");
        if( xPropertySetInfo->hasPropertyByName(sOrganizerMode) )
        {
            aAny = xImportInfo->getPropertyValue(sOrganizerMode);
            if( aAny.getValueType() == ::getBooleanCppuType() &&
                *static_cast<const sal_Bool*>(aAny.getValue()) )
                    setOrganizerMode();
        }
    }

    
    
    
    
    
    OTextCursorHelper *pTxtCrsr = 0;
    Reference < XTextCursor > xTextCursor;
    if( HasTextImport() )
           xTextCursor = GetTextImport()->GetCursor();
    if( !xTextCursor.is() )
    {
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();
        xTextCursor = xText->createTextCursor();
        SwCrsrShell *pCrsrSh = 0;
        SwDoc *pDoc = 0;
        if( IMPORT_ALL == getImportFlags() )
        {
            pTxtCrsr = lcl_xml_GetSwXTextCursor( xTextCursor );
            OSL_ENSURE( pTxtCrsr, "SwXTextCursor missing" );
            if( !pTxtCrsr )
                return;

            pDoc = pTxtCrsr->GetDoc();
            OSL_ENSURE( pDoc, "SwDoc missing" );
            if( !pDoc )
                return;

            
            
            
            
            pCrsrSh = pDoc->GetEditShell();
        }
        if( pCrsrSh )
        {
            const uno::Reference<text::XTextRange> xInsertTextRange(
                SwXTextRange::CreateXTextRange(
                    *pDoc, *pCrsrSh->GetCrsr()->GetPoint(), 0 ) );
            setTextInsertMode( xInsertTextRange );
            xTextCursor = GetTextImport()->GetCursor();
            pTxtCrsr = 0;
        }
        else
            GetTextImport()->SetCursor( xTextCursor );
    }

    if( (getImportFlags() & (IMPORT_CONTENT|IMPORT_MASTERSTYLES)) == 0 )
        return;

    if( !pTxtCrsr  )
        pTxtCrsr = lcl_xml_GetSwXTextCursor( xTextCursor );
    OSL_ENSURE( pTxtCrsr, "SwXTextCursor missing" );
    if( !pTxtCrsr )
        return;

    SwDoc *pDoc = pTxtCrsr->GetDoc();
    OSL_ENSURE( pDoc, "SwDoc missing" );
    if( !pDoc )
        return;

    if( (getImportFlags() & IMPORT_CONTENT) != 0 && !IsStylesOnlyMode() )
    {
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( IsInsertMode() )
        {
            SwPaM *pPaM = pTxtCrsr->GetPaM();
            const SwPosition* pPos = pPaM->GetPoint();

            
            pDoc->SplitNode( *pPos, false );
            *pSttNdIdx = pPos->nNode.GetIndex()-1;

            
            pDoc->SplitNode( *pPos, false );

            
            pPaM->Move( fnMoveBackward );
            pDoc->SetTxtFmtColl
                ( *pPaM, pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ) );
        }
    }

    
    pDoc->GetOrCreateDrawModel(); 

    
    SdrModel* pDrawModel = pDoc->GetDrawModel();
    if ( pDrawModel )
        pDrawModel->setLock(true);

    if( !GetGraphicResolver().is() )
    {
        pGraphicResolver = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_READ );
        Reference< document::XGraphicObjectResolver > xGraphicResolver( pGraphicResolver );
        SetGraphicResolver( xGraphicResolver );
    }

    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            pEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            EMBEDDEDOBJECTHELPER_MODE_READ );
            Reference< document::XEmbeddedObjectResolver > xEmbeddedResolver( pEmbeddedResolver );
            SetEmbeddedResolver( xEmbeddedResolver );
        }
    }
}

void SwXMLImport::endDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    OSL_ENSURE( GetModel().is(), "model missing; maybe startDocument wasn't called?" );
    if( !GetModel().is() )
        return;

    
    SolarMutexGuard aGuard;

    if( pGraphicResolver )
        SvXMLGraphicHelper::Destroy( pGraphicResolver );
    if( pEmbeddedResolver )
        SvXMLEmbeddedObjectHelper::Destroy( pEmbeddedResolver );
    
    
    
    if( HasShapeImport() )
        ClearShapeImport();


    SwDoc *pDoc = 0;
    if( (getImportFlags() & IMPORT_CONTENT) != 0 && !IsStylesOnlyMode() )
    {
        Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                              UNO_QUERY);
        OSL_ENSURE( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
        OTextCursorHelper *pTxtCrsr = reinterpret_cast< OTextCursorHelper *>(
                sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
        OSL_ENSURE( pTxtCrsr, "SwXTextCursor missing" );
        SwPaM *pPaM = pTxtCrsr->GetPaM();
        if( IsInsertMode() && pSttNdIdx->GetIndex() )
        {
            
            
            
            SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
            SwNodeIndex aNxtIdx( *pSttNdIdx );
            if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
            {
                
                
                if( pPaM && pPaM->GetPoint()->nNode == aNxtIdx )
                {
                    pPaM->GetPoint()->nNode = *pSttNdIdx;
                    pPaM->GetPoint()->nContent.Assign( pTxtNode,
                                            pTxtNode->GetTxt().getLength());
                }

#if OSL_DEBUG_LEVEL > 0
                
                OSL_ENSURE( pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( true ).nNode.GetIndex(),
                        "PaM.Bound1 point to new node " );
                OSL_ENSURE( pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( false ).nNode.GetIndex(),
                        "PaM.Bound2 points to new node" );

                if( pSttNdIdx->GetIndex()+1 ==
                                        pPaM->GetBound( true ).nNode.GetIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound( true ).nContent.GetIndex();
                    pPaM->GetBound( true ).nContent.Assign( pTxtNode,
                            pTxtNode->GetTxt().getLength() + nCntPos );
                }
                if( pSttNdIdx->GetIndex()+1 ==
                                pPaM->GetBound( false ).nNode.GetIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound( false ).nContent.GetIndex();
                    pPaM->GetBound( false ).nContent.Assign( pTxtNode,
                            pTxtNode->GetTxt().getLength() + nCntPos );
                }
#endif
                
                
                
                SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
                if (!pTxtNode->GetTxt().isEmpty())
                    pDelNd->FmtToTxtAttr( pTxtNode );
                else
                    pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
                pTxtNode->JoinNext();
            }
        }

        SwPosition* pPos = pPaM->GetPoint();
        OSL_ENSURE( !pPos->nContent.GetIndex(), "last paragraph isn't empty" );
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pCurrNd;
            sal_uLong nNodeIdx = pPos->nNode.GetIndex();
            pDoc = pPaM->GetDoc();

            OSL_ENSURE( pPos->nNode.GetNode().IsCntntNode(),
                        "insert position is not a content node" );
            if( !IsInsertMode() )
            {
                
                const SwNode *pPrev = pDoc->GetNodes()[nNodeIdx -1];
                if( pPrev->IsCntntNode() ||
                     ( pPrev->IsEndNode() &&
                      pPrev->StartOfSectionNode()->IsSectionNode() ) )
                {
                    SwCntntNode* pCNd = pPaM->GetCntntNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() )
                    {
                        pPaM->GetBound(true).nContent.Assign( 0, 0 );
                        pPaM->GetBound(false).nContent.Assign( 0, 0 );
                        pDoc->GetNodes().Delete( pPaM->GetPoint()->nNode );
                    }
                }
            }
            else if( 0 != (pCurrNd = pDoc->GetNodes()[nNodeIdx]->GetTxtNode()) )
            {
                
                
                if( pCurrNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pNextNd->JoinPrev();

                    
                    
                    if( pNextNd->CanJoinPrev(/* &pPos->nNode*/ ) &&
                         *pSttNdIdx != pPos->nNode )
                    {
                        pNextNd->JoinPrev();
                    }
                }
                else if (pCurrNd->GetTxt().isEmpty())
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPaM->Move( fnMoveBackward );
                }
            }
        }
    }

    /* Was called too early. Moved from SwXMLBodyContext_Impl::EndElement */

    GetTextImport()->RedlineAdjustStartNodeCursor( sal_False );

    if( (getImportFlags() & IMPORT_CONTENT) != 0 ||
        ((getImportFlags() & IMPORT_MASTERSTYLES) != 0 && IsStylesOnlyMode()) )
    {
        
        
        UpdateTxtCollConditions( pDoc );
    }

    GetTextImport()->ResetCursor();

    delete pSttNdIdx;
    pSttNdIdx = 0;

    if( (getImportFlags() == IMPORT_ALL ) )
    {
        
        
        if( IsInsertMode() )
            pDoc->PrtOLENotify( sal_False );
        else if ( pDoc->IsOLEPrtNotifyPending() )
            pDoc->PrtOLENotify( sal_True );
    }

    
    if ( pDoc )
    {
        SdrModel* pDrawModel = pDoc->GetDrawModel();
        if ( pDrawModel )
            pDrawModel->setLock(false);
    }

    
    if ( bInititedXForms )
    {
        Reference< xforms::XFormsSupplier > xFormsSupp( GetModel(), UNO_QUERY );
        Reference< XNameAccess > xXForms;
        if ( xFormsSupp.is() )
            xXForms = xFormsSupp->getXForms().get();

        if ( xXForms.is() )
        {
            try
            {
                Sequence< beans::PropertyValue > aXFormsSettings;

                OUString sXFormsSettingsName( GetXMLToken( XML_XFORM_MODEL_SETTINGS ) );
                if ( xLateInitSettings.is() && xLateInitSettings->hasByName( sXFormsSettingsName ) )
                {
                    OSL_VERIFY( xLateInitSettings->getByName( sXFormsSettingsName ) >>= aXFormsSettings );
                    applyXFormsSettings( xXForms, aXFormsSettings );
                }
            }
            catch( const Exception& )
            {
            }
        }
    }

    
    SvXMLImport::endDocument();
    ClearTextImport();
}




class SvTextShapeImportHelper : public XMLTextShapeImportHelper
{
    
    
    
    
    UniReference< ::xmloff::OFormLayerXMLImport > rFormImport;

    
    
    
    Reference<drawing::XDrawPage> xPage;

public:
    SvTextShapeImportHelper(SvXMLImport& rImp);
    virtual ~SvTextShapeImportHelper();
};

SvTextShapeImportHelper::SvTextShapeImportHelper(SvXMLImport& rImp) :
    XMLTextShapeImportHelper(rImp)
{
    Reference<drawing::XDrawPageSupplier> xSupplier(rImp.GetModel(),UNO_QUERY);
    if (xSupplier.is())
    {
        if (rImp.GetFormImport().is())
        {
            rImp.GetFormImport()->startPage(xSupplier->getDrawPage());
            rFormImport = rImp.GetFormImport();
        }

        xPage  = xSupplier->getDrawPage();
        Reference<XShapes> xShapes( xPage, UNO_QUERY );
        XMLShapeImportHelper::startPage( xShapes );
    }
}

SvTextShapeImportHelper::~SvTextShapeImportHelper()
{
    rFormImport->endPage();

    if (xPage.is())
    {
        Reference<XShapes> xShapes( xPage, UNO_QUERY );
        XMLShapeImportHelper::endPage(xShapes);
    }
}

XMLTextImportHelper* SwXMLImport::CreateTextImport()
{
    return new SwXMLTextImportHelper( GetModel(), *this, getImportInfo(),
                                      IsInsertMode(),
                                      IsStylesOnlyMode(), bShowProgress,
                                      IsBlockMode(), IsOrganizerMode(),
                                      bPreserveRedlineMode );
}

XMLShapeImportHelper* SwXMLImport::CreateShapeImport()
{
    return new SvTextShapeImportHelper( *this );
}

SvXMLImportContext *SwXMLImport::CreateFontDeclsContext(
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, XML_NAMESPACE_OFFICE,
                                      rLocalName, xAttrList,
                                      osl_getThreadTextEncoding() );
    SetFontDecls( pFSContext );
    return pFSContext;
}
void SwXMLImport::SetViewSettings(const Sequence < PropertyValue > & aViewProps)
{
    if (IsInsertMode() || IsStylesOnlyMode() || IsBlockMode() || IsOrganizerMode() || !GetModel().is() )
        return;

    
    SolarMutexGuard aGuard;

    SwDoc *pDoc = getDoc();
    Rectangle aRect;
    if( pDoc->GetDocShell() )
        aRect = pDoc->GetDocShell()->GetVisArea( ASPECT_CONTENT );
        //TODO/LATER: why that cast?!
        //aRect = ((SfxInPlaceObject *)pDoc->GetDocShell())->GetVisArea();

    sal_Int32 nCount = aViewProps.getLength();
    const PropertyValue *pValue = aViewProps.getConstArray();

    sal_Int64 nTmp = 0;
    sal_Bool bShowRedlineChanges = sal_False, bBrowseMode = sal_False;
    sal_Bool bChangeShowRedline = sal_False, bChangeBrowseMode = sal_False;

    //TODO/LATER: why that cast?!
    sal_Bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MAP_TWIP;
    //sal_Bool bTwip = pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit ( ) == MAP_TWIP;

    for (sal_Int32 i = 0; i < nCount ; i++)
    {
        if ( pValue->Name == "ViewAreaTop" )
        {
            pValue->Value >>= nTmp;
            aRect.setY( static_cast< long >(bTwip ? MM100_TO_TWIP ( nTmp ) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaLeft" )
        {
            pValue->Value >>= nTmp;
            aRect.setX( static_cast< long >(bTwip ? MM100_TO_TWIP ( nTmp ) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaWidth" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Width() = static_cast< long >(bTwip ? MM100_TO_TWIP ( nTmp ) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ViewAreaHeight" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Height() = static_cast< long >(bTwip ? MM100_TO_TWIP ( nTmp ) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ShowRedlineChanges" )
        {
            bShowRedlineChanges = *(sal_Bool *)(pValue->Value.getValue());
            bChangeShowRedline = sal_True;
        }

        else if ( pValue->Name == "InBrowseMode" )
        {
            bBrowseMode = *(sal_Bool *)(pValue->Value.getValue());
            bChangeBrowseMode = sal_True;
        }
        pValue++;
    }
    if( pDoc->GetDocShell() )
        pDoc->GetDocShell()->SetVisArea ( aRect );

    if (bChangeBrowseMode)
        pDoc->set(IDocumentSettingAccess::BROWSE_MODE, bBrowseMode );

    if (bChangeShowRedline)
        GetTextImport()->SetShowChanges( bShowRedlineChanges );
}

void SwXMLImport::SetConfigurationSettings(const Sequence < PropertyValue > & aConfigProps)
{
    
    SolarMutexGuard aGuard;

    Reference< lang::XMultiServiceFactory > xFac( GetModel(), UNO_QUERY );
    if( !xFac.is() )
        return;

    Reference< XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), UNO_QUERY );
    if( !xProps.is() )
        return;

    Reference< XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
    if( !xInfo.is() )
        return;

    boost::unordered_set< OUString, OUStringHash > aSet;
    aSet.insert(OUString("ForbiddenCharacters"));
    aSet.insert(OUString("IsKernAsianPunctuation"));
    aSet.insert(OUString("CharacterCompressionType"));
    aSet.insert(OUString("LinkUpdateMode"));
    aSet.insert(OUString("FieldAutoUpdate"));
    aSet.insert(OUString("ChartAutoUpdate"));
    aSet.insert(OUString("AddParaTableSpacing"));
    aSet.insert(OUString("AddParaTableSpacingAtStart"));
    aSet.insert(OUString("PrintAnnotationMode"));
    aSet.insert(OUString("PrintBlackFonts"));
    aSet.insert(OUString("PrintControls"));
    aSet.insert(OUString("PrintDrawings"));
    aSet.insert(OUString("PrintGraphics"));
    aSet.insert(OUString("PrintLeftPages"));
    aSet.insert(OUString("PrintPageBackground"));
    aSet.insert(OUString("PrintProspect"));
    aSet.insert(OUString("PrintReversed"));
    aSet.insert(OUString("PrintRightPages"));
    aSet.insert(OUString("PrintFaxName"));
    aSet.insert(OUString("PrintPaperFromSetup"));
    aSet.insert(OUString("PrintTables"));
    aSet.insert(OUString("PrintSingleJobs"));
    aSet.insert(OUString("UpdateFromTemplate"));
    aSet.insert(OUString("PrinterIndependentLayout"));
    aSet.insert(OUString("PrintEmptyPages"));
    aSet.insert(OUString("SmallCapsPercentage66"));
    aSet.insert(OUString("TabOverflow"));
    aSet.insert(OUString("UnbreakableNumberings"));
    aSet.insert(OUString("ClippedPictures"));
    aSet.insert(OUString("BackgroundParaOverDrawings"));
    aSet.insert(OUString("TabOverMargin"));

    sal_Int32 nCount = aConfigProps.getLength();
    const PropertyValue* pValues = aConfigProps.getConstArray();

    SvtSaveOptions aSaveOpt;
    sal_Bool bIsUserSetting = aSaveOpt.IsLoadUserSettings(),
         bSet = bIsUserSetting;

    
    
    
    bool bPrinterIndependentLayout = false;
    bool bUseOldNumbering = false;
    bool bOutlineLevelYieldsOutlineRule = false;
    bool bAddExternalLeading = false;
    bool bAddParaSpacingToTableCells = false;
    bool bUseFormerLineSpacing = false;
    bool bUseFormerObjectPositioning = false;
    bool bUseFormerTextWrapping = false;
    bool bConsiderWrapOnObjPos = false;
    bool bIgnoreFirstLineIndentInNumbering = false;
    bool bDoNotJustifyLinesWithManualBreak = false;
    bool bDoNotResetParaAttrsForNumFont    = false;
    bool bLoadReadonly = false;
    bool bDoNotCaptureDrawObjsOnPage( false );
    bool bClipAsCharacterAnchoredWriterFlyFrames( false );
    bool bUnixForceZeroExtLeading = false;
    bool bUseOldPrinterMetrics = false;
    bool bSmallCapsPercentage66 = false;
    bool bTabOverflow = false;
    bool bUnbreakableNumberings = false;
    bool bClippedPictures = false;
    bool bBackgroundParaOverDrawings = false;
    bool bTabOverMargin = false;

    OUString sRedlineProtectionKey( "RedlineProtectionKey" );

    const PropertyValue* currentDatabaseDataSource = NULL;
    const PropertyValue* currentDatabaseCommand = NULL;
    const PropertyValue* currentDatabaseCommandType = NULL;
    OUString currentDatabaseDataSourceKey( "CurrentDatabaseDataSource" );
    OUString currentDatabaseCommandKey( "CurrentDatabaseCommand" );
    OUString currentDatabaseCommandTypeKey( "CurrentDatabaseCommandType" );

    while( nCount-- )
    {
        if( !bIsUserSetting )
        {
            
            OUString aStr(pValues->Name);

            bSet = aSet.find(aStr) == aSet.end();
        }

        if( bSet )
        {
            try
            {
                if( xInfo->hasPropertyByName( pValues->Name ) )
                {
                    if( pValues->Name.equals( sRedlineProtectionKey ) )
                    {
                        Sequence<sal_Int8> aKey;
                        pValues->Value >>= aKey;
                        GetTextImport()->SetChangesProtectionKey( aKey );
                    }
                    else
                    {
                        
                        if( pValues->Name.equals( currentDatabaseDataSourceKey ))
                            currentDatabaseDataSource = pValues;
                        else if( pValues->Name.equals( currentDatabaseCommandKey ))
                            currentDatabaseCommand = pValues;
                        else if( pValues->Name.equals( currentDatabaseCommandTypeKey ))
                            currentDatabaseCommandType = pValues;
                        else
                            xProps->setPropertyValue( pValues->Name,
                                                  pValues->Value );
                    }
                }

                
                if ( pValues->Name == "PrinterIndependentLayout" )
                    bPrinterIndependentLayout = true;
                else if ( pValues->Name == "AddExternalLeading" )
                    bAddExternalLeading = true;
                else if ( pValues->Name == "AddParaSpacingToTableCells" )
                    bAddParaSpacingToTableCells = true;
                else if ( pValues->Name == "UseFormerLineSpacing" )
                    bUseFormerLineSpacing = true;
                else if ( pValues->Name == "UseFormerObjectPositioning" )
                    bUseFormerObjectPositioning = true;
                else if ( pValues->Name == "UseFormerTextWrapping" )
                    bUseFormerTextWrapping = true;
                else if ( pValues->Name == "UseOldNumbering" )
                    bUseOldNumbering = true;
                else if ( pValues->Name == "OutlineLevelYieldsNumbering" )
                    bOutlineLevelYieldsOutlineRule = true;
                else if ( pValues->Name == "ConsiderTextWrapOnObjPos" )
                    bConsiderWrapOnObjPos = true;
                else if ( pValues->Name == "IgnoreFirstLineIndentInNumbering" )
                    bIgnoreFirstLineIndentInNumbering = true;
                else if ( pValues->Name == "DoNotJustifyLinesWithManualBreak" )
                    bDoNotJustifyLinesWithManualBreak = true;
                else if ( pValues->Name == "DoNotResetParaAttrsForNumFont" )
                    bDoNotResetParaAttrsForNumFont = true;
                else if ( pValues->Name == "LoadReadonly" )
                    bLoadReadonly = true;
                else if ( pValues->Name == "DoNotCaptureDrawObjsOnPage" )
                    bDoNotCaptureDrawObjsOnPage = true;
                else if ( pValues->Name == "ClipAsCharacterAnchoredWriterFlyFrames" )
                    bClipAsCharacterAnchoredWriterFlyFrames = true;
                else if ( pValues->Name == "UnxForceZeroExtLeading" )
                    bUnixForceZeroExtLeading = true;
                else if ( pValues->Name == "UseOldPrinterMetrics" )
                    bUseOldPrinterMetrics = true;
                else if ( pValues->Name == "SmallCapsPercentage66" )
                    bSmallCapsPercentage66 = true;
                else if ( pValues->Name == "TabOverflow" )
                    bTabOverflow = true;
                else if ( pValues->Name == "UnbreakableNumberings" )
                    bUnbreakableNumberings = true;
                else if ( pValues->Name == "ClippedPictures" )
                    bClippedPictures = true;
                else if ( pValues->Name == "BackgroundParaOverDrawings" )
                    bBackgroundParaOverDrawings = true;
                else if ( pValues->Name == "TabOverMargin" )
                    bTabOverMargin = true;
            }
            catch( Exception& )
            {
                OSL_FAIL( "SwXMLImport::SetConfigurationSettings: Exception!" );
            }
        }
        pValues++;
    }

    try
    {
        if( currentDatabaseDataSource != NULL )
            xProps->setPropertyValue( currentDatabaseDataSource->Name, currentDatabaseDataSource->Value );
        if( currentDatabaseCommand != NULL )
            xProps->setPropertyValue( currentDatabaseCommand->Name, currentDatabaseCommand->Value );
        if( currentDatabaseCommandType != NULL )
            xProps->setPropertyValue( currentDatabaseCommandType->Name, currentDatabaseCommandType->Value );
    } catch( Exception& )
    {
        OSL_FAIL( "SwXMLImport::SetConfigurationSettings: Exception!" );
    }

    
    
    const bool bDocumentPriorSO8 = !bConsiderWrapOnObjPos;

    if( ! bPrinterIndependentLayout )
    {
        Any aAny;
        sal_Int16 nTmp = document::PrinterIndependentLayout::DISABLED;
        aAny <<= nTmp;
        xProps->setPropertyValue(
            OUString("PrinterIndependentLayout"),
            aAny );
    }

    if( ! bAddExternalLeading )
    {
        xProps->setPropertyValue(
            OUString("AddExternalLeading"), makeAny( false ) );
    }

    if( ! bUseFormerLineSpacing )
    {
        xProps->setPropertyValue(
            OUString("UseFormerLineSpacing"), makeAny( true ) );
    }

    if( !bUseFormerObjectPositioning )
    {
        xProps->setPropertyValue(
            OUString("UseFormerObjectPositioning"), makeAny( true ) );
    }

    if( !bUseOldNumbering )
    {
        Any aAny;
        sal_Bool bOldNum = true;
        aAny.setValue(&bOldNum, ::getBooleanCppuType());
        xProps->setPropertyValue
            (OUString("UseOldNumbering"),
                       aAny );
    }

    if( !bOutlineLevelYieldsOutlineRule )
    {
        Any aAny;
        sal_Bool bTmp = true;
        aAny.setValue(&bTmp, ::getBooleanCppuType());
        xProps->setPropertyValue
            (OUString("OutlineLevelYieldsNumbering"),
                       aAny );
    }

    if( !bAddParaSpacingToTableCells )
    {
        xProps->setPropertyValue(
            OUString("AddParaSpacingToTableCells"), makeAny( false ) );
    }

    if( !bUseFormerTextWrapping )
    {
        xProps->setPropertyValue(
            OUString("UseFormerTextWrapping"), makeAny( true ) );
    }

    if( !bConsiderWrapOnObjPos )
    {
        xProps->setPropertyValue(
            OUString("ConsiderTextWrapOnObjPos"), makeAny( false ) );
    }

    
    
    
    
    
    
    
    
    
    
    
    if( !bIgnoreFirstLineIndentInNumbering && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue(
            OUString("IgnoreFirstLineIndentInNumbering"), makeAny( true ) );
    }

    
    if ( !bDoNotJustifyLinesWithManualBreak && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue(
            OUString("DoNotJustifyLinesWithManualBreak"), makeAny( true ) );
    }

    
    if ( !bDoNotResetParaAttrsForNumFont && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue(
            OUString("DoNotResetParaAttrsForNumFont"), makeAny( true ) );
    }

    if ( !bLoadReadonly )
    {
        xProps->setPropertyValue(
            OUString("LoadReadonly"), makeAny( false ) );
    }

    
    if ( !bDoNotCaptureDrawObjsOnPage && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue(
            OUString("DoNotCaptureDrawObjsOnPage"), makeAny( true ) );
    }

    
    if ( !bClipAsCharacterAnchoredWriterFlyFrames && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue(
            OUString("ClipAsCharacterAnchoredWriterFlyFrames"), makeAny( true ) );
    }

    if ( !bUnixForceZeroExtLeading )
    {
        xProps->setPropertyValue(
            OUString("UnxForceZeroExtLeading"), makeAny( true ) );
    }

    if ( !bUseOldPrinterMetrics )
    {
        xProps->setPropertyValue(
            OUString("UseOldPrinterMetrics"), makeAny( true ) );
    }

    
    
    
    
    if ( !bSmallCapsPercentage66 )
    {
        xProps->setPropertyValue(
            OUString("SmallCapsPercentage66"), makeAny( true ) );
    }

    if ( !bTabOverflow )
    {
        xProps->setPropertyValue(
            OUString("TabOverflow"), makeAny( false ) );
    }

    if ( !bUnbreakableNumberings )
    {
        xProps->setPropertyValue(
            OUString("UnbreakableNumberings"), makeAny( false ) );
    }

    if ( !bClippedPictures )
    {
        xProps->setPropertyValue(
            OUString("ClippedPictures"), makeAny( false ) );
    }

    if ( !bBackgroundParaOverDrawings )
        xProps->setPropertyValue("BackgroundParaOverDrawings", makeAny( false ) );

    if ( !bTabOverMargin )
        xProps->setPropertyValue("TabOverMargin", makeAny( false ) );

    SwDoc *pDoc = getDoc();
    SfxPrinter *pPrinter = pDoc->getPrinter( false );
    if( pPrinter )
    {
        
        
        
        
        
        pDoc->SetOLEPrtNotifyPending( !pPrinter->IsKnown() );
    }
}


void SwXMLImport::SetDocumentSpecificSettings(
    const OUString& _rSettingsGroupName,
    const Sequence< PropertyValue>& _rSettings )
{
    
    if ( !IsXMLToken( _rSettingsGroupName, XML_XFORM_MODEL_SETTINGS ) )
        return;

    
    
    OSL_ENSURE( xLateInitSettings.is(), "SwXMLImport::SetDocumentSpecificSettings: no storage for those settings!" );
    if ( !xLateInitSettings.is() )
        return;

    try
    {
        if ( xLateInitSettings->hasByName( _rSettingsGroupName ) )
        {
            xLateInitSettings->replaceByName( _rSettingsGroupName, makeAny( _rSettings ) );
            OSL_FAIL( "SwXMLImport::SetDocumentSpecificSettings: already have settings for this model!" );
        }
        else
            xLateInitSettings->insertByName( _rSettingsGroupName, makeAny( _rSettings ) );
    }
    catch( const Exception& )
    {
    }
}

void SwXMLImport::initialize(
    const Sequence<Any>& aArguments )
    throw( uno::Exception, uno::RuntimeException)
{
    
    SvXMLImport::initialize(aArguments);

    
    sal_Int32 nLength = aArguments.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        beans::PropertyValue aValue;
        if ( aArguments[i] >>= aValue )
        {
            if (aValue.Name == "PreserveRedlineMode")
            {
                OSL_VERIFY( aValue.Value >>= bPreserveRedlineMode );
            }
            continue;
        }

        beans::NamedValue aNamedValue;
        if ( aArguments[i] >>= aNamedValue )
        {
            if (aNamedValue.Name == "LateInitSettings")
            {
                OSL_VERIFY( aNamedValue.Value >>= xLateInitSettings );
            }
        }
    }
}


OUString SAL_CALL SwXMLImport_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisImporter" );
}

uno::Sequence< OUString > SAL_CALL SwXMLImport_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( SwXMLImport_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImport_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport( comphelper::getComponentContext(rSMgr), SwXMLImport_getImplementationName(), IMPORT_ALL );
}

OUString SAL_CALL SwXMLImportStyles_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisStylesImporter" );
}

uno::Sequence< OUString > SAL_CALL SwXMLImportStyles_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( SwXMLImportStyles_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImportStyles_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport(
        comphelper::getComponentContext(rSMgr),
        SwXMLImportStyles_getImplementationName(),
        IMPORT_STYLES | IMPORT_MASTERSTYLES | IMPORT_AUTOSTYLES |
        IMPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLImportContent_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisContentImporter" );
}

uno::Sequence< OUString > SAL_CALL SwXMLImportContent_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( SwXMLImportContent_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImportContent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport(
        comphelper::getComponentContext(rSMgr),
        SwXMLImportContent_getImplementationName(),
        IMPORT_AUTOSTYLES | IMPORT_CONTENT | IMPORT_SCRIPTS |
        IMPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLImportMeta_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisMetaImporter" );
}

uno::Sequence< OUString > SAL_CALL SwXMLImportMeta_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( SwXMLImportMeta_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImportMeta_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport( comphelper::getComponentContext(rSMgr), SwXMLImportMeta_getImplementationName(), IMPORT_META );
}

OUString SAL_CALL SwXMLImportSettings_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisSettingsImporter" );
}

uno::Sequence< OUString > SAL_CALL SwXMLImportSettings_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( SwXMLImportSettings_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImportSettings_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport( comphelper::getComponentContext(rSMgr), SwXMLImportSettings_getImplementationName(), IMPORT_SETTINGS );
}

SwDoc* SwImport::GetDocFromXMLImport( SvXMLImport& rImport )
{
    uno::Reference<lang::XUnoTunnel> xModelTunnel( rImport.GetModel(), uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = reinterpret_cast< SwXTextDocument *>(
            sal::static_int_cast< sal_IntPtr >(  xModelTunnel->getSomething(SwXTextDocument::getUnoTunnelId() )));
    OSL_ENSURE( pTxtDoc, "Where is my model?" );
    OSL_ENSURE( pTxtDoc->GetDocShell(), "Where is my shell?" );
    SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
    OSL_ENSURE( pDoc, "Where is my document?" );
    return pDoc;
}

void SwXMLImport::initXForms()
{
    
    Reference<XUnoTunnel> xDocTunnel( GetModel(), UNO_QUERY );
    if( ! xDocTunnel.is() )
        return;
    SwXTextDocument* pXTextDocument = reinterpret_cast<SwXTextDocument*>(
        xDocTunnel->getSomething( SwXTextDocument::getUnoTunnelId() ) );
    if( pXTextDocument == NULL )
        return;

    SwDoc *pDoc = pXTextDocument->GetDocShell()->GetDoc();

    
    
    if( ! pDoc->isXForms() )
        pDoc->initXForms( false );

    bInititedXForms = true;
}

SwDoc* SwXMLImport::getDoc()
{
    if( doc != NULL )
        return doc;
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    assert( xTextTunnel.is());
    SwXText *pText = reinterpret_cast< SwXText *>(
            sal::static_int_cast< sal_IntPtr >( xTextTunnel->getSomething( SwXText::getUnoTunnelId() )));
    assert( pText != NULL );
    doc = pText->GetDoc();
    assert( doc != NULL );
    return doc;
}

const SwDoc* SwXMLImport::getDoc() const
{
    return const_cast< SwXMLImport* >( this )->getDoc();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
