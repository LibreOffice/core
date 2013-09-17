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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>

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
#include <pam.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <docstat.hxx>
#include <swerror.h>
#include <unotext.hxx>
#include <xmltexte.hxx>
#include <xmlexp.hxx>
#include <sfx2/viewsh.hxx>
#include <comphelper/processfactory.hxx>
#include <docary.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <comphelper/servicehelper.hxx>

// for locking SolarMutex: svapp + mutex
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <pausethreadstarting.hxx> // #i73788#


using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
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
    const uno::Reference< uno::XComponentContext > xContext,
    sal_uInt16 nExportFlags)
:   SvXMLExport( util::MeasureUnit::INCH, xContext, XML_TEXT,
        nExportFlags ),
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
    bBlock( sal_False ),
    bShowProgress( sal_True ),
    doc( NULL ),
    sNumberFormat("NumberFormat"),
    sIsProtected("IsProtected"),
    sCell("Cell")
{
    _InitItemExport();
}

void SwXMLExport::setBlockMode()
{
    bBlock = sal_True;

}

sal_uInt32 SwXMLExport::exportDoc( enum XMLTokenEnum eClass )
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
            OUString sAutoTextMode("AutoTextMode");
            if( rInfoSet->getPropertySetInfo()->hasPropertyByName(
                        sAutoTextMode ) )
            {
                Any aAny = rInfoSet->getPropertyValue(sAutoTextMode);
                if( aAny.getValueType() == ::getBooleanCppuType() &&
                    *static_cast<const sal_Bool*>(aAny.getValue()) )
                    setBlockMode();
            }
        }
    }

    SwDoc *pDoc = getDoc();

    sal_Bool bExtended = sal_False;
    if( (getExportFlags() & (EXPORT_FONTDECLS|EXPORT_STYLES|
                             EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
        {
            _GetNamespaceMap().Add(
                GetXMLToken(XML_NP_OFFICE_EXT),
                GetXMLToken(XML_N_OFFICE_EXT),
                XML_NAMESPACE_OFFICE_EXT);
        }

        GetTextParagraphExport()->SetBlockMode( bBlock );

        const SfxPoolItem* pItem;
        const SfxItemPool& rPool = pDoc->GetAttrPool();
        sal_uInt16 aWhichIds[5] = { RES_UNKNOWNATR_CONTAINER,
                                    RES_TXTATR_UNKNOWN_CONTAINER,
                                    SDRATTR_XMLATTRIBUTES,
                                    EE_PARA_XMLATTRIBS,
                                    EE_CHAR_XMLATTRIBS };

        sal_uInt16 nWhichIds = rPool.GetSecondaryPool() ? 5 : 2;
        for( sal_uInt16 j=0; j < nWhichIds; j++ )
        {
            sal_uInt16 nWhichId = aWhichIds[j];
            sal_uInt32 i=0, nItems = rPool.GetItemCount2( nWhichId );
            for( i = 0; i < nItems; ++i )
            {
                if( 0 != (pItem = rPool.GetItem2( nWhichId , i ) ) )
                {
                    const SvXMLAttrContainerItem *pUnknown =
                                PTR_CAST( SvXMLAttrContainerItem, pItem );
                    OSL_ENSURE( pUnknown, "illegal attribute container item" );
                    if( pUnknown && (pUnknown->GetAttrCount() > 0) )
                    {
                        sal_uInt16 nIdx = pUnknown->GetFirstNamespaceIndex();
                        while( USHRT_MAX != nIdx )
                        {
                            const OUString& rPrefix =
                                pUnknown->GetPrefix( nIdx );
                            _GetNamespaceMap().Add( rPrefix,
                                                pUnknown->GetNamespace( nIdx ),
                                                XML_NAMESPACE_UNKNOWN );
                            nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
                        }
                        bExtended = sal_True;
                    }
                }
            }
        }
    }

    sal_uInt16 const eUnit = SvXMLUnitConverter::GetMeasureUnit(
            SW_MOD()->GetMetric(pDoc->get(IDocumentSettingAccess::HTML_MODE)));
    if (GetMM100UnitConverter().GetXMLMeasureUnit() != eUnit )
    {
        GetMM100UnitConverter().SetXMLMeasureUnit( eUnit );
        pTwipUnitConv->SetXMLMeasureUnit( eUnit );
    }

    SetExtended( bExtended );

    if( (getExportFlags() & EXPORT_META) != 0 )
    {
        // Update doc stat, so that correct values are exported and
        // the progress works correctly.
        pDoc->UpdateDocStat();

        SfxObjectShell* pObjSh = pDoc->GetDocShell();
        if( pObjSh )
            pObjSh->UpdateDocInfoForSave();     // update information
    }
    if( bShowProgress )
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
            nRef += pDoc->GetCharFmts()->size() - 1;
            nRef += pDoc->GetFrmFmts()->size() - 1;
            nRef += pDoc->GetTxtFmtColls()->size() - 1;
            nRef *= 2; // for the above styles, xmloff will increment by 2!
            // #i93174#: count all paragraphs for the progress bar
            nRef += pDoc->GetUpdatedDocStat().nAllPara; // 1: only content, no autostyle
            pProgress->SetReference( nRef );
            pProgress->SetValue( 0 );
        }
    }

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        //Auf die Korrektheit der OrdNums sind wir schon angewiesen.
        SdrModel* pModel = pDoc->GetDrawModel();
        if( pModel )
            pModel->GetPage( 0 )->RecalcObjOrdNums();
    }

    // adjust document class (eClass)
    if (pDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT))
    {
        eClass = XML_TEXT_GLOBAL;

        // additionally, we take care of the save-linked-sections-thingy
        mbSaveLinkedSections = pDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
    }
    // MIB: 03/26/04: The Label information is saved in the settings, so
    // we don't need it here.
    // else: keep default pClass that we received

    SvXMLGraphicHelper *pGraphicResolver = 0;
    if( !GetGraphicResolver().is() )
    {
        pGraphicResolver = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_WRITE );
        Reference< XGraphicObjectResolver > xGraphicResolver( pGraphicResolver );
        SetGraphicResolver( xGraphicResolver );
    }

    SvXMLEmbeddedObjectHelper *pEmbeddedResolver = 0;
    if( !GetEmbeddedResolver().is() )
    {
        SfxObjectShell *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            pEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            EMBEDDEDOBJECTHELPER_MODE_WRITE );
            Reference< XEmbeddedObjectResolver > xEmbeddedResolver( pEmbeddedResolver );
            SetEmbeddedResolver( xEmbeddedResolver );
        }
    }

    // set redline mode if we export STYLES or CONTENT, unless redline
    // mode is taken care of outside (through info XPropertySet)
    sal_Bool bSaveRedline =
        ( (getExportFlags() & (EXPORT_CONTENT|EXPORT_STYLES)) != 0 );
    if( bSaveRedline )
    {
        // if the info property set has a ShowChanges property,
        // then change tracking is taken care of on the outside,
        // so we don't have to!
        Reference<XPropertySet> rInfoSet = getExportInfo();
        if( rInfoSet.is() )
        {
            OUString sShowChanges("ShowChanges");
            bSaveRedline = ! rInfoSet->getPropertySetInfo()->hasPropertyByName(
                                                                sShowChanges );
        }
    }
    sal_uInt16 nRedlineMode = 0;
    bSavedShowChanges = IDocumentRedlineAccess::IsShowChanges( pDoc->GetRedlineMode() );
    if( bSaveRedline )
    {
        // now save and switch redline mode
        nRedlineMode = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode(
                 (RedlineMode_t)(( nRedlineMode & nsRedlineMode_t::REDLINE_SHOW_MASK ) | nsRedlineType_t::REDLINE_INSERT ));
    }

     sal_uInt32 nRet = SvXMLExport::exportDoc( eClass );

    // now we can restore the redline mode (if we changed it previously)
    if( bSaveRedline )
    {
      pDoc->SetRedlineMode( (RedlineMode_t)(nRedlineMode ));
    }


    if( pGraphicResolver )
        SvXMLGraphicHelper::Destroy( pGraphicResolver );
    if( pEmbeddedResolver )
        SvXMLEmbeddedObjectHelper::Destroy( pEmbeddedResolver );

    OSL_ENSURE( !pTableLines, "there are table columns infos left" );

    return nRet;
}

