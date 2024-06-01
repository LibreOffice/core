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
#include <sal/log.hxx>

#include <cassert>

#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <o3tl/any.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/txtimp.hxx>
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
#include <TextCursorHelper.hxx>
#include <unotext.hxx>
#include <unotextrange.hxx>
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
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/fcm.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <vcl/svapp.hxx>
#include <unotxdoc.hxx>
#include <numrule.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xformsimport.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <officecfg/Office/Common.hxx>

#include <unordered_set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xforms;
using namespace ::xmloff::token;

namespace {

class SwXMLBodyContext_Impl : public SvXMLImportContext
{
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:

    SwXMLBodyContext_Impl( SwXMLImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

}

SwXMLBodyContext_Impl::SwXMLBodyContext_Impl( SwXMLImport& rImport ) :
    SvXMLImportContext( rImport )
{
    // tdf#107211: if at this point we don't have a defined char style "Default"
    // or "Default Style", add a mapping for it as it is not written
    // into the file since it's not really a style but "no style"
    // (hence referencing it actually makes no sense except for hyperlinks
    // which default to something other than "Default")
    OUString const sDefault(SwResId(STR_POOLCHR_STANDARD));
    uno::Reference<container::XNameContainer> const& xStyles(
            rImport.GetTextImport()->GetTextStyles());
    if (!xStyles->hasByName(u"Default"_ustr))
    {   // this old name was used before LO 4.0
        rImport.AddStyleDisplayName(XmlStyleFamily::TEXT_TEXT, u"Default"_ustr, sDefault);
    }
    if (!xStyles->hasByName(u"Default_20_Style"_ustr))
    {   // this new name contains a space which is converted to _20_ on export
        rImport.AddStyleDisplayName(XmlStyleFamily::TEXT_TEXT, u"Default_20_Style"_ustr, sDefault);
    }
    bool isEncoded(false);
    OUString const defaultEncoded(
        rImport.GetMM100UnitConverter().encodeStyleName(sDefault, &isEncoded));
    if (isEncoded && defaultEncoded != "Default_20_Style"
        && !xStyles->hasByName(defaultEncoded))
    {   // new name may contain a space which is converted to _20_ on export
        rImport.AddStyleDisplayName(XmlStyleFamily::TEXT_TEXT, defaultEncoded, sDefault);
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLBodyContext_Impl::createFastChildContext(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return GetSwImport().CreateBodyContentContext();
}

namespace {

// #i69629#
// enhance class <SwXMLDocContext_Impl> in order to be able to create subclasses
// NB: virtually inherit so we can multiply inherit properly
//     in SwXMLOfficeDocContext_Impl
class SwXMLDocContext_Impl : public virtual SvXMLImportContext
{
    sal_Int32 mnElement;

protected: // #i69629#
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:
    SwXMLDocContext_Impl( SwXMLImport& rImport, sal_Int32 nElement );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

}

SwXMLDocContext_Impl::SwXMLDocContext_Impl( SwXMLImport& rImport, sal_Int32 nElement ) :
    SvXMLImportContext( rImport ), mnElement(nElement)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SwXMLDocContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_SCRIPTS):
            return GetSwImport().CreateScriptContext();
        case XML_ELEMENT(OFFICE, XML_SETTINGS):
            return new XMLDocumentSettingsContext( GetImport() );
        case XML_ELEMENT(OFFICE, XML_STYLES):
            GetSwImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            return GetSwImport().CreateStylesContext( false );
        case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
            // don't use the autostyles from the styles-document for the progress
            if ( mnElement != 0 && (mnElement & TOKEN_MASK) != XML_DOCUMENT_STYLES )
                GetSwImport().GetProgressBarHelper()->Increment
                    ( PROGRESS_BAR_STEP );
            return GetSwImport().CreateStylesContext( true );
        case XML_ELEMENT(OFFICE, XML_MASTER_STYLES):
            return GetSwImport().CreateMasterStylesContext();
        case XML_ELEMENT(OFFICE, XML_FONT_FACE_DECLS):
            return GetSwImport().CreateFontDeclsContext();
        case XML_ELEMENT(OFFICE, XML_META):
            OSL_FAIL(" XML_ELEMENT(OFFICE, XML_META): should not have come here, maybe document is invalid?");
            break;
        case XML_ELEMENT(OFFICE, XML_BODY):
            GetSwImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            return new SwXMLBodyContext_Impl( GetSwImport() );
        case XML_ELEMENT(XFORMS, XML_MODEL):
            return createXFormsModelContext(GetImport());
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("sw", nElement);
    }
    return nullptr;
}

namespace {

// #i69629# - new subclass <SwXMLOfficeDocContext_Impl> of class <SwXMLDocContext_Impl>
class SwXMLOfficeDocContext_Impl :
         public SwXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:

    SwXMLOfficeDocContext_Impl( SwXMLImport& rImport, sal_Int32 nElement,
                const Reference< document::XDocumentProperties >& xDocProps);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
};

}

SwXMLOfficeDocContext_Impl::SwXMLOfficeDocContext_Impl(
                SwXMLImport& rImport,
                sal_Int32 nElement,
                const Reference< document::XDocumentProperties >& xDocProps) :
    SvXMLImportContext( rImport ),
    SwXMLDocContext_Impl( rImport, nElement ),
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

namespace {

// #i69629# - new subclass <SwXMLDocStylesContext_Impl> of class <SwXMLDocContext_Impl>
class SwXMLDocStylesContext_Impl : public SwXMLDocContext_Impl
{
public:

