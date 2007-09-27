/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: globdoc.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:53:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <sot/clsids.hxx>
#include <svtools/moduleoptions.hxx>

#include <unomid.h>

#include "swtypes.hxx"
#include "shellio.hxx"
#include "globdoc.hxx"
#include "globdoc.hrc"
#include "cfgid.h"


/*--------------------------------------------------------------------
    Beschreibung:   Alle Filter registrieren
 --------------------------------------------------------------------*/

TYPEINIT1(SwGlobalDocShell, SwDocShell);

//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY( SwGlobalDocShell, SvGlobalName(SO3_SWGLOB_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter/GlobalDocument" )

SwGlobalDocShell::SwGlobalDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode)
{
}

SwGlobalDocShell::~SwGlobalDocShell()
{
}

void SwGlobalDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   String * /*pAppName*/,
                                   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nVersion ) const
{
    if (nVersion == SOFFICE_FILEFORMAT_60)
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