XMLTextParagraphExport* SwXMLExport::CreateTextParagraphExport()
{
    return new SwXMLTextParagraphExport( *this, *GetAutoStylePool().get() );
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
    _FinitItemExport();
}


void SwXMLExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::_ExportFontDecls();
}

#define NUM_EXPORTED_VIEW_SETTINGS 11
void SwXMLExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    aProps.realloc( NUM_EXPORTED_VIEW_SETTINGS );
     // Currently exporting 9 properties
    PropertyValue *pValue = aProps.getArray();
    sal_Int32 nIndex = 0;

    Reference < XIndexContainer > xBox = IndexedPropertyValues::create( comphelper::getProcessComponentContext() );
    pValue[nIndex].Name = OUString( "Views");
    pValue[nIndex++].Value <<= xBox;

    SwDoc *pDoc = getDoc();
    const Rectangle rRect =
        pDoc->GetDocShell()->GetVisArea( ASPECT_CONTENT );
    sal_Bool bTwip = pDoc->GetDocShell()->GetMapUnit ( ) == MAP_TWIP;

   OSL_ENSURE( bTwip, "Map unit for visible area is not in TWIPS!" );

    pValue[nIndex].Name = OUString( "ViewAreaTop");
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.Top() ) : rRect.Top();

    pValue[nIndex].Name = OUString( "ViewAreaLeft");
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.Left() ) : rRect.Left();

    pValue[nIndex].Name = OUString( "ViewAreaWidth");
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.GetWidth() ) : rRect.GetWidth();

    pValue[nIndex].Name = OUString( "ViewAreaHeight");
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.GetHeight() ) : rRect.GetHeight();

    // "show redline mode" cannot simply be read from the document
    // since it gets changed during execution. If it's in the info
    // XPropertySet, we take it from there.
    sal_Bool bShowRedlineChanges = bSavedShowChanges;
    Reference<XPropertySet> xInfoSet( getExportInfo() );
    if ( xInfoSet.is() )
    {
        OUString sShowChanges( "ShowChanges" );
        if( xInfoSet->getPropertySetInfo()->hasPropertyByName( sShowChanges ) )
        {
            bShowRedlineChanges = *(sal_Bool*) xInfoSet->
                                   getPropertyValue( sShowChanges ).getValue();
        }
    }

    pValue[nIndex].Name = OUString( "ShowRedlineChanges");
    pValue[nIndex++].Value.setValue( &bShowRedlineChanges, ::getBooleanCppuType() );

    sal_Bool bInBrowse =  pDoc->get(IDocumentSettingAccess::BROWSE_MODE);
    pValue[nIndex].Name = OUString( "InBrowseMode");
    pValue[nIndex++].Value.setValue( &bInBrowse, ::getBooleanCppuType() );

    if ( nIndex < NUM_EXPORTED_VIEW_SETTINGS )
        aProps.realloc(nIndex);
}
#undef NUM_EXPORTED_VIEW_SETTINGS

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

