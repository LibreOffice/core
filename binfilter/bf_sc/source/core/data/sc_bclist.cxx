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

//------------------------------------------------------------------

#include <bf_svtools/lstner.hxx>
#include <tools/list.hxx>

#include "bclist.hxx"
namespace binfilter {

//------------------------------------------------------------------------

#define SC_LISTENERS_MAX 1024

/*N*/ DECLARE_LIST(ScBroadcasters, SfxBroadcaster*)//STRIP008 ;

//------------------------------------------------------------------------

/*N*/ ScBroadcasterList::ScBroadcasterList() :
/*N*/ 	pMoreBCs( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScBroadcasterList::~ScBroadcasterList()
/*N*/ {
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*?*/ 		ULONG nCount = pMoreBCs->Count();
/*?*/ 		for (ULONG i=0; i<nCount; i++)
/*?*/ 		{
/*?*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 			delete pBC;
/*?*/ 		}
/*?*/ 		delete pMoreBCs;
/*N*/ 	}
/*N*/ }

/*N*/ void ScBroadcasterList::StartBroadcasting( SfxListener& rLst, BOOL bCheckDup )
/*N*/ {
/*N*/ 	if (bCheckDup)
/*N*/ 	{
/*N*/ 		if (rLst.IsListening(aFirstBC))
/*N*/ 			return;
/*N*/ 		if (pMoreBCs)
/*N*/ 		{
/*?*/ 			ULONG nCount = pMoreBCs->Count();
/*?*/ 			for (ULONG i=0; i<nCount; i++)
/*?*/ 			{
/*?*/ 				SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 				if (rLst.IsListening(*pBC))
/*?*/ 					return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( aFirstBC.GetListenerCount() < SC_LISTENERS_MAX )
/*N*/ 	{
/*N*/ 		rLst.StartListening(aFirstBC, FALSE);
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*?*/ 		ULONG nCount = pMoreBCs->Count();
/*?*/ 		for (ULONG i=0; i<nCount; i++)
/*?*/ 		{
/*?*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 			if ( pBC->GetListenerCount() < SC_LISTENERS_MAX )
/*?*/ 			{
/*?*/ 				rLst.StartListening(*pBC, FALSE);
/*?*/ 				return;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if (!pMoreBCs)
/*?*/ 		pMoreBCs = new ScBroadcasters;
/*N*/ 	SfxBroadcaster* pNew = new SfxBroadcaster;
/*N*/ 	rLst.StartListening(*pNew, FALSE);
/*N*/ 	pMoreBCs->Insert(pNew, (ULONG)0);	// vorne
/*N*/ }

/*N*/ void ScBroadcasterList::EndBroadcasting( SfxListener& rLst )
/*N*/ {
/*N*/ 	rLst.EndListening(aFirstBC);
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*?*/ 		ULONG nCount = pMoreBCs->Count();
/*?*/ 		for (ULONG i=0; i<nCount; i++)
/*?*/ 		{
/*?*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 			rLst.EndListening(*pBC);
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScBroadcasterList::HasListeners() const
/*N*/ {
/*N*/ 	if (aFirstBC.HasListeners())
/*N*/ 		return TRUE;
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*?*/ 		ULONG nCount = pMoreBCs->Count();
/*?*/ 		for (ULONG i=0; i<nCount; i++)
/*?*/ 		{
/*?*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 			if (pBC->HasListeners())
/*?*/ 				return TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScBroadcasterList::Broadcast( const SfxHint &rHint )
/*N*/ {
/*N*/ 	aFirstBC.Broadcast( rHint );
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*?*/ 		ULONG nCount = pMoreBCs->Count();
/*?*/ 		for (ULONG i=0; i<nCount; i++)
/*?*/ 		{
/*?*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*?*/ 			pBC->Broadcast( rHint );
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScBroadcasterList::MoveListenersTo( ScBroadcasterList& rNew )
/*N*/ {
/*N*/ 	SfxBroadcaster* pNewBC = &rNew.aFirstBC;
/*N*/ 
/*N*/ 	USHORT nLstCount, nLstPos;
/*N*/ 	nLstCount = aFirstBC.GetListenerCount();
/*N*/ 	for (nLstPos=nLstCount; nLstPos>0;)
/*N*/ 	{
/*N*/ 		--nLstPos;
/*N*/ 		SfxListener* pLst = aFirstBC.GetListener(nLstPos);
/*N*/ 		rNew.StartBroadcasting(*pLst, TRUE);
/*N*/ 		pLst->EndListening(aFirstBC);
/*N*/ 	}
/*N*/ 	if (pMoreBCs)
/*N*/ 	{
/*N*/ 		ULONG nBCCount = pMoreBCs->Count();
/*N*/ 		for (ULONG i=0; i<nBCCount; i++)
/*N*/ 		{
/*N*/ 			SfxBroadcaster* pBC = pMoreBCs->GetObject(i);
/*N*/ 			nLstCount = pBC->GetListenerCount();
/*N*/ 			for (nLstPos=nLstCount; nLstPos>0;)
/*N*/ 			{
/*N*/ 				--nLstPos;
/*N*/ 				SfxListener* pLst = pBC->GetListener(nLstPos);
/*N*/ 				rNew.StartBroadcasting(*pLst, TRUE);
/*N*/ 				pLst->EndListening(*pBC);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
