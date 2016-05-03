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

#include <sal/config.h>

#include <cassert>

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
#include <drawdoc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <unofreg.hxx>
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

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotxdoc.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xformsimport.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>

#include <unordered_set>

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
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:

    SwXMLBodyContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLBodyContext_Impl();


    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) override;
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


SvXMLImportContext *SwXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & /*xAttrList*/ )
{
    return GetSwImport().CreateBodyContentContext( rLocalName );
}

// #i69629#
// enhance class <SwXMLDocContext_Impl> in order to be able to create subclasses
// NB: virtually inherit so we can multiply inherit properly
//     in SwXMLOfficeDocContext_Impl
class SwXMLDocContext_Impl : public virtual SvXMLImportContext
{

protected: // #i69629#
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:

    SwXMLDocContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLDocContext_Impl();


    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) override;
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


SvXMLImportContext *SwXMLDocContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

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
                                                      false );
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        // don't use the autostyles from the styles-document for the progress
        if ( ! IsXMLToken( GetLocalName(), XML_DOCUMENT_STYLES ) )
            GetSwImport().GetProgressBarHelper()->Increment
                ( PROGRESS_BAR_STEP );
        pContext = GetSwImport().CreateStylesContext( rLocalName, xAttrList,
                                                      true );
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

// #i69629# - new subclass <SwXMLOfficeDocContext_Impl> of class <SwXMLDocContext_Impl>
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


    virtual SvXMLImportContext *CreateChildContext(
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) override;
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


SvXMLImportContext* SwXMLOfficeDocContext_Impl::CreateChildContext(
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetSwImport().GetDocElemTokenMap();

    // assign paragraph styles to list levels of outline style after all styles
    // are imported and finished. This is the case, when <office:body> starts
    // in flat OpenDocument file format.
    {
        if ( rTokenMap.Get( nPrefix, rLocalName ) == XML_TOK_DOC_BODY )
        {
            GetImport().GetTextImport()->SetOutlineStyles( true );
        }
    }

    // behave like meta base class iff we encounter office:meta
    if ( XML_TOK_DOC_META == rTokenMap.Get( nPrefix, rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    nPrefix, rLocalName, xAttrList );
    } else {
        return SwXMLDocContext_Impl::CreateChildContext(
                    nPrefix, rLocalName, xAttrList );
    }
}

// #i69629# - new subclass <SwXMLDocStylesContext_Impl> of class <SwXMLDocContext_Impl>
class SwXMLDocStylesContext_Impl : public SwXMLDocContext_Impl
{
public:

    SwXMLDocStylesContext_Impl( SwXMLImport& rImport,
                                sal_uInt16 nPrfx,
                                const OUString& rLName,
                                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLDocStylesContext_Impl();


    virtual void EndElement() override;
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


void SwXMLDocStylesContext_Impl::EndElement()
{
    // assign paragraph styles to list levels of outline style after all styles
    // are imported and finished.
    SwXMLImport& rSwImport = dynamic_cast<SwXMLImport&>( GetImport());
    GetImport().GetTextImport()->SetOutlineStyles(
            (rSwImport.GetStyleFamilyMask() & SFX_STYLE_FAMILY_PARA ) != 0);
}

const SvXMLTokenMap& SwXMLImport::GetDocElemTokenMap()
{
    if( !m_pDocElemTokenMap )
        m_pDocElemTokenMap = new SvXMLTokenMap( aDocTokenMap );

    return *m_pDocElemTokenMap;
}

SvXMLImportContext *SwXMLImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    // #i69629# - own subclasses for <office:document> and <office:document-styles>
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
        // flat OpenDocument file format
        pContext = new SwXMLOfficeDocContext_Impl( *this, nPrefix, rLocalName,
                        xAttrList, xDocProps);
    }
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

SwXMLImport::SwXMLImport(
    const uno::Reference< uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags)
:   SvXMLImport( rContext, implementationName, nImportFlags ),
    m_pSttNdIdx( nullptr ),
    m_pTableItemMapper( nullptr ),
    m_pDocElemTokenMap( nullptr ),
    m_pTableElemTokenMap( nullptr ),
    m_pTableCellAttrTokenMap( nullptr ),
    m_pGraphicResolver( nullptr ),
    m_pEmbeddedResolver( nullptr ),
    m_nStyleFamilyMask( SFX_STYLE_FAMILY_ALL ),
    m_bLoadDoc( true ),
    m_bInsert( false ),
    m_bBlock( false ),
    m_bShowProgress( true ),
    m_bOrganizerMode( false ),
    m_bInititedXForms( false ),
    m_bPreserveRedlineMode( true ),
    m_pDoc( nullptr )
{
    _InitItemImport();

}

SwXMLImport::~SwXMLImport() throw ()
{
    delete m_pDocElemTokenMap;
    delete m_pTableElemTokenMap;
    delete m_pTableCellAttrTokenMap;
    _FinitItemImport();
}

void SwXMLImport::setTextInsertMode(
         const Reference< XTextRange > & rInsertPos )
{
    m_bInsert = true;

    Reference < XText > xText = rInsertPos->getText();
    Reference < XTextCursor > xTextCursor =
        xText->createTextCursorByRange( rInsertPos );
    GetTextImport()->SetCursor( xTextCursor );
}

void SwXMLImport::setStyleInsertMode( sal_uInt16 nFamilies,
                                      bool bOverwrite )
{
    m_bInsert = !bOverwrite;
    m_nStyleFamilyMask = nFamilies;
    m_bLoadDoc = false;
}

void SwXMLImport::setBlockMode( )
{
    m_bBlock = true;
}

void SwXMLImport::setOrganizerMode( )
{
    m_bOrganizerMode = true;
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
    throw(RuntimeException, std::exception)
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
    Reference<XUnoTunnel> xCursorTunnel( rTextCursor, UNO_QUERY );
    OSL_ENSURE( xCursorTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xCursorTunnel.is() )
        return nullptr;
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper *>(
            sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething(  OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    return pTextCursor;
}

void SwXMLImport::startDocument()
    throw (xml::sax::SAXException,
           uno::RuntimeException,
           std::exception)
{
    // delegate to parent
    SvXMLImport::startDocument();

    OSL_ENSURE( GetModel().is(), "model is missing" );
    if( !GetModel().is() )
        return;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    Reference< XPropertySet > xImportInfo( getImportInfo() );
    Reference< XPropertySetInfo > xPropertySetInfo;
       if( xImportInfo.is() )
        xPropertySetInfo = xImportInfo->getPropertySetInfo();
    if( xPropertySetInfo.is() )
    {
        Any aAny;
        // insert style mode?
        OUString sStyleInsertModeFamilies("StyleInsertModeFamilies");
        if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeFamilies) )
        {
            aAny = xImportInfo->getPropertyValue(sStyleInsertModeFamilies);
            Sequence< OUString> aFamiliesSeq;
            if( aAny >>= aFamiliesSeq )
            {
                sal_uInt16 nFamilyMask = 0U;
                sal_Int32 nCount = aFamiliesSeq.getLength();
                const OUString *pSeq = aFamiliesSeq.getConstArray();
                for( sal_Int32 i=0; i < nCount; i++ )
                {
                    const OUString& rFamily = pSeq[i];
                    if( rFamily=="FrameStyles" )
                        nFamilyMask |= SFX_STYLE_FAMILY_FRAME;
                    else if( rFamily=="PageStyles" )
                        nFamilyMask |= SFX_STYLE_FAMILY_PAGE;
                    else if( rFamily=="CharacterStyles" )
                        nFamilyMask |= SFX_STYLE_FAMILY_CHAR;
                    else if( rFamily=="ParagraphStyles" )
                        nFamilyMask |= SFX_STYLE_FAMILY_PARA;
                    else if( rFamily=="NumberingStyles" )
                        nFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
                }

                bool bOverwrite = false;
                const OUString sStyleInsertModeOverwrite("StyleInsertModeOverwrite");
                if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeOverwrite) )
                {
                    aAny = xImportInfo->getPropertyValue(sStyleInsertModeOverwrite);
                    if( aAny.getValueType() == cppu::UnoType<bool>::get() &&
                        *static_cast<const sal_Bool*>(aAny.getValue()) )
                        bOverwrite = true;
                }

                setStyleInsertMode( nFamilyMask, bOverwrite );
            }
        }

        // text insert mode?
        const OUString sTextInsertModeRange("TextInsertModeRange");
        if( xPropertySetInfo->hasPropertyByName(sTextInsertModeRange) )
        {
            aAny = xImportInfo->getPropertyValue(sTextInsertModeRange);
            Reference<XTextRange> xInsertTextRange;
            if( aAny >>= xInsertTextRange )
                setTextInsertMode( xInsertTextRange );
        }

        // auto text mode
        const OUString sAutoTextMode("AutoTextMode");
        if( xPropertySetInfo->hasPropertyByName(sAutoTextMode) )
        {
            aAny = xImportInfo->getPropertyValue(sAutoTextMode);
            if( aAny.getValueType() == cppu::UnoType<bool>::get() &&
                *static_cast<const sal_Bool*>(aAny.getValue()) )
                    setBlockMode();
        }