sal_Int32 SwXMLExport::GetDocumentSpecificSettings( ::std::list< SettingsGroup >& _out_rSettings )
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
        _out_rSettings.push_back( SettingsGroup( XML_XFORM_MODEL_SETTINGS, aXFormsSettings ) );
    }

    return aXFormsSettings.getLength() + SvXMLExport::GetDocumentSpecificSettings( _out_rSettings );
}

void SwXMLExport::SetBodyAttributes()
{
    // export use of soft page breaks
    SwDoc *pDoc = getDoc();
    if( pDoc->GetCurrentViewShell() &&
        pDoc->GetCurrentViewShell()->GetPageCount() > 1 )
    {
        bool bValue = true;
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, bValue);
        AddAttribute(XML_NAMESPACE_TEXT, XML_USE_SOFT_PAGE_BREAKS,
            sBuffer.makeStringAndClear());
    }
}


void SwXMLExport::_ExportContent()
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
            if ( GetFormExport()->pageContainsForms( xPage ) || GetFormExport()->documentContainsXForms() )
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
        OUString sTwoDigitYear("TwoDigitYear");

        Any aAny = xPropSet->getPropertyValue( sTwoDigitYear );
        aAny <<= (sal_Int16)1930;

        sal_Int16 nYear = 0;
        aAny >>= nYear;
        if (nYear != 1930 )
        {
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber(sBuffer, nYear);
            AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, sBuffer.makeStringAndClear());
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, sal_True, sal_True);
        }
    }

    GetTextParagraphExport()->exportTrackedChanges( sal_False );
    GetTextParagraphExport()->exportTextDeclarations();
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->exportFramesBoundToPage( bShowProgress );
    GetTextParagraphExport()->exportText( xText, bShowProgress );
}



