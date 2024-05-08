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

#include "pdffilter.hxx"
#include "pdfexport.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <svl/outstrm.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/glyphitemcache.hxx>
#include <memory>

#include <com/sun/star/io/XOutputStream.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>

#include <boost/property_tree/json_parser/error.hpp>

using namespace ::com::sun::star::io;

PDFFilter::PDFFilter( const Reference< XComponentContext > &rxContext ) :
    mxContext( rxContext )
{
}


PDFFilter::~PDFFilter()
{
}


bool PDFFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
{
    Reference< XOutputStream >  xOStm;
    Sequence< PropertyValue >   aFilterData;
    OUString aFilterOptions;
    bool                        bIsRedactMode = false;
    bool                    bRet = false;
    Reference< task::XStatusIndicator > xStatusIndicator;
    Reference< task::XInteractionHandler > xIH;

    for (const auto& rValue : rDescriptor)
    {
        if (rValue.Name == "OutputStream")
            rValue.Value >>= xOStm;
        else if (rValue.Name == "FilterData")
            rValue.Value >>= aFilterData;
        else if (rValue.Name == "FilterOptions")
            rValue.Value >>= aFilterOptions;
        else if (rValue.Name == "StatusIndicator")
            rValue.Value >>= xStatusIndicator;
        else if (rValue.Name == "InteractionHandler")
            rValue.Value >>= xIH;
        else if (rValue.Name == "IsRedactMode")
            rValue.Value >>= bIsRedactMode;
    }

    if (!aFilterData.hasElements() && aFilterOptions.startsWith("{"))
    {
        try
        {
            // Allow setting filter data keys from the cmdline.
            std::vector<PropertyValue> aData
                = comphelper::JsonToPropertyValues(aFilterOptions.toUtf8());
            aFilterData = comphelper::containerToSequence(aData);
        }
        catch (const boost::property_tree::json_parser::json_parser_error& e)
        {
            // This wasn't a valid json; maybe came from import filter (tdf#150846)
            SAL_WARN("filter.pdf", "error parsing FilterOptions: " << e.message());
        }
    }

    /* we don't get FilterData if we are exporting directly
       to pdf, but we have to use the last user settings (especially for the CompressMode) */
    if ( !aFilterData.hasElements() )
    {
        FilterConfigItem aCfgItem( u"Office.Common/Filter/PDF/Export/" );
        aCfgItem.ReadBool(  u"UseLosslessCompression"_ustr, false );
        aCfgItem.ReadInt32( u"Quality"_ustr, 90 );
        aCfgItem.ReadBool(  u"ReduceImageResolution"_ustr, false );
        aCfgItem.ReadInt32( u"MaxImageResolution"_ustr, 300 );
        aCfgItem.ReadBool(  u"UseTaggedPDF"_ustr, false );
        aCfgItem.ReadInt32( u"SelectPdfVersion"_ustr, 0 );
        aCfgItem.ReadBool(u"PDFUACompliance"_ustr, false);
        aCfgItem.ReadBool(  u"ExportNotes"_ustr, false );
        aCfgItem.ReadBool(  u"ExportNotesInMargin"_ustr, false );
        aCfgItem.ReadBool( u"ExportPlaceholders"_ustr, false );
        aCfgItem.ReadBool(  u"ExportNotesPages"_ustr, false );
        aCfgItem.ReadBool(  u"ExportOnlyNotesPages"_ustr, false );
        aCfgItem.ReadBool(  u"UseTransitionEffects"_ustr, true );
        aCfgItem.ReadBool(  u"IsSkipEmptyPages"_ustr, false );
        aCfgItem.ReadBool(  u"ExportFormFields"_ustr, true );
        aCfgItem.ReadInt32( u"FormsType"_ustr, 0 );
        aCfgItem.ReadBool(  u"HideViewerToolbar"_ustr, false );
        aCfgItem.ReadBool(  u"HideViewerMenubar"_ustr, false );
        aCfgItem.ReadBool(  u"HideViewerWindowControls"_ustr, false );
        aCfgItem.ReadBool(  u"ResizeWindowToInitialPage"_ustr, false );
        aCfgItem.ReadBool(  u"CenterWindow"_ustr, false );
        aCfgItem.ReadBool(  u"OpenInFullScreenMode"_ustr, false );
        aCfgItem.ReadBool(  u"DisplayPDFDocumentTitle"_ustr, true );
        aCfgItem.ReadInt32( u"InitialView"_ustr, 0 );
        aCfgItem.ReadInt32( u"Magnification"_ustr, 0 );
        aCfgItem.ReadInt32( u"Zoom"_ustr, 100 );
        aCfgItem.ReadInt32( u"PageLayout"_ustr, 0 );
        aCfgItem.ReadBool(  u"FirstPageOnLeft"_ustr, false );
        aCfgItem.ReadInt32( u"InitialPage"_ustr, 1 );
        aCfgItem.ReadBool(  u"IsAddStream"_ustr, false );

        // the encryption is not available when exporting directly, since the encryption is off by default and the selection
        // (encrypt or not) is not persistent; it's available through macro though,
        // provided the correct property values are set, see help

        // now, the relative link stuff
        aCfgItem.ReadBool( u"ExportLinksRelativeFsys"_ustr, false );
        aCfgItem.ReadInt32(u"PDFViewSelection"_ustr, 0 );
        aCfgItem.ReadBool( u"ConvertOOoTargetToPDFTarget"_ustr, false );
        aCfgItem.ReadBool( u"ExportBookmarksToPDFDestination"_ustr, false );

        aCfgItem.ReadBool(  u"ExportBookmarks"_ustr, true );
        aCfgItem.ReadBool(  u"ExportHiddenSlides"_ustr, false );
        aCfgItem.ReadBool(  u"SinglePageSheets"_ustr, false );
        aCfgItem.ReadInt32( u"OpenBookmarkLevels"_ustr, -1 );

        aCfgItem.ReadBool( u"IsRedactMode"_ustr, false);

        aFilterData = aCfgItem.GetFilterData();
    }


    if (bIsRedactMode)
    {
        bool bFound = false;

        for (PropertyValue& rProp : asNonConstRange(aFilterData))
        {
            if (rProp.Name == "IsRedactMode")
            {
                rProp.Value <<= bIsRedactMode;
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            sal_Int32 nNewSize = aFilterData.getLength() + 1;
            aFilterData.realloc( nNewSize );
            auto pFilterData = aFilterData.getArray();
            pFilterData[nNewSize - 1].Name = "IsRedactMode";
            pFilterData[nNewSize - 1].Value <<= bIsRedactMode;
        }
    }

    if( mxSrcDoc.is() && xOStm.is() )
    {
        PDFExport       aExport( mxSrcDoc, xStatusIndicator, xIH, mxContext );
        ::utl::TempFileNamed aTempFile;

        aTempFile.EnableKillingFile();
        bRet = aExport.Export( aTempFile.GetURL(), aFilterData );

        if( bRet )
        {
            std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), StreamMode::READ ));

            if( pIStm )
            {
                SvOutputStream aOStm( xOStm );

                aOStm.WriteStream( *pIStm );
                bRet = ( aOStm.Tell() && ( aOStm.GetError() == ERRCODE_NONE ) );
            }
        }
    }

    return bRet;
}