    SwXMLDocStylesContext_Impl( SwXMLImport& rImport, sal_Int32 nElement );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

}

SwXMLDocStylesContext_Impl::SwXMLDocStylesContext_Impl( SwXMLImport& rImport, sal_Int32 nElement ) :
    SvXMLImportContext( rImport ),
    SwXMLDocContext_Impl( rImport, nElement )
{
}

void SwXMLDocStylesContext_Impl::endFastElement(sal_Int32 )
{
    // assign paragraph styles to list levels of outline style after all styles
    // are imported and finished.
    SwXMLImport& rSwImport = dynamic_cast<SwXMLImport&>( GetImport());
    GetImport().GetTextImport()->SetOutlineStyles(
            bool(rSwImport.GetStyleFamilyMask() & SfxStyleFamily::Para));
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
            pContext = new SwXMLOfficeDocContext_Impl( *this, nElement, xDocProps );
        }
        break;
        // #i69629# - own subclasses for <office:document> and <office:document-styles>
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_SETTINGS):
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_CONTENT):
            pContext = new SwXMLDocContext_Impl( *this, nElement );
            break;
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_STYLES):
            pContext = new SwXMLDocStylesContext_Impl( *this, nElement );
            break;
    }
    return pContext;
}

SwXMLImport::SwXMLImport(
    const uno::Reference< uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags)
:   SvXMLImport( rContext, implementationName, nImportFlags ),
    m_nStyleFamilyMask( SfxStyleFamily::All ),
    m_bLoadDoc( true ),
    m_bInsert( false ),
    m_bBlock( false ),
    m_bOrganizerMode( false ),
    m_bInititedXForms( false ),
    m_pDoc( nullptr ),
    m_sDefTableName(SwResId(STR_TABLE_DEFNAME))
{
    InitItemImport();
}

