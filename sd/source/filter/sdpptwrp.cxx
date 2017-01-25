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
#include <sfx2/objsh.hxx>
#include <osl/module.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <svx/svxerr.hxx>
#include <unotools/fltrcfg.hxx>

#include "sdpptwrp.hxx"
#include "ppt/pptin.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include <tools/urlobj.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_Bool ( SAL_CALL *ExportPPTPointer )( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage>&,
                                             Reference< XModel > &,
                                             Reference< XStatusIndicator > &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( SAL_CALL *ImportPPTPointer )( SdDrawDocument*, SvStream&, SotStorage&, SfxMedium& );

typedef sal_Bool ( SAL_CALL *SaveVBAPointer )( SfxObjectShell&, SvMemoryStream*& );

#ifdef DISABLE_DYNLOADING

extern "C" sal_Bool ExportPPT( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage>&,
                               Reference< XModel > &,
                               Reference< XStatusIndicator > &,
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
    bool    bRet = false;
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
        SvStream* pDocStream = pStorage->OpenSotStream( "PowerPoint Document" , StreamMode::STD_READ );
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetCryptMaskKey(pStorage->GetKey());

            if ( pStorage->IsStream( "EncryptedSummary" ) )
                mrMedium.SetError( ERRCODE_SVX_READ_FILTER_PPOINT, OSL_LOG_PREFIX );
            else
            {
#ifndef DISABLE_DYNLOADING
                ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
                if ( pLibrary )
                {
                    ImportPPTPointer PPTImport = reinterpret_cast< ImportPPTPointer >( pLibrary->getFunctionSymbol( "ImportPPT" ) );
                    if ( PPTImport )
                        bRet = PPTImport( &mrDocument, *pDocStream, *pStorage, mrMedium );

                    if ( !bRet )
                        mrMedium.SetError( SVSTREAM_WRONGVERSION, OSL_LOG_PREFIX );
                    pLibrary->release(); //TODO: let it get unloaded?
                    delete pLibrary;
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

bool SdPPTFilter::Export()
{
#ifndef DISABLE_DYNLOADING
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
#endif
    bool        bRet = false;

#ifndef DISABLE_DYNLOADING
    if( pLibrary )
#endif
    {
        if( mxModel.is() )
        {
            tools::SvRef<SotStorage>    xStorRef = new SotStorage( mrMedium.GetOutStream(), false );
#ifndef DISABLE_DYNLOADING
            ExportPPTPointer PPTExport = reinterpret_cast<ExportPPTPointer>(pLibrary->getFunctionSymbol( "ExportPPT" ));
#else
            ExportPPTPointer PPTExport = ExportPPT;
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

                mrDocument.SetSwapGraphicsMode( SdrSwapGraphicsMode::TEMP );

                CreateStatusIndicator();

                //OUString sBaseURI( "BaseURI");
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
            SaveVBAPointer pSaveVBA= reinterpret_cast<SaveVBAPointer>(pLibrary->getFunctionSymbol( "SaveVBA" ));
            if( pSaveVBA )
            {
                pSaveVBA( static_cast<SfxObjectShell&>(mrDocShell), pBas );
            }
            delete pLibrary;
        }
#else
        SaveVBA( (SfxObjectShell&) mrDocShell, pBas );
#endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
