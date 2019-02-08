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

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>

#include <o3tl/any.hxx>
#include <officecfg/Office/Common.hxx>
#include <sax/tools/converter.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svddef.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/xmlcnitm.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xformsexport.hxx>
#include <drawdoc.hxx>
#include <pam.hxx>
#include <unofreg.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <docstat.hxx>
#include <swerror.h>
#include <unotext.hxx>
#include "xmltexte.hxx"
#include "xmlexp.hxx"
#include "xmlexpit.hxx"
#include <sfx2/viewsh.hxx>
#include <comphelper/processfactory.hxx>
#include <docary.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>


#include <pausethreadstarting.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::xforms;
using namespace ::xmloff::token;

SwXMLExport::SwXMLExport(
    const uno::Reference< uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLExportFlags nExportFlags)
:   SvXMLExport( util::MeasureUnit::INCH, rContext, implementationName, XML_TEXT,
        nExportFlags ),
    m_bBlock( false ),
    m_bShowProgress( true ),
    m_bSavedShowChanges( false ),
    m_pDoc( nullptr )
{
    InitItemExport();
}

ErrCode SwXMLExport::exportDoc( enum XMLTokenEnum eClass )
{
    if( !GetModel().is() )
        return ERR_SWG_WRITE_ERROR;

    SwPauseThreadStarting aPauseThreadStarting; // #i73788#

    // from here, we use core interfaces -> lock Solar-Mutex
    SolarMutexGuard aGuard;

    {
        Reference<XPropertySet> rInfoSet = getExportInfo();
        if( rInfoSet.is() )
        {
            const OUString sAutoTextMode("AutoTextMode");
            if( rInfoSet->getPropertySetInfo()->hasPropertyByName(
                        sAutoTextMode ) )
            {
                Any aAny = rInfoSet->getPropertyValue(sAutoTextMode);
                if( auto b = o3tl::tryAccess<bool>(aAny) )
                {
                    if( *b )
                        m_bBlock = true;
                }
            }
        }
    }

    SwDoc *pDoc = getDoc();
    if (!pDoc)
        return ERR_SWG_WRITE_ERROR;

    if( getExportFlags() & (SvXMLExportFlags::FONTDECLS|SvXMLExportFlags::STYLES|
                            SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT))
    {
        if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
        {
            GetNamespaceMap_().Add(
                GetXMLToken(XML_NP_OFFICE_EXT),
                GetXMLToken(XML_N_OFFICE_EXT),
                XML_NAMESPACE_OFFICE_EXT);
        }

        GetTextParagraphExport()->SetBlockMode( m_bBlock );

        const SfxItemPool& rPool = pDoc->GetAttrPool();
        sal_uInt16 aWhichIds[5] = { RES_UNKNOWNATR_CONTAINER,
                                    RES_TXTATR_UNKNOWN_CONTAINER,
                                    SDRATTR_XMLATTRIBUTES,
                                    EE_PARA_XMLATTRIBS,
                                    EE_CHAR_XMLATTRIBS };

        const int nWhichIds = rPool.GetSecondaryPool() ? 5 : 2;
        for( int j=0; j < nWhichIds; ++j )
        {
            const sal_uInt16 nWhichId = aWhichIds[j];
            const sal_uInt32 nItems = rPool.GetItemCount2( nWhichId );
            for( sal_uInt32 i = 0; i < nItems; ++i )
            {
                const SfxPoolItem* const pItem = rPool.GetItem2( nWhichId , i );
                if( nullptr != pItem )
                {
                    const SvXMLAttrContainerItem *pUnknown =
                                dynamic_cast<const SvXMLAttrContainerItem*>( pItem  );
                    OSL_ENSURE( pUnknown, "illegal attribute container item" );
                    if( pUnknown && (pUnknown->GetAttrCount() > 0) )
                    {
                        sal_uInt16 nIdx = pUnknown->GetFirstNamespaceIndex();
                        while( USHRT_MAX != nIdx )
                        {
                            GetNamespaceMap_().Add( pUnknown->GetPrefix( nIdx ),
                                                pUnknown->GetNamespace( nIdx ) );
                            nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
                        }
                    }
                }
            }
        }
    }

    sal_uInt16 const eUnit = SvXMLUnitConverter::GetMeasureUnit(
            SW_MOD()->GetMetric(pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE)));
    if (GetMM100UnitConverter().GetXMLMeasureUnit() != eUnit )
    {
        GetMM100UnitConverter().SetXMLMeasureUnit( eUnit );
        m_pTwipUnitConverter->SetXMLMeasureUnit( eUnit );
    }

    if( getExportFlags() & SvXMLExportFlags::META)
    {
        // Update doc stat, so that correct values are exported and
        // the progress works correctly.
        pDoc->getIDocumentStatistics().UpdateDocStat( false, true );
    }
    if( m_bShowProgress )
    {
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        if( -1 == pProgress->GetReference() )
        {
            // progress isn't initialized:
            // We assume that the whole doc is exported, and the following
            // durations:
            // - meta information: 2
            // - settings: 4 (TODO: not now!)
            // - styles (except page styles): 2
            // - page styles: 2 (TODO: not now!) + 2 for each paragraph
            // - paragraph: 2 (1 for automatic styles and one for content)

            // count each item once, and then multiply by two to reach the
            // figures given above
            // The styles in pDoc also count the default style that never
            // gets exported -> subtract one.
            sal_Int32 nRef = 1; // meta.xml
            nRef += pDoc->GetCharFormats()->size() - 1;
            nRef += pDoc->GetFrameFormats()->size() - 1;
            nRef += pDoc->GetTextFormatColls()->size() - 1;
            nRef *= 2; // for the above styles, xmloff will increment by 2!
            // #i93174#: count all paragraphs for the progress bar
            nRef += pDoc->getIDocumentStatistics().GetUpdatedDocStat( false, true ).nAllPara; // 1: only content, no autostyle
            pProgress->SetReference( nRef );
            pProgress->SetValue( 0 );
        }
    }

    if( getExportFlags() & (SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT))
    {
        //We depend on the correctness of OrdNums.
        SwDrawModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
        if( pModel )
            pModel->GetPage( 0 )->RecalcObjOrdNums();
    }

    // adjust document class (eClass)
    if (pDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT))
    {
        eClass = XML_TEXT_GLOBAL;

        // additionally, we take care of the save-linked-sections-thingy
        mbSaveLinkedSections = pDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS);
    }
    // MIB: 03/26/04: The Label information is saved in the settings, so
    // we don't need it here.
    // else: keep default pClass that we received

    rtl::Reference<SvXMLGraphicHelper> xGraphicStorageHandler;
    if (!GetGraphicStorageHandler().is())
    {
        xGraphicStorageHandler = SvXMLGraphicHelper::Create(SvXMLGraphicHelperMode::Write, GetImageFilterName());
        SetGraphicStorageHandler(xGraphicStorageHandler.get());
    }

    rtl::Reference<SvXMLEmbeddedObjectHelper> xEmbeddedResolver;
    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            xEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            SvXMLEmbeddedObjectHelperMode::Write );
            SetEmbeddedResolver( Reference<XEmbeddedObjectResolver>( xEmbeddedResolver.get() ) );
        }
    }

    // set redline mode if we export STYLES or CONTENT, unless redline
    // mode is taken care of outside (through info XPropertySet)
    bool bSaveRedline =
        bool( getExportFlags() & (SvXMLExportFlags::CONTENT|SvXMLExportFlags::STYLES) );
    if( bSaveRedline )
    {
        // if the info property set has a ShowChanges property,
        // then change tracking is taken care of on the outside,
        // so we don't have to!
        Reference<XPropertySet> rInfoSet = getExportInfo();
        if( rInfoSet.is() )
        {
            bSaveRedline = ! rInfoSet->getPropertySetInfo()->hasPropertyByName(
                                                                "ShowChanges" );
        }
    }
    RedlineFlags nRedlineFlags = RedlineFlags::NONE;
    SwRootFrame const*const pLayout(m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    m_bSavedShowChanges = pLayout == nullptr || !pLayout->IsHideRedlines();
    if( bSaveRedline )
    {
        // now save and switch redline mode
        nRedlineFlags = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags(
                 ( nRedlineFlags & RedlineFlags::ShowMask ) | RedlineFlags::ShowInsert );
    }

    ErrCode nRet = SvXMLExport::exportDoc( eClass );

    // now we can restore the redline mode (if we changed it previously)
    if( bSaveRedline )
    {
      pDoc->getIDocumentRedlineAccess().SetRedlineFlags( nRedlineFlags );
    }

    if (xGraphicStorageHandler)
        xGraphicStorageHandler->dispose();
    xGraphicStorageHandler.clear();
    if( xEmbeddedResolver )
        xEmbeddedResolver->dispose();
    xEmbeddedResolver.clear();

    OSL_ENSURE( !m_pTableLines, "there are table columns infos left" );

    return nRet;
}

