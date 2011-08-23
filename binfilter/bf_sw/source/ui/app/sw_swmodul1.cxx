/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>
#include <uiparam.hxx>

#include <bf_sfx2/request.hxx>
#include <bf_svtools/useroptions.hxx>

#include <cppuhelper/weak.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#include <bf_svx/dataaccessdescriptor.hxx>

#include <bf_offmgr/app.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/cmapitem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/brshitem.hxx>
#include <vcl/msgbox.hxx>

#include <swmodule.hxx>
#include <swtypes.hxx>
#include <usrpref.hxx>
#include <modcfg.hxx>

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <docsh.hxx>
#include <dbmgr.hxx>
#include <uinums.hxx>
#include <prtopt.hxx>		// fuer PrintOptions
#include <doc.hxx>
#include <cmdid.h>
#include <app.hrc>
#include "helpid.h"
namespace binfilter {

using namespace ::rtl;
using namespace ::binfilter ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;
#define C2U(char) ::rtl::OUString::createFromAscii(char)

/*------------------------------------------------------------------------
 Beschreibung:	Neuer Master fuer die Einstellungen wird gesetzt;
                dieser wirkt sich auf die aktuelle Sicht und alle
                folgenden aus.
------------------------------------------------------------------------*/

/*M*/ void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
/*M*/ 							sal_uInt16 nDest )
/*M*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*M*/ }
/*-----------------05.02.97 08.03-------------------

--------------------------------------------------*/

/*N*/ SwPrintOptions* 	SwModule::GetPrtOptions(sal_Bool bWeb)
/*N*/ {
/*N*/ 	if(bWeb && !pWebPrtOpt)
/*N*/ 	{
/*N*/ 		pWebPrtOpt = new SwPrintOptions(TRUE);
/*N*/ 	}
/*N*/ 	else if(!bWeb && !pPrtOpt)
/*N*/ 	{
/*N*/ 		pPrtOpt = new SwPrintOptions(FALSE);
/*N*/ 	}
/*N*/
/*N*/ 	return bWeb ? pWebPrtOpt : pPrtOpt;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Redlining
 --------------------------------------------------------------------*/

/*N*/ sal_uInt16 SwModule::GetRedlineAuthor()
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	if (!bAuthorInitialised)
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ const String& SwModule::GetRedlineAuthor(sal_uInt16 nPos)
/*N*/ {
/*N*/ 	return *pAuthorNames->GetObject(nPos);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ sal_uInt16 SwModule::InsertRedlineAuthor(const String& rAuthor) //SW50.SDW
/*N*/ {
/*N*/ 	sal_uInt16 nPos = 0;
/*N*/
/*N*/ 	while (nPos < pAuthorNames->Count() && *pAuthorNames->GetObject(nPos) != rAuthor)
/*N*/ 		nPos++;
/*N*/
/*N*/ 	if (nPos == pAuthorNames->Count())
/*N*/ 		pAuthorNames->Insert(new String(rAuthor), nPos);
/*N*/
/*N*/ 	return nPos;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ sal_uInt16 SwModule::GetRedlineMarkPos()
/*N*/ {
/*N*/ 	return pModuleConfig->GetMarkAlignMode();
/*N*/ }


/*-----------------03.03.98 16:47-------------------

--------------------------------------------------*/
/*N*/ const SwViewOption*	SwModule::GetViewOption(sal_Bool bWeb)
/*N*/ {
/*N*/ 	return GetUsrPref( bWeb );
/*N*/ }

// returne den definierten DocStat - WordDelimiter
/*N*/ const String& SwModule::GetDocStatWordDelim() const
/*N*/ {
/*N*/ 	return pModuleConfig->GetWordDelimiter();
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
/*N*/ sal_uInt16 SwModule::GetMetric( sal_Bool bWeb ) const
/*N*/ {
/*N*/ 	SwMasterUsrPref* pPref;
/*N*/ 	if(bWeb)
/*N*/ 	{
/*?*/ 		if(!pWebUsrPref)
/*?*/ 			GetUsrPref(sal_True);
/*?*/ 		pPref = pWebUsrPref;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(!pUsrPref)
/*?*/ 			GetUsrPref(sal_False);
/*N*/ 		pPref = pUsrPref;
/*N*/ 	}
/*N*/ 	return pPref->GetMetric();
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Update-Stati durchreichen
/*N*/ sal_uInt16 SwModule::GetLinkUpdMode( sal_Bool ) const
/*N*/ {
/*N*/ 	if(!pUsrPref)
/*N*/ 		GetUsrPref(sal_False);
/*N*/     return (sal_uInt16)pUsrPref->GetUpdateLinkMode();
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt16 SwModule::GetFldUpdateFlags( sal_Bool ) const
/*N*/ {
/*N*/ 	if(!pUsrPref)
/*N*/ 		GetUsrPref(sal_False);
/*N*/ 	return (sal_uInt16)pUsrPref->GetFldUpdateFlags();
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
