/*************************************************************************
 *
 *  $RCSfile: attrib.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------


#ifndef _COM_SUN_STAR_UTIL_CELLPROTECTION_HPP_
#include <com/sun/star/util/CellProtection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XPROTECTABLE_HPP_
#include <com/sun/star/util/XProtectable.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/boxitem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>
#include <svtools/args.hxx>

#include "attrib.hxx"
#include "global.hxx"
#include "editutil.hxx"
#include "sc.hrc"
#include "globstr.hrc"

#include "textuno.hxx"  // ScHeaderFooterContentObj

using namespace com::sun::star;

//------------------------------------------------------------------------

TYPEINIT1(ScMergeAttr,          SfxPoolItem);
TYPEINIT1(ScProtectionAttr,     SfxPoolItem);
TYPEINIT1(ScRangeItem,          SfxPoolItem);
TYPEINIT1(ScTableListItem,      SfxPoolItem);
TYPEINIT1(ScPageHFItem,         SfxPoolItem);
TYPEINIT1(ScViewObjectModeItem, SfxEnumItem);
TYPEINIT1(ScDoubleItem,         SfxPoolItem);

//------------------------------------------------------------------------

//
//      allgemeine Hilfsfunktionen
//

BOOL HasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther )
{
//    DBG_ASSERT( pThis || pOther, "LineAttr == 0" );

    if (!pThis)
        return FALSE;
    if (!pOther)
        return TRUE;

    USHORT nThisSize = pThis->GetOutWidth() + pThis->GetDistance() + pThis->GetInWidth();
    USHORT nOtherSize = pOther->GetOutWidth() + pOther->GetDistance() + pOther->GetInWidth();

    if (nThisSize > nOtherSize)
        return TRUE;
    else if (nThisSize < nOtherSize)
        return FALSE;
    else
    {
        if ( pOther->GetInWidth() && !pThis->GetInWidth() )
            return TRUE;
        else if ( pThis->GetInWidth() && !pOther->GetInWidth() )
            return FALSE;
        else
        {
            return TRUE;            //! ???
        }
    }
}


//
//      Item - Implementierungen
//

//------------------------------------------------------------------------
// Merge
//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr():
    SfxPoolItem(ATTR_MERGE),
    nColMerge(0),
    nRowMerge(0)
{}

//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr( INT16 nCol, INT16 nRow):
    SfxPoolItem(ATTR_MERGE),
    nColMerge(nCol),
    nRowMerge(nRow)
{}

//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr(const ScMergeAttr& rItem):
    SfxPoolItem(ATTR_MERGE)
{
    nColMerge = rItem.nColMerge;
    nRowMerge = rItem.nRowMerge;
}

__EXPORT ScMergeAttr::~ScMergeAttr()
{
}

//------------------------------------------------------------------------

String __EXPORT ScMergeAttr::GetValueText() const
{
    String aString( '(' );
    aString += String::CreateFromInt32( nColMerge );
    aString += ',';
    aString += String::CreateFromInt32( nRowMerge );
    aString += ')';
    return aString;
}

//------------------------------------------------------------------------

int __EXPORT ScMergeAttr::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( Which() != rItem.Which() || Type() == rItem.Type(), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (nColMerge == ((ScMergeAttr&)rItem).nColMerge)
             && (nRowMerge == ((ScMergeAttr&)rItem).nRowMerge);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScMergeAttr::Clone( SfxItemPool * ) const
{
    return new ScMergeAttr(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScMergeAttr::Create( SvStream& rStream, USHORT nVer ) const
{
    INT16   nCol;
    INT16   nRow;
    rStream >> nCol;
    rStream >> nRow;
    return new ScMergeAttr(nCol,nRow);
}

//------------------------------------------------------------------------

SvStream& __EXPORT ScMergeAttr::Store( SvStream& rStream, USHORT nVer ) const
{
    rStream << nColMerge;
    rStream << nRowMerge;
    return rStream;
}

//------------------------------------------------------------------------
// MergeFlag
//------------------------------------------------------------------------

ScMergeFlagAttr::ScMergeFlagAttr():
    SfxInt16Item(ATTR_MERGE_FLAG, 0)
{
}

//------------------------------------------------------------------------

ScMergeFlagAttr::ScMergeFlagAttr(INT16 nFlags):
    SfxInt16Item(ATTR_MERGE_FLAG, nFlags)
{
}

__EXPORT ScMergeFlagAttr::~ScMergeFlagAttr()
{
}

//------------------------------------------------------------------------
// Protection
//------------------------------------------------------------------------

ScProtectionAttr::ScProtectionAttr():
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(TRUE),
    bHideFormula(FALSE),
    bHideCell(FALSE),
    bHidePrint(FALSE)
{
}

//------------------------------------------------------------------------

ScProtectionAttr::ScProtectionAttr( BOOL bProtect, BOOL bHFormula,
                                    BOOL bHCell, BOOL bHPrint):
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(bProtect),
    bHideFormula(bHFormula),
    bHideCell(bHCell),
    bHidePrint(bHPrint)
{
}

//------------------------------------------------------------------------

ScProtectionAttr::ScProtectionAttr(const ScProtectionAttr& rItem):
    SfxPoolItem(ATTR_PROTECTION)
{
    bProtection  = rItem.bProtection;
    bHideFormula = rItem.bHideFormula;
    bHideCell    = rItem.bHideCell;
    bHidePrint   = rItem.bHidePrint;
}

__EXPORT ScProtectionAttr::~ScProtectionAttr()
{
}

//------------------------------------------------------------------------

BOOL __EXPORT ScProtectionAttr::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    util::CellProtection aProtection;
    aProtection.IsLocked        = bProtection;
    aProtection.IsFormulaHidden = bHideFormula;
    aProtection.IsHidden        = bHideCell;
    aProtection.IsPrintHidden   = bHidePrint;
    rVal <<= aProtection;
    return TRUE;
}

BOOL __EXPORT ScProtectionAttr::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    BOOL bRet = FALSE;
    util::CellProtection aProtection;
    if ( rVal >>= aProtection )
    {
        bProtection  = aProtection.IsLocked;
        bHideFormula = aProtection.IsFormulaHidden;
        bHideCell    = aProtection.IsHidden;
        bHidePrint   = aProtection.IsPrintHidden;
        bRet = TRUE;
    }
    else
        DBG_ERROR("exception - wrong argument");

    return bRet;
}

//------------------------------------------------------------------------

String __EXPORT ScProtectionAttr::GetValueText() const
{
    String aValue;
    String aStrYes ( ScGlobal::GetRscString(STR_YES) );
    String aStrNo  ( ScGlobal::GetRscString(STR_NO) );
    sal_Unicode cDelim = ',';

    aValue  = '(';
    aValue += (bProtection  ? aStrYes : aStrNo);    aValue += cDelim;
    aValue += (bHideFormula ? aStrYes : aStrNo);    aValue += cDelim;
    aValue += (bHideCell    ? aStrYes : aStrNo);    aValue += cDelim;
    aValue += (bHidePrint   ? aStrYes : aStrNo);
    aValue += ')';

    return aValue;
}

//------------------------------------------------------------------------

SfxItemPresentation __EXPORT ScProtectionAttr::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric,
        SfxMapUnit ePresMetric,
        String& rText,
        const International* pIntl
    ) const
{
    String aStrYes  ( ScGlobal::GetRscString(STR_YES) );
    String aStrNo   ( ScGlobal::GetRscString(STR_NO) );
    String aStrSep   = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    String aStrDelim = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValueText();
            break;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_PROTECTION); rText += aStrSep;
            rText += (bProtection ? aStrYes : aStrNo);       rText += aStrDelim;
            rText += ScGlobal::GetRscString(STR_FORMULAS);   rText += aStrSep;
            rText += (!bHideFormula ? aStrYes : aStrNo);     rText += aStrDelim;
            rText += ScGlobal::GetRscString(STR_HIDE);       rText += aStrSep;
            rText += (bHideCell ? aStrYes : aStrNo);         rText += aStrDelim;
            rText += ScGlobal::GetRscString(STR_PRINT);      rText += aStrSep;
            rText += (!bHidePrint ? aStrYes : aStrNo);
            break;

        default:
            ePres = SFX_ITEM_PRESENTATION_NONE;
    }

    return ePres;
}

//------------------------------------------------------------------------

int __EXPORT ScProtectionAttr::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( Which() != rItem.Which() || Type() == rItem.Type(), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (bProtection == ((ScProtectionAttr&)rItem).bProtection)
             && (bHideFormula == ((ScProtectionAttr&)rItem).bHideFormula)
             && (bHideCell == ((ScProtectionAttr&)rItem).bHideCell)
             && (bHidePrint == ((ScProtectionAttr&)rItem).bHidePrint);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScProtectionAttr::Clone( SfxItemPool * ) const
{
    return new ScProtectionAttr(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScProtectionAttr::Create( SvStream& rStream, USHORT n ) const
{
    BOOL bProtect;
    BOOL bHFormula;
    BOOL bHCell;
    BOOL bHPrint;

    rStream >> bProtect;
    rStream >> bHFormula;
    rStream >> bHCell;
    rStream >> bHPrint;

    return new ScProtectionAttr(bProtect,bHFormula,bHCell,bHPrint);
}

//------------------------------------------------------------------------

SvStream& __EXPORT ScProtectionAttr::Store( SvStream& rStream, USHORT nVer ) const
{
    rStream << bProtection;
    rStream << bHideFormula;
    rStream << bHideCell;
    rStream << bHidePrint;

    return rStream;
}

//------------------------------------------------------------------------

BOOL ScProtectionAttr::SetProtection( BOOL bProtect)
{
    bProtection =  bProtect;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScProtectionAttr::SetHideFormula( BOOL bHFormula)
{
    bHideFormula = bHFormula;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScProtectionAttr::SetHideCell( BOOL bHCell)
{
    bHideCell = bHCell;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScProtectionAttr::SetHidePrint( BOOL bHPrint)
{
    bHidePrint = bHPrint;
    return TRUE;
}

// -----------------------------------------------------------------------
//      ScRangeItem - Tabellenbereich
// -----------------------------------------------------------------------

void __EXPORT ScRangeItem::Record( SfxArguments& rArgs ) const
{
    const ScAddress& rStart = aRange.aStart;
    const ScAddress& rEnd   = aRange.aEnd;

    rArgs.AppendInteger( rStart.Col() );
    rArgs.AppendInteger( rStart.Row() );
    rArgs.AppendInteger( rStart.Tab() );
    rArgs.AppendInteger( rEnd  .Col() );
    rArgs.AppendInteger( rEnd  .Row() );
    rArgs.AppendInteger( rEnd.Tab() );
    rArgs.AppendInteger( nFlags );
}

// -----------------------------------------------------------------------

SfxArgumentError __EXPORT ScRangeItem::Construct( USHORT nId, const SfxArguments& rArgs )
{
    if ( rArgs.Count() < 7 )
        return SFX_ARGUMENT_ERROR( rArgs.Count(), SFX_ERR_ARGUMENT_EXPECTED );
    if ( rArgs.Count() > 7 )
        return SFX_ARGUMENT_ERROR( rArgs.Count()-1, SFX_ERR_TOO_MANY_ARGUMENTS );

    ScAddress& rStart = aRange.aStart;
    ScAddress& rEnd   = aRange.aEnd;

    SetWhich( nId );
    rStart.SetCol( rArgs.Get( 0 ).GetInteger() );
    rStart.SetRow( rArgs.Get( 1 ).GetInteger() );
    rStart.SetTab( rArgs.Get( 2 ).GetInteger() );
    rEnd  .SetCol( rArgs.Get( 3 ).GetInteger() );
    rEnd  .SetRow( rArgs.Get( 4 ).GetInteger() );
    rEnd  .SetTab( rArgs.Get( 5 ).GetInteger() );
    nFlags = (USHORT)rArgs.Get( 6 ).GetInteger();

    return 0;
}

// -----------------------------------------------------------------------

int __EXPORT ScRangeItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( aRange == ( (ScRangeItem&)rAttr ).aRange );
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT ScRangeItem::Clone( SfxItemPool* ) const
{
    return new ScRangeItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation __EXPORT ScRangeItem::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit          eCoreUnit,
        SfxMapUnit          ePresUnit,
        String&             rText,
        const International* pIntl
    ) const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
        rText  = ScGlobal::GetRscString(STR_AREA);
        rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
//      break;// Durchfallen !!!

        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            String aText;
            aRange.Format( aText );
            rText += aText;
        }
        break;
    }

    return ePres;
}

//-----------------------------------------------------------------------

USHORT __EXPORT ScRangeItem::GetVersion( USHORT nFileVersion ) const
{
    return 2;
}

//-----------------------------------------------------------------------

SvStream& __EXPORT ScRangeItem::Store( SvStream& rStrm, USHORT nVer ) const
{
    rStrm << aRange;
    rStrm << nFlags;

    return rStrm;
}

//-----------------------------------------------------------------------

SfxPoolItem* __EXPORT ScRangeItem::Create( SvStream& rStream, USHORT nVersion ) const
{
    ScRange aNewRange;
    BOOL    nNewFlags = FALSE;

    switch ( nVersion )
    {
        case 2:
            rStream >> aNewRange;
            rStream >> nNewFlags;
            break;

        case 1:
            rStream >> aNewRange;
            nNewFlags = 0;
            break;

        case 0:
            {
                // alte Version mit ScArea -> 5 USHORTs lesen
                ScAddress&  rStart = aNewRange.aStart;
                ScAddress&  rEnd   = aNewRange.aEnd;
                USHORT      n;

                rStream >> n;

                if ( n > MAXTAB )
                {
                    nNewFlags = SCR_ALLTABS;
                    rStart.SetTab( 0 ); rEnd.SetTab( 0 );
                }
                else
                {
                    nNewFlags = 0;
                    rStart.SetTab( n ); rEnd.SetTab( n );
                }

                rStream >> n; rStart.SetCol( n );
                rStream >> n; rStart.SetRow( n );
                rStream >> n; rEnd  .SetCol( n );
                rStream >> n; rEnd  .SetRow( n );
            }
            break;

        default:
            DBG_ERROR( "ScRangeItem::Create: Unknown Version!" );
    }

    return ( new ScRangeItem( Which(), aNewRange, nNewFlags ) );
}


// -----------------------------------------------------------------------
//      ScTableListItem - Liste von Tabellen(-nummern)
// -----------------------------------------------------------------------

ScTableListItem::ScTableListItem( const ScTableListItem& rCpy )
    :   SfxPoolItem ( rCpy.Which() ),
        nCount      ( rCpy.nCount )
{
    if ( nCount > 0 )
    {
        pTabArr = new USHORT [nCount];

        for ( USHORT i=0; i<nCount; i++ )
            pTabArr[i] = rCpy.pTabArr[i];
    }
    else
        pTabArr = NULL;
}

// -----------------------------------------------------------------------

ScTableListItem::ScTableListItem( const USHORT nWhich, const List& rList )
    :   SfxPoolItem ( nWhich ),
        nCount      ( 0 ),
        pTabArr     ( NULL )
{
    SetTableList( rList );
}

// -----------------------------------------------------------------------

__EXPORT ScTableListItem::~ScTableListItem()
{
    delete [] pTabArr;
}

// -----------------------------------------------------------------------

void __EXPORT ScTableListItem::Record( SfxArguments& rArgs ) const
{
    rArgs.AppendInteger( nCount );

    if ( nCount>0 && pTabArr )
        for ( USHORT i=0; i<nCount; i++ )
            rArgs.AppendInteger( pTabArr[i] );
}

// -----------------------------------------------------------------------

SfxArgumentError __EXPORT ScTableListItem::Construct( USHORT nId, const SfxArguments& rArgs )
{
    USHORT nCount = rArgs.Get( 0 ).GetInteger();

    if ( pTabArr )
        delete [] pTabArr, pTabArr = NULL;

    if ( nCount > 0 )
    {
        if ( rArgs.Count()-1 < nCount )
            return SFX_ARGUMENT_ERROR( rArgs.Count(), SFX_ERR_ARGUMENT_EXPECTED );
        if ( rArgs.Count()-1 > nCount )
            return SFX_ARGUMENT_ERROR( rArgs.Count()-1, SFX_ERR_TOO_MANY_ARGUMENTS );

        SetWhich( nId );
        pTabArr = new USHORT [nCount];

        for ( USHORT i=0; i<nCount; i++ )
            pTabArr[i] = rArgs.Get( i+1 ).GetInteger();
    }

    return 0;
}

// -----------------------------------------------------------------------

ScTableListItem& ScTableListItem::operator=( const ScTableListItem& rCpy )
{
    delete [] pTabArr;

    if ( rCpy.nCount > 0 )
    {
        pTabArr = new USHORT [rCpy.nCount];
        for ( USHORT i=0; i<rCpy.nCount; i++ )
            pTabArr[i] = rCpy.pTabArr[i];
    }
    else
        pTabArr = NULL;

    nCount = rCpy.nCount;

    return *this;
}

// -----------------------------------------------------------------------

int __EXPORT ScTableListItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    ScTableListItem&    rCmp   = (ScTableListItem&)rAttr;
    BOOL                bEqual = (nCount == rCmp.nCount);

    if ( nCount > 0 )
    {
        USHORT  i=0;

        bEqual = ( pTabArr && rCmp.pTabArr );

        while ( bEqual && i<nCount )
        {
            bEqual = ( pTabArr[i] == rCmp.pTabArr[i] );
            i++;
        }
    }
    return bEqual;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTableListItem::Clone( SfxItemPool* ) const
{
    return new ScTableListItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation __EXPORT ScTableListItem::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit          eCoreUnit,
        SfxMapUnit          ePresUnit,
        String&             rText,
        const International* pIntl
    ) const
{
    const sal_Unicode cDelim = ',';

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            {
            rText  = '(';
            if ( nCount>0 && pTabArr )
                for ( USHORT i=0; i<nCount; i++ )
                {
                    rText += String::CreateFromInt32( pTabArr[i] );
                    if ( i<(nCount-1) )
                        rText += cDelim;
                }
            rText += ')';
            }
            return ePres;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
    }

    return SFX_ITEM_PRESENTATION_NONE;
}

//-----------------------------------------------------------------------

SvStream& __EXPORT ScTableListItem::Store( SvStream& rStrm, USHORT nVer ) const
{
    rStrm << nCount;

    if ( nCount>0 && pTabArr )
        for ( USHORT i=0; i<nCount; i++ )
                rStrm << pTabArr[i];

    return rStrm;
}

//-----------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTableListItem::Create( SvStream& rStrm, USHORT ) const
{
    ScTableListItem* pNewItem;
    List             aList;
    USHORT*          p;
    USHORT           nTabCount;
    USHORT           nTabNo;

    rStrm >> nTabCount;

    if ( nTabCount > 0 )
    {
        for ( USHORT i=0; i<nTabCount; i++ )
        {
            rStrm >> nTabNo;
            aList.Insert( new USHORT(nTabNo) );
        }
    }

    pNewItem = new ScTableListItem( Which(), aList );

    aList.First();
    while ( p = (USHORT*)aList.Remove() )
        delete p;

    return pNewItem;
}

// -----------------------------------------------------------------------

BOOL ScTableListItem::GetTableList( List& aList ) const
{
    for ( USHORT i=0; i<nCount; i++ )
        aList.Insert( new USHORT( pTabArr[i] ) );

    return ( nCount > 0 );
}

// -----------------------------------------------------------------------

void ScTableListItem::SetTableList( const List& rList )
{
    nCount = (USHORT)rList.Count();

    delete [] pTabArr;

    if ( nCount > 0 )
    {
        pTabArr = new USHORT [nCount];

        for ( USHORT i=0; i<nCount; i++ )
            pTabArr[i] = *( (USHORT*)rList.GetObject( i ) );
    }
    else
        pTabArr = NULL;
}


// -----------------------------------------------------------------------
//      ScPageHFItem - Daten der Kopf-/Fußzeilen
// -----------------------------------------------------------------------

ScPageHFItem::ScPageHFItem( USHORT nWhich )
    :   SfxPoolItem ( nWhich ),
        pLeftArea   ( NULL ),
        pCenterArea ( NULL ),
        pRightArea  ( NULL )
{
}

//------------------------------------------------------------------------

ScPageHFItem::ScPageHFItem( const ScPageHFItem& rItem )
    :   SfxPoolItem ( rItem ),
        pLeftArea   ( NULL ),
        pCenterArea ( NULL ),
        pRightArea  ( NULL )
{
    if ( rItem.pLeftArea )
        pLeftArea = rItem.pLeftArea->Clone();
    if ( rItem.pCenterArea )
        pCenterArea = rItem.pCenterArea->Clone();
    if ( rItem.pRightArea )
        pRightArea = rItem.pRightArea->Clone();
}

//------------------------------------------------------------------------

__EXPORT ScPageHFItem::~ScPageHFItem()
{
    delete pLeftArea;
    delete pCenterArea;
    delete pRightArea;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScPageHFItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    uno::Reference<sheet::XHeaderFooterContent> xContent =
        new ScHeaderFooterContentObj( pLeftArea, pCenterArea, pRightArea );

    rVal <<= xContent;
    return TRUE;
}

BOOL __EXPORT ScPageHFItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    BOOL bRet = FALSE;
    uno::Reference<sheet::XHeaderFooterContent> xContent;
    if ( rVal >>= xContent )
    {
        if ( xContent.is() )
        {
            ScHeaderFooterContentObj* pImp =
                    ScHeaderFooterContentObj::getImplementation( xContent );
            if (pImp)
            {
                const EditTextObject* pImpLeft = pImp->GetLeftEditObject();
                delete pLeftArea;
                pLeftArea = pImpLeft ? pImpLeft->Clone() : NULL;

                const EditTextObject* pImpCenter = pImp->GetCenterEditObject();
                delete pCenterArea;
                pCenterArea = pImpCenter ? pImpCenter->Clone() : NULL;

                const EditTextObject* pImpRight = pImp->GetRightEditObject();
                delete pRightArea;
                pRightArea = pImpRight ? pImpRight->Clone() : NULL;

                if ( !pLeftArea || !pCenterArea || !pRightArea )
                {
                    // keine Texte auf NULL stehen lassen
                    ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), TRUE );
                    if (!pLeftArea)
                        pLeftArea = aEngine.CreateTextObject();
                    if (!pCenterArea)
                        pCenterArea = aEngine.CreateTextObject();
                    if (!pRightArea)
                        pRightArea = aEngine.CreateTextObject();
                }

                bRet = TRUE;
            }
        }
    }

    if (!bRet)
    {
        DBG_ERROR("exception - wrong argument");
    }

    return bRet;
}

//------------------------------------------------------------------------

String __EXPORT ScPageHFItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScPageHFItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScPageHFItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScPageHFItem& r = (const ScPageHFItem&)rItem;

    return    ScGlobal::EETextObjEqual(pLeftArea,   r.pLeftArea)
           && ScGlobal::EETextObjEqual(pCenterArea, r.pCenterArea)
           && ScGlobal::EETextObjEqual(pRightArea,  r.pRightArea);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScPageHFItem::Clone( SfxItemPool* ) const
{
    return new ScPageHFItem( *this );
}

//------------------------------------------------------------------------

USHORT __EXPORT ScPageHFItem::GetVersion( USHORT nFileVersion ) const
{
    // 0 = ohne Feldbefehle
    // 1 = Titel bzw. Dateiname mit SvxFileField
    // 2 = Pfad und/oder Dateiname mit SvxExtFileField, Titel mit SvxFileField
    return 2;
}

//------------------------------------------------------------------------

void lcl_SetSpace( String& rStr, const ESelection& rSel )
{
    // Text durch ein Leerzeichen ersetzen, damit Positionen stimmen:

    xub_StrLen nLen = rSel.nEndPos-rSel.nStartPos;
    rStr.Erase( rSel.nStartPos, nLen-1 );
    rStr.SetChar( rSel.nStartPos, ' ' );
}

BOOL lcl_ConvertFields(EditEngine& rEng, const String* pCommands)
{
    BOOL bChange = FALSE;
    USHORT nParCnt = rEng.GetParagraphCount();
    for (USHORT nPar = 0; nPar<nParCnt; nPar++)
    {
        String aStr = rEng.GetText( nPar );
        xub_StrLen nPos;

        while ((nPos = aStr.Search(pCommands[0])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[0].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxPageField()), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
        while ((nPos = aStr.Search(pCommands[1])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[1].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxPagesField()), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
        while ((nPos = aStr.Search(pCommands[2])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[2].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
        while ((nPos = aStr.Search(pCommands[3])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[3].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxTimeField()), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
        while ((nPos = aStr.Search(pCommands[4])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[4].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxFileField()), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
        while ((nPos = aStr.Search(pCommands[5])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[5].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxTableField()), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = TRUE;
        }
    }
    return bChange;
}

#define SC_FIELD_COUNT  6

SfxPoolItem* __EXPORT ScPageHFItem::Create( SvStream& rStream, USHORT nVer ) const
{
    EditTextObject* pLeft   = EditTextObject::Create(rStream);
    EditTextObject* pCenter = EditTextObject::Create(rStream);
    EditTextObject* pRight  = EditTextObject::Create(rStream);

    DBG_ASSERT( pLeft && pCenter && pRight, "Error reading ScPageHFItem" );

    if ( nVer < 1 )             // alte Feldbefehle umsetzen
    {
        USHORT i;
        const String& rDel = ScGlobal::GetRscString( STR_HFCMD_DELIMITER );
        String aCommands[SC_FIELD_COUNT];
        for (i=0; i<SC_FIELD_COUNT; i++)
            aCommands[i] = rDel;
        aCommands[0] += ScGlobal::GetRscString(STR_HFCMD_PAGE);
        aCommands[1] += ScGlobal::GetRscString(STR_HFCMD_PAGES);
        aCommands[2] += ScGlobal::GetRscString(STR_HFCMD_DATE);
        aCommands[3] += ScGlobal::GetRscString(STR_HFCMD_TIME);
        aCommands[4] += ScGlobal::GetRscString(STR_HFCMD_FILE);
        aCommands[5] += ScGlobal::GetRscString(STR_HFCMD_TABLE);
        for (i=0; i<SC_FIELD_COUNT; i++)
            aCommands[i] += rDel;

        ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), TRUE );
        aEngine.SetText(*pLeft);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pLeft;
            pLeft = aEngine.CreateTextObject();
        }
        aEngine.SetText(*pCenter);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pCenter;
            pCenter = aEngine.CreateTextObject();
        }
        aEngine.SetText(*pRight);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pRight;
            pRight = aEngine.CreateTextObject();
        }
    }
    else if ( nVer < 2 )
    {   // nichts tun, SvxFileField nicht gegen SvxExtFileField austauschen
    }

    ScPageHFItem* pItem = new ScPageHFItem( Which() );
    pItem->SetArea( pLeft,    SC_HF_LEFTAREA   );
    pItem->SetArea( pCenter, SC_HF_CENTERAREA );
    pItem->SetArea( pRight,  SC_HF_RIGHTAREA  );

    return pItem;
}

//------------------------------------------------------------------------

class ScFieldChangerEditEngine : public ScEditEngineDefaulter
{
    TypeId      aExtFileId;
    USHORT      nConvPara;
    xub_StrLen  nConvPos;
    BOOL        bConvert;

public:
                ScFieldChangerEditEngine( SfxItemPool* pEnginePool, BOOL bDeleteEnginePool );
    virtual     ~ScFieldChangerEditEngine() {}

    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara,
                                    USHORT nPos, Color*& rTxtColor,
                                    Color*& rFldColor );

    BOOL            ConvertFields();
};

ScFieldChangerEditEngine::ScFieldChangerEditEngine( SfxItemPool* pEnginePool,
            BOOL bDeleteEnginePool ) :
        ScEditEngineDefaulter( pEnginePool, bDeleteEnginePool ),
        aExtFileId( TYPE( SvxExtFileField ) ),
        nConvPara( 0 ),
        nConvPos( 0 ),
        bConvert( FALSE )
{
}

String ScFieldChangerEditEngine::CalcFieldValue( const SvxFieldItem& rField,
            USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor )
{
    const SvxFieldData* pFieldData = rField.GetField();
    if ( pFieldData && pFieldData->Type() == aExtFileId )
    {
        bConvert = TRUE;
        nConvPara = nPara;
        nConvPos = nPos;
    }
    return EMPTY_STRING;
}

BOOL ScFieldChangerEditEngine::ConvertFields()
{
    BOOL bConverted = FALSE;
    do
    {
        bConvert = FALSE;
        UpdateFields();
        if ( bConvert )
        {
            ESelection aSel( nConvPara, nConvPos, nConvPara, nConvPos+1 );
            QuickInsertField( SvxFileField(), aSel );
            bConverted = TRUE;
        }
    } while ( bConvert );
    return bConverted;
}

void lcl_StoreOldFields( ScFieldChangerEditEngine& rEngine,
            const EditTextObject* pArea, SvStream& rStream )
{
    rEngine.SetText( *pArea );
    if ( rEngine.ConvertFields() )
    {
        EditTextObject* pObj = rEngine.CreateTextObject();
        pObj->Store( rStream );
        delete pObj;
    }
    else
        pArea->Store( rStream );
}

SvStream& __EXPORT ScPageHFItem::Store( SvStream& rStream, USHORT nVer ) const
{
    if ( pLeftArea && pCenterArea && pRightArea )
    {
        if ( rStream.GetVersion() < SOFFICE_FILEFORMAT_50 )
        {
            ScFieldChangerEditEngine aEngine( EditEngine::CreatePool(), TRUE );
            lcl_StoreOldFields( aEngine, pLeftArea, rStream );
            lcl_StoreOldFields( aEngine, pCenterArea, rStream );
            lcl_StoreOldFields( aEngine, pRightArea, rStream );
        }
        else
        {
            pLeftArea->Store(rStream);
            pCenterArea->Store(rStream);
            pRightArea->Store(rStream);
        }
    }
    else
    {
        //  soll eigentlich nicht sein, kommt aber vor, wenn das Default-Item
        //  fuer ein ItemSet kopiert wird (#61826#) ...

        ScFieldChangerEditEngine aEngine( EditEngine::CreatePool(), TRUE );
        EditTextObject* pEmpytObj = aEngine.CreateTextObject();

        DBG_ASSERT( pEmpytObj, "Error creating empty EditTextObject :-(" );

        if ( rStream.GetVersion() < SOFFICE_FILEFORMAT_50 )
        {
            if ( pLeftArea )
                lcl_StoreOldFields( aEngine, pLeftArea, rStream );
            else
                pEmpytObj->Store( rStream );

            if ( pCenterArea )
                lcl_StoreOldFields( aEngine, pCenterArea, rStream );
            else
                pEmpytObj->Store( rStream );

            if ( pRightArea )
                lcl_StoreOldFields( aEngine, pRightArea, rStream );
            else
                pEmpytObj->Store( rStream );
        }
        else
        {
            (pLeftArea   ? pLeftArea   : pEmpytObj )->Store(rStream);
            (pCenterArea ? pCenterArea : pEmpytObj )->Store(rStream);
            (pRightArea  ? pRightArea  : pEmpytObj )->Store(rStream);
        }

        delete pEmpytObj;
    }

    return rStream;
}

//------------------------------------------------------------------------

void __EXPORT ScPageHFItem::SetLeftArea( const EditTextObject& rNew )
{
    delete pLeftArea;
    pLeftArea = rNew.Clone();
}

//------------------------------------------------------------------------

void __EXPORT ScPageHFItem::SetCenterArea( const EditTextObject& rNew )
{
    delete pCenterArea;
    pCenterArea = rNew.Clone();
}

//------------------------------------------------------------------------

void __EXPORT ScPageHFItem::SetRightArea( const EditTextObject& rNew )
{
    delete pRightArea;
    pRightArea = rNew.Clone();
}

void __EXPORT ScPageHFItem::SetArea( EditTextObject *pNew, int nArea )
{
    switch ( nArea )
    {
        case SC_HF_LEFTAREA:    delete pLeftArea;   pLeftArea   = pNew; break;
        case SC_HF_CENTERAREA:  delete pCenterArea; pCenterArea = pNew; break;
        case SC_HF_RIGHTAREA:   delete pRightArea;  pRightArea  = pNew; break;
        default:
            DBG_ERROR( "New Area?" );
    }
}

//-----------------------------------------------------------------------
//  ScViewObjectModeItem - Darstellungsmodus von ViewObjekten
//-----------------------------------------------------------------------

ScViewObjectModeItem::ScViewObjectModeItem( USHORT nWhich )
    : SfxEnumItem( nWhich, VOBJ_MODE_SHOW )
{
}

//------------------------------------------------------------------------

ScViewObjectModeItem::ScViewObjectModeItem( USHORT nWhich, ScVObjMode eMode )
    : SfxEnumItem( nWhich, eMode )
{
}

//------------------------------------------------------------------------

__EXPORT ScViewObjectModeItem::~ScViewObjectModeItem()
{
}

//------------------------------------------------------------------------

SfxItemPresentation __EXPORT ScViewObjectModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International* pIntl
)   const
{
    String  aDel = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(": "));
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
        switch( Which() )
        {
            case SID_SCATTR_PAGE_CHARTS:
            rText  = ScGlobal::GetRscString(STR_VOBJ_CHART);
            rText += aDel;
            break;

            case SID_SCATTR_PAGE_OBJECTS:
            rText  = ScGlobal::GetRscString(STR_VOBJ_OBJECT);
            rText += aDel;
            break;

            case SID_SCATTR_PAGE_DRAWINGS:
            rText  = ScGlobal::GetRscString(STR_VOBJ_DRAWINGS);
            rText += aDel;
            break;

            default:
            ePres = SFX_ITEM_PRESENTATION_NAMELESS;//das geht immer!
            break;
        }
//      break; // DURCHFALLEN!!!

        case SFX_ITEM_PRESENTATION_NAMELESS:
        rText += ScGlobal::GetRscString(STR_VOBJ_MODE_SHOW+GetValue());
        break;
    }

    return ePres;
}

//------------------------------------------------------------------------

String __EXPORT ScViewObjectModeItem::GetValueText( USHORT nVal ) const
{
    DBG_ASSERT( nVal <= VOBJ_MODE_DUMMY, "enum overflow!" );

    return ScGlobal::GetRscString( STR_VOBJ_MODE_SHOW + nVal );
}

//------------------------------------------------------------------------

USHORT __EXPORT ScViewObjectModeItem::GetValueCount() const
{
    return 3;
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScViewObjectModeItem::Clone( SfxItemPool* ) const
{
    return new ScViewObjectModeItem( *this );
}

//------------------------------------------------------------------------

USHORT __EXPORT ScViewObjectModeItem::GetVersion( USHORT nFileVersion ) const
{
    return 1;
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScViewObjectModeItem::Create(
                                    SvStream&   rStream,
                                    USHORT      nVersion ) const
{
    if ( nVersion == 0 )
    {
        // alte Version mit AllEnumItem -> mit Mode "Show" erzeugen
        return new ScViewObjectModeItem( Which() );
    }
    else
    {
        USHORT nVal;
        rStream >> nVal;
        return new ScViewObjectModeItem( Which(), (ScVObjMode)nVal );
    }
}

// -----------------------------------------------------------------------
//      double
// -----------------------------------------------------------------------

ScDoubleItem::ScDoubleItem( USHORT nWhich, double nVal )
    :   SfxPoolItem ( nWhich ),
        nValue  ( nVal )
{
}

//------------------------------------------------------------------------

ScDoubleItem::ScDoubleItem( const ScDoubleItem& rItem )
    :   SfxPoolItem ( rItem )
{
        nValue = rItem.nValue;
}

//------------------------------------------------------------------------

String __EXPORT ScDoubleItem::GetValueText() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScDoubleItem"));
}

//------------------------------------------------------------------------

int __EXPORT ScDoubleItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );
    return int(nValue == ((const ScDoubleItem&)rItem).nValue);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScDoubleItem::Clone( SfxItemPool* ) const
{
    return new ScDoubleItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScDoubleItem::Create( SvStream& rStream, USHORT nVer ) const
{
    double nTmp=0;
    rStream >> nTmp;

    ScDoubleItem* pItem = new ScDoubleItem( Which(), nTmp );

    return pItem;
}

//------------------------------------------------------------------------

SvStream& __EXPORT ScDoubleItem::Store( SvStream& rStream, USHORT nVer ) const
{
    rStream << nValue;

    return rStream;
}

//------------------------------------------------------------------------

__EXPORT ScDoubleItem::~ScDoubleItem()
{
}

// -----------------------------------------------------------------------




