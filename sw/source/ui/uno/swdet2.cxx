/*************************************************************************
 *
 *  $RCSfile: swdet2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 11:40:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SWLIB_CXX

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif

#ifndef _SHELLIO_HXX //autogen
#include <shellio.hxx>
#endif

#undef _DLL_

#include <swdetect.hxx>
#include "iodetect.cxx"

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>

#include <svtools/moduleoptions.hxx>

#define C2S(cChar) String::CreateFromAscii(cChar)

USHORT AutoDetec( const String& FileName, USHORT & rVersion );

bool IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

extern char __FAR_DATA sHTML[];

//-------------------------------------------------------------------------

ULONG SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    SfxFilterFlags nMust = SFX_FILTER_IMPORT;
    SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;
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
//          else if( aPrefFlt == C2S(sHTML) )
//          {
//              nRet = ERRCODE_NONE;
//              break;
//          }
        }

        const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(),
                                                            aPrefFlt, &rMedium );
        if( !pTmp )
            nRet = ERRCODE_ABORT;


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
        }
        // sollte der voreingestellte Filter ASCII sein und wir haben
        // ASCII erkannt, dann ist das ein gultiger Filter, ansonsten ist das
        // ein Fehler und wir wollen die Filterbox sehen
        else if( pTmp->GetUserData().EqualsAscii( FILTER_TEXT ))
        {
            // Bug 28974: "Text" erkannt, aber "Text Dos" "Text ..." eingestellt
            //  -> keine FilterBox, sondern den eingestellten Filter benutzen
            if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ))
                ;
            else
//          if( !*ppFilter || COMPARE_EQUAL != pTmp->GetUserData().Compare(
//              (*ppFilter)->GetUserData(), 4 ))
            {
//              nRet = ERRCODE_ABORT;
                *ppFilter = pTmp;
            }
        }
        else
        {
            //Bug 41417: JP 09.07.97: HTML auf die WebDocShell defaulten
            if( pTmp->GetUserData() != C2S(sHTML) ||
                String::CreateFromAscii( "com.sun.star.text.WebDocument" ) ==
                String( pTmp->GetServiceName() ) ||
                0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( C2S(sHTML),
                     &SfxFilterContainer(String::CreateFromAscii("swriter/web") ) ) ) )
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

ULONG SwFilterDetect::GlobDetectFilter( SfxMedium& rMedium, const SfxFilter **ppFilter )
{
    SfxFilterFlags nMust = SFX_FILTER_IMPORT;
    SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;
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
            const SfxFilterContainer aFltContainer( String::CreateFromAscii("swriter/global") );
            USHORT nFltrCount = aFltContainer.GetFilterCount();
            for( USHORT nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( (sal_Unicode)'C' == ( pFltr = aFltContainer.GetFilter( nCnt ))->GetUserData().GetChar(0) &&
                    aStg.Is() && SwIoSystem::IsValidStgFilter( *aStg, *pFltr ) )
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
