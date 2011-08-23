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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

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
    sal_Int32					nLength = rDescriptor.getLength();
    const PropertyValue*		pValue = rDescriptor.getConstArray();
    sal_Bool                    bRet = sal_False;
    Reference< task::XStatusIndicator > xStatusIndicator;

    for ( sal_Int32 i = 0 ; ( i < nLength ) && !xOStm.is(); ++i)
    {
        if( pValue[ i ].Name.equalsAscii( "OutputStream" ) )
            pValue[ i ].Value >>= xOStm;
        else if( pValue[ i ].Name.equalsAscii( "FilterData" ) )
            pValue[ i ].Value >>= aFilterData;
        else if ( pValue[ i ].Name.equalsAscii( "StatusIndicator" ) )
            pValue[ i ].Value >>= xStatusIndicator;
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
        aCfgItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "OpenBookmarkLevels" ) ), -1 );
        aFilterData = aCfgItem.GetFilterData();
    }
    if( mxSrcDoc.is() && xOStm.is() )
    {
        PDFExport       aExport( mxSrcDoc, xStatusIndicator, mxMSF );
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
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
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