        // organizer mode
        const OUString sOrganizerMode("OrganizerMode");
        if( xPropertySetInfo->hasPropertyByName(sOrganizerMode) )
        {
            aAny = xImportInfo->getPropertyValue(sOrganizerMode);
            if( aAny.getValueType() == cppu::UnoType<bool>::get() &&
                *static_cast<const sal_Bool*>(aAny.getValue()) )
                    setOrganizerMode();
        }
    }

    // There only is a text cursor by now if we are in insert mode. In any
    // other case we have to create one at the start of the document.
    // We also might change into the insert mode later, so we have to make
    // sure to first set the insert mode and then create the text import
    // helper. Otherwise it won't have the insert flag set!
    OTextCursorHelper *pTextCursor = nullptr;
    Reference < XTextCursor > xTextCursor;
    if( HasTextImport() )
           xTextCursor = GetTextImport()->GetCursor();
    if( !xTextCursor.is() )
    {
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();
        xTextCursor = xText->createTextCursor();
        SwCursorShell *pCursorSh = nullptr;
        SwDoc *pDoc = nullptr;
        if( SvXMLImportFlags::ALL == getImportFlags() )
        {
            pTextCursor = lcl_xml_GetSwXTextCursor( xTextCursor );
            OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
            if( !pTextCursor )
                return;

            pDoc = pTextCursor->GetDoc();
            OSL_ENSURE( pDoc, "SwDoc missing" );
            if( !pDoc )
                return;

            // Is there a edit shell. If yes, then we are currently inserting
            // a document. We then have to insert at the current edit shell's
            // cursor position. That not quite clean code, but there is no other
            // way currently.
            pCursorSh = pDoc->GetEditShell();
        }
        if( pCursorSh )
        {
            const uno::Reference<text::XTextRange> xInsertTextRange(
                SwXTextRange::CreateXTextRange(
                    *pDoc, *pCursorSh->GetCursor()->GetPoint(), nullptr ) );
            setTextInsertMode( xInsertTextRange );
            xTextCursor = GetTextImport()->GetCursor();
            pTextCursor = nullptr;
        }
        else
            GetTextImport()->SetCursor( xTextCursor );
    }

    if( (!(getImportFlags() & (SvXMLImportFlags::CONTENT|SvXMLImportFlags::MASTERSTYLES))))
        return;

    if( !pTextCursor  )
        pTextCursor = lcl_xml_GetSwXTextCursor( xTextCursor );
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    if( !pTextCursor )
        return;

    SwDoc *pDoc = pTextCursor->GetDoc();
    OSL_ENSURE( pDoc, "SwDoc missing" );
    if( !pDoc )
        return;

    if (SvXMLImportFlags::ALL == getImportFlags())
    {
        // for flat ODF - this is done in SwReader::Read() for package ODF
        pDoc->SetInReading(true);
        pDoc->SetInXMLImport(true);
    }

    if( (getImportFlags() & SvXMLImportFlags::CONTENT) && !IsStylesOnlyMode() )
    {
        m_pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( IsInsertMode() )
        {
            SwPaM *pPaM = pTextCursor->GetPaM();
            const SwPosition* pPos = pPaM->GetPoint();

            // Split once and remember the node that has been splitted.
            pDoc->getIDocumentContentOperations().SplitNode( *pPos, false );
            *m_pSttNdIdx = pPos->nNode.GetIndex()-1;

            // Split again.
            pDoc->getIDocumentContentOperations().SplitNode( *pPos, false );

            // Insert all content into the new node
            pPaM->Move( fnMoveBackward );
            pDoc->SetTextFormatColl
                ( *pPaM, pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, false ) );
        }
    }

    // We need a draw model to be able to set the z order
    pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel(); // #i52858# - method name changed

    // SJ: #i49801# locking the modell to disable repaints
    SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    if ( pDrawModel )
        pDrawModel->setLock(true);

    if( !GetGraphicResolver().is() )
    {
        m_pGraphicResolver = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_READ );
        Reference< document::XGraphicObjectResolver > xGraphicResolver( m_pGraphicResolver );
        SetGraphicResolver( xGraphicResolver );
    }

    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            m_pEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            EMBEDDEDOBJECTHELPER_MODE_READ );
            Reference< document::XEmbeddedObjectResolver > xEmbeddedResolver( m_pEmbeddedResolver );
            SetEmbeddedResolver( xEmbeddedResolver );
        }
    }
}

