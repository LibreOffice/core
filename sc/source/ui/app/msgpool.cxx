/*************************************************************************
 *
 *  $RCSfile: msgpool.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svx/dialogs.hrc>

#include "sc.hrc"
#include "docpool.hxx"
#include "msgpool.hxx"

//------------------------------------------------------------------------

static SfxItemInfo __READONLY_DATA aMsgItemInfos[] =
{
    { 0,                         SFX_ITEM_POOLABLE },   // SCITEM_STRING
    { 0,                         SFX_ITEM_POOLABLE },   // SCITEM_SEARCHDATA - nicht mehr benutzt !!!
    { SID_SORT,                  SFX_ITEM_POOLABLE },   // SCITEM_SORTDATA
    { SID_QUERY,                 SFX_ITEM_POOLABLE },   // SCITEM_QUERYDATA
    { SID_SUBTOTALS,             SFX_ITEM_POOLABLE },   // SCITEM_SUBTDATA
    { SID_CONSOLIDATE,           SFX_ITEM_POOLABLE },   // SCITEM_CONSOLIDATEDATA
    { SID_PIVOT_TABLE,           SFX_ITEM_POOLABLE },   // SCITEM_PIVOTDATA
    { SID_SOLVE,                 SFX_ITEM_POOLABLE },   // SCITEM_SOLVEDATA
    { SID_SCUSERLISTS,           SFX_ITEM_POOLABLE },   // SCITEM_USERLIST
    { SID_PRINTER_NOTFOUND_WARN, SFX_ITEM_POOLABLE }    // SCITEM_PRINTWARN
};

//------------------------------------------------------------------------

ScMessagePool::ScMessagePool()
    :   SfxItemPool         ( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScMessagePool")),
                              MSGPOOL_START, MSGPOOL_END,
                              aMsgItemInfos, NULL ),
    //
    aGlobalStringItem       ( SfxStringItem         ( SCITEM_STRING, String() ) ),
    aGlobalSearchItem       ( SvxSearchItem         ( SCITEM_SEARCHDATA ) ),
    aGlobalSortItem         ( ScSortItem            ( SCITEM_SORTDATA, NULL ) ),
    aGlobalQueryItem        ( ScQueryItem           ( SCITEM_QUERYDATA, NULL, NULL ) ),
    aGlobalSubTotalItem     ( ScSubTotalItem        ( SCITEM_SUBTDATA, NULL, NULL ) ),
    aGlobalConsolidateItem  ( ScConsolidateItem     ( SCITEM_CONSOLIDATEDATA, NULL ) ),
    aGlobalPivotItem        ( ScPivotItem           ( SCITEM_PIVOTDATA, NULL, NULL, FALSE ) ),
    aGlobalSolveItem        ( ScSolveItem           ( SCITEM_SOLVEDATA, NULL ) ),
    aGlobalUserListItem     ( ScUserListItem        ( SCITEM_USERLIST ) ),
    //
    aPrintWarnItem          ( SfxBoolItem           ( SCITEM_PRINTWARN, FALSE ) )
{
    ppPoolDefaults = new SfxPoolItem*[MSGPOOL_END - MSGPOOL_START + 1];

    ppPoolDefaults[SCITEM_STRING            - MSGPOOL_START] = &aGlobalStringItem;
    ppPoolDefaults[SCITEM_SEARCHDATA        - MSGPOOL_START] = &aGlobalSearchItem;
    ppPoolDefaults[SCITEM_SORTDATA          - MSGPOOL_START] = &aGlobalSortItem;
    ppPoolDefaults[SCITEM_QUERYDATA         - MSGPOOL_START] = &aGlobalQueryItem;
    ppPoolDefaults[SCITEM_SUBTDATA          - MSGPOOL_START] = &aGlobalSubTotalItem;
    ppPoolDefaults[SCITEM_CONSOLIDATEDATA   - MSGPOOL_START] = &aGlobalConsolidateItem;
    ppPoolDefaults[SCITEM_PIVOTDATA         - MSGPOOL_START] = &aGlobalPivotItem;
    ppPoolDefaults[SCITEM_SOLVEDATA         - MSGPOOL_START] = &aGlobalSolveItem;
    ppPoolDefaults[SCITEM_USERLIST          - MSGPOOL_START] = &aGlobalUserListItem;
    ppPoolDefaults[SCITEM_PRINTWARN         - MSGPOOL_START] = &aPrintWarnItem;

    SetDefaults( ppPoolDefaults );

    pDocPool = new ScDocumentPool;

    SetSecondaryPool( pDocPool );
}


__EXPORT ScMessagePool::~ScMessagePool()
{
    Delete();
    SetSecondaryPool( NULL );       // before deleting defaults (accesses defaults)

    for ( USHORT i=0; i <= MSGPOOL_END-MSGPOOL_START; i++ )
        SetRef( *ppPoolDefaults[i], 0 );

    delete[] ppPoolDefaults;

    delete pDocPool;
}


SfxMapUnit __EXPORT ScMessagePool::GetMetric( USHORT nWhich ) const
{
    //  eigene Attribute: Twips, alles andere 1/100 mm

    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return SFX_MAPUNIT_TWIP;
    else
        return SFX_MAPUNIT_100TH_MM;
}





