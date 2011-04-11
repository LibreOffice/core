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
#include "precompiled_sw.hxx"

#define _SWLIB_CXX
#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/parhtml.hxx>
#include <sot/clsids.hxx>
#include <iodetect.hxx>

#include <swdetect.hxx>

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>

#include <unotools/moduleoptions.hxx>

#include <unomid.h>

bool IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

//-------------------------------------------------------------------------

sal_uLong SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    sal_uLong nRet = ERRCODE_NONE;
    if( *ppFilter )
    {
        // verify the given filter
        String aPrefFlt = (*ppFilter)->GetUserData();

        // detection for TextFilter needs an additional checking
        sal_Bool bDetected = SwIoSystem::IsFileFilter( rMedium, aPrefFlt );
        return bDetected ? nRet : ERRCODE_ABORT;
    }

    // mba: without preselection there is no PrefFlt
    String aPrefFlt;
    const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(), aPrefFlt, &rMedium );
    if( !pTmp )
        return ERRCODE_ABORT;

    else
    {
        //Bug 41417: JP 09.07.97: HTML documents should be loaded by WebWriter
        SfxFilterContainer aFilterContainer( String::CreateFromAscii("swriter/web") );
        if( pTmp->GetUserData() != C2S(sHTML) ||
            String::CreateFromAscii( "com.sun.star.text.WebDocument" ) ==
            String( pTmp->GetServiceName() ) ||
            0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( C2S(sHTML),
                    &aFilterContainer ) ) )
            *ppFilter = pTmp;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
