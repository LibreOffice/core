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



#define _SWLIB_CXX

#include <bf_sfx2/fcontnr.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <comphelper/classids.hxx>

#include <wdocsh.hxx>
#include <globdoc.hxx>
#include <shellio.hxx>

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>
namespace binfilter {

#define C2S(cChar) String::CreateFromAscii(cChar)

extern char __FAR_DATA sHTML[];

SwDLL::SwDLL()
{
}

SwDLL::~SwDLL()
{
}

//-------------------------------------------------------------------------
void 	SwDLL::Init()
{
}
//-------------------------------------------------------------------------
void 	SwDLL::Exit()
{
}

//-------------------------------------------------------------------------

ULONG	SwDLL::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                            SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    ULONG nRet = ERRCODE_NONE;
    const SfxFilter* pSavFilter = *ppFilter;

    do {
            // dann ueberpruefe mal ob der richtige ausgewaehlt wurde
        if( *ppFilter )
        {
            const String& rUData = (*ppFilter)->GetUserData();

            BOOL bTxtFilter = rUData.EqualsAscii( FILTER_TEXT, 0, 4 );

            if (SwIoSystem::IsFileFilter( rMedium, rUData ) && !bTxtFilter)
                break;

            //JP 08.06.98: Bugfix 50498
            if (bTxtFilter)
            {
                //JP 09.11.98: der SWDOS - Filter hat dieselbe Extension und
                // wird vom SFX vorgeschlagen. Das es auch eine Textdatei ist,
                // muss die hier ausgefilter werden!
                if (SwIoSystem::IsFileFilter( rMedium, C2S("SW6"), ppFilter))
                    break;
            }
        }

        if( SFX_FILTER_TEMPLATE & nMust )
        {
            // nur einen Vorlagen Filter
            BOOL bStorage = rMedium.IsStorage();
            if( bStorage && *ppFilter )
                break;
            else if( bStorage &&
                ( SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_XMLV), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW5V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW4V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW3V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_XMLVW), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWW5V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWW4V), ppFilter ) ))
                break;
            else if( !bStorage &&
                SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWGV), ppFilter ) )
                break;

            nRet = ERRCODE_ABORT;
            break;
        }


        String aPrefFlt;
        if( *ppFilter )
        {
            aPrefFlt = (*ppFilter)->GetUserData();
            if( SwIoSystem::IsFileFilter( rMedium, aPrefFlt ) )
            {
                nRet = ERRCODE_NONE;
                break;
            }

            // beim Browsen soll keine Filterbox kommen, wenn das Dokument nicht
            // in den ersten paar Bytes HTML-Tags hat (MA/ST/...). Solche Dok.
            // erzeugen z.B. SearchEngines
//JP 20.07.00: from now on we are not a browser
//			else if( aPrefFlt == C2S(sHTML) )
//			{
//				nRet = ERRCODE_NONE;
//				break;
//			}
        }

        const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(),
                                                            aPrefFlt, &rMedium );
        if( !pTmp )
            nRet = ERRCODE_ABORT;


        else if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( "W4W", 0, 3 )
                    && pTmp->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ) )
        {
            // Bug 95262 - if the user (or short  detect) select a
            //				Word 4 Word filter, but the autodect of mastersoft
            //				can't detect it, we normally return the ascii filter
            // 				But the user may have a change to use the W4W filter,
            //				so the SFX must show now a dialog with the 2 filters
            nRet = ERRCODE_SFX_CONSULTUSER;
            *ppFilter = pTmp;
        }
        // sollte der voreingestellte Filter ASCII sein und wir haben
        // ASCII erkannt, dann ist das ein gultiger Filter, ansonsten ist das
        // ein Fehler und wir wollen die Filterbox sehen
        else if( pTmp->GetUserData().EqualsAscii( FILTER_TEXT ))
        {
            // Bug 28974: "Text" erkannt, aber "Text Dos" "Text ..." eingestellt
            // 	-> keine FilterBox, sondern den eingestellten Filter benutzen
            if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ))
                ;
            else
//			if( !*ppFilter || COMPARE_EQUAL != pTmp->GetUserData().Compare(
//				(*ppFilter)->GetUserData(), 4 ))
            {
//				nRet = ERRCODE_ABORT;
                *ppFilter = pTmp;
            }
        }
        else
        {
            //Bug 41417: JP 09.07.97: HTML auf die WebDocShell defaulten
            if( pTmp->GetUserData() != C2S(sHTML) ||
                SwWebDocShell::Factory().GetFilterContainer() ==
                pTmp->GetFilterContainer() ||
                0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( C2S(sHTML),
                     SwWebDocShell::Factory().GetFilterContainer() ) ))

                *ppFilter = pTmp;
        }

    } while( FALSE );

    if( ERRCODE_NONE == nRet && (
        nMust != ( (*ppFilter)->GetFilterFlags() & nMust ) ||
        0 != ( (*ppFilter)->GetFilterFlags() & nDont )) )
    {
        nRet = ERRCODE_ABORT;
        *ppFilter = pSavFilter;
    }
    return nRet;
}

//-------------------------------------------------------------------------

ULONG SwDLL::GlobDetectFilter( SfxMedium& rMedium, const SfxFilter **ppFilter,
                                ULONG nMust, ULONG nDont )
{
    ULONG nRet = ERRCODE_ABORT;
    const SfxFilter* pSavFilter = *ppFilter;
    do {
            // dann ueberpruefe mal ob der richtige ausgewaehlt wurde
        if( rMedium.IsStorage() )
        {
            SvStorageRef aStg = rMedium.GetStorage();

            if( *ppFilter &&
                aStg.Is() && SVSTREAM_OK == aStg->GetError() &&
                SwIoSystem::IsValidStgFilter( *aStg, **ppFilter ))
            {
                nRet = ERRCODE_NONE;
                break;
            }

            if( SFX_FILTER_TEMPLATE & nMust )
                break;

            const SfxFilter* pFltr;
            const SfxFactoryFilterContainer& rFltContainer =
                            *SwGlobalDocShell::Factory().GetFilterContainer();
            USHORT nFltrCount = rFltContainer.GetFilterCount();
            for( USHORT nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( (sal_Unicode)'C' == ( pFltr = rFltContainer.GetFilter( nCnt ))->GetUserData().GetChar(0) &&
                    aStg.Is() && SwIoSystem::IsValidStgFilter( *aStg, *pFltr ))
                {
                    *ppFilter = pFltr;
                    nRet = ERRCODE_NONE;
                    break;
                }
        }

    } while( FALSE );

    if( ERRCODE_NONE == nRet && (
        nMust != ( (*ppFilter)->GetFilterFlags() & nMust ) ||
        0 != ( (*ppFilter)->GetFilterFlags() & nDont )) )
    {
        nRet = ERRCODE_ABORT;
        *ppFilter = pSavFilter;
    }

    return nRet;
}

/*-----------------18.03.98 08.09-------------------

--------------------------------------------------*/
SwModuleDummy::~SwModuleDummy()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
