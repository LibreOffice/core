/*************************************************************************
 *
 *  $RCSfile: globdoc.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:11:19 $
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


#pragma hdrstop

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif

#include <sot/clsids.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/moduleoptions.hxx>

#include "swtypes.hxx"
#include "shellio.hxx"
#include "globdoc.hxx"
#include "globdoc.hrc"
#include "cfgid.h"

#define C2S(cChar) UniString::CreateFromAscii(cChar)

/*--------------------------------------------------------------------
    Beschreibung:   Alle Filter registrieren
 --------------------------------------------------------------------*/

TYPEINIT1(SwGlobalDocShell, SwDocShell);

//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY(SwGlobalDocShell, SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, swriter/GlobalDocument, SvGlobalName(SO3_SWGLOB_CLASSID) )
{
    SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
    rFactory.SetDocumentServiceName(C2S("com.sun.star.text.GlobalDocument"));
    //rFactory.GetFilterContainer()->SetDetectFilter( &SwDLL::GlobDetectFilter );
    SwGlobalDocShell::Factory().RegisterMenuBar(SW_RES(CFG_SWGLOBAL_MENU));
    SwGlobalDocShell::Factory().RegisterAccel(SW_RES(CFG_SW_ACCEL));
    if ( SvtModuleOptions().IsWriter() )
    {
        SwGlobalDocShell::Factory().RegisterHelpFile(String::CreateFromAscii("swriter.svh"));
        //SwGlobalDocShell::Factory().RegisterHelpPIFile(String::CreateFromAscii("swriter.svh"));
    }
}

SwGlobalDocShell::SwGlobalDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode)
{
}

SwGlobalDocShell::~SwGlobalDocShell()
{
}

void SwGlobalDocShell::FillClass( SvGlobalName * pClassName,
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
    else if (nVersion == SOFFICE_FILEFORMAT_50)
    {
        *pClassName = SvGlobalName( SO3_SWGLOB_CLASSID_50 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERGLOB_50;
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE_50);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName = SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERGLOB_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITERGLOB_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE);
    }

    *pUserName = SW_RESSTR(STR_HUMAN_SWGLOBDOC_NAME);
}



