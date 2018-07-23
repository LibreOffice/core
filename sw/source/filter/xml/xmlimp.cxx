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

#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
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
#include <IDocumentListsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <unofreg.hxx>
#include <TextCursorHelper.hxx>
#include <unotext.hxx>
#include <unotextrange.hxx>
#include <unocrsr.hxx>
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <editsh.hxx>
#include <strings.hrc>
#include <svl/stritem.hxx>
#include "xmlimp.hxx"
#include "xmlimpit.hxx"
#include "xmltexti.hxx"
#include <list.hxx>
#include <swdll.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <docsh.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/printer.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/helpers.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/svapp.hxx>
#include <unotxdoc.hxx>
#include <numrule.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xformsimport.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

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
};

static const SvXMLTokenMapEntry aDocTokenMap[] =
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
                const OUString& rLName );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) override;
};

SwXMLBodyContext_Impl::SwXMLBodyContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // tdf#107211: if at this point we don't have a defined char style "Default"
    // or "Default Style", add a mapping for it as it is not written
    // into the file since it's not really a style but "no style"
    // (hence referencing it actually makes no sense except for hyperlinks
    // which default to something other than "Default")
    OUString const sDefault(SwResId(STR_POOLCOLL_STANDARD));
    uno::Reference<container::XNameContainer> const& xStyles(
            rImport.GetTextImport()->GetTextStyles());
    if (!xStyles->hasByName("Default"))
    {   // this old name was used before LO 4.0
        rImport.AddStyleDisplayName(XML_STYLE_FAMILY_TEXT_TEXT, "Default", sDefault);
    }
    if (!xStyles->hasByName("Default_20_Style"))
    {   // this new name contains a space which is converted to _20_ on export
        rImport.AddStyleDisplayName(XML_STYLE_FAMILY_TEXT_TEXT, "Default_20_Style", sDefault);
    }
    bool isEncoded(false);
    OUString const defaultEncoded(
        rImport.GetMM100UnitConverter().encodeStyleName(sDefault, &isEncoded));
    if (isEncoded && defaultEncoded != "Default_20_Style"
        && !xStyles->hasByName(defaultEncoded))
    {   // new name may contain a space which is converted to _20_ on export
        rImport.AddStyleDisplayName(XML_STYLE_FAMILY_TEXT_TEXT, defaultEncoded, sDefault);
    }
}

SvXMLImportContextRef SwXMLBodyContext_Impl::CreateChildContext(
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
                const OUString& rLName );

    SwXMLDocContext_Impl( SwXMLImport& rImport );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

SwXMLDocContext_Impl::SwXMLDocContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLDocContext_Impl::SwXMLDocContext_Impl( SwXMLImport& rImport ) :
    SvXMLImportContext( rImport )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SwXMLDocContext_Impl::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport() );
}

SvXMLImportContextRef SwXMLDocContext_Impl::CreateChildContext(
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
                                              rLocalName );
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
                const Reference< document::XDocumentProperties >& xDocProps);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
};

SwXMLOfficeDocContext_Impl::SwXMLOfficeDocContext_Impl(
                SwXMLImport& rImport,
                const Reference< document::XDocumentProperties >& xDocProps) :
    SvXMLImportContext( rImport ),
    SwXMLDocContext_Impl( rImport ),
    SvXMLMetaDocumentContext( rImport, xDocProps )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SwXMLOfficeDocContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    // assign paragraph styles to list levels of outline style after all styles
    // are imported and finished. This is the case, when <office:body> starts
    // in flat OpenDocument file format.
    {
        if( nElement == XML_ELEMENT( OFFICE, XML_BODY ) )
        {
            GetImport().GetTextImport()->SetOutlineStyles( true );
        }
    }

    // behave like meta base class iff we encounter office:meta
    if ( nElement == XML_ELEMENT( OFFICE, XML_META ) ) {
        return SvXMLMetaDocumentContext::createFastChildContext(
                    nElement, xAttrList );
    } else {
        return SwXMLDocContext_Impl::createFastChildContext(
                    nElement, xAttrList );
    }
}

