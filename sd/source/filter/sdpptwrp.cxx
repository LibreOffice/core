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

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <svx/svxerr.hxx>
#include <unotools/fltrcfg.hxx>
#include <sot/storage.hxx>

#include <sdpptwrp.hxx>
#include <DrawDocShell.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_Bool ( *ExportPPTPointer )( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage> const&,
                                             Reference< XModel > const &,
                                             Reference< XStatusIndicator > const &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( *ImportPPTPointer )( SdDrawDocument*, SvStream&, SotStorage&, SfxMedium& );

typedef sal_Bool ( *SaveVBAPointer )( SfxObjectShell&, SvMemoryStream*& );

#ifdef DISABLE_DYNLOADING

extern "C" sal_Bool ExportPPT( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage> const&,
                               Reference< XModel > const &,
                               Reference< XStatusIndicator > const &,
                               SvMemoryStream*, sal_uInt32 nCnvrtFlags );

extern "C" sal_Bool ImportPPT( SdDrawDocument*, SvStream&, SotStorage&, SfxMedium& );

extern "C" sal_Bool SaveVBA( SfxObjectShell&, SvMemoryStream*& );

#endif


SdPPTFilter::SdPPTFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell ),
    pBas    ( nullptr )
{
}

SdPPTFilter::~SdPPTFilter()
{
    delete pBas;    // deleting the compressed basic storage
}

bool SdPPTFilter::Import()
{
    bool bRet = false;
    tools::SvRef<SotStorage> pStorage = new SotStorage( mrMedium.GetInStream(), false );
    if( !pStorage->GetError() )
    {
        /* check if there is a dualstorage, then the
        document is probably a PPT95 containing PPT97 */
        tools::SvRef<SotStorage> xDualStorage;
        OUString sDualStorage( "PP97_DUALSTORAGE"  );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, StreamMode::STD_READ );
            pStorage = xDualStorage;
        }
        std::unique_ptr<SvStream> pDocStream(pStorage->OpenSotStream( "PowerPoint Document" , StreamMode::STD_READ ));
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetCryptMaskKey(pStorage->GetKey());

            if ( pStorage->IsStream( "EncryptedSummary" ) )
                mrMedium.SetError(ERRCODE_SVX_READ_FILTER_PPOINT);
            else
            {
#ifdef DISABLE_DYNLOADING
                ImportPPTPointer pPPTImport = ImportPPT;
#else
                ImportPPTPointer pPPTImport = reinterpret_cast< ImportPPTPointer >(
                    SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "ImportPPT"));
#endif

                if ( pPPTImport )
                    bRet = pPPTImport( &mrDocument, *pDocStream, *pStorage, mrMedium );

                if ( !bRet )
                    mrMedium.SetError(SVSTREAM_WRONGVERSION);
            }
        }
    }

    return bRet;
}

bool SdPPTFilter::Export()
{
    bool bRet = false;

    if( mxModel.is() )
    {
        tools::SvRef<SotStorage> xStorRef = new SotStorage( mrMedium.GetOutStream(), false );

#ifdef DISABLE_DYNLOADING
        ExportPPTPointer PPTExport = ExportPPT;
#else
        ExportPPTPointer PPTExport = reinterpret_cast< ExportPPTPointer >(
            SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "ExportPPT"));
#endif

        if( PPTExport && xStorRef.is() )
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

            CreateStatusIndicator();

            //OUString sBaseURI( "BaseURI");
            std::vector< PropertyValue > aProperties;
            PropertyValue aProperty;
            aProperty.Name = "BaseURI";
            aProperty.Value <<= mrMedium.GetBaseURL( true );
            aProperties.push_back( aProperty );

            bRet = PPTExport( aProperties, xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags );
            xStorRef->Commit();
        }
    }

    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    const SvtFilterOptions& rFilterOptions = SvtFilterOptions::Get();
    if( rFilterOptions.IsLoadPPointBasicStorage() )
    {
#ifdef DISABLE_DYNLOADING
        SaveVBAPointer pSaveVBA= SaveVBA;
#else
        SaveVBAPointer pSaveVBA = reinterpret_cast< SaveVBAPointer >(
            SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "SaveVBA"));
#endif
        if( pSaveVBA )
            pSaveVBA( static_cast<SfxObjectShell&>(mrDocShell), pBas );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