namespace {

class FocusWindowWaitCursor
{
private:

    VclPtr<vcl::Window>         m_pFocusWindow;

public:
    FocusWindowWaitCursor() :
        m_pFocusWindow( Application::GetFocusWindow() )
    {
        if( m_pFocusWindow )
        {
            m_pFocusWindow->AddEventListener( LINK( this, FocusWindowWaitCursor, DestroyedLink ) );
            m_pFocusWindow->EnterWait();
        }
    }

    ~FocusWindowWaitCursor()
    {
        if( m_pFocusWindow )
        {
            m_pFocusWindow->LeaveWait();
            m_pFocusWindow->RemoveEventListener( LINK( this, FocusWindowWaitCursor, DestroyedLink ) );
        }
    }

    DECL_LINK( DestroyedLink, VclWindowEvent&, void );
};

}

IMPL_LINK( FocusWindowWaitCursor, DestroyedLink, VclWindowEvent&, rEvent, void )
{
    if( rEvent.GetId() == VclEventId::ObjectDying )
        m_pFocusWindow = nullptr;
}


sal_Bool SAL_CALL PDFFilter::filter( const Sequence< PropertyValue >& rDescriptor )
{
    FocusWindowWaitCursor aCur;

    SalLayoutGlyphsCache::self()->SetCacheGlyphsWhenDoingFallbackFonts(true);

    const bool bRet = implExport( rDescriptor );

    SalLayoutGlyphsCache::self()->SetCacheGlyphsWhenDoingFallbackFonts(false);

    return bRet;
}


void SAL_CALL PDFFilter::cancel( )
{
}


void SAL_CALL PDFFilter::setSourceDocument( const Reference< XComponent >& xDoc )
{
    mxSrcDoc = xDoc;
}


void SAL_CALL PDFFilter::initialize( const css::uno::Sequence< css::uno::Any >& )
{
}


OUString SAL_CALL PDFFilter::getImplementationName()
{
    return u"com.sun.star.comp.PDF.PDFFilter"_ustr;
}


sal_Bool SAL_CALL PDFFilter::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}


css::uno::Sequence< OUString > SAL_CALL PDFFilter::getSupportedServiceNames(  )
{
    return { u"com.sun.star.document.PDFFilter"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_PDFFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PDFFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