XMLTextParagraphExport* SwXMLExport::CreateTextParagraphExport()
{
    return new SwXMLTextParagraphExport(*this, *GetAutoStylePool());
}

XMLShapeExport* SwXMLExport::CreateShapeExport()
{
    XMLShapeExport* pShapeExport = new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
    Reference < XDrawPageSupplier > xDPS( GetModel(), UNO_QUERY );
    if( xDPS.is() )
    {
        Reference < XShapes > xShapes( xDPS->getDrawPage(), UNO_QUERY );
        pShapeExport->seekShapes( xShapes );
    }

    return pShapeExport;
}

SwXMLExport::~SwXMLExport()
{
    DeleteTableLines();
    FinitItemExport();
}

void SwXMLExport::ExportFontDecls_()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::ExportFontDecls_();
}

void SwXMLExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    aProps.realloc(7);
     // Currently exporting 9 properties
    PropertyValue *pValue = aProps.getArray();

    Reference < XIndexContainer > xBox = IndexedPropertyValues::create( comphelper::getProcessComponentContext() );
    pValue[0].Name = "Views";
    pValue[0].Value <<= xBox;

    SwDoc *pDoc = getDoc();
    const tools::Rectangle rRect =
        pDoc->GetDocShell()->GetVisArea( ASPECT_CONTENT );
    bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MapUnit::MapTwip;

    OSL_ENSURE( bTwip, "Map unit for visible area is not in TWIPS!" );

    pValue[1].Name = "ViewAreaTop";
    pValue[1].Value <<= bTwip ? convertTwipToMm100 ( rRect.Top() ) : rRect.Top();

    pValue[2].Name = "ViewAreaLeft";
    pValue[2].Value <<= bTwip ? convertTwipToMm100 ( rRect.Left() ) : rRect.Left();

    pValue[3].Name = "ViewAreaWidth";
    pValue[3].Value <<= bTwip ? convertTwipToMm100 ( rRect.GetWidth() ) : rRect.GetWidth();

    pValue[4].Name = "ViewAreaHeight";
    pValue[4].Value <<= bTwip ? convertTwipToMm100 ( rRect.GetHeight() ) : rRect.GetHeight();

    // "show redline mode" cannot simply be read from the document
    // since it gets changed during execution. If it's in the info
    // XPropertySet, we take it from there.
    bool bShowRedlineChanges = m_bSavedShowChanges;
    Reference<XPropertySet> xInfoSet( getExportInfo() );
    if ( xInfoSet.is() )
    {
        const OUString sShowChanges( "ShowChanges" );
        if( xInfoSet->getPropertySetInfo()->hasPropertyByName( sShowChanges ) )
        {
            bShowRedlineChanges = *o3tl::doAccess<bool>(xInfoSet->
                                   getPropertyValue( sShowChanges ));
        }
    }

    pValue[5].Name = "ShowRedlineChanges";
    pValue[5].Value <<= bShowRedlineChanges;

    pValue[6].Name = "InBrowseMode";
    pValue[6].Value <<= pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE);
}

