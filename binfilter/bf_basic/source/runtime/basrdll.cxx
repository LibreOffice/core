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

/*?*/ // #ifndef _SV_SVAPP_HXX //autogen
/*?*/ // #include <vcl/svapp.hxx>
/*?*/ // #endif
/*?*/ // #ifndef _SOLAR_HRC
/*?*/ // #include <bf_svtools/solar.hrc>
/*?*/ // #endif
/*?*/ // #ifndef _TOOLS_DEBUG_HXX //autogen
/*?*/ // #include <tools/debug.hxx>
/*?*/ // #endif
/*?*/ // #ifndef _SV_MSGBOX_HXX //autogen
/*?*/ // #include <vcl/msgbox.hxx>
/*?*/ // #endif

/*?*/ // #include "sbstar.hxx"
#include "basrdll.hxx"
/*?*/ // #include "basrid.hxx"
/*?*/ // #include "sb.hrc"

namespace binfilter {

/*?*/ // BasicResId::BasicResId( USHORT nId ):
/*?*/ // 	ResId( nId, (*(BasicDLL**)GetAppData(SHL_BASIC))->GetResMgr() )
/*?*/ // {
/*?*/ // }

BasicDLL::BasicDLL()
{
/*?*/ // 	*(BasicDLL**)GetAppData(SHL_BASIC) = this;
/*?*/ // 	::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
/*?*/ // 	pResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(ofa), aLocale );
    bDebugMode = FALSE;
    bBreakEnabled = TRUE;
}

BasicDLL::~BasicDLL()
{
/*?*/ //    delete pResMgr;
}

/*?*/ // void BasicDLL::EnableBreak( BOOL bEnable )
/*?*/ // {
/*?*/ // 	BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
/*?*/ // 	DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
/*?*/ // 	if ( pThis )
/*?*/ // 		pThis->bBreakEnabled = bEnable;
/*?*/ // }
/*?*/ // 
/*?*/ // void BasicDLL::SetDebugMode( BOOL bDebugMode )
/*?*/ // {
/*?*/ // 	BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
/*?*/ // 	DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
/*?*/ // 	if ( pThis )
/*?*/ // 		pThis->bDebugMode = bDebugMode;
/*?*/ // }
/*?*/ // 
/*?*/ // 
/*?*/ // void BasicDLL::BasicBreak()
/*?*/ // {
/*?*/ // 	//bJustStopping: Wenn jemand wie wild x-mal STOP drueckt, aber das Basic
/*?*/ // 	// nicht schnell genug anhaelt, kommt die Box ggf. oefters...
/*?*/ // 	static BOOL bJustStopping = FALSE;
/*?*/ // 
/*?*/ // 	BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
/*?*/ // 	DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
/*?*/ // 	if ( pThis )
/*?*/ // 	{
/*?*/ // 		if ( StarBASIC::IsRunning() && !bJustStopping && ( pThis->bBreakEnabled || pThis->bDebugMode ) )
/*?*/ // 		{
/*?*/ // 			bJustStopping = TRUE;
/*?*/ // 			StarBASIC::Stop();
/*?*/ // 			String aMessageStr( BasicResId( IDS_SBERR_TERMINATED ) );
/*?*/ // 			InfoBox( 0, aMessageStr ).Execute();
/*?*/ // 			bJustStopping = FALSE;
/*?*/ // 		}
/*?*/ // 	}
/*?*/ // }

}

