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
#include <vcl/FilterConfigItem.hxx>
#include <memory>

// - PDFFilter -
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
    sal_Int32                   nLength = rDescriptor.getLength();
    const PropertyValue*        pValue = rDescriptor.getConstArray();
    bool                    bRet = false;
    Reference< task::XStatusIndicator > xStatusIndicator;
    Reference< task::XInteractionHandler > xIH;

    for ( sal_Int32 i = 0 ; ( i < nLength ) && !xOStm.is(); ++i)
    {
        if ( pValue[ i ].Name == "OutputStream" )
            pValue[ i ].Value >>= xOStm;
        else if ( pValue[ i ].Name == "FilterData" )
            pValue[ i ].Value >>= aFilterData;
        else if ( pValue[ i ].Name == "StatusIndicator" )
            pValue[ i ].Value >>= xStatusIndicator;
        else if ( pValue[i].Name == "InteractionHandler" )
            pValue[i].Value >>= xIH;
    }

    /* we don't get FilterData if we are exporting directly
     to pdf, but we have to use the last user settings (especially for the CompressMode) */
    if ( !aFilterData.getLength() )
    {
        FilterConfigItem aCfgItem( "Office.Common/Filter/PDF/Export/" );
        aCfgItem.ReadBool(  "UseLosslessCompression", false );
        aCfgItem.ReadInt32( "Quality", 90 );
        aCfgItem.ReadBool(  "ReduceImageResolution", false );
        aCfgItem.ReadInt32( "MaxImageResolution", 300 );
        aCfgItem.ReadBool(  "UseTaggedPDF", false );
        aCfgItem.ReadInt32( "SelectPdfVersion", 0 );
        aCfgItem.ReadBool(  "ExportNotes", false );
        aCfgItem.ReadBool(  "ExportNotesPages", false );
        aCfgItem.ReadBool(  "UseTransitionEffects", true );
        aCfgItem.ReadBool(  "IsSkipEmptyPages", false );
        aCfgItem.ReadBool(  "ExportFormFields", true );
        aCfgItem.ReadInt32( "FormsType", 0 );
        aCfgItem.ReadBool(  "HideViewerToolbar", false );
        aCfgItem.ReadBool(  "HideViewerMenubar", false );
        aCfgItem.ReadBool(  "HideViewerWindowControls", false );
        aCfgItem.ReadBool(  "ResizeWindowToInitialPage", false );
        aCfgItem.ReadBool(  "CenterWindow", false );
        aCfgItem.ReadBool(  "OpenInFullScreenMode", false );
        aCfgItem.ReadBool(  "DisplayPDFDocumentTitle", true );
        aCfgItem.ReadInt32( "InitialView", 0 );
        aCfgItem.ReadInt32( "Magnification", 0 );
        aCfgItem.ReadInt32( "PageLayout", 0 );
        aCfgItem.ReadBool(  "FirstPageOnLeft", false );
        aCfgItem.ReadBool(  "IsAddStream", false );
//
// the encryption is not available when exporting directly, since the encryption is off by default and the selection
// (encrypt or not) is not persistent; it's available through macro though,
// provided the correct property values are set, see help
//
// now, the relative link stuff
        aCfgItem.ReadBool( "ExportLinksRelativeFsys", false );
        aCfgItem.ReadInt32("PDFViewSelection", 0 );
        aCfgItem.ReadBool( "ConvertOOoTargetToPDFTarget", false );
        aCfgItem.ReadBool( "ExportBookmarksToPDFDestination", false );

        aCfgItem.ReadBool(  "ExportBookmarks", true );
        aCfgItem.ReadBool(  "ExportHiddenSlides", false );
        aCfgItem.ReadInt32( "OpenBookmarkLevels", -1 );
        aFilterData = aCfgItem.GetFilterData();
    }
    if( mxSrcDoc.is() && xOStm.is() )
    {
        PDFExport       aExport( mxSrcDoc, xStatusIndicator, xIH, mxContext );
        ::utl::TempFile aTempFile;

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


class FocusWindowWaitCursor
{
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

    DECL_LINK_TYPED( DestroyedLink, VclWindowEvent&, void );
};

IMPL_LINK_TYPED( FocusWindowWaitCursor, DestroyedLink, VclWindowEvent&, rEvent, void )
{
    if( rEvent.GetId() == VCLEVENT_OBJECT_DYING )
        m_pFocusWindow = nullptr;
}

sal_Bool SAL_CALL PDFFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException, std::exception)
{
    FocusWindowWaitCursor aCur;

    const bool bRet = implExport( rDescriptor );

    return bRet;
}



void SAL_CALL PDFFilter::cancel( ) throw (RuntimeException, std::exception)
{
}



void SAL_CALL PDFFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException, std::exception)
{
    mxSrcDoc = xDoc;
}



void SAL_CALL PDFFilter::initialize( const css::uno::Sequence< css::uno::Any >& )
    throw (Exception, RuntimeException, std::exception)
{
}

OUString PDFFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( "com.sun.star.comp.PDF.PDFFilter" );
}

Sequence< OUString > SAL_CALL PDFFilter_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence<OUString> aRet { "com.sun.star.document.PDFFilter" };
    return aRet;
}

Reference< XInterface > SAL_CALL PDFFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new PDFFilter( comphelper::getComponentContext(rSMgr) ));
}



OUString SAL_CALL PDFFilter::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return PDFFilter_getImplementationName();
}



sal_Bool SAL_CALL PDFFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}



css::uno::Sequence< OUString > SAL_CALL PDFFilter::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return PDFFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