void SwXMLExport::GetConfigurationSettings( Sequence < PropertyValue >& rProps)
{
    Reference< XMultiServiceFactory > xFac( GetModel(), UNO_QUERY );
    if( xFac.is() )
    {
        Reference< XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), UNO_QUERY );
        if( xProps.is() )
            SvXMLUnitConverter::convertPropertySet( rProps, xProps );
    }
}

sal_Int32 SwXMLExport::GetDocumentSpecificSettings( std::vector< SettingsGroup >& _out_rSettings )
{
    // the only doc-specific settings group we know so far are the XForms settings
    uno::Sequence<beans::PropertyValue> aXFormsSettings;
    Reference< XFormsSupplier > xXFormsSupp( GetModel(), UNO_QUERY );
    Reference< XNameAccess > xXForms;
    if ( xXFormsSupp.is() )
        xXForms = xXFormsSupp->getXForms().get();
    if ( xXForms.is() )
    {
        getXFormsSettings( xXForms, aXFormsSettings );
        _out_rSettings.emplace_back( XML_XFORM_MODEL_SETTINGS, aXFormsSettings );
    }

    return aXFormsSettings.getLength() + SvXMLExport::GetDocumentSpecificSettings( _out_rSettings );
}

void SwXMLExport::SetBodyAttributes()
{
    // export use of soft page breaks
    SwDoc *pDoc = getDoc();
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() &&
        pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->GetPageCount() > 1 )
    {
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, true);
        AddAttribute(XML_NAMESPACE_TEXT, XML_USE_SOFT_PAGE_BREAKS,
            sBuffer.makeStringAndClear());
    }
}