//
// uno component registration
// helper functions for export service(s)
//

// OOo
OUString SAL_CALL SwXMLExportOOO_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportOOO_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportOOO_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_ALL);
}

OUString SAL_CALL SwXMLExportStylesOOO_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLStylesExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportStylesOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportStylesOOO_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportStylesOOO_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr),
        EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES |
        EXPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLExportContentOOO_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLContentExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportContentOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportContentOOO_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportContentOOO_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr),
        EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_SCRIPTS |
        EXPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLExportMetaOOO_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLMetaExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportMetaOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportMetaOOO_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportMetaOOO_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_META);
}

OUString SAL_CALL SwXMLExportSettingsOOO_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLSettingsExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportSettingsOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportSettingsOOO_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportSettingsOOO_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_SETTINGS);
}

// OASIS
OUString SAL_CALL SwXMLExport_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExport_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExport_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_ALL|EXPORT_OASIS);
}

OUString SAL_CALL SwXMLExportStyles_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisStylesExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportStyles_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportStyles_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportStyles_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr),
        EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES |
        EXPORT_FONTDECLS|EXPORT_OASIS );
}

OUString SAL_CALL SwXMLExportContent_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisContentExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportContent_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportContent_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportContent_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(
        comphelper::getComponentContext(rSMgr),
        EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_SCRIPTS |
        EXPORT_FONTDECLS|EXPORT_OASIS );
}

OUString SAL_CALL SwXMLExportMeta_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisMetaExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportMeta_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportMeta_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportMeta_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_META|EXPORT_OASIS);
}

OUString SAL_CALL SwXMLExportSettings_getImplementationName() throw()
{
    return OUString(
        "com.sun.star.comp.Writer.XMLOasisSettingsExporter" );
}

Sequence< OUString > SAL_CALL SwXMLExportSettings_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportSettings_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportSettings_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_SETTINGS|EXPORT_OASIS);
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
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return SvXMLExport::getSomething( rId );
}


// XServiceInfo
// override empty method from parent class
OUString SAL_CALL SwXMLExport::getImplementationName()
    throw(RuntimeException)
{
    switch( getExportFlags() )
    {
        case EXPORT_ALL:
            return SwXMLExport_getImplementationName();
        case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS):
            return SwXMLExportStyles_getImplementationName();
        case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
            return SwXMLExportContent_getImplementationName();
        case EXPORT_META:
            return SwXMLExportMeta_getImplementationName();
        case EXPORT_SETTINGS:
            return SwXMLExportSettings_getImplementationName();
        default:
            // generic name for 'unknown' cases
            return OUString(
                "com.sun.star.comp.Writer.SwXMLExport" );
    }
}

SwDoc* SwXMLExport::getDoc()
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

const SwDoc* SwXMLExport::getDoc() const
{
    return const_cast< SwXMLExport* >( this )->getDoc();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