SwXMLImport::~SwXMLImport() noexcept
{
    if (HasShapeImport())
    {
        SAL_WARN("sw", "endDocument skipped, dropping shapes now to avoid dangling SvTextShapeImportHelper pointing to this");
        ClearShapeImport();
    }
    FinitItemImport();
    // Call cleanup() here because the destruction of some stuff like XMLRedlineImportHelper will cast
    // to cast their mrImport to SwXMLImport and that is illegal after this destructor is done.
    cleanup();
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

static OTextCursorHelper *lcl_xml_GetSwXTextCursor( const Reference < XTextCursor >& rTextCursor )
{
    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(rTextCursor.get());
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
        OUString sStyleInsertModeFamilies(u"StyleInsertModeFamilies"_ustr);
        if( xPropertySetInfo->hasPropertyByName(sStyleInsertModeFamilies) )
        {
            aAny = xImportInfo->getPropertyValue(sStyleInsertModeFamilies);
            Sequence< OUString> aFamiliesSeq;
            if( aAny >>= aFamiliesSeq )
            {
                SfxStyleFamily nFamilyMask = SfxStyleFamily::None;
                for (const OUString& rFamily : aFamiliesSeq)
                {
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
                static constexpr OUString sStyleInsertModeOverwrite(u"StyleInsertModeOverwrite"_ustr);
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
        static constexpr OUString sTextInsertModeRange(u"TextInsertModeRange"_ustr);
        if( xPropertySetInfo->hasPropertyByName(sTextInsertModeRange) )
        {
            aAny = xImportInfo->getPropertyValue(sTextInsertModeRange);
            Reference<XTextRange> xInsertTextRange;
            if( aAny >>= xInsertTextRange )
                setTextInsertMode( xInsertTextRange );
        }

        // auto text mode
        static constexpr OUString sAutoTextMode(u"AutoTextMode"_ustr);
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
        static constexpr OUString sOrganizerMode(u"OrganizerMode"_ustr);
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
        static constexpr OUString sDefSettings(u"DefaultDocumentSettings"_ustr);
        if (xPropertySetInfo->hasPropertyByName(sDefSettings))
        {
            aAny = xImportInfo->getPropertyValue(sDefSettings);
            Sequence<PropertyValue> aProps;
            if (aAny >>= aProps)
            {
                Reference<lang::XMultiServiceFactory> xFac(GetModel(), UNO_QUERY);
                Reference<XPropertySet> xProps(
                    xFac->createInstance(u"com.sun.star.document.Settings"_ustr), UNO_QUERY);
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

            // Is there an edit shell. If yes, then we are currently inserting
            // a document. We then have to insert at the current edit shell's
            // cursor position. That not quite clean code, but there is no other
            // way currently.
            pCursorSh = pDoc->GetEditShell();
        }
        if( pCursorSh )
        {
            const rtl::Reference<SwXTextRange> xInsertTextRange(
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
        m_oSttNdIdx.emplace( pDoc->GetNodes() );
        if( IsInsertMode() )
        {
            SwPaM *pPaM = pTextCursor->GetPaM();
            const SwPosition* pPos = pPaM->GetPoint();

            // Split once and remember the node that has been split.
            pDoc->getIDocumentContentOperations().SplitNode( *pPos, false );
            *m_oSttNdIdx = pPos->GetNodeIndex()-1;

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
        SetGraphicStorageHandler(m_xGraphicStorageHandler);
    }

    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            m_xEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            SvXMLEmbeddedObjectHelperMode::Read );
            SetEmbeddedResolver( m_xEmbeddedResolver );
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
        Reference<XInterface> xCursorTunnel( GetTextImport()->GetCursor(),
                                              UNO_QUERY);
        assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
        OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(xCursorTunnel.get());
        assert(pTextCursor && "SwXTextCursor missing");
        SwPaM *pPaM = pTextCursor->GetPaM();
        if( IsInsertMode() && m_oSttNdIdx->GetIndex() )
        {
            // If we are in insert mode, join the split node that is in front
            // of the new content with the first new node. Or in other words:
            // Revert the first split node.
            SwTextNode* pTextNode = m_oSttNdIdx->GetNode().GetTextNode();
            SwNodeIndex aNxtIdx( *m_oSttNdIdx );
            if( pTextNode && pTextNode->CanJoinNext( &aNxtIdx ) &&
                m_oSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
            {
                // If the PaM points to the first new node, move the PaM to the
                // end of the previous node.
                if( pPaM->GetPoint()->GetNode() == aNxtIdx.GetNode() )
                {
                    pPaM->GetPoint()->Assign( *pTextNode,
                                            pTextNode->GetText().getLength());
                }

#if OSL_DEBUG_LEVEL > 0
                // !!! This should be impossible !!!!
                OSL_ENSURE( m_oSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound().GetNodeIndex(),
                        "PaM.Bound1 point to new node " );
                OSL_ENSURE( m_oSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( false ).GetNodeIndex(),
                        "PaM.Bound2 points to new node" );

                if( m_oSttNdIdx->GetIndex()+1 ==
                                        pPaM->GetBound().GetNodeIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound().GetContentIndex();
                    pPaM->GetBound().SetContent(
                            pTextNode->GetText().getLength() + nCntPos );
                }
                if( m_oSttNdIdx->GetIndex()+1 ==
                                pPaM->GetBound( false ).GetNodeIndex() )
                {
                    const sal_Int32 nCntPos =
                            pPaM->GetBound( false ).GetContentIndex();
                    pPaM->GetBound( false ).SetContent(
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
                {
                    pTextNode->ResetAttr(RES_CHRATR_BEGIN, RES_CHRATR_END);
                    pTextNode->ChgFormatColl( pDelNd->GetTextColl() );
                    if (!pDelNd->GetNoCondAttr(RES_PARATR_LIST_ID, /*bInParents=*/false))
                    {
                        // MergeListsAtDocumentInsertPosition() will deal with lists below, copy
                        // paragraph direct formatting otherwise.
                        pDelNd->CopyCollFormat(*pTextNode);
                    }
                }
                pTextNode->JoinNext();
            }
        }

        SwPosition* pPos = pPaM->GetPoint();
        OSL_ENSURE( !pPos->GetContentIndex(), "last paragraph isn't empty" );
        if( !pPos->GetContentIndex() )
        {
            SwTextNode* pCurrNd;
            SwNodeOffset nNodeIdx = pPos->GetNodeIndex();
            pDoc = &pPaM->GetDoc();

            OSL_ENSURE( pPos->GetNode().IsContentNode(),
                        "insert position is not a content node" );
            if( !IsInsertMode() )
            {
                // If we're not in insert mode, the last node is deleted.
                const SwNode *pPrev = pDoc->GetNodes()[nNodeIdx -1];
                if( pPrev->IsContentNode() ||
                     ( pPrev->IsEndNode() &&
                      pPrev->StartOfSectionNode()->IsSectionNode() ) )
                {
                    SwContentNode* pCNd = pPaM->GetPointContentNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() )
                    {
                        SwNode& rDelNode = pPaM->GetPoint()->GetNode();
                        // move so we don't have a dangling SwContentIndex to the deleted node
                        pPaM->GetPoint()->Adjust(SwNodeOffset(+1));
                        if (pPaM->HasMark())
                            pPaM->GetMark()->Adjust(SwNodeOffset(+1));
                        pDoc->GetNodes().Delete( rDelNode );
                    }
                }
            }
            else if( nullptr != (pCurrNd = pDoc->GetNodes()[nNodeIdx]->GetTextNode()) )
            {
                // Id we're in insert mode, the empty node is joined with
                // the next and the previous one.
                if( pCurrNd->CanJoinNext( pPos ))
                {
                    SwTextNode* pNextNd = pPos->GetNode().GetTextNode();
                    bool endNodeFound = pDoc->GetNodes()[nNodeIdx-1]->IsEndNode();
                    SwNode *pLastPar = pDoc->GetNodes()[nNodeIdx -2];
                    if ( !pLastPar->IsTextNode() ) {
                        pLastPar = pDoc->GetNodes()[nNodeIdx -1];
                    }
                    if ( !endNodeFound && pLastPar->IsTextNode() )
                    {
                        pNextNd->ChgFormatColl(pLastPar->GetTextNode()->GetTextColl());
                    }

                    pPaM->SetMark(); pPaM->DeleteMark();
                    pNextNd->JoinPrev();

                    // Remove line break that has been inserted by the import,
                    // but only if one has been inserted and
                    // no endNode found to avoid removing section
                    if( pNextNd->CanJoinPrev(/* &pPos->nNode*/ ) && !endNodeFound &&
                         *m_oSttNdIdx != pPos->GetNode() )
                    {
                        pNextNd->JoinPrev();
                    }
                }
                else if (pCurrNd->GetText().isEmpty())
                {
                    pPaM->SetMark(); pPaM->DeleteMark();
                    SwNode& rDelNode = pPos->GetNode();
                    // move so we don't have a dangling SwContentIndex to the deleted node
                    pPaM->GetPoint()->Adjust(SwNodeOffset(+1));
                    pDoc->GetNodes().Delete( rDelNode );
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

    GetTextImport()->RedlineAdjustStartNodeCursor();

    if( (getImportFlags() & SvXMLImportFlags::CONTENT) ||
        ((getImportFlags() & SvXMLImportFlags::MASTERSTYLES) && IsStylesOnlyMode()) )
    {
        // pDoc might be 0. In this case UpdateTextCollCondition is looking
        // for it itself.
        UpdateTextCollConditions( pDoc );
    }

    GetTextImport()->ResetCursor();

    m_oSttNdIdx.reset();

    // tdf#150753: pDoc may be null e.g. when the package lacks content.xml;
    // we should not forget to tidy up here, including unlocking draw model
    if (!pDoc)
        pDoc = getDoc();
    assert(pDoc);
    // SJ: #i49801# -> now permitting repaints
    if (getImportFlags() == SvXMLImportFlags::ALL)
    {
        // Notify math objects. If we are in the package filter this will
        // be done by the filter object itself
        if (IsInsertMode())
            pDoc->PrtOLENotify(false);
        else if (pDoc->IsOLEPrtNotifyPending())
            pDoc->PrtOLENotify(true);

        assert(pDoc->IsInReading());
        assert(pDoc->IsInXMLImport());
        pDoc->SetInReading(false);
        pDoc->SetInXMLImport(false);
    }

    SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    if (pDrawModel)
        pDrawModel->setLock(false);

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

    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTableNode *const pTableNode = pDoc->GetNodes()[i]->GetTableNode())
        {
            if (!pTableNode->GetTable().IsNewModel()
                && pTableNode->GetTable().CanConvertSubtables())
            {
                pTableNode->GetTable().ConvertSubtables();
            }
        }
        // don't skip to the end; nested tables could have subtables too...
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

    if (! IsInsertMode() || ! m_oSttNdIdx->GetIndex())
        return;

    SwNodeOffset index(1);

    // the last node of the main document where we have inserted a document
    SwNode* const node1 = pDoc->GetNodes()[m_oSttNdIdx->GetIndex() + 0];

    // the first node of the inserted document
    SwNode* node2 = pDoc->GetNodes()[m_oSttNdIdx->GetIndex() + index];

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

        auto pStringListId1 = dynamic_cast<const SfxStringItem*>(pListId1);
        assert(pStringListId1);
        const OUString& sListId1 = pStringListId1->GetValue();
        auto pStringListId2 = dynamic_cast<const SfxStringItem*>(pListId2);
        assert(pStringListId2);
        const OUString& sListId2 = pStringListId2->GetValue();

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
                    if( pNumRule1->Get( n ) != pNumRule2->Get( n ) )
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

        node2 = pDoc->GetNodes()[m_oSttNdIdx->GetIndex() + index];
    }
}

namespace {

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

}

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
        XMLShapeImportHelper::startPage( xPage );
    }
}

SvTextShapeImportHelper::~SvTextShapeImportHelper()
{
    rFormImport->endPage();

    if (xPage.is())
    {
        XMLShapeImportHelper::endPage(xPage);
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

SvXMLImportContext *SwXMLImport::CreateFontDeclsContext()
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, osl_getThreadTextEncoding() );
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

    sal_Int64 nTmp = 0;
    bool bShowRedlineChanges = false, bBrowseMode = false;
    bool bChangeShowRedline = false, bChangeBrowseMode = false;

    //TODO/LATER: why that cast?!
    bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MapUnit::MapTwip;
    //sal_Bool bTwip = pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit ( ) == MapUnit::MapTwip;

    for (const PropertyValue& rValue : aViewProps)
    {
        if ( rValue.Name == "ViewAreaTop" )
        {
            rValue.Value >>= nTmp;
            aRect.SetPosY(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp);
        }
        else if ( rValue.Name == "ViewAreaLeft" )
        {
            rValue.Value >>= nTmp;
            aRect.SetPosX(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp);
        }
        else if ( rValue.Name == "ViewAreaWidth" )
        {
            rValue.Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.setWidth(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( rValue.Name == "ViewAreaHeight" )
        {
            rValue.Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.setHeight(bTwip ? sanitiseMm100ToTwip(nTmp) : nTmp);
            aRect.SetSize( aSize );
        }
        else if ( rValue.Name == "ShowRedlineChanges" )
        {
            bShowRedlineChanges = *o3tl::doAccess<bool>(rValue.Value);
            bChangeShowRedline = true;
        }
// Headers and footers are not displayed in BrowseView anymore
        else if ( rValue.Name == "InBrowseMode" )
        {
            bBrowseMode = *o3tl::doAccess<bool>(rValue.Value);
            bChangeBrowseMode = true;
        }
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

    Reference< XPropertySet > xProps( xFac->createInstance(u"com.sun.star.document.Settings"_ustr), UNO_QUERY );
    if( !xProps.is() )
        return;

    Reference< XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
    if( !xInfo.is() )
        return;

    std::unordered_set< OUString > aExcludeAlways;
    aExcludeAlways.insert(u"LinkUpdateMode"_ustr);
    // this should contain things that are actually user-settable, via Tools->Options
    std::unordered_set< OUString > aExcludeWhenNotLoadingUserSettings {
        u"ForbiddenCharacters"_ustr,
        u"IsKernAsianPunctuation"_ustr,
        u"CharacterCompressionType"_ustr,
        u"FieldAutoUpdate"_ustr,
        u"ChartAutoUpdate"_ustr,
        u"AddParaTableSpacing"_ustr,
        u"AddParaTableSpacingAtStart"_ustr,
        u"PrintAnnotationMode"_ustr,
        u"PrintBlackFonts"_ustr,
        u"PrintControls"_ustr,
        u"PrintGraphics"_ustr,
        u"PrintHiddenText"_ustr,
        u"PrintLeftPages"_ustr,
        u"PrintPageBackground"_ustr,
        u"PrintProspect"_ustr,
        u"PrintRightPages"_ustr,
        u"PrintFaxName"_ustr,
        u"PrintPaperFromSetup"_ustr,
        u"PrintTextPlaceholder"_ustr,
        u"UpdateFromTemplate"_ustr,
        u"PrinterIndependentLayout"_ustr,
        u"PrintEmptyPages"_ustr,
        u"ConsiderTextWrapOnObjPos"_ustr,
        u"DoNotJustifyLinesWithManualBreak"_ustr,
        u"ProtectForm"_ustr,
        u"MsWordCompTrailingBlanks"_ustr,
        u"SubtractFlysAnchoredAtFlys"_ustr,
        u"EmptyDbFieldHidesPara"_ustr,
        u"UseVariableWidthNBSP"_ustr,
    };

    bool bAreUserSettingsFromDocument = officecfg::Office::Common::Load::UserDefinedSettings::get();

    // for some properties we don't want to use the application
    // default if they're missing. So we watch for them in the loop
    // below, and set them if not found
    bool bPrinterIndependentLayout = false;
    bool bUseOldNumbering = false;
    bool bAddExternalLeading = false;
    bool bAddParaSpacingToTableCells = false;
    bool bAddParaLineSpacingToTableCells = false;
    bool bUseFormerLineSpacing = false;
    bool bUseFormerObjectPositioning = false;
    bool bUseFormerTextWrapping = false;
    bool bConsiderWrapOnObjPos = false;
    bool bIgnoreFirstLineIndentInNumbering = false;
    bool bDoNotJustifyLinesWithManualBreak = false;
    bool bDoNotResetParaAttrsForNumFont    = false;
    bool bDoNotCaptureDrawObjsOnPage( false );
    bool bClipAsCharacterAnchoredWriterFlyFrames( false );
    bool bUnixForceZeroExtLeading = false;
    bool bSmallCapsPercentage66 = false;
    bool bTabOverflow = false;
    bool bTabOverMarginValue = false;
    bool bPropLineSpacingShrinksFirstLine = false;
    bool bSubtractFlysAnchoredAtFlys = false;
    bool bEmptyDbFieldHidesPara = false;
    bool bCollapseEmptyCellPara = false;
    bool bAutoFirstLineIndentDisregardLineSpace = false;
    bool bHyphenateURLs = false;
    bool bApplyTextAttrToEmptyLineAtEndOfParagraph = false;
    bool bDoNotBreakWrappedTables = false;
    bool bAllowTextAfterFloatingTableBreak = false;
    bool bDropCapPunctuation = false;
    bool bDoNotMirrorRtlDrawObjs = false;

    const PropertyValue* currentDatabaseDataSource = nullptr;
    const PropertyValue* currentDatabaseCommand = nullptr;
    const PropertyValue* currentDatabaseCommandType = nullptr;
    const PropertyValue* embeddedDatabaseName = nullptr;

    for( const PropertyValue& rValue : aConfigProps )
    {
        bool bSet = aExcludeAlways.find(rValue.Name) == aExcludeAlways.end();
        if( bSet && !bAreUserSettingsFromDocument
            && (aExcludeWhenNotLoadingUserSettings.find(rValue.Name)
                != aExcludeWhenNotLoadingUserSettings.end()) )
        {
            bSet = false;
        }

        if( bSet )
        {
            try
            {
                if( xInfo->hasPropertyByName( rValue.Name ) )
                {
                    if( rValue.Name == "RedlineProtectionKey" )
                    {
                        Sequence<sal_Int8> aKey;
                        rValue.Value >>= aKey;
                        GetTextImport()->SetChangesProtectionKey( aKey );
                    }
                    else
                    {
                        // HACK: Setting these out of order does not work.
                        if( rValue.Name == "CurrentDatabaseDataSource" )
                            currentDatabaseDataSource = &rValue;
                        else if( rValue.Name == "CurrentDatabaseCommand" )
                            currentDatabaseCommand = &rValue;
                        else if( rValue.Name == "CurrentDatabaseCommandType" )
                            currentDatabaseCommandType = &rValue;
                        else if (rValue.Name == "EmbeddedDatabaseName")
                            embeddedDatabaseName = &rValue;
                        else
                            xProps->setPropertyValue( rValue.Name, rValue.Value );
                    }
                }

                // did we find any of the non-default cases?
                if ( rValue.Name == "PrinterIndependentLayout" )
                    bPrinterIndependentLayout = true;
                else if ( rValue.Name == "AddExternalLeading" )
                    bAddExternalLeading = true;
                else if ( rValue.Name == "AddParaSpacingToTableCells" )
                    bAddParaSpacingToTableCells = true;
                else if ( rValue.Name == "AddParaLineSpacingToTableCells" )
                    bAddParaLineSpacingToTableCells = true;
                else if ( rValue.Name == "UseFormerLineSpacing" )
                    bUseFormerLineSpacing = true;
                else if ( rValue.Name == "UseFormerObjectPositioning" )
                    bUseFormerObjectPositioning = true;
                else if ( rValue.Name == "UseFormerTextWrapping" )
                    bUseFormerTextWrapping = true;
                else if ( rValue.Name == "UseOldNumbering" )
                    bUseOldNumbering = true;
                else if ( rValue.Name == "ConsiderTextWrapOnObjPos" )
                    bConsiderWrapOnObjPos = true;
                else if ( rValue.Name == "IgnoreFirstLineIndentInNumbering" )
                    bIgnoreFirstLineIndentInNumbering = true;
                else if ( rValue.Name == "DoNotJustifyLinesWithManualBreak" )
                    bDoNotJustifyLinesWithManualBreak = true;
                else if ( rValue.Name == "DoNotResetParaAttrsForNumFont" )
                    bDoNotResetParaAttrsForNumFont = true;
                else if ( rValue.Name == "DoNotCaptureDrawObjsOnPage" )
                    bDoNotCaptureDrawObjsOnPage = true;
                else if ( rValue.Name == "ClipAsCharacterAnchoredWriterFlyFrames" )
                    bClipAsCharacterAnchoredWriterFlyFrames = true;
                else if ( rValue.Name == "UnxForceZeroExtLeading" )
                    bUnixForceZeroExtLeading = true;
                else if ( rValue.Name == "SmallCapsPercentage66" )
                    bSmallCapsPercentage66 = true;
                else if ( rValue.Name == "TabOverflow" )
                    bTabOverflow = true;
                else if ( rValue.Name == "TabOverMargin" )
                {
                    rValue.Value >>= bTabOverMarginValue;
                }
                else if ( rValue.Name == "PropLineSpacingShrinksFirstLine" )
                    bPropLineSpacingShrinksFirstLine = true;
                else if (rValue.Name == "SubtractFlysAnchoredAtFlys")
                    bSubtractFlysAnchoredAtFlys = true;
                else if (rValue.Name == "EmptyDbFieldHidesPara")
                    bEmptyDbFieldHidesPara = true;
                else if (rValue.Name == "CollapseEmptyCellPara")
                    bCollapseEmptyCellPara = true;
                else if (rValue.Name == "AutoFirstLineIndentDisregardLineSpace")
                    bAutoFirstLineIndentDisregardLineSpace = true;
                else if (rValue.Name == "HyphenateURLs")
                {
                    bHyphenateURLs = true;
                }
                else if (rValue.Name == "ApplyTextAttrToEmptyLineAtEndOfParagraph")
                {
                    bApplyTextAttrToEmptyLineAtEndOfParagraph = true;
                }
                else if (rValue.Name == "DoNotBreakWrappedTables")
                {
                    rValue.Value >>= bDoNotBreakWrappedTables;
                }
                else if (rValue.Name == "AllowTextAfterFloatingTableBreak")
                {
                    rValue.Value >>= bAllowTextAfterFloatingTableBreak;
                }
                else if ( rValue.Name == "DropCapPunctuation" )
                    bDropCapPunctuation = true;
                else if (rValue.Name == "DoNotMirrorRtlDrawObjs")
                {
                    rValue.Value >>= bDoNotMirrorRtlDrawObjs;
                }
            }
            catch( Exception& )
            {
                TOOLS_WARN_EXCEPTION( "sw", "SwXMLImport::SetConfigurationSettings" );
            }
        }
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
        TOOLS_WARN_EXCEPTION( "sw", "SwXMLImport::SetConfigurationSettings" );
    }

    // finally, treat the non-default cases
    // introduce boolean, that indicates a document, written by version prior SO8.
    // If user settings are not loaded, we can't know if this is an old document. Better to assume no?
    const bool bDocumentPriorSO8 = !bConsiderWrapOnObjPos && bAreUserSettingsFromDocument;

    // Use old behaviour if this setting didn't exist, but only if this setting is being read from the document.
    // (Obviously the setting doesn't exist if we are explicitly ignoring it, so then stick with program/user defaults)
    if(!bPrinterIndependentLayout && bAreUserSettingsFromDocument)
    {
        xProps->setPropertyValue( u"PrinterIndependentLayout"_ustr, Any(sal_Int16(document::PrinterIndependentLayout::DISABLED)) );
    }

    if( ! bAddExternalLeading )
    {
        xProps->setPropertyValue( u"AddExternalLeading"_ustr, Any( false ) );
    }

    if( ! bUseFormerLineSpacing )
    {
        xProps->setPropertyValue( u"UseFormerLineSpacing"_ustr, Any( true ) );
    }

    if( !bUseFormerObjectPositioning )
    {
        xProps->setPropertyValue( u"UseFormerObjectPositioning"_ustr, Any( true ) );
    }

    if( !bUseOldNumbering )
    {
        xProps->setPropertyValue( u"UseOldNumbering"_ustr, Any(true) );
    }

    if( !bAddParaSpacingToTableCells )
    {
        xProps->setPropertyValue( u"AddParaSpacingToTableCells"_ustr,
            Any( false ) );
    }
    if (!bAddParaLineSpacingToTableCells)
    {
        xProps->setPropertyValue(u"AddParaLineSpacingToTableCells"_ustr, Any(false));
    }

    if( !bUseFormerTextWrapping )
    {
        xProps->setPropertyValue( u"UseFormerTextWrapping"_ustr, Any( true ) );
    }

    if (!bConsiderWrapOnObjPos && bAreUserSettingsFromDocument)
    {
        xProps->setPropertyValue( u"ConsiderTextWrapOnObjPos"_ustr, Any( false ) );
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
        xProps->setPropertyValue( u"IgnoreFirstLineIndentInNumbering"_ustr,
            Any( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotJustifyLinesWithManualBreak && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( u"DoNotJustifyLinesWithManualBreak"_ustr,
            Any( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotResetParaAttrsForNumFont && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( u"DoNotResetParaAttrsForNumFont"_ustr,
            Any( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bDoNotCaptureDrawObjsOnPage && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( u"DoNotCaptureDrawObjsOnPage"_ustr,
            Any( true ) );
    }

    // This flag has to be set for all documents < SO8
    if ( !bClipAsCharacterAnchoredWriterFlyFrames && bDocumentPriorSO8 )
    {
        xProps->setPropertyValue( u"ClipAsCharacterAnchoredWriterFlyFrames"_ustr,
            Any( true ) );
    }

    if ( !bUnixForceZeroExtLeading )
    {
        xProps->setPropertyValue( u"UnxForceZeroExtLeading"_ustr, Any( true ) );
    }

    // Old LO versions had 66 as the value for small caps percentage, later changed to 80.
    // In order to keep backwards compatibility, SmallCapsPercentage66 option is written to .odt
    // files, and the default for new documents is 'false'. Files without this option
    // are considered to be old files, so set the compatibility option too.
    if ( !bSmallCapsPercentage66 )
    {
        xProps->setPropertyValue( u"SmallCapsPercentage66"_ustr, Any( true ) );
    }

    if ( !bTabOverflow )
    {
        xProps->setPropertyValue( u"TabOverflow"_ustr, Any( false ) );
    }

    if (bTabOverMarginValue)
        // Let TabOverMargin imply the new default for
        // PrinterIndependentLayout, knowing the first is set by Word import
        // filters and Word defaults to our new default as well.
        xProps->setPropertyValue(
            u"PrinterIndependentLayout"_ustr,
            uno::Any(document::PrinterIndependentLayout::HIGH_RESOLUTION));

    if (!bPropLineSpacingShrinksFirstLine)
        xProps->setPropertyValue(u"PropLineSpacingShrinksFirstLine"_ustr, Any(false));

    if (!bSubtractFlysAnchoredAtFlys && bAreUserSettingsFromDocument)
        xProps->setPropertyValue(u"SubtractFlysAnchoredAtFlys"_ustr, Any(true));

    if (!bEmptyDbFieldHidesPara && bAreUserSettingsFromDocument)
        xProps->setPropertyValue(u"EmptyDbFieldHidesPara"_ustr, Any(false));

    if (!bCollapseEmptyCellPara)
        xProps->setPropertyValue(u"CollapseEmptyCellPara"_ustr, Any(false));

    if (!bAutoFirstLineIndentDisregardLineSpace)
        xProps->setPropertyValue(u"AutoFirstLineIndentDisregardLineSpace"_ustr, Any(false));

    if (!bHyphenateURLs)
    {
        xProps->setPropertyValue(u"HyphenateURLs"_ustr, Any(true));
    }

    if (!bApplyTextAttrToEmptyLineAtEndOfParagraph)
    {
        xProps->setPropertyValue(u"ApplyTextAttrToEmptyLineAtEndOfParagraph"_ustr, Any(false));
    }

    if (bDoNotMirrorRtlDrawObjs)
    {
        xProps->setPropertyValue(u"DoNotMirrorRtlDrawObjs"_ustr, Any(true));
    }

    if (bDoNotBreakWrappedTables)
    {
        xProps->setPropertyValue(u"DoNotBreakWrappedTables"_ustr, Any(true));
    }

    if (bAllowTextAfterFloatingTableBreak)
    {
        xProps->setPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr, Any(true));
    }

    // LO 7.4 and previous versions had different drop cap punctuation: very long dashes.
    // In order to keep backwards compatibility, DropCapPunctuation option is written to .odt
    // files, and the default for new documents is 'true'. Files without this option
    // are considered to be old files, so set the compatibility option too.
    if ( !bDropCapPunctuation )
    {
        xProps->setPropertyValue( u"DropCapPunctuation"_ustr, Any( false ) );
    }

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
            m_xLateInitSettings->replaceByName( _rSettingsGroupName, Any( _rSettings ) );
            OSL_FAIL( "SwXMLImport::SetDocumentSpecificSettings: already have settings for this model!" );
        }
        else
            m_xLateInitSettings->insertByName( _rSettingsGroupName, Any( _rSettings ) );
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
    for(const auto& rArgument : aArguments)
    {
        beans::NamedValue aNamedValue;
        if ( rArgument >>= aNamedValue )
        {
            if (aNamedValue.Name == "LateInitSettings")
            {
                OSL_VERIFY( aNamedValue.Value >>= m_xLateInitSettings );
            }
        }
    }
}

void SwXMLImport::initXForms()
{
    // obtain SwDoc
    auto pXTextDocument = comphelper::getFromUnoTunnel<SwXTextDocument>(GetModel());
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
    SwXText* pText = dynamic_cast<SwXText*>(xText.get());
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
    return cppu::acquire(new SwXMLImport(context, u"com.sun.star.comp.Writer.XMLOasisImporter"_ustr,
                SvXMLImportFlags::ALL));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, u"com.sun.star.comp.Writer.XMLOasisStylesImporter"_ustr,
                SvXMLImportFlags::STYLES | SvXMLImportFlags::MASTERSTYLES | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, u"com.sun.star.comp.Writer.XMLOasisContentImporter"_ustr,
                SvXMLImportFlags::CONTENT | SvXMLImportFlags::SCRIPTS | SvXMLImportFlags::AUTOSTYLES |
                SvXMLImportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, u"com.sun.star.comp.Writer.XMLOasisMetaImporter"_ustr,
                SvXMLImportFlags::META));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLImport(context, u"com.sun.star.comp.Writer.XMLOasisSettingsImporter"_ustr,
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
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr), uno::UNO_SET_THROW);

    css::uno::Sequence<OUString> aUserData
    {
        u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
        u""_ustr,
        u"com.sun.star.comp.Writer.XMLOasisImporter"_ustr,
        u"com.sun.star.comp.Writer.XMLOasisExporter"_ustr,
        u""_ustr,
        u""_ustr,
        u"true"_ustr
    };
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(u"private:stream"_ustr) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestPDFExportFODT(SvStream &rStream)
{
    // do the same sort of check as FilterDetect::detect
    OString const str(read_uInt8s_ToOString(rStream, 4000));
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    OUString resultString(str.getStr(), str.getLength(), RTL_TEXTENCODING_ASCII_US,
                          RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT);
    if (!resultString.startsWith("<?xml") || resultString.indexOf("office:mimetype=\"application/vnd.oasis.opendocument.text\"") == -1)
        return false;

    Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    Reference<css::frame::XFrame> xTargetFrame = xDesktop->findFrame(u"_blank"_ustr, 0);

    Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<css::frame::XModel2> xModel(xContext->getServiceManager()->createInstanceWithContext(
                u"com.sun.star.text.TextDocument"_ustr, xContext), UNO_QUERY_THROW);

    Reference<css::frame::XLoadable> xModelLoad(xModel, UNO_QUERY_THROW);
    xModelLoad->initNew();

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr), uno::UNO_SET_THROW);

    css::uno::Sequence<OUString> aUserData
    {
        u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
        u""_ustr,
        u"com.sun.star.comp.Writer.XMLOasisImporter"_ustr,
        u"com.sun.star.comp.Writer.XMLOasisExporter"_ustr,
        u""_ustr,
        u""_ustr,
        u"true"_ustr
    };
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(u"private:stream"_ustr) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFODTFilter(xInterface, uno::UNO_QUERY_THROW);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(xModel.get());
    SfxObjectShell* pObjSh = pTextDoc ? pTextDoc->GetDocShell() : nullptr;

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    if (pObjSh)
        pObjSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFODTFilter->filter(aArgs);
    if (pObjSh)
        pObjSh->SetLoading(SfxLoadedFlags::ALL);

    if (ret)
    {
        uno::Reference<text::XTextDocument> xTextDocument(xModel, uno::UNO_QUERY);
        uno::Reference<text::XText> xText(xTextDocument->getText());
        uno::Reference<container::XEnumerationAccess> xParaAccess(xText, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum(xParaAccess->createEnumeration());
        while (xParaEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            // discourage very long paragraphs for fuzzing performance
            if (xPara && xPara->getString().getLength() > 15000)
            {
                ret = false;
                break;
            }
        }
    }

    if (ret)
    {
        css::uno::Reference<css::frame::XController2> xController(xModel->createDefaultViewController(xTargetFrame), UNO_SET_THROW);
        utl::ConnectFrameControllerModel(xTargetFrame, xController, xModel);

        utl::TempFileFast aTempFile;

        uno::Reference<document::XFilter> xPDFFilter(
            xMultiServiceFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
        uno::Reference<document::XExporter> xExporter(xPDFFilter, uno::UNO_QUERY);
        xExporter->setSourceDocument(xModel);

        uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(*aTempFile.GetStream(StreamMode::READWRITE)));

        // ofz#60533 fuzzer learned to use fo:font-size="842pt" which generate timeouts trying
        // to export thousands of pages from minimal input size
        uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
            { "PageRange", uno::Any(u"1-100"_ustr) }
        }));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
            { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
            { "OutputStream", uno::Any(xOutputStream) },
            { "FilterData", uno::Any(aFilterData) }
        }));
        xPDFFilter->filter(aDescriptor);
    }

    css::uno::Reference<css::util::XCloseable> xClose(xModel, css::uno::UNO_QUERY);
    xClose->close(false);

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

    uno::Reference<document::XFilter> xFilter(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.WriterFilter"_ustr), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "InputMode", uno::Any(true) },
    }));
    xImporter->setTargetDocument(xModel);

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = false;
    try
    {
        ret = xFilter->filter(aArgs);
    }
    catch (...)
    {
    }
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
