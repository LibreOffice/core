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
    /*
    else if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( "W4W", 0, 3 )
                && pTmp->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ) )
    {
        // Bug 95262 - if the user (or short  detect) select a
        //              Word 4 Word filter, but the autodect of mastersoft
        //              can't detect it, we normally return the ascii filter
        //              But the user may have a change to use the W4W filter,
        //              so the SFX must show now a dialog with the 2 filters
        nRet = ERRCODE_SFX_CONSULTUSER;
        *ppFilter = pTmp;
    } */

    // sollte der voreingestellte Filter ASCII sein und wir haben
    // ASCII erkannt, dann ist das ein gultiger Filter, ansonsten ist das
    // ein Fehler und wir wollen die Filterbox sehen
    /*
    else if( pTmp->GetUserData().EqualsAscii( FILTER_TEXT ) )
    {
        // Bug 28974: "Text" erkannt, aber "Text Dos" "Text ..." eingestellt
        //  -> keine FilterBox, sondern den eingestellten Filter benutzen
        if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ) )
            ;
        else
//          if( !*ppFilter || COMPARE_EQUAL != pTmp->GetUserData().Compare((*ppFilter)->GetUserData(), 4 ))
        {
//              nRet = ERRCODE_ABORT;
            *ppFilter = pTmp;
        }
    } */
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

