/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basrdll.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:37:50 $
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

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include <sbstar.hxx>
#include <basrdll.hxx>
#include <basrid.hxx>
#include <sb.hrc>

BasicResId::BasicResId( USHORT nId ):
    ResId( nId, (*(BasicDLL**)GetAppData(SHL_BASIC))->GetResMgr() )
{
}

BasicDLL::BasicDLL()
{
     *(BasicDLL**)GetAppData(SHL_BASIC) = this;
    pResMgr = NULL;
    bDebugMode = FALSE;
    bBreakEnabled = TRUE;
}

BasicDLL::~BasicDLL()
{
    delete pResMgr;
}

void BasicDLL::EnableBreak( BOOL bEnable )
{
    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bBreakEnabled = bEnable;
}

void BasicDLL::SetDebugMode( BOOL bDebugMode )
{
    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bDebugMode = bDebugMode;
}


void BasicDLL::BasicBreak()
{
    //bJustStopping: Wenn jemand wie wild x-mal STOP drueckt, aber das Basic
    // nicht schnell genug anhaelt, kommt die Box ggf. oefters...
    static BOOL bJustStopping = FALSE;

    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
    {
        if ( StarBASIC::IsRunning() && !bJustStopping && ( pThis->bBreakEnabled || pThis->bDebugMode ) )
        {
            bJustStopping = TRUE;
            StarBASIC::Stop();
            String aMessageStr( BasicResId( IDS_SBERR_TERMINATED ) );
            InfoBox( 0, aMessageStr ).Execute();
            bJustStopping = FALSE;
        }
    }
}

