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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyValue_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <bf_svtools/itempool.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <bf_svtools/aeitem.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <bf_svtools/slstitm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <bf_svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SFXSZITEM_HXX
#include <bf_svtools/szitem.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <bf_svtools/undo.hxx>
#endif

#define _SVSTDARR_STRINGS
#include <bf_svtools/svstdarr.hxx>

#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include <bf_svtools/ttprops.hxx>
#endif
#ifndef _SFXSIDS_HRC // handmade
#include <sfxsids.hrc>
#endif
#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _SFXISETHINT_HXX
#include <bf_svtools/isethint.hxx>
#endif

#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>
#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/helpopt.hxx>
#include <bf_svtools/undoopt.hxx>
#include <bf_svtools/securityoptions.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/inetoptions.hxx>
#include <bf_svtools/miscopt.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>

#include "docfile.hxx"
#include "sfxtypes.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "request.hxx"
#include "evntconf.hxx"
#include "cfgmgr.hxx"
#include "docinf.hxx"
#include "appdata.hxx"
#include "misccfg.hxx"
#include "appimp.hxx"
#include "helper.hxx"	// SfxContentHelper::...
#include "app.hxx"

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

//--------------------------------------------------------------------

/*N*/ SfxEventConfiguration* SfxApplication::GetEventConfig() const
/*N*/ {
/*N*/     if (!pAppData_Impl->pEventConfig)
/*N*/         pAppData_Impl->pEventConfig = new SfxEventConfiguration;
/*N*/     return pAppData_Impl->pEventConfig;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, FASTBOOL bSynchron )
/*N*/ {
/*N*/     DBG_ASSERT(pAppData_Impl->pEventConfig,"Keine Events angemeldet!");
/*N*/ 
/*N*/     SfxObjectShell *pDoc = rEventHint.GetObjShell();
/*N*/     if ( pDoc )
/*N*/ 	{
/*N*/ 		if ( pDoc->IsPreview() )
/*N*/         	return;
/*N*/         SFX_ITEMSET_ARG( pDoc->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_HIDDEN, sal_False );
/*N*/ 		if ( pItem && pItem->GetValue() )
/*N*/ 			bSynchron = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/     {
/*N*/         Broadcast(rEventHint);
/*N*/         if ( pDoc )
/*N*/             pDoc->Broadcast( rEventHint );
/*N*/     }
/*N*/ }

/*N*/ SfxMiscCfg* SfxApplication::GetMiscConfig()
/*N*/ {
/*N*/ 	if ( !pAppData_Impl->pMiscConfig )
/*N*/ 		pAppData_Impl->pMiscConfig = new SfxMiscCfg;
/*N*/ 
/*N*/ 	return pAppData_Impl->pMiscConfig;
/*N*/ }


}
