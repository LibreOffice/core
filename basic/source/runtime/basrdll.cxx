/*************************************************************************
 *
 *  $RCSfile: basrdll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

