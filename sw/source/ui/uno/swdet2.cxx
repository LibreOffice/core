/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