// #i69629# - new subclass <SwXMLDocStylesContext_Impl> of class <SwXMLDocContext_Impl>
class SwXMLDocStylesContext_Impl : public SwXMLDocContext_Impl
{
public:

    SwXMLDocStylesContext_Impl( SwXMLImport& rImport,
                                sal_uInt16 nPrfx,
                                const OUString& rLName );

    virtual void EndElement() override;
};

SwXMLDocStylesContext_Impl::SwXMLDocStylesContext_Impl(
                    SwXMLImport& rImport,
                    sal_uInt16 nPrfx,
                    const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    SwXMLDocContext_Impl( rImport, nPrfx, rLName )
{
}

void SwXMLDocStylesContext_Impl::EndElement()
{
    // assign paragraph styles to list levels of outline style after all styles
    // are imported and finished.
    SwXMLImport& rSwImport = dynamic_cast<SwXMLImport&>( GetImport());
    GetImport().GetTextImport()->SetOutlineStyles(
            bool(rSwImport.GetStyleFamilyMask() & SfxStyleFamily::Para));
}

const SvXMLTokenMap& SwXMLImport::GetDocElemTokenMap()
{
    if( !m_pDocElemTokenMap )
        m_pDocElemTokenMap.reset( new SvXMLTokenMap( aDocTokenMap ) );

    return *m_pDocElemTokenMap;
}

SvXMLImportContext *SwXMLImport::CreateDocumentContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    // #i69629# - own subclasses for <office:document> and <office:document-styles>
    if( XML_NAMESPACE_OFFICE==nPrefix &&
        ( IsXMLToken( rLocalName, XML_DOCUMENT_SETTINGS ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_CONTENT ) ))
        pContext = new SwXMLDocContext_Impl( *this, nPrefix, rLocalName );
    else if ( XML_NAMESPACE_OFFICE==nPrefix &&
              IsXMLToken( rLocalName, XML_DOCUMENT_STYLES ) )
    {
        pContext = new SwXMLDocStylesContext_Impl( *this, nPrefix, rLocalName );
    }
    else
        pContext = SvXMLImport::CreateDocumentContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

SvXMLImportContext *SwXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
            pContext = CreateMetaContext(nElement);
        break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT ):
        {
            uno::Reference<document::XDocumentProperties> const xDocProps(
                GetDocumentProperties());
            // flat OpenDocument file format
            pContext = new SwXMLOfficeDocContext_Impl( *this, xDocProps );
        }
        break;
        default:
            pContext = new SvXMLImportContext( *this );
    }
    return pContext;
}

SwXMLImport::SwXMLImport(
    const uno::Reference< uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags)
:   SvXMLImport( rContext, implementationName, nImportFlags ),
    m_pTableItemMapper( nullptr ),
    m_pDocElemTokenMap( nullptr ),
    m_pTableElemTokenMap( nullptr ),
    m_pTableCellAttrTokenMap( nullptr ),
    m_nStyleFamilyMask( SfxStyleFamily::All ),
    m_bLoadDoc( true ),
    m_bInsert( false ),
    m_bBlock( false ),
    m_bOrganizerMode( false ),
    m_bInititedXForms( false ),
    m_pDoc( nullptr )
{
    InitItemImport();
}

