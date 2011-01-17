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
#include "precompiled_sd.hxx"

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <osl/module.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <svx/svxerr.hxx>
#include <unotools/fltrcfg.hxx>

#include "sdpptwrp.hxx"
#include "ppt/pptin.hxx"
#include "drawdoc.hxx"
#include <tools/urlobj.hxx>
#include <filter/msfilter/msfiltertracer.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_Bool ( __LOADONCALLAPI *ExportPPT )( SvStorageRef&,
                                             Reference< XModel > &,
                                             Reference< XStatusIndicator > &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( SAL_CALL *ImportPPT )( const ::rtl::OUString&, Sequence< PropertyValue >*,
                                          SdDrawDocument*, SvStream&, SvStorage&, SfxMedium& );

typedef sal_Bool ( __LOADONCALLAPI *SaveVBA )( SfxObjectShell&, SvMemoryStream*& );

// ---------------
// - SdPPTFilter -
// ---------------

SdPPTFilter::SdPPTFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress ),
    pBas    ( NULL )
{
}

// -----------------------------------------------------------------------------

SdPPTFilter::~SdPPTFilter()
{
    delete pBas;    // deleting the compressed basic storage
}

// -----------------------------------------------------------------------------

sal_Bool SdPPTFilter::Import()
{
    sal_Bool    bRet = sal_False;
    SotStorageRef pStorage = new SotStorage( mrMedium.GetInStream(), sal_False );
    if( !pStorage->GetError() )
    {
        /* check if there is a dualstorage, then the
        document is propably a PPT95 containing PPT97 */
        SvStorageRef xDualStorage;
        String sDualStorage( RTL_CONSTASCII_USTRINGPARAM( "PP97_DUALSTORAGE" ) );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, STREAM_STD_READ );
            pStorage = xDualStorage;
        }
        SvStream* pDocStream = pStorage->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM("PowerPoint Document") ), STREAM_STD_READ );
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetKey( pStorage->GetKey() );

            String aTraceConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Tracing/Import/PowerPoint" ) );
            Sequence< PropertyValue > aConfigData( 1 );
            PropertyValue aPropValue;
            aPropValue.Value <<= rtl::OUString( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
            aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentURL" ) );
            aConfigData[ 0 ] = aPropValue;

            if ( pStorage->IsStream( String( RTL_CONSTASCII_USTRINGPARAM("EncryptedSummary") ) ) )
                mrMedium.SetError( ERRCODE_SVX_READ_FILTER_PPOINT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            else
            {
                ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
                if ( pLibrary )
                {
                    ImportPPT PPTImport = reinterpret_cast< ImportPPT >( pLibrary->getFunctionSymbol( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImportPPT" ) ) ) );
                    if ( PPTImport )
                        bRet = PPTImport( aTraceConfigPath, &aConfigData, &mrDocument, *pDocStream, *pStorage, mrMedium );

                    if ( !bRet )
                        mrMedium.SetError( SVSTREAM_WRONGVERSION, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                }
            }

            delete pDocStream;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdPPTFilter::Export()
{
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
    sal_Bool        bRet = sal_False;

    if( pLibrary )
    {
        if( mxModel.is() )
        {
            SotStorageRef    xStorRef = new SotStorage( mrMedium.GetOutStream(), sal_False );
            ExportPPT       PPTExport = reinterpret_cast<ExportPPT>(pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii("ExportPPT") ));

            /* !!!
            if ( pViewShell && pViewShell->GetView() )
                pViewShell->GetView()->SdrEndTextEdit();
            */
            if( PPTExport && xStorRef.Is() )
            {
                sal_uInt32          nCnvrtFlags = 0;
                SvtFilterOptions* pFilterOptions = SvtFilterOptions::Get();
                if ( pFilterOptions )
                {
                    if ( pFilterOptions->IsMath2MathType() )
                        nCnvrtFlags |= OLE_STARMATH_2_MATHTYPE;
                    if ( pFilterOptions->IsWriter2WinWord() )
                        nCnvrtFlags |= OLE_STARWRITER_2_WINWORD;
                    if ( pFilterOptions->IsCalc2Excel() )
                        nCnvrtFlags |= OLE_STARCALC_2_EXCEL;
                    if ( pFilterOptions->IsImpress2PowerPoint() )
                        nCnvrtFlags |= OLE_STARIMPRESS_2_POWERPOINT;
                    if ( pFilterOptions->IsEnablePPTPreview() )
                        nCnvrtFlags |= 0x8000;
                }

                mrDocument.SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

                if( mbShowProgress )
                    CreateStatusIndicator();

                bRet = PPTExport( xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags );
                xStorRef->Commit();
            }
        }
        delete pLibrary;
    }
    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    SvtFilterOptions* pFilterOptions = SvtFilterOptions::Get();
    if( pFilterOptions && pFilterOptions->IsLoadPPointBasicStorage() )
    {
        ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
        if( pLibrary )
        {
            SaveVBA pSaveVBA= reinterpret_cast<SaveVBA>(pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii("SaveVBA") ));
            if( pSaveVBA )
            {
                pSaveVBA( (SfxObjectShell&) mrDocShell, pBas );
            }
        }
    }
}
