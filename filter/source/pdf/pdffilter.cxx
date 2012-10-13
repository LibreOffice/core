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
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <svl/outstrm.hxx>
#include <svtools/FilterConfigItem.hxx>

// -------------
// - PDFFilter -
// -------------

PDFFilter::PDFFilter( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF )
{
}

// -----------------------------------------------------------------------------

PDFFilter::~PDFFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool PDFFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
{
    Reference< XOutputStream >  xOStm;
    Sequence< PropertyValue >   aFilterData;
    sal_Int32                   nLength = rDescriptor.getLength();
    const PropertyValue*        pValue = rDescriptor.getConstArray();
    sal_Bool                    bRet = sal_False;
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
        FilterConfigItem aCfgItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/PDF/Export/" ) ) );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ), sal_False );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), 90 );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ), sal_False );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ), 300 );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ), sal_False );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "SelectPdfVersion" ) ), 0 );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes" ) ), sal_False );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) ), sal_False );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects" ) ), sal_True );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "IsSkipEmptyPages" ) ), sal_False );
        aCfgItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "ExportFormFields" ) ), sal_True );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FormsType" ) ), 0 );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "HideViewerToolbar" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "HideViewerMenubar" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "HideViewerWindowControls" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "ResizeWindowToInitialPage" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "CenterWindow" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "OpenInFullScreenMode" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "DisplayPDFDocumentTitle" ) ), sal_True );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "InitialView" ) ), 0 );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Magnification" ) ), 0 );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PageLayout" ) ), 0 );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "FirstPageOnLeft" ) ), sal_False );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "IsAddStream" ) ), sal_False );
//
// the encryption is not available when exporting directly, since the encryption is off by default and the selection
// (encrypt or not) is not persistent; it's available through macro though,
// provided the correct property values are set, see help
//
// now, the relative link stuff
        aCfgItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportLinksRelativeFsys" ) ), sal_False );
        aCfgItem.ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "PDFViewSelection" ) ), 0 );
        aCfgItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ConvertOOoTargetToPDFTarget" ) ), sal_False );
        aCfgItem.ReadBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarksToPDFDestination" ) ), sal_False );

        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "ExportBookmarks" ) ), sal_True );
        aCfgItem.ReadBool(  String( RTL_CONSTASCII_USTRINGPARAM( "ExportHiddenSlides" ) ), sal_False );
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "OpenBookmarkLevels" ) ), -1 );
        aFilterData = aCfgItem.GetFilterData();
    }
    if( mxSrcDoc.is() && xOStm.is() )
    {
        PDFExport       aExport( mxSrcDoc, xStatusIndicator, xIH, mxMSF );
        ::utl::TempFile aTempFile;

        aTempFile.EnableKillingFile();
        bRet = aExport.Export( aTempFile.GetURL(), aFilterData );

        if( bRet )
        {
            SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );

            if( pIStm )
            {
                SvOutputStream aOStm( xOStm );

                aOStm << *pIStm;
                bRet = ( aOStm.Tell() && ( aOStm.GetError() == ERRCODE_NONE ) );

                delete pIStm;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------
class FocusWindowWaitCursor
{
    Window*         m_pFocusWindow;
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

    DECL_LINK( DestroyedLink, VclWindowEvent* );
};

IMPL_LINK( FocusWindowWaitCursor, DestroyedLink, VclWindowEvent*, pEvent )
{
    if( pEvent->GetId() == VCLEVENT_OBJECT_DYING )
        m_pFocusWindow = NULL;
    return 0;
}

sal_Bool SAL_CALL PDFFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    FocusWindowWaitCursor aCur;

    const sal_Bool bRet = implExport( rDescriptor );

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::cancel( ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString PDFFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.PDF.PDFFilter" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.document.PDFFilter"

sal_Bool SAL_CALL PDFFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName == SERVICE_NAME;
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFFilter_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PDFFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*) new PDFFilter( rSMgr );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL PDFFilter::getImplementationName()
    throw (RuntimeException)
{
    return PDFFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PDFFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return PDFFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL PDFFilter::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return PDFFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
