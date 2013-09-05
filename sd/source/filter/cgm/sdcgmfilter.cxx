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


#include <osl/module.hxx>
#include <tools/urlobj.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdcgmfilter.hxx"

#define CGM_IMPORT_CGM      0x00000001

#define CGM_EXPORT_IMPRESS  0x00000100

#define CGM_BIG_ENDIAN      0x00020000

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_uInt32 ( SAL_CALL *ImportCGMPointer )( OUString&, Reference< XModel >&, sal_uInt32, Reference< XStatusIndicator >& );
typedef sal_Bool ( SAL_CALL *ExportCGMPointer )( OUString&, Reference< XModel >&, Reference< XStatusIndicator >&, void* );

#ifdef DISABLE_DYNLOADING

extern "C" sal_uInt32 ImportCGM( OUString&, Reference< XModel >&, sal_uInt32, Reference< XStatusIndicator >& );

#endif

SdCGMFilter::SdCGMFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

SdCGMFilter::~SdCGMFilter()
{
}

sal_Bool SdCGMFilter::Import()
{
#ifndef DISABLE_DYNLOADING
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
#endif
    sal_Bool        bRet = sal_False;

    if(
#ifndef DISABLE_DYNLOADING
       pLibrary &&
#endif
       mxModel.is() )
    {
#ifndef DISABLE_DYNLOADING
        ImportCGMPointer FncImportCGM = reinterpret_cast< ImportCGMPointer >( pLibrary->getFunctionSymbol(  "ImportCGM" ) );
#else
        ImportCGMPointer FncImportCGM = ImportCGM;
#endif
        OUString aFileURL( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
        sal_uInt32          nRetValue;

        if( mrDocument.GetPageCount() == 0L )
            mrDocument.CreateFirstPages();

        CreateStatusIndicator();
        nRetValue = FncImportCGM( aFileURL, mxModel, CGM_IMPORT_CGM | CGM_BIG_ENDIAN | CGM_EXPORT_IMPRESS, mxStatusIndicator );

        if( nRetValue )
        {
            bRet = sal_True;

            if( ( nRetValue &~0xff000000 ) != 0xffffff )    // maybe the backgroundcolor is already white
            {                                               // so we must not set a master page
                mrDocument.StopWorkStartupDelay();
                SdPage* pSdPage = mrDocument.GetMasterSdPage(0, PK_STANDARD);

                if(pSdPage)
                {
                    // set PageFill to given color
                    const Color aColor((sal_uInt8)(nRetValue >> 16), (sal_uInt8)(nRetValue >> 8), (sal_uInt8)(nRetValue >> 16));
                    pSdPage->getSdrPageProperties().PutItem(XFillColorItem(OUString(), aColor));
                    pSdPage->getSdrPageProperties().PutItem(XFillStyleItem(XFILL_SOLID));
                }
            }
        }
    }
#ifndef DISABLE_DYNLOADING
    delete pLibrary;
#endif
    return bRet;
}

sal_Bool SdCGMFilter::Export()
{
#ifdef DISABLE_DYNLOADING
    // No ExportCGM function exists(!)
    return sal_False;
#else
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
    sal_Bool        bRet = sal_False;

    if( pLibrary && mxModel.is() )
    {
        ExportCGMPointer FncCGMExport = reinterpret_cast< ExportCGMPointer >( pLibrary->getFunctionSymbol( "ExportCGM" ) );

        if( FncCGMExport )
        {
            OUString aPhysicalName( mrMedium.GetPhysicalName() );

            CreateStatusIndicator();
            bRet = FncCGMExport( aPhysicalName, mxModel, mxStatusIndicator, NULL );
        }
    }

    delete pLibrary;
    return bRet;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
