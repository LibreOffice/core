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

#include <config_features.h>

#include <basic/basmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/svxmsbas.hxx>

using namespace com::sun::star;

ErrCode SvxImportMSVBasic::SaveOrDelMSVBAStorage( bool bSaveInto,
                                                const OUString& rStorageName )
{
    ErrCode nRet = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xSrcRoot( rDocSh.GetStorage() );
    OUString aDstStgName( GetMSBasicStorageName() );
    tools::SvRef<SotStorage> xVBAStg( SotStorage::OpenOLEStorage( xSrcRoot, aDstStgName,
                                StreamMode::READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYALL ) );
    if( xVBAStg.is() && !xVBAStg->GetError() )
    {
        xVBAStg = nullptr;
        if( bSaveInto )
        {
#if HAVE_FEATURE_SCRIPTING
            BasicManager *pBasicMan = rDocSh.GetBasicManager();
            if( pBasicMan && pBasicMan->IsBasicModified() )
                nRet = ERRCODE_SVX_MODIFIED_VBASIC_STORAGE;
#endif
            tools::SvRef<SotStorage> xSrc = SotStorage::OpenOLEStorage( xSrcRoot, aDstStgName, StreamMode::STD_READ );
            tools::SvRef<SotStorage> xDst = xRoot->OpenSotStorage( rStorageName, StreamMode::READWRITE | StreamMode::TRUNC );
            xSrc->CopyTo( xDst.get() );
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
ErrCode SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocSh)
{
    uno::Reference < embed::XStorage > xSrcRoot( rDocSh.GetStorage() );
    tools::SvRef<SotStorage> xVBAStg( SotStorage::OpenOLEStorage( xSrcRoot, GetMSBasicStorageName(),
                    StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYALL ));
    return ( xVBAStg.is() && !xVBAStg->GetError() )
                    ? ERRCODE_SVX_VBASIC_STORAGE_EXIST
                    : ERRCODE_NONE;
}

OUString SvxImportMSVBasic::GetMSBasicStorageName()
{
    return OUString( "_MS_VBA_Macros" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
