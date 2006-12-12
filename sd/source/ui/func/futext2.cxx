/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: futext2.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:25:38 $
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
#include "precompiled_sd.hxx"

#include "futext.hxx"

#include <svx/editdata.hxx>
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#include <comphelper/extract.hxx>

#include "app.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "optsitem.hxx"

using namespace ::com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Setzen der Spell Optionen
|*
\************************************************************************/

void FuText::SetSpellOptions( ULONG& rCntrl )
{
    BOOL bHideSpell = mpDoc->GetHideSpell();
    BOOL bOnlineSpell = mpDoc->GetOnlineSpell();

    if( bHideSpell )
        rCntrl |= EE_CNTRL_NOREDLINES;
    else
        rCntrl &= ~EE_CNTRL_NOREDLINES;

    if( bOnlineSpell )
        rCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        rCntrl &= ~EE_CNTRL_ONLINESPELLING;
}

} // end of namespace sd

