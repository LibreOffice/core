/*************************************************************************
 *
 *  $RCSfile: wrt_fn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:57 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif


#include "shellio.hxx"
#include "wrt_fn.hxx"
#include "pam.hxx"
#include "node.hxx"
#include "format.hxx"



Writer& Out( const SwAttrFnTab pTab, const SfxPoolItem& rHt, Writer & rWrt )
{
    USHORT nId = rHt.Which();
    ASSERT(  nId < POOLATTR_END && nId >= POOLATTR_BEGIN, "SwAttrFnTab::Out()" );
    FnAttrOut pOut;
    if( 0 != ( pOut = pTab[ nId - RES_CHRATR_BEGIN] ))
        (*pOut)( rWrt, rHt );
    return rWrt;

}

Writer& Out_SfxItemSet( const SwAttrFnTab pTab, Writer& rWrt,
                        const SfxItemSet& rSet, BOOL bDeep,
                        BOOL bTstForDefault )
{
    // erst die eigenen Attribute ausgeben
    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxItemSet* pSet = &rSet;
    if( !pSet->Count() )        // Optimierung - leere Sets
    {
        if( !bDeep )
            return rWrt;
        while( 0 != ( pSet = pSet->GetParent() ) && !pSet->Count() )
            ;
        if( !pSet )
            return rWrt;
    }
    const SfxPoolItem* pItem;
    FnAttrOut pOut;
    if( !bDeep || !pSet->GetParent() )
    {
        ASSERT( rSet.Count(), "Wurde doch schon behandelt oder?" );
        SfxItemIter aIter( *pSet );
        pItem = aIter.GetCurItem();
        do {
            if( 0 != ( pOut = pTab[ pItem->Which() - RES_CHRATR_BEGIN] ))
                    (*pOut)( rWrt, *pItem );
        } while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
    }
    else
    {
        SfxWhichIter aIter( *pSet );
        register USHORT nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( nWhich, bDeep, &pItem ) &&
                ( !bTstForDefault || (
                    *pItem != rPool.GetDefaultItem( nWhich )
                    || ( pSet->GetParent() &&
                        *pItem != pSet->GetParent()->Get( nWhich ))
                )) && 0 != ( pOut = pTab[ nWhich - RES_CHRATR_BEGIN] ))
                    (*pOut)( rWrt, *pItem );
            nWhich = aIter.NextWhich();
        }
    }
    return rWrt;
}



Writer& Out( const SwNodeFnTab pTab, SwNode& rNode, Writer & rWrt )
{
    // es muss ein CntntNode sein !!
    SwCntntNode * pCNd = rNode.GetCntntNode();
    if( !pCNd )
        return rWrt;

    USHORT nId;
    switch( pCNd->GetNodeType() )
    {
    case ND_TEXTNODE:   nId = RES_TXTNODE;  break;
    case ND_GRFNODE:    nId = RES_GRFNODE;  break;
    case ND_OLENODE:    nId = RES_OLENODE;  break;
    default:
        ASSERT( FALSE, "was fuer ein Node ist es denn nun?" );
    }
    FnNodeOut pOut;
    if( 0 != ( pOut = pTab[ nId - RES_NODE_BEGIN ] ))
        (*pOut)( rWrt, *pCNd );
    return rWrt;
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/writer/wrt_fn.cxx,v 1.1.1.1 2000-09-18 17:14:57 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.32  2000/09/18 16:04:55  willem.vandorp
      OpenOffice header added.

      Revision 1.31  1999/07/06 12:55:38  JP
      Out_SfxItemSet: test to default items optional


      Rev 1.30   06 Jul 1999 14:55:38   JP
   Out_SfxItemSet: test to default items optional

      Rev 1.29   26 Nov 1997 14:29:14   MA
   headerfiles

      Rev 1.28   03 Nov 1997 14:12:48   MA
   precomp entfernt

      Rev 1.27   22 Jan 1997 12:01:18   JP
   neu: Tabellen Box Attribute

      Rev 1.26   16 Oct 1996 16:58:54   JP
   unbenutzte Methoden entfernt

      Rev 1.25   01 Jul 1996 16:03:14   MA
   includes

      Rev 1.24   22 Mar 1996 14:52:12   SWG
   include hinzugefuegt

      Rev 1.23   24 Nov 1995 17:24:42   OM
   PCH->PRECOMPILED

      Rev 1.22   25 Oct 1994 15:57:12   MA
   PreHdr.

      Rev 1.21   04 Oct 1994 16:34:06   JP
   Out_SfxItemSet: optimiert

      Rev 1.20   28 Sep 1994 12:37:34   JP
   neue Methode: Out_SfxItemSet

      Rev 1.19   25 Aug 1994 18:09:12   JP
   Umstellung Attribute (von SwHint -> SfxPoolItem)

      Rev 1.18   15 Feb 1994 15:31:30   MI
   handsegmentierung

      Rev 1.17   28 Jan 1994 11:34:06   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.16   13 Jan 1994 08:31:32   MI
   Segmentierung per #define ermoeglicht

*************************************************************************/


