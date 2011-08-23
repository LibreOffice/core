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

#include <basic/basmgr.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/svxmsbas.hxx>

using namespace com::sun::star;

ULONG SvxImportMSVBasic::SaveOrDelMSVBAStorage( BOOL bSaveInto,
                                                const String& rStorageName )
{
    ULONG nRet = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xSrcRoot( rDocSh.GetStorage() );
    String aDstStgName( GetMSBasicStorageName() );
    SotStorageRef xVBAStg( SotStorage::OpenOLEStorage( xSrcRoot, aDstStgName,
                                STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYALL ) );
    if( xVBAStg.Is() && !xVBAStg->GetError() )
    {
        xVBAStg = 0;
        if( bSaveInto )
        {
            BasicManager *pBasicMan = rDocSh.GetBasicManager();
            if( pBasicMan && pBasicMan->IsBasicModified() )
                nRet = ERRCODE_SVX_MODIFIED_VBASIC_STORAGE;

            SotStorageRef xSrc = SotStorage::OpenOLEStorage( xSrcRoot, aDstStgName, STREAM_STD_READ );
            SotStorageRef xDst = xRoot->OpenSotStorage( rStorageName, STREAM_READWRITE | STREAM_TRUNC );
            xSrc->CopyTo( xDst );
            xDst->Commit();
            ErrCode nError = xDst->GetError();
            if ( nError == ERRCODE_NONE )
                nError = xSrc->GetError();
            if ( nError != ERRCODE_NONE )
                xRoot->SetError( nError );
        }
    }

    return nRet;
}

// check if the MS-VBA-Storage exists in the RootStorage of the DocShell.
// If it exists, then return the WarningId for losing the information.
ULONG SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocSh)
{
    uno::Reference < embed::XStorage > xSrcRoot( rDocSh.GetStorage() );
    SvStorageRef xVBAStg( SotStorage::OpenOLEStorage( xSrcRoot, GetMSBasicStorageName(),
                    STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL ));
    return ( xVBAStg.Is() && !xVBAStg->GetError() )
                    ? ERRCODE_SVX_VBASIC_STORAGE_EXIST
                    : ERRCODE_NONE;
}

String SvxImportMSVBasic::GetMSBasicStorageName()
{
    return String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Macros" ) );
}