void SwXMLImport::endDocument()
    throw( xml::sax::SAXException, uno::RuntimeException, std::exception )
{
    OSL_ENSURE( GetModel().is(), "model missing; maybe startDocument wasn't called?" );
    if( !GetModel().is() )
        return;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    if( m_pGraphicResolver )
        SvXMLGraphicHelper::Destroy( m_pGraphicResolver );
    if( m_pEmbeddedResolver )
        SvXMLEmbeddedObjectHelper::Destroy( m_pEmbeddedResolver );
    // Clear the shape import to sort the shapes  (and not in the
    // destructor that might be called after the import has finished
    // for Java filters.
    if( HasShapeImport() )
        ClearShapeImport();

    SwDoc *pDoc = nullptr;
    if( (getImportFlags() & SvXMLImportFlags::CONTENT) && !IsStylesOnlyMode() )
    {
        Reference<XUnoTunnel> xCursorTunnel( GetTextImport()->GetCursor(),
                                              UNO_QUERY);
        assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
        OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper *>(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
        assert(pTextCursor && "SwXTextCursor missing");
        SwPaM *pPaM = pTextCursor->GetPaM();
        if( IsInsertMode() && m_pSttNdIdx->GetIndex() )
        {
            // If we are in insert mode, join the splitted node that is in front
            // of the new content with the first new node. Or in other words:
            // Revert the first split node.
            SwTextNode* pTextNode = m_pSttNdIdx->GetNode().GetTextNode();
            SwNodeIndex aNxtIdx( *m_pSttNdIdx );
            if( pTextNode && pTextNode->CanJoinNext( &aNxtIdx ) &&
                m_pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
            {
                // If the PaM points to the first new node, move the PaM to the
                // end of the previous node.
                if( pPaM->GetPoint()->nNode == aNxtIdx )
                {
                    pPaM->GetPoint()->nNode = *m_pSttNdIdx;
                    pPaM->GetPoint()->nContent.Assign( pTextNode,
                                            pTextNode->GetText().getLength());
                }

#if OSL_DEBUG_LEVEL > 0
                // !!! This should be impossible !!!!
                OSL_ENSURE( m_pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound().nNode.GetIndex(),
                        "PaM.Bound1 point to new node " );
                OSL_ENSURE( m_pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( false ).nNode.GetIndex(),
                        "PaM.Bound2 points to new node" );

                if( m_pSttNdIdx->GetIndex()+1 ==
                                        pPaM->GetBound().nNode.GetIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound().nContent.GetIndex();
                    pPaM->GetBound().nContent.Assign( pTextNode,
                            pTextNode->GetText().getLength() + nCntPos );
                }
                if( m_pSttNdIdx->GetIndex()+1 ==
                                pPaM->GetBound( false ).nNode.GetIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound( false ).nContent.GetIndex();
                    pPaM->GetBound( false ).nContent.Assign( pTextNode,
                            pTextNode->GetText().getLength() + nCntPos );
                }
#endif
                // If the first new node isn't empty, convert  the node's text
                // attributes into hints. Otherwise, set the new node's
                // paragraph style at the previous (empty) node.
                SwTextNode* pDelNd = aNxtIdx.GetNode().GetTextNode();
                if (!pTextNode->GetText().isEmpty())
                    pDelNd->FormatToTextAttr( pTextNode );
                else
                    pTextNode->ChgFormatColl( pDelNd->GetTextColl() );
                pTextNode->JoinNext();
            }
        }

        SwPosition* pPos = pPaM->GetPoint();
        OSL_ENSURE( !pPos->nContent.GetIndex(), "last paragraph isn't empty" );
        if( !pPos->nContent.GetIndex() )
        {
            SwTextNode* pCurrNd;
            sal_uLong nNodeIdx = pPos->nNode.GetIndex();
            pDoc = pPaM->GetDoc();

            OSL_ENSURE( pPos->nNode.GetNode().IsContentNode(),
                        "insert position is not a content node" );
            if( !IsInsertMode() )
            {
                // If we're not in insert mode, the last node is deleted.
                const SwNode *pPrev = pDoc->GetNodes()[nNodeIdx -1];
                if( pPrev->IsContentNode() ||
                     ( pPrev->IsEndNode() &&
                      pPrev->StartOfSectionNode()->IsSectionNode() ) )
                {
                    SwContentNode* pCNd = pPaM->GetContentNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() )
                    {
                        pPaM->GetBound().nContent.Assign( nullptr, 0 );
                        pPaM->GetBound(false).nContent.Assign( nullptr, 0 );
                        pDoc->GetNodes().Delete( pPaM->GetPoint()->nNode );
                    }
                }
            }
            else if( nullptr != (pCurrNd = pDoc->GetNodes()[nNodeIdx]->GetTextNode()) )
            {
                // Id we're in insert mode, the empty node is joined with
                // the next and the previous one.
                if( pCurrNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTextNode* pNextNd = pPos->nNode.GetNode().GetTextNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pNextNd->JoinPrev();

                    // Remove line break that has been inserted by the import,
                    // but only if one has been inserted!
                    if( pNextNd->CanJoinPrev(/* &pPos->nNode*/ ) &&
                         *m_pSttNdIdx != pPos->nNode )
                    {
                        pNextNd->JoinPrev();
                    }
                }
                else if (pCurrNd->GetText().isEmpty())
                {
                    pPos->nContent.Assign( nullptr, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode );
                    pPaM->Move( fnMoveBackward );
                }
            }
        }
    }

    /* Was called too early. Moved from SwXMLBodyContext_Impl::EndElement */

    GetTextImport()->RedlineAdjustStartNodeCursor( false );

    if( (getImportFlags() & SvXMLImportFlags::CONTENT) ||
        ((getImportFlags() & SvXMLImportFlags::MASTERSTYLES) && IsStylesOnlyMode()) )
    {
        // pDoc might be 0. In this case UpdateTextCollCondition is looking
        // for it itself.
        UpdateTextCollConditions( pDoc );
    }

    GetTextImport()->ResetCursor();

    delete m_pSttNdIdx;
    m_pSttNdIdx = nullptr;

    // SJ: #i49801# -> now permitting repaints
    if ( pDoc )
    {
        if( (getImportFlags() == SvXMLImportFlags::ALL ) )
        {
            // Notify math objects. If we are in the package filter this will
            // be done by the filter object itself
            if( IsInsertMode() )
                pDoc->PrtOLENotify( false );
            else if ( pDoc->IsOLEPrtNotifyPending() )
                pDoc->PrtOLENotify( true );

            assert(pDoc->IsInReading());
            assert(pDoc->IsInXMLImport());
            pDoc->SetInReading(false);
            pDoc->SetInXMLImport(false);
        }

        SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
        if ( pDrawModel )
            pDrawModel->setLock(false);
    }

    // #i90243#
    if ( m_bInititedXForms )
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

                const OUString sXFormsSettingsName( GetXMLToken( XML_XFORM_MODEL_SETTINGS ) );
                if ( m_xLateInitSettings.is() && m_xLateInitSettings->hasByName( sXFormsSettingsName ) )
                {
                    OSL_VERIFY( m_xLateInitSettings->getByName( sXFormsSettingsName ) >>= aXFormsSettings );
                    applyXFormsSettings( xXForms, aXFormsSettings );
                }
            }
            catch( const Exception& )
            {
            }
        }
    }

    // delegate to parent: takes care of error handling
    SvXMLImport::endDocument();
    ClearTextImport();
}