SwXMLImport::~SwXMLImport() throw ()
{
    if (HasShapeImport())
    {
        SAL_WARN("sw", "endDocument skipped, dropping shapes now to avoid dangling SvTextShapeImportHelper pointing to this");
        ClearShapeImport();
    }
    m_pDocElemTokenMap.reset();
    m_pTableElemTokenMap.reset();
    m_pTableCellAttrTokenMap.reset();
    FinitItemImport();
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

void SwXMLImport::setStyleInsertMode( SfxStyleFamily nFamilies,
                                      bool bOverwrite )
{
    m_bInsert = !bOverwrite;
    m_nStyleFamilyMask = nFamilies;
    m_bLoadDoc = false;
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
                SfxStyleFamily nFamilyMask = SfxStyleFamily::None;
                sal_Int32 nCount = aFamiliesSeq.getLength();
                const OUString *pSeq = aFamiliesSeq.getConstArray();
                for( sal_Int32 i=0; i < nCount; i++ )
                {
                    const OUString& rFamily = pSeq[i];
                    if( rFamily=="FrameStyles" )
                        nFamilyMask |= SfxStyleFamily::Frame;
                    else if( rFamily=="PageStyles" )
                        nFamilyMask |= SfxStyleFamily::Page;
                    else if( rFamily=="CharacterStyles" )
                        nFamilyMask |= SfxStyleFamily::Char;
                    else if( rFamily=="ParagraphStyles" )
                        nFamilyMask |= SfxStyleFamily::Para;
                    else if( rFamily=="NumberingStyles" )
                        nFamilyMask |= SfxStyleFamily::Pseudo;
                }

                bool bOverwrite = false;
                const OUString sStyleInsertModeOverwrite("StyleInsertModeOverwrite");
                if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeOverwrite) )
                {
                    aAny = xImportInfo->getPropertyValue(sStyleInsertModeOverwrite);
                    if( auto b = o3tl::tryAccess<bool>(aAny) )
                    {
                        if( *b )
                            bOverwrite = true;
                    }
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
            if( auto b = o3tl::tryAccess<bool>(aAny) )
            {
                if( *b )
                    m_bBlock = true;
            }
        }

        // organizer mode
        const OUString sOrganizerMode("OrganizerMode");
        if( xPropertySetInfo->hasPropertyByName(sOrganizerMode) )
        {
            aAny = xImportInfo->getPropertyValue(sOrganizerMode);
            if( auto b = o3tl::tryAccess<bool>(aAny) )
            {
                if( *b )
                    m_bOrganizerMode = true;
            }
        }

        // default document properties
        const OUString sDefSettings("DefaultDocumentSettings");
        if (xPropertySetInfo->hasPropertyByName(sDefSettings))
        {
            aAny = xImportInfo->getPropertyValue(sDefSettings);
            Sequence<PropertyValue> aProps;
            if (aAny >>= aProps)
            {
                Reference<lang::XMultiServiceFactory> xFac(GetModel(), UNO_QUERY);
                Reference<XPropertySet> xProps(
                    xFac->createInstance("com.sun.star.document.Settings"), UNO_QUERY);
                Reference<XPropertySetInfo> xInfo(xProps->getPropertySetInfo());

                if (xProps.is() && xInfo.is())
                {
                    for (const auto& rProp : aProps)
                    {
                        if (xInfo->hasPropertyByName(rProp.Name))
                        {
                            xProps->setPropertyValue(rProp.Name, rProp.Value);
                        }
                    }
                }
            }
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

    if( !(getImportFlags() & (SvXMLImportFlags::CONTENT|SvXMLImportFlags::MASTERSTYLES)) )
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
        m_pSttNdIdx.reset(new SwNodeIndex( pDoc->GetNodes() ));
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

    // SJ: #i49801# locking the model to disable repaints
    SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    if ( pDrawModel )
        pDrawModel->setLock(true);

    if (!GetGraphicStorageHandler().is())
    {
        m_xGraphicStorageHandler = SvXMLGraphicHelper::Create(SvXMLGraphicHelperMode::Read);
        SetGraphicStorageHandler(m_xGraphicStorageHandler.get());
    }

    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            m_xEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            SvXMLEmbeddedObjectHelperMode::Read );
            Reference< document::XEmbeddedObjectResolver > xEmbeddedResolver( m_xEmbeddedResolver.get() );
            SetEmbeddedResolver( xEmbeddedResolver );
        }
    }
}

