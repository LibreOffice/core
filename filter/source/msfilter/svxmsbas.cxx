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

#include <tools/debug.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <msvbasic.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <sot/storinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::awt;
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;

using rtl::OUString;

sal_Bool SvxImportMSVBasic::CopyStorage_Impl( const String& rStorageName,
                                         const String& rSubStorageName)
{
    sal_Bool bValidStg = sal_False;
    {
        SvStorageRef xVBAStg( xRoot->OpenSotStorage( rStorageName,
                                    STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
        if( xVBAStg.Is() && !xVBAStg->GetError() )
        {
            SvStorageRef xVBASubStg( xVBAStg->OpenSotStorage( rSubStorageName,
                                     STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
            if( xVBASubStg.Is() && !xVBASubStg->GetError() )
            {
                // then we will copy these storages into the (temporary) storage of the document
                bValidStg = sal_True;
            }
        }
    }

    if( bValidStg )
    {
        String aDstStgName( GetMSBasicStorageName() );
        SotStorageRef xDst = SotStorage::OpenOLEStorage( rDocSh.GetStorage(), aDstStgName, STREAM_READWRITE | STREAM_TRUNC );
        SotStorageRef xSrc = xRoot->OpenSotStorage( rStorageName, STREAM_STD_READ );

        // TODO/LATER: should we commit the storage?
        xSrc->CopyTo( xDst );
        xDst->Commit();
        ErrCode nError = xDst->GetError();
        if ( nError == ERRCODE_NONE )
            nError = xSrc->GetError();
        if ( nError != ERRCODE_NONE )
            xRoot->SetError( nError );
        else
            bValidStg = sal_True;
    }

    return bValidStg;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