void SwXMLExport::ExportContent_()
{
    // export forms
    Reference<XDrawPageSupplier> xDrawPageSupplier(GetModel(), UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        // export only if we actually have elements
        Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
        if (xPage.is())
        {
            // prevent export of form controls which are embedded in mute sections
            GetTextParagraphExport()->PreventExportOfControlsInMuteSections(
                xPage, GetFormExport() );

            // #i36597#
            if ( xmloff::OFormLayerXMLExport::pageContainsForms( xPage ) || GetFormExport()->documentContainsXForms() )
            {
                ::xmloff::OOfficeFormsExport aOfficeForms(*this);

                GetFormExport()->exportXForms();

                GetFormExport()->seekPage(xPage);
                GetFormExport()->exportForms(xPage);
            }
        }
    }

    Reference<XPropertySet> xPropSet(GetModel(), UNO_QUERY);
    if (xPropSet.is())
    {
        Any aAny = xPropSet->getPropertyValue( "TwoDigitYear" );
        aAny <<= sal_Int16(1930);

        sal_Int16 nYear = 0;
        aAny >>= nYear;
        if (nYear != 1930 )
        {
            AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, OUString::number(nYear));
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, true, true);
        }
    }

    GetTextParagraphExport()->exportTrackedChanges( false );
    GetTextParagraphExport()->exportTextDeclarations();
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->exportFramesBoundToPage( m_bShowProgress );
    GetTextParagraphExport()->exportText( xText, m_bShowProgress );
}

namespace
{
    class theSwXMLExportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXMLExportUnoTunnelId > {};
}

const Sequence< sal_Int8 > & SwXMLExport::getUnoTunnelId() throw()
{
    return theSwXMLExportUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXMLExport::getSomething( const Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return SvXMLExport::getSomething( rId );
}

SwDoc* SwXMLExport::getDoc()
{
    if( m_pDoc != nullptr )
        return m_pDoc;
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    if (!xTextDoc)
    {
        SAL_WARN("sw.filter", "Problem of mismatching filter for export.");
        return nullptr;
    }

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

const SwDoc* SwXMLExport::getDoc() const
{
    return const_cast< SwXMLExport* >( this )->getDoc();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLExporter",
                SvXMLExportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLStylesExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLStylesExporter",
                SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES | SvXMLExportFlags::AUTOSTYLES |
                SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLContentExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLContentExporter",
                SvXMLExportFlags::SCRIPTS | SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES |
                SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLMetaExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLMetaExporter",
                SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLSettingsExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLSettingsExporter",
                SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLOasisExporter",
                SvXMLExportFlags::ALL | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisStylesExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLOasisStylesExporter",
                SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES | SvXMLExportFlags::AUTOSTYLES |
                SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisContentExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLOasisContentExporter",
                SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS |
                SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisMetaExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLOasisMetaExporter",
                SvXMLExportFlags::META | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisSettingsExporter_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXMLExport(context, "com.sun.star.comp.Writer.XMLOasisSettingsExporter",
                SvXMLExportFlags::SETTINGS | SvXMLExportFlags::OASIS));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
