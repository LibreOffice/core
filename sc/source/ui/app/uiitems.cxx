/*************************************************************************
 *
 *  $RCSfile: uiitems.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:48 $
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

#include <svx/editobj.hxx>

#include "userlist.hxx"
#include "uiitems.hxx"
#include "dpsave.hxx"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScInputStatusItem,    SfxPoolItem);
TYPEINIT1(ScSortItem,           SfxPoolItem);
TYPEINIT1(ScQueryItem,          SfxPoolItem);
TYPEINIT1(ScSubTotalItem,       SfxPoolItem);
TYPEINIT1(ScUserListItem,       SfxPoolItem);
TYPEINIT1(ScConsolidateItem,    SfxPoolItem);
TYPEINIT1(ScPivotItem,          SfxPoolItem);
TYPEINIT1(ScSolveItem,          SfxPoolItem);
TYPEINIT1(ScTabOpItem,          SfxPoolItem);
TYPEINIT1(ScCondFrmtItem,       SfxPoolItem);

TYPEINIT1(ScTablesHint,         SfxHint);
TYPEINIT1(ScEditViewHint,       SfxHint);
TYPEINIT1(ScIndexHint,          SfxHint);

// -----------------------------------------------------------------------
//      ScInputStatusItem - Status-Update fuer Eingabezeile
// -----------------------------------------------------------------------

ScInputStatusItem::ScInputStatusItem( USHORT nWhich,
                                      USHORT nTab,
                                      USHORT nCol, USHORT nRow,
                                      USHORT nStartCol, USHORT nStartRow,
                                      USHORT nEndCol,   USHORT nEndRow,
                                      const String& rString, const EditTextObject* pData )

    :   SfxPoolItem ( nWhich ),
        aCursorPos  ( nCol, nRow, nTab ),
        aStartPos   ( nStartCol, nStartRow, nTab ),
        aEndPos     ( nEndCol,   nEndRow,   nTab ),
        aString     ( rString ),
        pEditData   ( pData ? pData->Clone() : NULL )
{
}

ScInputStatusItem::ScInputStatusItem( USHORT nWhich,
                                      const ScAddress& rCurPos,
                                      const ScAddress& rStartPos,
                                      const ScAddress& rEndPos,
                                      const String& rString,
                                      const EditTextObject* pData )
    :   SfxPoolItem ( nWhich ),
        aCursorPos  ( rCurPos ),
        aStartPos   ( rStartPos ),
        aEndPos     ( rEndPos ),
        aString     ( rString ),
        pEditData   ( pData ? pData->Clone() : NULL )
{
}

ScInputStatusItem::ScInputStatusItem( const ScInputStatusItem& rItem )
    :   SfxPoolItem ( rItem ),
        aCursorPos  ( rItem.aCursorPos ),
        aStartPos   ( rItem.aStartPos ),
        aEndPos     ( rItem.aEndPos ),
        aString     ( rItem.aString ),
        pEditData   ( rItem.pEditData ? rItem.pEditData->Clone() : NULL )
{
}

__EXPORT ScInputStatusItem::~ScInputStatusItem()
{
    delete pEditData;
}

String __EXPORT ScInputStatusItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("InputStatus"));
}

int __EXPORT ScInputStatusItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    return (    (aStartPos  == ((ScInputStatusItem&)rItem).aStartPos)
             && (aEndPos    == ((ScInputStatusItem&)rItem).aEndPos)
             && (aCursorPos == ((ScInputStatusItem&)rItem).aCursorPos)
             && (aString    == ((ScInputStatusItem&)rItem).aString) );
                                                        //! Edit-Daten vergleichen!
}

SfxPoolItem* __EXPORT ScInputStatusItem::Clone( SfxItemPool * ) const
{
    return new ScInputStatusItem( *this );
}

//
//  ScPaintHint ist nach schints.cxx verschoben
//

// -----------------------------------------------------------------------
//  ScTablesHint - Views anpassen, wenn Tabellen eingefuegt / geloescht
// -----------------------------------------------------------------------

ScTablesHint::ScTablesHint(USHORT nNewId, USHORT nTable1, USHORT nTable2) :
    nId( nNewId ),
    nTab1( nTable1 ),
    nTab2( nTable2 )
{
}

ScTablesHint::~ScTablesHint()
{
}


// -----------------------------------------------------------------------
//  ScIndexHint
// -----------------------------------------------------------------------

ScIndexHint::ScIndexHint(USHORT nNewId, USHORT nIdx) :
    nId( nNewId ),
    nIndex( nIdx )
{
}

ScIndexHint::~ScIndexHint()
{
}


// -----------------------------------------------------------------------
//      ScEditViewHint - neue EditView fuer Cursorposition anlegen
// -----------------------------------------------------------------------

ScEditViewHint::ScEditViewHint() :
    aCursorPos( 0,0,0 ),
    pEditEngine( NULL )
{
}

ScEditViewHint::ScEditViewHint( ScEditEngineDefaulter* pEngine, const ScAddress& rCurPos ) :
    aCursorPos( rCurPos ),
    pEditEngine( pEngine )
{
}

ScEditViewHint::~ScEditViewHint()
{
}

// -----------------------------------------------------------------------
//      ScSortItem - Daten fuer den Sortierdialog
// -----------------------------------------------------------------------

ScSortItem::ScSortItem( USHORT              nWhich,
                        ScViewData*         ptrViewData,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhich ),
        pViewData   ( ptrViewData )
{
    if ( pSortData ) theSortData = *pSortData;
}

//------------------------------------------------------------------------

ScSortItem::ScSortItem( USHORT              nWhich,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhich ),
        pViewData   ( NULL )
{
    if ( pSortData ) theSortData = *pSortData;
}

//------------------------------------------------------------------------

ScSortItem::ScSortItem( const ScSortItem& rItem ) :
        SfxPoolItem ( rItem ),
        pViewData   ( rItem.pViewData ),
        theSortData ( rItem.theSortData )
{
}

__EXPORT ScSortItem::~ScSortItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScSortItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("SortItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScSortItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSortItem& rOther = (const ScSortItem&)rItem;

    return (   (pViewData   == rOther.pViewData)
            && (theSortData == rOther.theSortData) );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScSortItem::Clone( SfxItemPool * ) const
{
    return new ScSortItem( *this );
}

// -----------------------------------------------------------------------
//      ScQueryItem - Daten fuer den Filterdialog
// -----------------------------------------------------------------------

ScQueryItem::ScQueryItem( USHORT                nWhich,
                          ScViewData*           ptrViewData,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhich ),
        bIsAdvanced ( FALSE ),
        pViewData   ( ptrViewData )
{
    if ( pQueryData ) theQueryData = *pQueryData;
}

//------------------------------------------------------------------------

ScQueryItem::ScQueryItem( USHORT                nWhich,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhich ),
        bIsAdvanced ( FALSE ),
        pViewData   ( NULL )
{
    if ( pQueryData ) theQueryData = *pQueryData;
}

//------------------------------------------------------------------------

ScQueryItem::ScQueryItem( const ScQueryItem& rItem ) :
        SfxPoolItem ( rItem ),
        bIsAdvanced ( rItem.bIsAdvanced ),
        aAdvSource  ( rItem.aAdvSource ),
        pViewData   ( rItem.pViewData ),
        theQueryData( rItem.theQueryData )
{
}

__EXPORT ScQueryItem::~ScQueryItem()
{
}

//------------------------------------------------------------------------

void ScQueryItem::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = TRUE;
    }
    else
        bIsAdvanced = FALSE;
}

BOOL ScQueryItem::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

//------------------------------------------------------------------------

String __EXPORT ScQueryItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("QueryItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScQueryItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScQueryItem& rQueryItem = (const ScQueryItem&)rItem;

    return (   (pViewData    == rQueryItem.pViewData)
            && (bIsAdvanced  == rQueryItem.bIsAdvanced)
            && (aAdvSource   == rQueryItem.aAdvSource)
            && (theQueryData == rQueryItem.theQueryData) );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScQueryItem::Clone( SfxItemPool * ) const
{
    return new ScQueryItem( *this );
}

// -----------------------------------------------------------------------
//      ScSubTotalItem - Daten fuer den Zwischenergebnisdialog
// -----------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( USHORT                  nWhich,
                                ScViewData*             ptrViewData,
                                const ScSubTotalParam*  pSubTotalData ) :
        SfxPoolItem ( nWhich ),
        pViewData   ( ptrViewData )
{
    if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
}

//------------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( USHORT                  nWhich,
                                const ScSubTotalParam*  pSubTotalData ) :
        SfxPoolItem ( nWhich ),
        pViewData   ( NULL )
{
    if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
}

//------------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( const ScSubTotalItem& rItem ) :
        SfxPoolItem     ( rItem ),
        pViewData       ( rItem.pViewData ),
        theSubTotalData ( rItem.theSubTotalData )
{
}

__EXPORT ScSubTotalItem::~ScSubTotalItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScSubTotalItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("SubTotalItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScSubTotalItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSubTotalItem& rSTItem = (const ScSubTotalItem&)rItem;

    return (   (pViewData       == rSTItem.pViewData)
            && (theSubTotalData == rSTItem.theSubTotalData) );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScSubTotalItem::Clone( SfxItemPool * ) const
{
    return new ScSubTotalItem( *this );
}

// -----------------------------------------------------------------------
//      ScUserListItem - Transporter fuer den Benutzerlisten-TabPage
// -----------------------------------------------------------------------

ScUserListItem::ScUserListItem( USHORT nWhich )
    :   SfxPoolItem ( nWhich ),
        pUserList   ( NULL )
{
}

//------------------------------------------------------------------------

ScUserListItem::ScUserListItem( const ScUserListItem& rItem )
    :   SfxPoolItem ( rItem )
{
    if ( rItem.pUserList )
        pUserList = new ScUserList( *(rItem.pUserList) );
    else
        pUserList = NULL;
}

__EXPORT ScUserListItem::~ScUserListItem()
{
    delete pUserList;
}

//------------------------------------------------------------------------

String __EXPORT ScUserListItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScUserListItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScUserListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScUserListItem& r = (const ScUserListItem&)rItem;
    BOOL bEqual = FALSE;

    if ( !pUserList || !(r.pUserList) )
        bEqual = ( !pUserList && !(r.pUserList) );
    else
        bEqual = ( *pUserList == *(r.pUserList) );

    return bEqual;
}


//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScUserListItem::Clone( SfxItemPool * ) const
{
    return new ScUserListItem( *this );
}

//------------------------------------------------------------------------

void ScUserListItem::SetUserList( const ScUserList& rUserList )
{
    delete pUserList;
    pUserList = new ScUserList( rUserList );
}

// -----------------------------------------------------------------------
//      ScConsolidateItem - Daten fuer den Konsolidieren-Dialog
// -----------------------------------------------------------------------

ScConsolidateItem::ScConsolidateItem(
                            USHORT                      nWhich,
                            const ScConsolidateParam*   pConsolidateData ) :
        SfxPoolItem ( nWhich )
{
    if ( pConsolidateData ) theConsData = *pConsolidateData;
}

//------------------------------------------------------------------------

ScConsolidateItem::ScConsolidateItem( const ScConsolidateItem& rItem ) :
        SfxPoolItem ( rItem ),
        theConsData ( rItem.theConsData )
{
}

//------------------------------------------------------------------------

__EXPORT ScConsolidateItem::~ScConsolidateItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScConsolidateItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScConsolidateItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScConsolidateItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScConsolidateItem& rCItem = (const ScConsolidateItem&)rItem;

    return ( theConsData == rCItem.theConsData);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScConsolidateItem::Clone( SfxItemPool * ) const
{
    return new ScConsolidateItem( *this );
}


// -----------------------------------------------------------------------
//      ScPivotItem - Daten fuer den Pivot-Dialog
// -----------------------------------------------------------------------

ScPivotItem::ScPivotItem( USHORT nWhich, const ScDPSaveData* pData,
                             const ScRange* pRange, BOOL bNew ) :
        SfxPoolItem ( nWhich )
{
    //  pSaveData must always exist
    if ( pData )
        pSaveData = new ScDPSaveData(*pData);
    else
        pSaveData = new ScDPSaveData;
    if ( pRange ) aDestRange = *pRange;
    bNewSheet = bNew;
}

//------------------------------------------------------------------------

ScPivotItem::ScPivotItem( const ScPivotItem& rItem ) :
        SfxPoolItem ( rItem ),
        aDestRange  ( rItem.aDestRange ),
        bNewSheet   ( rItem.bNewSheet )
{
    DBG_ASSERT(rItem.pSaveData, "pSaveData");
    pSaveData = new ScDPSaveData(*rItem.pSaveData);
}

//------------------------------------------------------------------------

__EXPORT ScPivotItem::~ScPivotItem()
{
    delete pSaveData;
}

//------------------------------------------------------------------------

String __EXPORT ScPivotItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScPivotItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScPivotItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScPivotItem& rPItem = (const ScPivotItem&)rItem;
    DBG_ASSERT( pSaveData && rPItem.pSaveData, "pSaveData" );
    return ( *pSaveData == *rPItem.pSaveData &&
             aDestRange == rPItem.aDestRange &&
             bNewSheet  == rPItem.bNewSheet );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScPivotItem::Clone( SfxItemPool * ) const
{
    return new ScPivotItem( *this );
}


// -----------------------------------------------------------------------
//      ScSolveItem - Daten fuer den Solver-Dialog
// -----------------------------------------------------------------------

ScSolveItem::ScSolveItem( USHORT                nWhich,
                          const ScSolveParam*   pSolveData )
    :   SfxPoolItem ( nWhich )
{
    if ( pSolveData ) theSolveData = *pSolveData;
}

//------------------------------------------------------------------------

ScSolveItem::ScSolveItem( const ScSolveItem& rItem )
    :   SfxPoolItem     ( rItem ),
        theSolveData    ( rItem.theSolveData )
{
}

//------------------------------------------------------------------------

__EXPORT ScSolveItem::~ScSolveItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScSolveItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScSolveItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScSolveItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSolveItem& rPItem = (const ScSolveItem&)rItem;

    return ( theSolveData == rPItem.theSolveData );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScSolveItem::Clone( SfxItemPool * ) const
{
    return new ScSolveItem( *this );
}

// -----------------------------------------------------------------------
//      ScTabOpItem - Daten fuer den TabOp-Dialog
// -----------------------------------------------------------------------

ScTabOpItem::ScTabOpItem( USHORT                nWhich,
                          const ScTabOpParam*   pTabOpData )
    :   SfxPoolItem ( nWhich )
{
    if ( pTabOpData ) theTabOpData = *pTabOpData;
}

//------------------------------------------------------------------------

ScTabOpItem::ScTabOpItem( const ScTabOpItem& rItem )
    :   SfxPoolItem     ( rItem ),
        theTabOpData    ( rItem.theTabOpData )
{
}

//------------------------------------------------------------------------

__EXPORT ScTabOpItem::~ScTabOpItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScTabOpItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScTabOpItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScTabOpItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTabOpItem& rPItem = (const ScTabOpItem&)rItem;

    return ( theTabOpData == rPItem.theTabOpData );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTabOpItem::Clone( SfxItemPool * ) const
{
    return new ScTabOpItem( *this );
}


// -----------------------------------------------------------------------
//      ScCondFrmtItem - Daten fuer den Dialog bedingte Formatierung
// -----------------------------------------------------------------------

ScCondFrmtItem::ScCondFrmtItem( USHORT nWhich,
//!                             const ScConditionalFormat* pCondFrmt )
                                const ScConditionalFormat& rCondFrmt )
    :   SfxPoolItem ( nWhich ),
        theCondFrmtData ( rCondFrmt )   //!
{
//! if ( pCondFrmt ) theCondFrmtData = *pCondFrmt;
}

//------------------------------------------------------------------------

ScCondFrmtItem::ScCondFrmtItem( const ScCondFrmtItem& rItem )
    :   SfxPoolItem     ( rItem ),
        theCondFrmtData ( rItem.theCondFrmtData )
{
}

//------------------------------------------------------------------------

__EXPORT ScCondFrmtItem::~ScCondFrmtItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScCondFrmtItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScCondFrmtItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScCondFrmtItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScCondFrmtItem& rPItem = (const ScCondFrmtItem&)rItem;

    return ( theCondFrmtData == rPItem.theCondFrmtData );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScCondFrmtItem::Clone( SfxItemPool * ) const
{
    return new ScCondFrmtItem( *this );
}


#pragma SEG_EOFMODULE


