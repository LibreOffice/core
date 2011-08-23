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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hintids.hxx"

#ifdef WIN
#define NEEDED_BY_FESHVIEW
#define _FESHVIEW_ONLY_INLINE_NEEDED
#endif

#include <bf_svx/svdopath.hxx>
#include <bf_svx/svdvmark.hxx>
#include <bf_svx/protitem.hxx>
#include <bf_so3/ipobj.hxx>

#include <poolfmt.hrc>      // fuer InitFldTypes
#include <frmfmt.hxx>
#include <frmatr.hxx>

#include <horiornt.hxx>

#include "fesh.hxx"
#include "doc.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "viewimp.hxx"
#include "flyfrm.hxx"
#include "ndole.hxx"
#include "txtfrm.hxx"
namespace binfilter {

#define SCROLLVAL 75

//Tattergrenze fuer Drawing-SS
#define MINMOVE ((USHORT)GetOut()->PixelToLogic(Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())

/*N*/ SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, ViewShell *pSh )
/*N*/ {
/*N*/ 	if ( !pLst )
/*?*/ 		pLst = pSh->HasDrawView() ? &pSh->Imp()->GetDrawView()->GetMarkList():0;
/*N*/ 
/*N*/ 	if ( pLst && pLst->GetMarkCount() == 1 )
/*N*/ 	{
/*?*/ 		SdrObject *pO = pLst->GetMark( 0 )->GetObj();
/*?*/ 		if ( pO->IsWriterFlyFrame() )
/*?*/ 			return ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ BOOL SwFEShell::IsFrmSelected() const
/*N*/ {
/*N*/ 	if ( !Imp()->HasDrawView() )
/*N*/ 		return FALSE;
/*N*/ 	else
/*N*/ 		return 0 != ::binfilter::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkList(),
/*N*/ 										(ViewShell*)this );
/*N*/ }



// zeige das akt. selektierte "Object" an
/*N*/ void SwFEShell::MakeSelVisible()
/*N*/ {
/*N*/ 	if( Imp()->HasDrawView() &&
/*N*/ 		Imp()->GetDrawView()->GetMarkList().GetMarkCount() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 MakeVisible( Imp()->GetDrawView()->GetAllMarkedRect() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SwCrsrShell::MakeSelVisible();
/*N*/ }


/*N*/ void SwFEShell::HideChainMarker()
/*N*/ {
/*N*/ 	if ( pChainFrom )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pChainFrom->Hide();
/*N*/ 	if ( pChainTo )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pChainTo->Hide();
/*N*/ }

/*N*/ void SwFEShell::SetChainMarker()
/*N*/ {
/*N*/ 	FASTBOOL bDelFrom = TRUE,
/*N*/ 			 bDelTo	  = TRUE;
/*N*/ 	if ( IsFrmSelected() )
/*N*/ 	{
/*?*/ 		SwFlyFrm *pFly = FindFlyFrm();
/*?*/ 
/*?*/ 		XPolygon aPoly(3);
/*?*/ 		if ( pFly->GetPrevLink() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 		}
/*?*/ 		if ( pFly->GetNextLink() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if ( bDelFrom )
/*N*/ 		delete pChainFrom, pChainFrom = 0;
/*N*/ 	if ( bDelTo )
/*N*/ 		delete pChainTo,   pChainTo = 0;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
