/*************************************************************************
 *
 *  $RCSfile: swdet2.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 15:20:28 $
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
#include <sot/clsids.hxx>

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

extern const char __FAR_DATA sHTML[];

//-------------------------------------------------------------------------

ULONG SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    ULONG nRet = ERRCODE_NONE;
    if( *ppFilter )
    {
        // verify the given filter
        String aPrefFlt = (*ppFilter)->GetUserData();

        // detection for TextFilter needs an additional checking
        BOOL bTxtFilter = aPrefFlt.EqualsAscii( FILTER_TEXT, 0, 4 );
        BOOL bDetected = SwIoSystem::IsFileFilter( rMedium, aPrefFlt );
        if (!bTxtFilter && bDetected )
            return nRet;
        else if (bTxtFilter)
        {
            // #50498#: SWDOS uses "txt" extension - so a file detected as "Text" could
            // also be an SWDOS file
            if (SwIoSystem::IsFileFilter( rMedium, C2S("SW6"), ppFilter))
                return nRet;
        }
        else if ( !bDetected )
            // mba: don't guess filters, only verify it
            return ERRCODE_ABORT;
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