void SwXMLImport::endDocument()
{
    OSL_ENSURE( GetModel().is(), "model missing; maybe startDocument wasn't called?" );
    if( !GetModel().is() )
        return;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    if (m_xGraphicStorageHandler)
        m_xGraphicStorageHandler->dispose();
    m_xGraphicStorageHandler.clear();

    if( m_xEmbeddedResolver )
        m_xEmbeddedResolver->dispose();
    m_xEmbeddedResolver.clear();
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

            // tdf#113877
            // when we insert one document with list inside into another one with list at the insert position,
            // the resulting numbering in these lists is not consequent.
            //
            // Main document:
            //  1. One
            //  2. Two
            //  3. Three
            //  4.                      <-- insert position
            //
            // Inserted document:
            //  1. One
            //  2. Two
            //  3. Three
            //  4.
            //
            // Expected result
            //  1. One
            //  2. Two
            //  3. Three
            //  4. One
            //  5. Two
            //  6. Three
            //  7.
            //
            MergeListsAtDocumentInsertPosition(pDoc);
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

    m_pSttNdIdx.reset();

    // SJ: #i49801# -> now permitting repaints
    if ( pDoc )
    {
        if( getImportFlags() == SvXMLImportFlags::ALL )
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

                const OUString& sXFormsSettingsName( GetXMLToken( XML_XFORM_MODEL_SETTINGS ) );
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

// tdf#113877
// when we insert one document with list inside into another one with list at the insert position,
// the resulting numbering in these lists is not consequent.
//
// CASE-1: Main document:
//  1. One
//  2. Two
//  3. Three
//  4.                      <-- insert position
//
// Inserted document:
//  1. One
//  2. Two
//  3. Three
//  4.
//
// Expected result
//  1. One
//  2. Two
//  3. Three
//  4. One
//  5. Two
//  6. Three
//  7.
//
// CASE-2: Main document:
//  1. One
//  2. Two
//  3. Three
//  4.                      <-- insert position
//
// Inserted document:
//  A) One
//  B) Two
//  C) Three
//  D)
//
// Expected result
//  1. One
//  2. Two
//  3. Three
//  4. One
//  A) Two
//  B) Three
//  5.
//
void SwXMLImport::MergeListsAtDocumentInsertPosition(SwDoc *pDoc)
{
    // 1. check environment
    if (! pDoc)
        return;

    if (! IsInsertMode() || ! m_pSttNdIdx->GetIndex())
        return;

    sal_uLong index = 1;

    // the last node of the main document where we have inserted a document
    const SwNodePtr node1 = pDoc->GetNodes()[m_pSttNdIdx->GetIndex() + 0];

    // the first node of the inserted document
    SwNodePtr node2 = pDoc->GetNodes()[m_pSttNdIdx->GetIndex() + index];

    if (! (node1 && node2
        && (node1->GetNodeType() == node2->GetNodeType())
        && (node1->IsTextNode() == node2->IsTextNode())
        ))
    {
        // not a text node at insert position
        return;
    }

    // 2. get the first node of the inserted document,
    // which will be used to detect if inside inserted document a new list was started after the first list
    const SfxPoolItem* pListId2Initial = nullptr;
    {
        SwContentNode* contentNode1 = static_cast<SwContentNode *>(node1);
        SwContentNode* contentNode2 = static_cast<SwContentNode *>(node2);

        // check if both lists have the same list properties
        const SfxPoolItem* pListId1 = contentNode1->GetNoCondAttr( RES_PARATR_LIST_ID, false );
        const SfxPoolItem* pListId2 = contentNode2->GetNoCondAttr( RES_PARATR_LIST_ID, false );

        if (! pListId1)
            return;
        if (! pListId2)
            return;

        const OUString& sListId1 = dynamic_cast<const SfxStringItem*>(pListId1)->GetValue();
        const OUString& sListId2 = dynamic_cast<const SfxStringItem*>(pListId2)->GetValue();

        const SwList* pList1 = pDoc->getIDocumentListsAccess().getListByName( sListId1 );
        const SwList* pList2 = pDoc->getIDocumentListsAccess().getListByName( sListId2 );

        if (! pList1)
            return;
        if (! pList2)
            return;

        const OUString& sDefaultListStyleName1 = pList1->GetDefaultListStyleName();
        const OUString& sDefaultListStyleName2 = pList2->GetDefaultListStyleName();

        if (sDefaultListStyleName1 != sDefaultListStyleName2)
        {
            const SwNumRule* pNumRule1 = pDoc->FindNumRulePtr( sDefaultListStyleName1 );
            const SwNumRule* pNumRule2 = pDoc->FindNumRulePtr( sDefaultListStyleName2 );

            if (pNumRule1 && pNumRule2)
            {
                // check style of the each list level
                for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
                {
                    if( !( pNumRule1->Get( n ) == pNumRule2->Get( n ) ))
                    {
                        return;
                    }
                }

                // our list should be merged
                pListId2Initial = pListId2;
            }
        }
        else
        {
            // our list should be merged
            pListId2Initial = pListId2;
        }
    }

    if (! pListId2Initial)
    {
        // two lists have different styles => they should not be merged
        return;
    }

    // 3. merge two lists
    while (
        node1 && node2
        && (node1->GetNodeType() == node2->GetNodeType())
        && (node1->IsTextNode() == node2->IsTextNode())
        )
    {
        SwContentNode* contentNode1 = static_cast<SwContentNode *>( node1 );
        SwContentNode* contentNode2 = static_cast<SwContentNode *>( node2 );

        const SfxPoolItem* pListId1 = contentNode1->GetNoCondAttr( RES_PARATR_LIST_ID, false );
        const SfxPoolItem* pListId2 = contentNode2->GetNoCondAttr( RES_PARATR_LIST_ID, false );

        if (! pListId1)
            return;
        if (! pListId2)
            return;

        if (*pListId2Initial != *pListId2)
        {
            // no more list items of the first list inside inserted document
            return;
        }

        // set list style to this list element
        contentNode2->SetAttr(*pListId1);

        // get next item
        index++;
        if (index >= pDoc->GetNodes().Count())
        {
            // no more items
            return;
        }

        node2 = pDoc->GetNodes()[m_pSttNdIdx->GetIndex() + index];
    }
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
    virtual ~SvTextShapeImportHelper() override;
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
                                      IsStylesOnlyMode(),
                                      IsBlockMode(), m_bOrganizerMode );
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
    if (IsInsertMode() || IsStylesOnlyMode() || IsBlockMode() || m_bOrganizerMode || !GetModel().is() )
        return;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    SwDoc *pDoc = getDoc();
    tools::Rectangle aRect;
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
    bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MapUnit::MapTwip;
    //sal_Bool bTwip = pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit ( ) == MapUnit::MapTwip;

    for (sal_Int32 i = 0; i < nCount ; i++)
    {
        if ( pValue->Name == "ViewAreaTop" )
        {
            pValue->Value >>= nTmp;
            aRect.setY( static_cast< long >(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaLeft" )
        {
            pValue->Value >>= nTmp;
            aRect.setX( static_cast< long >(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp) );
        }
        else if ( pValue->Name == "ViewAreaWidth" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.setWidth( static_cast< long >(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp) );
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ViewAreaHeight" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.setHeight( static_cast< long >(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp) );
            aRect.SetSize( aSize );
        }
        else if ( pValue->Name == "ShowRedlineChanges" )
        {
            bShowRedlineChanges = *o3tl::doAccess<bool>(pValue->Value);
            bChangeShowRedline = true;
        }
// Headers and footers are not displayed in BrowseView anymore
        else if ( pValue->Name == "InBrowseMode" )
        {
            bBrowseMode = *o3tl::doAccess<bool>(pValue->Value);
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

    std::unordered_set< OUString > aExcludeAlways;
    aExcludeAlways.insert("LinkUpdateMode");
    std::unordered_set< OUString > aExcludeWhenNotLoadingUserSettings;
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
    aExcludeWhenNotLoadingUserSettings.insert("DisableOffPagePositioning");

    sal_Int32 nCount = aConfigProps.getLength();
    const PropertyValue* pValues = aConfigProps.getConstArray();

    SvtSaveOptions aSaveOpt;
    bool bIsUserSetting = aSaveOpt.IsLoadUserSettings();

    // for some properties we don't want to use the application
    // default if they're missing. So we watch for them in the loop
    // below, and set them if not found
    bool bPrinterIndependentLayout = false;
    bool bUseOldNumbering = false;
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
    bool bSmallCapsPercentage66 = false;
    bool bTabOverflow = false;
    bool bUnbreakableNumberings = false;
    bool bClippedPictures = false;
    bool bBackgroundParaOverDrawings = false;
    bool bTabOverMargin = false;
    bool bPropLineSpacingShrinksFirstLine = false;
    bool bSubtractFlysAnchoredAtFlys = false;
    bool bDisableOffPagePositioning = false;

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
                else if (pValues->Name == "DisableOffPagePositioning")
                    bDisableOffPagePositioning = true;
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
        xProps->setPropertyValue( "PrinterIndependentLayout", Any(sal_Int16(document::PrinterIndependentLayout::DISABLED)) );
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

    if ( bDisableOffPagePositioning )
        xProps->setPropertyValue("DisableOffPagePositioning", makeAny(true));

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
{
    // delegate to super class
    SvXMLImport::initialize(aArguments);

    // we are only looking for a NamedValue "LateInitSettings"
    sal_Int32 nLength = aArguments.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
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

SwDoc* SwImport::GetDocFromXMLImport( SvXMLImport const & rImport )
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, "com.sun.star.comp.Writer.XMLOasisImporter",
                SvXMLImportFlags::ALL));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, "com.sun.star.comp.Writer.XMLOasisStylesImporter",
                SvXMLImportFlags::STYLES | SvXMLImportFlags::MASTERSTYLES | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, "com.sun.star.comp.Writer.XMLOasisContentImporter",
                SvXMLImportFlags::CONTENT | SvXMLImportFlags::SCRIPTS | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, "com.sun.star.comp.Writer.XMLOasisMetaImporter",
                SvXMLImportFlags::META));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, "com.sun.star.comp.Writer.XMLOasisSettingsImporter",
                SvXMLImportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportFODT(SvStream &rStream)
{
    SwGlobals::ensure();

    SfxObjectShellLock xDocSh(new SwDocShell(SfxObjectCreateMode::INTERNAL));
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance("com.sun.star.comp.Writer.XmlFilterAdaptor"), uno::UNO_QUERY_THROW);

    css::uno::Sequence<OUString> aUserData(7);
    aUserData[0] = "com.sun.star.comp.filter.OdfFlatXml";
    aUserData[2] = "com.sun.star.comp.Writer.XMLOasisImporter";
    aUserData[3] = "com.sun.star.comp.Writer.XMLOasisExporter";
    aUserData[6] = "true";
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs(1);
    aOuterArgs[0] <<= aAdaptorArgs;

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(OUString("private:stream")) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while its considered uninitialized,
    //setting a property will inform the document its modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportDOCX(SvStream &rStream)
{
    SwGlobals::ensure();

    SfxObjectShellLock xDocSh(new SwDocShell(SfxObjectCreateMode::INTERNAL));
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));

    uno::Reference<document::XFilter> xFilter(xMultiServiceFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::makeAny(xStream) },
        { "InputMode", uno::makeAny(true) },
    }));
    xImporter->setTargetDocument(xModel);

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while its considered uninitialized,
    //setting a property will inform the document its modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = false;
    try
    {
        ret = xFilter->filter(aArgs);
    }
    catch (const css::io::IOException&)
    {
    }
    catch (const css::lang::IllegalArgumentException&)
    {
    }
    catch (const css::lang::WrappedTargetRuntimeException&)
    {
    }
    catch (const std::exception&)
    {
    }
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
