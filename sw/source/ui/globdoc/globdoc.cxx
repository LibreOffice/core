/*************************************************************************
 *
 *  $RCSfile: globdoc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:38 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif

#include "swtypes.hxx"


#include "shellio.hxx"
#include "globdoc.hxx"
#include "globdoc.hrc"

/*--------------------------------------------------------------------
    Beschreibung:   Alle Filter registrieren
 --------------------------------------------------------------------*/

// 4.0: {340AC970-E30D-11d0-A53F-00A0249D57B1}
// 4.0: 0x340ac970, 0xe30d, 0x11d0, 0xa5, 0x3f, 0x0, 0xa0, 0x24, 0x9d, 0x57, 0xb1
// 5.0: 0xc20cf9d3, 0x85ae, 0x11d1, 0xaa, 0xb4, 0x0, 0x60, 0x97, 0xda, 0x56, 0x1a
SFX_IMPL_OBJECTFACTORY_LOD(SwGlobalDocShell, SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, /*swriter4/GlobalDocument,*/ \
        SvGlobalName(SO3_SWGLOB_CLASSID) , Sw)

TYPEINIT1(SwGlobalDocShell, SwDocShell);

SwGlobalDocShell::SwGlobalDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode)
{
}

__EXPORT SwGlobalDocShell::~SwGlobalDocShell()
{
}

// Anforderung der pIo-Struktur fuer den Zugriff auf Substorages
// und Streams

void __EXPORT SwGlobalDocShell::FillRegInfo( SvEmbeddedRegistryInfo * pInfo)
{
    SfxInPlaceObject::FillRegInfo( pInfo );
    pInfo->aObjName = String::CreateFromAscii("StarWriterGlobalDocument");
    pInfo->nMajorVers = 5;
    pInfo->nMinorVers = 0;
    pInfo->aHumanShortTypeName = SW_RESSTR(STR_HUMAN_SWGLOBDOC_NAME);
    pInfo->nIconResId =
#ifdef REG_ICON_RESID_SW_GLOBDOC
    REG_ICON_RESID_SW_GLOBDOC;
#else
    9;
#endif
}

void __EXPORT SwGlobalDocShell::FillClass( SvGlobalName * pClassName,
                                   ULONG * pClipFormat,
                                   String * pAppName,
                                   String * pLongUserName,
                                   String * pUserName,
                                   long nVersion ) const
{
    SfxInPlaceObject::FillClass(pClassName, pClipFormat, pAppName, pLongUserName,
                                pUserName, nVersion);

    if (nVersion == SOFFICE_FILEFORMAT_40)
    {
        *pClassName = SvGlobalName( SO3_SWGLOB_CLASSID_40 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERGLOB_40;
        *pAppName = String::CreateFromAscii("StarWriter 4.0/GlobalDocument");

        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE_40);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_NOW)
    {
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE);
    }
    *pUserName = SW_RESSTR(STR_HUMAN_SWGLOBDOC_NAME);
}


/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.9  2000/09/18 16:05:35  willem.vandorp
    OpenOffice header added.

    Revision 1.8  2000/04/18 15:32:07  os
    UNICODE

    Revision 1.7  2000/02/11 14:47:35  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.6  1999/09/10 13:19:20  os
    Chg: resource types removed

    Revision 1.5  1998/11/18 14:16:34  JP
    Task #59398#: ClipboardFormatId Umstellungen


      Rev 1.4   18 Nov 1998 15:16:34   JP
   Task #59398#: ClipboardFormatId Umstellungen

      Rev 1.3   07 Jan 1998 18:38:32   MIB
   5.0 Fileformat

      Rev 1.2   28 Nov 1997 19:52:58   MA
   includes

      Rev 1.1   01 Sep 1997 13:11:20   OS
   DLL-Umstellung

      Rev 1.0   12 Jun 1997 16:14:44   MA
   Initial revision.

------------------------------------------------------------------------*/