// Locally derive XMLTextShapeImportHelper, so we can take care of the
// form import This is Writer, but not text specific, so it should go
// here!
class SvTextShapeImportHelper : public XMLTextShapeImportHelper
{
    // hold own reference form import helper, because the SvxImport
    // stored in the superclass, from whom we originally got the
    // reference, is already destroyed when we want to use it in the
    // destructor
    rtl::Reference< ::xmloff::OFormLayerXMLImport > rFormImport;

    // hold reference to the one page (if it exists) for calling startPage()
    // and endPage. If !xPage.is(), then this document doesn't have a
    // XDrawPage.
    Reference<drawing::XDrawPage> xPage;

public:
    explicit SvTextShapeImportHelper(SvXMLImport& rImp);
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
                                      IsStylesOnlyMode(), m_bShowProgress,
                                      IsBlockMode(), IsOrganizerMode(),
                                      m_bPreserveRedlineMode );
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

    // this method will modify the document directly -> lock SolarMutex
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
    bool bShowRedlineChanges = false, bBrowseMode = false;
    bool bChangeShowRedline = false, bChangeBrowseMode = false;

    //TODO/LATER: why that cast?!
    bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MAP_TWIP;
    //sal_Bool bTwip = pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit ( ) == MAP_TWIP;

    for (sal_Int32 i = 0; i < nCount ; i++)
    {
        if ( pValue->Name == "ViewAreaTop" )
        {
            pValue->Value >>= nTmp;
            aRect.setY( static_cast< long >(bTwip ? convertMm100ToTwip ( nTmp ) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaLeft" )
        {
            pValue->Value >>= nTmp;
            aRect.setX( static_cast< long >(bTwip ? convertMm100ToTwip ( nTmp ) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaWidth" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Width() = static_cast< long >(bTwip ? convertMm100ToTwip ( nTmp ) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ViewAreaHeight" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Height() = static_cast< long >(bTwip ? convertMm100ToTwip ( nTmp ) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ShowRedlineChanges" )
        {
            bShowRedlineChanges = *static_cast<sal_Bool const *>(pValue->Value.getValue());
            bChangeShowRedline = true;
        }
// Headers and footers are not displayed in BrowseView anymore
        else if ( pValue->Name == "InBrowseMode" )
        {
            bBrowseMode = *static_cast<sal_Bool const *>(pValue->Value.getValue());
            bChangeBrowseMode = true;
        }
        pValue++;
    }
    if( pDoc->GetDocShell() )
        pDoc->GetDocShell()->SetVisArea ( aRect );

    if (bChangeBrowseMode)
        pDoc->getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, bBrowseMode );

    if (bChangeShowRedline)
        GetTextImport()->SetShowChanges( bShowRedlineChanges );
}

// Note: this will be called only if there are OOo elements in settings.xml.
// So if a setting is missing there we can assume that it was written
// by an OOo/LO version that is older than the introduction of the setting!
void SwXMLImport::SetConfigurationSettings(const Sequence < PropertyValue > & aConfigProps)
{
    // this method will modify the document directly -> lock SolarMutex
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

    std::unordered_set< OUString, OUStringHash > aExcludeAlways;
    aExcludeAlways.insert("LinkUpdateMode");
    std::unordered_set< OUString, OUStringHash > aExcludeWhenNotLoadingUserSettings;
    aExcludeWhenNotLoadingUserSettings.insert("ForbiddenCharacters");
    aExcludeWhenNotLoadingUserSettings.insert("IsKernAsianPunctuation");
    aExcludeWhenNotLoadingUserSettings.insert("CharacterCompressionType");
    aExcludeWhenNotLoadingUserSettings.insert("FieldAutoUpdate");
    aExcludeWhenNotLoadingUserSettings.insert("ChartAutoUpdate");
    aExcludeWhenNotLoadingUserSettings.insert("AddParaTableSpacing");
    aExcludeWhenNotLoadingUserSettings.insert("AddParaTableSpacingAtStart");
    aExcludeWhenNotLoadingUserSettings.insert("PrintAnnotationMode");
    aExcludeWhenNotLoadingUserSettings.insert("PrintBlackFonts");
    aExcludeWhenNotLoadingUserSettings.insert("PrintControls");
    aExcludeWhenNotLoadingUserSettings.insert("PrintDrawings");
    aExcludeWhenNotLoadingUserSettings.insert("PrintGraphics");
    aExcludeWhenNotLoadingUserSettings.insert("PrintLeftPages");
    aExcludeWhenNotLoadingUserSettings.insert("PrintPageBackground");
    aExcludeWhenNotLoadingUserSettings.insert("PrintProspect");
    aExcludeWhenNotLoadingUserSettings.insert("PrintReversed");
    aExcludeWhenNotLoadingUserSettings.insert("PrintRightPages");
    aExcludeWhenNotLoadingUserSettings.insert("PrintFaxName");
    aExcludeWhenNotLoadingUserSettings.insert("PrintPaperFromSetup");
    aExcludeWhenNotLoadingUserSettings.insert("PrintTables");
    aExcludeWhenNotLoadingUserSettings.insert("PrintSingleJobs");
    aExcludeWhenNotLoadingUserSettings.insert("UpdateFromTemplate");
    aExcludeWhenNotLoadingUserSettings.insert("PrinterIndependentLayout");
    aExcludeWhenNotLoadingUserSettings.insert("PrintEmptyPages");
    aExcludeWhenNotLoadingUserSettings.insert("SmallCapsPercentage66");
    aExcludeWhenNotLoadingUserSettings.insert("TabOverflow");
    aExcludeWhenNotLoadingUserSettings.insert("UnbreakableNumberings");
    aExcludeWhenNotLoadingUserSettings.insert("ClippedPictures");
    aExcludeWhenNotLoadingUserSettings.insert("BackgroundParaOverDrawings");
    aExcludeWhenNotLoadingUserSettings.insert("TabOverMargin");
    aExcludeWhenNotLoadingUserSettings.insert("PropLineSpacingShrinksFirstLine");
    aExcludeWhenNotLoadingUserSettings.insert("SubtractFlysAnchoredAtFlys");

    sal_Int32 nCount = aConfigProps.getLength();
    const PropertyValue* pValues = aConfigProps.getConstArray();

    SvtSaveOptions aSaveOpt;
    bool bIsUserSetting = aSaveOpt.IsLoadUserSettings();

    // for some properties we don't want to use the application
    // default if they're missing. So we watch for them in the loop
    // below, and set them if not found
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
    bool bPropLineSpacingShrinksFirstLine = false;
    bool bSubtractFlysAnchoredAtFlys = false;

    const PropertyValue* currentDatabaseDataSource = nullptr;
    const PropertyValue* currentDatabaseCommand = nullptr;
    const PropertyValue* currentDatabaseCommandType = nullptr;
    const PropertyValue* embeddedDatabaseName = nullptr;

    while( nCount-- )
    {
        bool bSet = aExcludeAlways.find(pValues->Name) == aExcludeAlways.end();
        if( bSet && !bIsUserSetting
            && (aExcludeWhenNotLoadingUserSettings.find(pValues->Name)
                != aExcludeWhenNotLoadingUserSettings.end()) )
        {
            bSet = false;
        }

        if( bSet )
        {
            try
            {
                if( xInfo->hasPropertyByName( pValues->Name ) )
                {
                    if( pValues->Name == "RedlineProtectionKey" )
                    {
                        Sequence<sal_Int8> aKey;
                        pValues->Value >>= aKey;
                        GetTextImport()->SetChangesProtectionKey( aKey );
                    }
                    else
                    {
                        // HACK: Setting these out of order does not work.
                        if( pValues->Name == "CurrentDatabaseDataSource" )
                            currentDatabaseDataSource = pValues;
                        else if( pValues->Name == "CurrentDatabaseCommand" )
                            currentDatabaseCommand = pValues;
                        else if( pValues->Name == "CurrentDatabaseCommandType" )
                            currentDatabaseCommandType = pValues;
                        else if (pValues->Name == "EmbeddedDatabaseName")
                            embeddedDatabaseName = pValues;
                        else
                            xProps->setPropertyValue( pValues->Name,
                                                  pValues->Value );
                    }
                }

                // did we find any of the non-default cases?
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
                else if ( pValues->Name == "PropLineSpacingShrinksFirstLine" )
                    bPropLineSpacingShrinksFirstLine = true;
                else if (pValues->Name == "SubtractFlysAnchoredAtFlys")
                    bSubtractFlysAnchoredAtFlys = true;
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
        if( currentDatabaseDataSource != nullptr )
            xProps->setPropertyValue( currentDatabaseDataSource->Name, currentDatabaseDataSource->Value );
        if( currentDatabaseCommand != nullptr )
            xProps->setPropertyValue( currentDatabaseCommand->Name, currentDatabaseCommand->Value );
        if( currentDatabaseCommandType != nullptr )
            xProps->setPropertyValue( currentDatabaseCommandType->Name, currentDatabaseCommandType->Value );
        if (embeddedDatabaseName)
            xProps->setPropertyValue(embeddedDatabaseName->Name, embeddedDatabaseName->Value);
    } catch( Exception& )
    {
        OSL_FAIL( "SwXMLImport::SetConfigurationSettings: Exception!" );
    }

    // finally, treat the non-default cases
    // introduce boolean, that indicates a document, written by version prior SO8.
    const bool bDocumentPriorSO8 = !bConsiderWrapOnObjPos;

    if( ! bPrinterIndependentLayout )
    {
        sal_Int16 nTmp = document::PrinterIndependentLayout::DISABLED;
        xProps->setPropertyValue( "PrinterIndependentLayout", Any(nTmp) );
    }

    if( ! bAddExternalLeading )
    {
        xProps->setPropertyValue( "AddExternalLeading", makeAny( false ) );
    }

    if( ! bUseFormerLineSpacing )
    {
        xProps->setPropertyValue( "UseFormerLineSpacing", makeAny( true ) );
    }

    if( !bUseFormerObjectPositioning )
    {
        xProps->setPropertyValue( "UseFormerObjectPositioning", makeAny( true ) );
    }

    if( !bUseOldNumbering )
    {
        xProps->setPropertyValue( "UseOldNumbering", makeAny(true) );
    }

    if( !bOutlineLevelYieldsOutlineRule )
    {
        xProps->setPropertyValue( "OutlineLevelYieldsNumbering", makeAny(true) );
    }

    if( !bAddParaSpacingToTableCells )
    {
        xProps->setPropertyValue( "AddParaSpacingToTableCells",
            makeAny( false ) );
    }

    if( !bUseFormerTextWrapping )
    {
        xProps->setPropertyValue( "UseFormerTextWrapping", makeAny( true ) );
    }

    if( !bConsiderWrapOnObjPos )
    {
        xProps->setPropertyValue( "ConsiderTextWrapOnObjPos", makeAny( false ) );
    }

    // #i47448#
    // For SO7pp4, part of the 'new numbering' stuff has been backported from
    // SO8. Unfortunately, only part of it and by using the same compatibility option
    // like in SO8. Therefore documents generated with SO7pp4, containing
    // numbered paragraphs with first line indent differ between SO7pp4 and
    // SO8. In order to fix this for SO8pp1, I introduce a new compatibility
    // flag 'bIgnoreFirstLineIndentInNumbering'. This flag has to be set for all
    // documents < SO8, but not for SO8. So if the property is not present, the
    // flag will be set to 'true'. SO8 documents surely have the
    // 'ConsiderWrapOnObjPos' property set (no matter if 'true' or 'false'),
    // therefore the correct condition to set this flag is this:
    if( !bIgnoreFirstLineIndentInNumbering && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( "IgnoreFirstLineIndentInNumbering",
            makeAny( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotJustifyLinesWithManualBreak && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( "DoNotJustifyLinesWithManualBreak",
            makeAny( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotResetParaAttrsForNumFont && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( "DoNotResetParaAttrsForNumFont",
            makeAny( true ) );
    }

    if ( !bLoadReadonly )
    {
        xProps->setPropertyValue( "LoadReadonly", makeAny( false ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotCaptureDrawObjsOnPage && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( "DoNotCaptureDrawObjsOnPage",
            makeAny( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bClipAsCharacterAnchoredWriterFlyFrames && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( "ClipAsCharacterAnchoredWriterFlyFrames",
            makeAny( true ) );
    }

    if ( !bUnixForceZeroExtLeading )
    {
        xProps->setPropertyValue( "UnxForceZeroExtLeading", makeAny( true ) );
    }

    if ( !bUseOldPrinterMetrics )
    {
        xProps->setPropertyValue( "UseOldPrinterMetrics", makeAny( true ) );
    }

    // Old LO versions had 66 as the value for small caps percentage, later changed to 80.
    // In order to keep backwards compatibility, SmallCapsPercentage66 option is written to .odt
    // files, and the default for new documents is 'false'. Files without this option
    // are considered to be old files, so set the compatibility option too.
    if ( !bSmallCapsPercentage66 )
    {
        xProps->setPropertyValue( "SmallCapsPercentage66", makeAny( true ) );
    }

    if ( !bTabOverflow )
    {
        xProps->setPropertyValue( "TabOverflow", makeAny( false ) );
    }

    if ( !bUnbreakableNumberings )
    {
        xProps->setPropertyValue( "UnbreakableNumberings", makeAny( false ) );
    }

    if ( !bClippedPictures )
    {
        xProps->setPropertyValue( "ClippedPictures", makeAny( false ) );
    }

    if ( !bBackgroundParaOverDrawings )
        xProps->setPropertyValue("BackgroundParaOverDrawings", makeAny( false ) );

    if ( !bTabOverMargin )
        xProps->setPropertyValue("TabOverMargin", makeAny( false ) );

    if (!bPropLineSpacingShrinksFirstLine)
        xProps->setPropertyValue("PropLineSpacingShrinksFirstLine", makeAny(false));

    if (!bSubtractFlysAnchoredAtFlys)
        xProps->setPropertyValue("SubtractFlysAnchoredAtFlys", makeAny(true));

    SwDoc *pDoc = getDoc();
    SfxPrinter *pPrinter = pDoc->getIDocumentDeviceAccess().getPrinter( false );
    if( pPrinter )
    {
        // If the printer is known, then the OLE objects will
        // already have correct sizes, and we don't have to call
        // PrtOLENotify again. Otherwise we have to call it.
        // The flag might be set from setting the printer, so it
        // it is required to clear it.
        pDoc->SetOLEPrtNotifyPending( !pPrinter->IsKnown() );
    }
}

void SwXMLImport::SetDocumentSpecificSettings(
    const OUString& _rSettingsGroupName,
    const Sequence< PropertyValue>& _rSettings )
{
    // the only doc-specific settings group we know so far are the XForms settings
    if ( !IsXMLToken( _rSettingsGroupName, XML_XFORM_MODEL_SETTINGS ) )
        return;

    // preserve the settings for a later iteration - we are currently reading the settings.xml,
    // the content.xml will be read later, by another instance of SwXMLImport
    OSL_ENSURE( m_xLateInitSettings.is(), "SwXMLImport::SetDocumentSpecificSettings: no storage for those settings!" );
    if ( !m_xLateInitSettings.is() )
        return;

    try
    {
        if ( m_xLateInitSettings->hasByName( _rSettingsGroupName ) )
        {
            m_xLateInitSettings->replaceByName( _rSettingsGroupName, makeAny( _rSettings ) );
            OSL_FAIL( "SwXMLImport::SetDocumentSpecificSettings: already have settings for this model!" );
        }
        else
            m_xLateInitSettings->insertByName( _rSettingsGroupName, makeAny( _rSettings ) );
    }
    catch( const Exception& )
    {
    }
}

void SwXMLImport::initialize(
    const Sequence<Any>& aArguments )
    throw( uno::Exception, uno::RuntimeException, std::exception)
{
    // delegate to super class
    SvXMLImport::initialize(aArguments);

    // we are only looking for a PropertyValue "PreserveRedlineMode"
    sal_Int32 nLength = aArguments.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        beans::PropertyValue aValue;
        if ( aArguments[i] >>= aValue )
        {
            if (aValue.Name == "PreserveRedlineMode")
            {
                OSL_VERIFY( aValue.Value >>= m_bPreserveRedlineMode );
            }
            continue;
        }

        beans::NamedValue aNamedValue;
        if ( aArguments[i] >>= aNamedValue )
        {
            if (aNamedValue.Name == "LateInitSettings")
            {
                OSL_VERIFY( aNamedValue.Value >>= m_xLateInitSettings );
            }
        }
    }
}

SwDoc* SwImport::GetDocFromXMLImport( SvXMLImport& rImport )
{
    uno::Reference<lang::XUnoTunnel> xModelTunnel( rImport.GetModel(), uno::UNO_QUERY );
    SwXTextDocument *pTextDoc = reinterpret_cast< SwXTextDocument *>(
            sal::static_int_cast< sal_IntPtr >(  xModelTunnel->getSomething(SwXTextDocument::getUnoTunnelId() )));
    assert( pTextDoc );
    assert( pTextDoc->GetDocShell() );
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    OSL_ENSURE( pDoc, "Where is my document?" );
    return pDoc;
}

void SwXMLImport::initXForms()
{
    // obtain SwDoc
    Reference<XUnoTunnel> xDocTunnel( GetModel(), UNO_QUERY );
    if( ! xDocTunnel.is() )
        return;
    SwXTextDocument* pXTextDocument = reinterpret_cast<SwXTextDocument*>(
        xDocTunnel->getSomething( SwXTextDocument::getUnoTunnelId() ) );
    if( pXTextDocument == nullptr )
        return;

    SwDoc *pDoc = pXTextDocument->GetDocShell()->GetDoc();

    // init XForms (if not already done)
    // (no default model, since we'll load the models)
    if( ! pDoc->isXForms() )
        pDoc->initXForms( false );

    m_bInititedXForms = true;
}

SwDoc* SwXMLImport::getDoc()
{
    if( m_pDoc != nullptr )
        return m_pDoc;
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    assert( xTextTunnel.is());
    SwXText *pText = reinterpret_cast< SwXText *>(
            sal::static_int_cast< sal_IntPtr >( xTextTunnel->getSomething( SwXText::getUnoTunnelId() )));
    assert( pText != nullptr );
    m_pDoc = pText->GetDoc();
    assert( m_pDoc != nullptr );
    return m_pDoc;
}

const SwDoc* SwXMLImport::getDoc() const
{
    return const_cast< SwXMLImport* >( this )->getDoc();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_XMLOasisImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, OUString("com.sun.star.comp.Writer.XMLOasisImporter"),
                SvXMLImportFlags::ALL));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, OUString("com.sun.star.comp.Writer.XMLOasisStylesImporter"),
                SvXMLImportFlags::STYLES | SvXMLImportFlags::MASTERSTYLES | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, OUString("com.sun.star.comp.Writer.XMLOasisContentImporter"),
                SvXMLImportFlags::CONTENT | SvXMLImportFlags::SCRIPTS | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, OUString("com.sun.star.comp.Writer.XMLOasisMetaImporter"),
                SvXMLImportFlags::META));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, OUString("com.sun.star.comp.Writer.XMLOasisSettingsImporter"),
                SvXMLImportFlags::SETTINGS));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
