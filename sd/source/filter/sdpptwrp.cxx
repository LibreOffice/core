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

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_Bool ( __LOADONCALLAPI *ExportPPTPointer )( const std::vector< com::sun::star::beans::PropertyValue >&, SvStorageRef&,
                                             Reference< XModel > &,
                                             Reference< XStatusIndicator > &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( SAL_CALL *ImportPPTPointer )( SdDrawDocument*, SvStream&, SvStorage&, SfxMedium& );

typedef sal_Bool ( __LOADONCALLAPI *SaveVBAPointer )( SfxObjectShell&, SvMemoryStream*& );

#ifdef DISABLE_DYNLOADING

extern "C" sal_Bool ExportPPT( const std::vector< com::sun::star::beans::PropertyValue >&, SvStorageRef&,
                               Reference< XModel > &,
                               Reference< XStatusIndicator > &,
                               SvMemoryStream*, sal_uInt32 nCnvrtFlags );

extern "C" sal_Bool ImportPPT( SdDrawDocument*, SvStream&, SvStorage&, SfxMedium& );

extern "C" sal_Bool SaveVBA( SfxObjectShell&, SvMemoryStream*& );

#endif

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
        String sDualStorage( "PP97_DUALSTORAGE"  );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, STREAM_STD_READ );
            pStorage = xDualStorage;
        }
        SvStream* pDocStream = pStorage->OpenSotStream( rtl::OUString("PowerPoint Document") , STREAM_STD_READ );
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetCryptMaskKey(pStorage->GetKey());

            if ( pStorage->IsStream( rtl::OUString("EncryptedSummary" ) ) )
                mrMedium.SetError( ERRCODE_SVX_READ_FILTER_PPOINT, OSL_LOG_PREFIX );
            else
            {
#ifndef DISABLE_DYNLOADING
                ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
                if ( pLibrary )
                {
                    ImportPPTPointer PPTImport = reinterpret_cast< ImportPPT >( pLibrary->getFunctionSymbol( "ImportPPT" ) );
                    if ( PPTImport )
                        bRet = PPTImport( &mrDocument, *pDocStream, *pStorage, mrMedium );

                    if ( !bRet )
                        mrMedium.SetError( SVSTREAM_WRONGVERSION, OSL_LOG_PREFIX );
                }
#else
                bRet = ImportPPT( &mrDocument, *pDocStream, *pStorage, mrMedium );
                if ( !bRet )
                    mrMedium.SetError( SVSTREAM_WRONGVERSION, OSL_LOG_PREFIX );
#endif
            }

            delete pDocStream;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdPPTFilter::Export()
{
#ifndef DISABLE_DYNLOADING
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
#endif
    sal_Bool        bRet = sal_False;

#ifndef DISABLE_DYNLOADING
    if( pLibrary )
#endif
    {
        if( mxModel.is() )
        {
            SotStorageRef    xStorRef = new SotStorage( mrMedium.GetOutStream(), sal_False );
#ifndef DISABLE_DYNLOADING
            ExportPPTPointer PPTExport = reinterpret_cast<ExportPPT>(pLibrary->getFunctionSymbol( "ExportPPT" ));
#else
            ExportPPTPointer PPTExport = ExportPPT;
#endif

            if( PPTExport && xStorRef.Is() )
            {
                sal_uInt32          nCnvrtFlags = 0;
                const SvtFilterOptions& rFilterOptions = SvtFilterOptions::Get();
                if ( rFilterOptions.IsMath2MathType() )
                    nCnvrtFlags |= OLE_STARMATH_2_MATHTYPE;
                if ( rFilterOptions.IsWriter2WinWord() )
                    nCnvrtFlags |= OLE_STARWRITER_2_WINWORD;
                if ( rFilterOptions.IsCalc2Excel() )
                    nCnvrtFlags |= OLE_STARCALC_2_EXCEL;
                if ( rFilterOptions.IsImpress2PowerPoint() )
                    nCnvrtFlags |= OLE_STARIMPRESS_2_POWERPOINT;
                if ( rFilterOptions.IsEnablePPTPreview() )
                    nCnvrtFlags |= 0x8000;

                mrDocument.SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

                if( mbShowProgress )
                    CreateStatusIndicator();

                //rtl::OUString sBaseURI( "BaseURI");
                std::vector< PropertyValue > aProperties;
                PropertyValue aProperty;
                aProperty.Name = "BaseURI";
                aProperty.Value = makeAny( mrMedium.GetBaseURL( true ) );
                aProperties.push_back( aProperty );
                
                bRet = PPTExport( aProperties, xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags );
                xStorRef->Commit();
            }
        }
#ifndef DISABLE_DYNLOADING
        delete pLibrary;
#endif
    }
    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    const SvtFilterOptions& rFilterOptions = SvtFilterOptions::Get();
    if( rFilterOptions.IsLoadPPointBasicStorage() )
    {
#ifndef DISABLE_DYNLOADING
        ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
        if( pLibrary )
        {
            SaveVBAPointer pSaveVBA= reinterpret_cast<SaveVBA>(pLibrary->getFunctionSymbol( "SaveVBA" ));
            if( pSaveVBA )
            {
                pSaveVBA( (SfxObjectShell&) mrDocShell, pBas );
            }
        }
#else
        SaveVBA( (SfxObjectShell&) mrDocShell, pBas );
#endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
