/*************************************************************************
 *
 *  $RCSfile: chgtrack.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: er $ $Date: 2001-02-16 17:48:54 $
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

#pragma hdrstop

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>        // SHL_CALC
#endif
#ifndef _STACK_HXX //autogen
#include <tools/stack.hxx>
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXISETHINT_HXX //autogen
#include <svtools/isethint.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "scerrors.hxx"
#include "scmod.hxx"        // SC_MOD
#include "inputopt.hxx"     // GetExpandRefs
#include "patattr.hxx"

#include "globstr.hrc"

#define SC_CHGTRACK_CXX
#include "chgtrack.hxx"

DECLARE_STACK( ScChangeActionStack, ScChangeAction* );

const USHORT nMemPoolChangeActionCellListEntry = (0x2000 - 64) / sizeof(ScChangeActionCellListEntry);
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry, nMemPoolChangeActionCellListEntry, nMemPoolChangeActionCellListEntry );

const USHORT nMemPoolChangeActionLinkEntry = (0x8000 - 64) / sizeof(ScChangeActionLinkEntry);
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry, nMemPoolChangeActionLinkEntry, nMemPoolChangeActionLinkEntry );

// loaded MSB > eigenes => inkompatibel
#define SC_CHGTRACK_FILEFORMAT_FIRST    0x0001
#define SC_CHGTRACK_FILEFORMAT  0x0001

// --- ScChangeAction ------------------------------------------------------

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScRange& rRange )
        :
        aBigRange( rRange ),
        pNext( NULL ),
        pPrev( NULL ),
        pLinkAny( NULL ),
        pLinkDeletedIn( NULL ),
        pLinkDeleted( NULL ),
        pLinkDependent( NULL ),
        nAction( 0 ),
        nRejectAction( 0 ),
        eType( eTypeP ),
        eState( SC_CAS_VIRGIN )
{
    aDateTime.ConvertToUTC();
}

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScBigRange& rRange,
                        const ULONG nTempAction, const ULONG nTempRejectAction,
                        const ScChangeActionState eTempState, const DateTime& aTempDateTime,
                        const String& aTempUser,  const String& aTempComment)
        :
        aBigRange( rRange ),
        pNext( NULL ),
        pPrev( NULL ),
        pLinkAny( NULL ),
        pLinkDeletedIn( NULL ),
        pLinkDeleted( NULL ),
        pLinkDependent( NULL ),
        nAction( nTempAction ),
        nRejectAction( nTempRejectAction ),
        eType( eTypeP ),
        eState( eTempState ),
        aDateTime( aTempDateTime ),
        aUser( aTempUser ),
        aComment( aTempComment )
{
}

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScBigRange& rRange,
                        const ULONG nTempAction)
        :
        aBigRange( rRange ),
        pNext( NULL ),
        pPrev( NULL ),
        pLinkAny( NULL ),
        pLinkDeletedIn( NULL ),
        pLinkDeleted( NULL ),
        pLinkDependent( NULL ),
        nAction( nTempAction ),
        nRejectAction( 0 ),
        eType( eTypeP ),
        eState( SC_CAS_VIRGIN )
{
    aDateTime.ConvertToUTC();
}

ScChangeAction::ScChangeAction( SvStream& rStrm, ScMultipleReadHeader& rHdr,
            ScChangeTrack* pTrack )
        :
        pNext( NULL ),
        pPrev( NULL ),
        pLinkAny( NULL ),
        pLinkDeletedIn( NULL ),
        pLinkDeleted( NULL ),
        pLinkDependent( NULL )
{
    // ScChangeTrack speichert aUser als Index auf Collection und eType selber
    UINT32 n32;
    UINT16 n16;
    rStrm >> aBigRange;
    rStrm >> n32; aDateTime.SetDate( n32 );
    rStrm >> n32; aDateTime.SetTime( n32 );
    rStrm >> n32; nAction = n32;
    rStrm >> n32; nRejectAction = n32;
    rStrm >> n16; eState = (ScChangeActionState) n16;
    rStrm.ReadByteString( aComment, rStrm.GetStreamCharSet() );
    // LinkEntries in zweiter Runde
}


ScChangeAction::~ScChangeAction()
{
    RemoveAllLinks();
}


BOOL ScChangeAction::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    // ScChangeTrack speichert aUser als Index auf Collection und eType selber
    rStrm << aBigRange;
    rStrm << (UINT32) aDateTime.GetDate();
    rStrm << (UINT32) aDateTime.GetTime();
    rStrm << (UINT32) nAction;
    rStrm << (UINT32) nRejectAction;
    rStrm << (UINT16) eState;
    rStrm.WriteByteString( aComment, rStrm.GetStreamCharSet() );
    // LinkEntries in zweiter Runde

    return TRUE;
}


BOOL ScChangeAction::StoreLinks( SvStream& rStrm ) const
{
    BOOL bOk = ScChangeAction::StoreLinkChain( pLinkDeleted, rStrm );
    bOk &= ScChangeAction::StoreLinkChain( pLinkDependent, rStrm );
    return bOk;
}


BOOL ScChangeAction::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
{
    BOOL bOk = ScChangeAction::LoadLinkChain( this, &pLinkDeleted, rStrm,
        pTrack, TRUE );
    bOk &= ScChangeAction::LoadLinkChain( this, &pLinkDependent, rStrm,
        pTrack, FALSE );
    return bOk;
}


BOOL ScChangeAction::IsVisible() const
{
    //! sequence order of execution is significant
    if ( IsRejected() || GetType() == SC_CAT_DELETE_TABS || IsDeletedIn() )
        return FALSE;
    if ( GetType() == SC_CAT_CONTENT )
        return ((ScChangeActionContent*)this)->IsTopContent();
    return TRUE;
}


BOOL ScChangeAction::IsTouchable() const
{
    //! sequence order of execution is significant
    if ( IsRejected() || GetType() == SC_CAT_REJECT || IsDeletedIn() )
        return FALSE;
    // content may reject and be touchable if on top
    if ( GetType() == SC_CAT_CONTENT )
        return ((ScChangeActionContent*)this)->IsTopContent();
    if ( IsRejecting() )
        return FALSE;
    return TRUE;
}


BOOL ScChangeAction::IsClickable() const
{
    //! sequence order of execution is significant
    if ( !IsVirgin() )
        return FALSE;
    if ( IsDeletedIn() )
        return FALSE;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContentCellType eCCT =
            ScChangeActionContent::GetContentCellType(
            ((ScChangeActionContent*)this)->GetNewCell() );
        if ( eCCT == SC_CACCT_MATREF )
            return FALSE;
        if ( eCCT == SC_CACCT_MATORG )
        {   // no Accept-Select if one of the references is in a deleted col/row
            const ScChangeActionLinkEntry* pL =
                ((ScChangeActionContent*)this)->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p && p->IsDeletedIn() )
                    return FALSE;
                pL = pL->GetNext();
            }
        }
        return TRUE;    // for Select() a content doesn't have to be touchable
    }
    return IsTouchable();   // Accept()/Reject() only on touchables
}


BOOL ScChangeAction::IsRejectable() const
{
    //! sequence order of execution is significant
    if ( !IsClickable() )
        return FALSE;
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( ((ScChangeActionContent*)this)->IsOldMatrixReference() )
            return FALSE;
        ScChangeActionContent* pNextContent =
            ((ScChangeActionContent*)this)->GetNextContent();
        if ( pNextContent == NULL )
            return TRUE;        // *this is TopContent
        return pNextContent->IsRejected();      // *this is next rejectable
    }
    return IsTouchable();
}


BOOL ScChangeAction::IsInternalRejectable() const
{
    //! sequence order of execution is significant
    if ( !IsVirgin() )
        return FALSE;
    if ( IsDeletedIn() )
        return FALSE;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContent* pNextContent =
            ((ScChangeActionContent*)this)->GetNextContent();
        if ( pNextContent == NULL )
            return TRUE;        // *this is TopContent
        return pNextContent->IsRejected();      // *this is next rejectable
    }
    return IsTouchable();
}


BOOL ScChangeAction::IsDialogRoot() const
{
    return IsInternalRejectable();      // only rejectables in root
}


BOOL ScChangeAction::IsDialogParent() const
{
    //! sequence order of execution is significant
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( !IsDialogRoot() )
            return FALSE;
        if ( ((ScChangeActionContent*)this)->IsMatrixOrigin() && HasDependent() )
            return TRUE;
        ScChangeActionContent* pPrevContent =
            ((ScChangeActionContent*)this)->GetPrevContent();
        return pPrevContent && pPrevContent->IsVirgin();
    }
    if ( HasDependent() )
        return IsDeleteType() ? TRUE : !IsDeletedIn();
    if ( HasDeleted() )
    {
        if ( IsDeleteType() )
        {
            if ( IsDialogRoot() )
                return TRUE;
            ScChangeActionLinkEntry* pL = pLinkDeleted;
            while ( pL )
            {
                ScChangeAction* p = pL->GetAction();
                if ( p && p->GetType() != eType )
                    return TRUE;
                pL = pL->GetNext();
            }
        }
        else
            return TRUE;
    }
    return FALSE;
}


BOOL ScChangeAction::IsMasterDelete() const
{
    if ( !IsDeleteType() )
        return FALSE;
    ScChangeActionDel* pDel = (ScChangeActionDel*) this;
    return pDel->IsMultiDelete() && (pDel->IsTopDelete() || pDel->IsRejectable());
}


void ScChangeAction::RemoveAllLinks()
{
    RemoveAllAnyLinks();
    RemoveAllDeletedIn();
    RemoveAllDeleted();
    RemoveAllDependent();
}


void ScChangeAction::RemoveLink( ScChangeAction* p )
{
    ScChangeActionLinkEntry* pL = pLinkAny;
    while ( pL )
    {
        ScChangeActionLinkEntry* pNextLink = pL->GetNext();
        if ( pL->GetAction() == p )
            delete pL;
        pL = pNextLink;
    }
}


void ScChangeAction::RemoveAllAnyLinks()
{
    while ( pLinkAny )
        delete pLinkAny;        // rueckt sich selbst hoch
}


BOOL ScChangeAction::DependsOn( ScChangeAction* p ) const
{
    ScChangeActionLinkEntry* pL = pLinkAny;
    while ( pL )
    {
        if ( pL->GetAction() == p )
            return TRUE;
        pL = pL->GetNext();
    }
    return FALSE;
}


BOOL ScChangeAction::RemoveDeletedIn( const ScChangeAction* p )
{
    BOOL bRemoved = FALSE;
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    while ( pL )
    {
        ScChangeActionLinkEntry* pNextLink = pL->GetNext();
        if ( pL->GetAction() == p )
        {
            delete pL;
            bRemoved = TRUE;
        }
        pL = pNextLink;
    }
    return bRemoved;
}


BOOL ScChangeAction::IsDeletedIn( const ScChangeAction* p ) const
{
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    while ( pL )
    {
        if ( pL->GetAction() == p )
            return TRUE;
        pL = pL->GetNext();
    }
    return FALSE;
}


void ScChangeAction::RemoveAllDeletedIn()
{
    //! nicht vom evtl. TopContent sondern wirklich dieser
    while ( pLinkDeletedIn )
        delete pLinkDeletedIn;      // rueckt sich selbst hoch
}


BOOL ScChangeAction::IsDeletedInDelType( ScChangeActionType eDelType ) const
{
    ScChangeAction* p;
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    if ( pL )
    {
        // InsertType fuer MergePrepare/MergeOwn
        ScChangeActionType eInsType;
        switch ( eDelType )
        {
            case SC_CAT_DELETE_COLS :
                eInsType = SC_CAT_INSERT_COLS;
            break;
            case SC_CAT_DELETE_ROWS :
                eInsType = SC_CAT_INSERT_ROWS;
            break;
            case SC_CAT_DELETE_TABS :
                eInsType = SC_CAT_INSERT_TABS;
            break;
            default:
                eInsType = SC_CAT_NONE;
        }
        while ( pL )
        {
            if ( (p = pL->GetAction()) &&
                    (p->GetType() == eDelType || p->GetType() == eInsType) )
                return TRUE;
            pL = pL->GetNext();
        }
    }
    return FALSE;
}


void ScChangeAction::SetDeletedIn( ScChangeAction* p )
{
    ScChangeActionLinkEntry* pLink1 = AddDeletedIn( p );
    ScChangeActionLinkEntry* pLink2;
    if ( GetType() == SC_CAT_CONTENT )
        pLink2 = p->AddDeleted( ((ScChangeActionContent*)this)->GetTopContent() );
    else
        pLink2 = p->AddDeleted( this );
    pLink1->SetLink( pLink2 );
}


BOOL ScChangeAction::RemoveDeleted( const ScChangeAction* p )
{
    BOOL bRemoved = FALSE;
    ScChangeActionLinkEntry* pL = pLinkDeleted;
    while ( pL )
    {
        ScChangeActionLinkEntry* pNextLink = pL->GetNext();
        if ( pL->GetAction() == p )
        {
            delete pL;
            bRemoved = TRUE;
        }
        pL = pNextLink;
    }
    return bRemoved;
}


void ScChangeAction::RemoveAllDeleted()
{
    while ( pLinkDeleted )
        delete pLinkDeleted;        // rueckt sich selbst hoch
}


void ScChangeAction::RemoveDependent( ScChangeAction* p )
{
    ScChangeActionLinkEntry* pL = pLinkDependent;
    while ( pL )
    {
        ScChangeActionLinkEntry* pNextLink = pL->GetNext();
        if ( pL->GetAction() == p )
            delete pL;
        pL = pNextLink;
    }
}


void ScChangeAction::RemoveAllDependent()
{
    while ( pLinkDependent )
        delete pLinkDependent;      // rueckt sich selbst hoch
}


DateTime ScChangeAction::GetDateTime() const
{
    DateTime aDT( aDateTime );
    aDT.ConvertToLocalTime();
    return aDT;
}


void ScChangeAction::UpdateReference( const ScChangeTrack* pTrack,
        UpdateRefMode eMode, const ScBigRange& rRange,
        INT32 nDx, INT32 nDy, INT32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
}


String ScChangeAction::GetRefString( const ScBigRange& rRange,
        ScDocument* pDoc, BOOL bFlag3D ) const
{
    String aStr;
    USHORT nFlags = ( rRange.IsValid( pDoc ) ? SCA_VALID : 0 );
    if ( !nFlags )
        aStr = ScGlobal::GetRscString( STR_NOREF_STR );
    else
    {
        ScRange aTmpRange( rRange.MakeRange() );
        switch ( GetType() )
        {
            case SC_CAT_INSERT_COLS :
            case SC_CAT_DELETE_COLS :
                if ( bFlag3D )
                {
                    pDoc->GetName( aTmpRange.aStart.Tab(), aStr );
                    aStr += '.';
                }
                aStr += ::ColToAlpha( aTmpRange.aStart.Col() );
                aStr += ':';
                aStr += ::ColToAlpha( aTmpRange.aEnd.Col() );
            break;
            case SC_CAT_INSERT_ROWS :
            case SC_CAT_DELETE_ROWS :
                if ( bFlag3D )
                {
                    pDoc->GetName( aTmpRange.aStart.Tab(), aStr );
                    aStr += '.';
                }
                aStr += String::CreateFromInt32( aTmpRange.aStart.Row() + 1 );
                aStr += ':';
                aStr += String::CreateFromInt32( aTmpRange.aEnd.Row() + 1 );
            break;
            default:
                if ( bFlag3D || GetType() == SC_CAT_INSERT_TABS )
                    nFlags |= SCA_TAB_3D;
                aTmpRange.Format( aStr, nFlags, pDoc );
        }
        if ( (bFlag3D && IsDeleteType()) || IsDeletedIn() )
        {
            aStr.Insert( '(', 0 );
            aStr += ')';
        }
    }
    return aStr;
}


void ScChangeAction::GetRefString( String& rStr, ScDocument* pDoc,
        BOOL bFlag3D ) const
{
    rStr = GetRefString( GetBigRange(), pDoc, bFlag3D );
}


void ScChangeAction::Accept()
{
    if ( IsVirgin() )
    {
        SetState( SC_CAS_ACCEPTED );
        DeleteCellEntries();
    }
}


void ScChangeAction::SetRejected()
{
    if ( IsVirgin() )
    {
        SetState( SC_CAS_REJECTED );
        RemoveAllLinks();
        DeleteCellEntries();
    }
}


void ScChangeAction::RejectRestoreContents( ScChangeTrack* pTrack,
        short nDx, short nDy )
{
    // Liste der Contents aufbauen
    ScChangeActionCellListEntry* pListContents = NULL;
    for ( ScChangeActionLinkEntry* pL = pLinkDeleted; pL; pL = pL->GetNext() )
    {
        ScChangeAction* p = pL->GetAction();
        if ( p && p->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
                (ScChangeActionContent*) p, pListContents );
            pListContents = pE;
        }
    }
    SetState( SC_CAS_REJECTED );        // vor UpdateReference fuer Move
    pTrack->UpdateReference( this, TRUE );      // LinkDeleted freigeben
    DBG_ASSERT( !pLinkDeleted, "ScChangeAction::RejectRestoreContents: pLinkDeleted != NULL" );
    // Liste der Contents abarbeiten und loeschen
    ScDocument* pDoc = pTrack->GetDocument();
    ScChangeActionCellListEntry* pE = pListContents;
    while ( pE )
    {
        if ( !pE->pContent->IsDeletedIn() &&
                pE->pContent->GetBigRange().aStart.IsValid( pDoc ) )
            pE->pContent->PutNewValueToDoc( pDoc, nDx, nDy );
        ScChangeActionCellListEntry* pNext;
        pNext = pE->pNext;
        delete pE;
        pE = pNext;
    }
    DeleteCellEntries();        // weg mit den generierten
}


// static
void ScChangeAction::StoreCell( ScBaseCell* pCell, SvStream& rStrm,
        ScMultipleWriteHeader& rHdr )
{
    if ( pCell )
    {
        CellType eCellType = pCell->GetCellType();
        switch( eCellType )
        {
            case CELLTYPE_VALUE:
                rStrm << (BYTE) eCellType;
                ((ScValueCell*)pCell)->Save( rStrm );
            break;
            case CELLTYPE_STRING:
                rStrm << (BYTE) eCellType;
                ((ScStringCell*)pCell)->Save( rStrm );
            break;
            case CELLTYPE_EDIT:
                rStrm << (BYTE) eCellType;
                ((ScEditCell*)pCell)->Save( rStrm );
            break;
            case CELLTYPE_FORMULA:
                rStrm << (BYTE) eCellType;
                rStrm << ((ScFormulaCell*)pCell)->aPos;
                ((ScFormulaCell*)pCell)->Save( rStrm, rHdr );
            break;
            default:
                DBG_ERROR( "ScChangeAction::StoreCell: unknown CellType" );
                rStrm << (BYTE) CELLTYPE_NONE;
        }
    }
    else
        rStrm << (BYTE) CELLTYPE_NONE;
}


// static
ScBaseCell* ScChangeAction::LoadCell( SvStream& rStrm,
        ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer )
{
    ScBaseCell* pCell;
    BYTE nByte;
    rStrm >> nByte;
    switch ( (CellType) nByte )
    {
        case CELLTYPE_VALUE:
        {
            pCell = new ScValueCell( rStrm, nVer );
        }
        break;
        case CELLTYPE_STRING:
        {
            pCell = new ScStringCell( rStrm, nVer );
        }
        break;
        case CELLTYPE_EDIT:
        {
            pCell = new ScEditCell( rStrm, nVer, pDoc );
        }
        break;
        case CELLTYPE_FORMULA:
        {
            ScAddress aPos;
            rStrm >> aPos;
            pCell = new ScFormulaCell( pDoc, aPos, rStrm, rHdr );
            ((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
        }
        break;
        case CELLTYPE_NONE :
            pCell = NULL;
        break;
        default:
            DBG_ERROR( "ScChangeAction::LoadCell: unknown CellType" );
            rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            pCell = NULL;
    }
    return pCell;
}


// static
BOOL ScChangeAction::StoreLinkChain( ScChangeActionLinkEntry* pLinkFirst,
        SvStream& rStrm )
{
    BOOL bOk = TRUE;
    UINT32 nCount = 0;
    if ( pLinkFirst )
    {
        // rueckwaerts speichern, damit onLoad mit Insert die Reihenfolge wieder stimmt
        Stack* pStack = new Stack;
        for ( ScChangeActionLinkEntry* pL = pLinkFirst; pL; pL = pL->GetNext() )
        {
            ++nCount;
            pStack->Push( pL );
        }
        rStrm << nCount;
        ScChangeActionLinkEntry* pHere;
        while ( pHere = (ScChangeActionLinkEntry*) pStack->Pop() )
        {
            ScChangeAction* p = pHere->GetAction();
            rStrm << (UINT32) ( p ? p->GetActionNumber() : 0 );
        }
        delete pStack;
    }
    else
        rStrm << nCount;
    return bOk;
}


// static
BOOL ScChangeAction::LoadLinkChain( ScChangeAction* pOfAction,
        ScChangeActionLinkEntry** ppLinkFirst, SvStream& rStrm,
        ScChangeTrack* pTrack, BOOL bLinkDeleted )
{
    BOOL bOk = TRUE;
    UINT32 nCount;
    rStrm >> nCount;
    for ( UINT32 j = 0; j < nCount; j++ )
    {
        ScChangeAction* pAct = NULL;
        UINT32 nAct;
        rStrm >> nAct;
        if ( nAct )
        {
            pAct = pTrack->GetActionOrGenerated( nAct );
            DBG_ASSERT( pAct, "ScChangeAction::LoadLinkChain: missing Action" );
        }
        if ( bLinkDeleted )
        {
            if ( pAct )
                pAct->SetDeletedIn( pOfAction );
        }
        else
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                ppLinkFirst, pAct );
            if ( pAct )
                pAct->AddLink( pOfAction, pLink );
        }
    }
    return bOk;
}


void ScChangeAction::SetDeletedInThis( ULONG nActionNumber,
        const ScChangeTrack* pTrack )
{
    if ( nActionNumber )
    {
        ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
        DBG_ASSERT( pAct, "ScChangeAction::SetDeletedInThis: missing Action" );
        if ( pAct )
            pAct->SetDeletedIn( this );
    }
}


void ScChangeAction::AddDependent( ULONG nActionNumber,
        const ScChangeTrack* pTrack )
{
    if ( nActionNumber )
    {
        ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
        DBG_ASSERT( pAct, "ScChangeAction::AddDependent: missing Action" );
        if ( pAct )
        {
            ScChangeActionLinkEntry* pLink = AddDependent( pAct );
            pAct->AddLink( this, pLink );
        }
    }
}

// static
BOOL ScChangeAction::StoreCellList( ScChangeActionCellListEntry* pFirstCell,
        SvStream& rStrm )
{
    UINT32 nCount = 0;
    for ( const ScChangeActionCellListEntry* pE = pFirstCell; pE;
            pE = pE->pNext )
        ++nCount;
    rStrm << nCount;

    if ( nCount )
    {
        for ( const ScChangeActionCellListEntry* pE = pFirstCell; pE;
                pE = pE->pNext )
        {   // Store/Load vertauscht die Reihenfolge, aber das ist hierbei egal
            rStrm << (UINT32) pE->pContent->GetActionNumber();
        }
    }
    return TRUE;
}


BOOL ScChangeAction::LoadCellList( ScChangeAction* pOfAction,
        ScChangeActionCellListEntry*& pFirstCell, SvStream& rStrm,
        ScChangeTrack* pTrack )
{
    UINT32 nCount;
    rStrm >> nCount;
    if ( nCount )
    {
        for ( UINT32 j = 0; j < nCount; j++ )
        {
            ScChangeActionContent* pContent;
            UINT32 nContent;
            rStrm >> nContent;
            pContent = (ScChangeActionContent*) pTrack->GetActionOrGenerated( nContent );
            if ( pContent )
                pOfAction->AddContent( pContent );
            else
                DBG_ERROR( "ScChangeActionDel::LoadLinks: missing Content" );
        }
    }
    return TRUE;
}


// --- ScChangeActionIns ---------------------------------------------------

ScChangeActionIns::ScChangeActionIns( const ScRange& rRange )
        : ScChangeAction( SC_CAT_NONE, rRange )
{
    if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL )
    {
        aBigRange.aStart.SetCol( nInt32Min );
        aBigRange.aEnd.SetCol( nInt32Max );
        if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
        {
            SetType( SC_CAT_INSERT_TABS );
            aBigRange.aStart.SetRow( nInt32Min );
            aBigRange.aEnd.SetRow( nInt32Max );
        }
        else
            SetType( SC_CAT_INSERT_ROWS );
    }
    else if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
    {
        SetType( SC_CAT_INSERT_COLS );
        aBigRange.aStart.SetRow( nInt32Min );
        aBigRange.aEnd.SetRow( nInt32Max );
    }
    else
        DBG_ERROR( "ScChangeActionIns: Block not supported!" );
}


ScChangeActionIns::ScChangeActionIns( SvStream& rStrm,
            ScMultipleReadHeader& rHdr, ScChangeTrack* pTrack )
        :
        ScChangeAction( rStrm, rHdr, pTrack )
{
}

ScChangeActionIns::ScChangeActionIns(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
                                                const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String& sComment,
                                                const ScChangeActionType eType)
        :
        ScChangeAction(eType, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment)
{
}

ScChangeActionIns::~ScChangeActionIns()
{
}


BOOL ScChangeActionIns::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
    return bOk;
}


void ScChangeActionIns::GetDescription( String& rStr, ScDocument* pDoc,
        BOOL bSplitRange ) const
{
    USHORT nWhatId;
    switch ( GetType() )
    {
        case SC_CAT_INSERT_COLS :
            nWhatId = STR_COLUMN;
        break;
        case SC_CAT_INSERT_ROWS :
            nWhatId = STR_ROW;
        break;
        default:
            nWhatId = STR_AREA;
    }

    String aRsc( ScGlobal::GetRscString( STR_CHANGED_INSERT ) );
    xub_StrLen nPos = aRsc.SearchAscii( "#1" );
    rStr += aRsc.Copy( 0, nPos );
    rStr += ScGlobal::GetRscString( nWhatId );
    rStr += ' ';
    rStr += GetRefString( GetBigRange(), pDoc );
    rStr += aRsc.Copy( nPos+2 );
}


BOOL ScChangeActionIns::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return FALSE;

    ScRange aRange( aBigRange.MakeRange() );
    if ( !pDoc->IsBlockEditable( aRange.aStart.Tab(), aRange.aStart.Col(),
            aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() ) )
        return FALSE;

    switch ( GetType() )
    {
        case SC_CAT_INSERT_COLS :
            pDoc->DeleteCol( aRange );
        break;
        case SC_CAT_INSERT_ROWS :
            pDoc->DeleteRow( aRange );
        break;
        case SC_CAT_INSERT_TABS :
            pDoc->DeleteTab( aRange.aStart.Tab() );
        break;
    }
    SetState( SC_CAS_REJECTED );
    RemoveAllLinks();
    return TRUE;
}


// --- ScChangeActionDel ---------------------------------------------------

ScChangeActionDel::ScChangeActionDel( const ScRange& rRange,
            short nDxP, short nDyP, ScChangeTrack* pTrackP )
        :
        ScChangeAction( SC_CAT_NONE, rRange ),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        pLinkMove( NULL ),
        pCutOff( NULL ),
        nCutOff( 0 ),
        nDx( nDxP ),
        nDy( nDyP )
{
    if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL )
    {
        aBigRange.aStart.SetCol( nInt32Min );
        aBigRange.aEnd.SetCol( nInt32Max );
        if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
        {
            SetType( SC_CAT_DELETE_TABS );
            aBigRange.aStart.SetRow( nInt32Min );
            aBigRange.aEnd.SetRow( nInt32Max );
        }
        else
            SetType( SC_CAT_DELETE_ROWS );
    }
    else if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
    {
        SetType( SC_CAT_DELETE_COLS );
        aBigRange.aStart.SetRow( nInt32Min );
        aBigRange.aEnd.SetRow( nInt32Max );
    }
    else
        DBG_ERROR( "ScChangeActionDel: Block not supported!" );
}


ScChangeActionDel::ScChangeActionDel( SvStream& rStrm,
            ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer,
            ScChangeTrack* pTrackP )
        :
        ScChangeAction( rStrm, rHdr, pTrackP ),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        pLinkMove( NULL )
{
    UINT32 n32;
    INT16 n16s;
    rStrm >> n32; pCutOff = (ScChangeActionIns*)(ULONG) n32;
    rStrm >> n16s; nCutOff = n16s;
    rStrm >> n16s; nDx = n16s;
    rStrm >> n16s; nDy = n16s;
}

ScChangeActionDel::ScChangeActionDel(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
                                    const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String &sComment,
                                    const ScChangeActionType eType, const short nD, ScChangeTrack* pTrackP) // wich of nDx and nDy is set is depend on the type
        :
        ScChangeAction(eType, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        pLinkMove( NULL ),
        pCutOff( NULL ),
        nCutOff( 0 ),
        nDx( 0 ),
        nDy( 0 )
{
    if (eType == SC_CAT_DELETE_COLS)
        nDx = nD;
    else if (eType == SC_CAT_DELETE_ROWS)
        nDy = nD;
}

ScChangeActionDel::~ScChangeActionDel()
{
    DeleteCellEntries();
    while ( pLinkMove )
        delete pLinkMove;
}


BOOL ScChangeActionDel::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
    rStrm << (UINT32) ( pCutOff ? pCutOff->GetActionNumber() : 0 );
    rStrm << (INT16) nCutOff;
    rStrm << (INT16) nDx;
    rStrm << (INT16) nDy;
    return bOk;
}


BOOL ScChangeActionDel::StoreLinks( SvStream& rStrm ) const
{
    BOOL bOk = ScChangeAction::StoreLinks( rStrm );
    UINT32 nCount = 0;
    if ( pLinkMove )
    {
        // rueckwaerts speichern, damit onLoad mit Insert die Reihenfolge wieder stimmt
        Stack* pStack = new Stack;
        for ( ScChangeActionDelMoveEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
        {
            ++nCount;
            pStack->Push( pL );
        }
        rStrm << nCount;
        ScChangeActionDelMoveEntry* pHere;
        while ( pHere = (ScChangeActionDelMoveEntry*) pStack->Pop() )
        {
            ScChangeAction* p = pHere->GetAction();
            rStrm << (UINT32) ( p ? p->GetActionNumber() : 0 );
            rStrm << (INT16) pHere->GetCutOffFrom();
            rStrm << (INT16) pHere->GetCutOffTo();
        }
        delete pStack;
    }
    else
        rStrm << nCount;

    bOk &= ScChangeAction::StoreCellList( pFirstCell, rStrm );

    return bOk;
}


BOOL ScChangeActionDel::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
{
    BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
    UINT32 nCount;
    rStrm >> nCount;
    for ( UINT32 j = 0; j < nCount; j++ )
    {
        ScChangeActionMove* pAct = NULL;
        UINT32 nAct;
        rStrm >> nAct;
        if ( nAct )
        {
            pAct = (ScChangeActionMove*) pTrack->GetAction( nAct );
            DBG_ASSERT( pAct, "ScChangeActionDel::LoadLinks: missing Move" );
        }
        INT16 nFrom, nTo;
        rStrm >> nFrom >> nTo;
        ScChangeActionDelMoveEntry* pLink = new ScChangeActionDelMoveEntry(
            &pLinkMove, pAct, nFrom, nTo );
        if ( pAct )
            pAct->AddLink( this, pLink );
    }
    if ( pCutOff )
    {
        pCutOff = (ScChangeActionIns*) pTrack->GetAction( (ULONG) pCutOff );
        DBG_ASSERT( pCutOff, "ScChangeActionDel::LoadLinks: missing Insert" );
    }

    bOk &= ScChangeAction::LoadCellList( this, pFirstCell, rStrm, pTrack );

    return bOk;
}

void ScChangeActionDel::AddContent( ScChangeActionContent* pContent )
{
    ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
        pContent, pFirstCell );
    pFirstCell = pE;
}


void ScChangeActionDel::DeleteCellEntries()
{
    pTrack->DeleteCellEntries( pFirstCell, this );
}


BOOL ScChangeActionDel::IsBaseDelete() const
{
    return !GetDx() && !GetDy();
}


BOOL ScChangeActionDel::IsTopDelete() const
{
    const ScChangeAction* p = GetNext();
    if ( !p || p->GetType() != GetType() )
        return TRUE;
    return ((ScChangeActionDel*)p)->IsBaseDelete();
}


BOOL ScChangeActionDel::IsMultiDelete() const
{
    if ( GetDx() || GetDy() )
        return TRUE;
    const ScChangeAction* p = GetNext();
    if ( !p || p->GetType() != GetType() )
        return FALSE;
    const ScChangeActionDel* pDel = (const ScChangeActionDel*) p;
    if ( (pDel->GetDx() > GetDx() || pDel->GetDy() > GetDy()) &&
            pDel->GetBigRange() == aBigRange )
        return TRUE;
    return FALSE;
}


BOOL ScChangeActionDel::IsTabDeleteCol() const
{
    if ( GetType() != SC_CAT_DELETE_COLS )
        return FALSE;
    const ScChangeAction* p = this;
    while ( p && p->GetType() == SC_CAT_DELETE_COLS &&
            !((const ScChangeActionDel*)p)->IsTopDelete() )
        p = p->GetNext();
    return p && p->GetType() == SC_CAT_DELETE_TABS;
}


void ScChangeActionDel::UpdateReference( const ScChangeTrack* pTrack,
        UpdateRefMode eMode, const ScBigRange& rRange,
        INT32 nDx, INT32 nDy, INT32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
    if ( !IsDeletedIn() )
        return ;
    // evtl. in "druntergerutschten" anpassen
    for ( ScChangeActionLinkEntry* pL = pLinkDeleted; pL; pL = pL->GetNext() )
    {
        ScChangeAction* p = pL->GetAction();
        if ( p && p->GetType() == SC_CAT_CONTENT &&
                !GetBigRange().In( p->GetBigRange() ) )
        {
            switch ( GetType() )
            {
                case SC_CAT_DELETE_COLS :
                    p->GetBigRange().aStart.SetCol( GetBigRange().aStart.Col() );
                    p->GetBigRange().aEnd.SetCol( GetBigRange().aStart.Col() );
                break;
                case SC_CAT_DELETE_ROWS :
                    p->GetBigRange().aStart.SetRow( GetBigRange().aStart.Row() );
                    p->GetBigRange().aEnd.SetRow( GetBigRange().aStart.Row() );
                break;
                case SC_CAT_DELETE_TABS :
                    p->GetBigRange().aStart.SetTab( GetBigRange().aStart.Tab() );
                    p->GetBigRange().aEnd.SetTab( GetBigRange().aStart.Tab() );
                break;
            }
        }
    }
}


ScBigRange ScChangeActionDel::GetOverAllRange() const
{
    ScBigRange aTmpRange( GetBigRange() );
    aTmpRange.aEnd.SetCol( aTmpRange.aEnd.Col() + GetDx() );
    aTmpRange.aEnd.SetRow( aTmpRange.aEnd.Row() + GetDy() );
    return aTmpRange;
}


void ScChangeActionDel::GetDescription( String& rStr, ScDocument* pDoc,
        BOOL bSplitRange ) const
{
    USHORT nWhatId;
    switch ( GetType() )
    {
        case SC_CAT_DELETE_COLS :
            nWhatId = STR_COLUMN;
        break;
        case SC_CAT_DELETE_ROWS :
            nWhatId = STR_ROW;
        break;
        default:
            nWhatId = STR_AREA;
    }

    ScBigRange aTmpRange( GetBigRange() );
    if ( !IsRejected() )
    {
        if ( bSplitRange )
        {
            aTmpRange.aStart.SetCol( aTmpRange.aStart.Col() + GetDx() );
            aTmpRange.aStart.SetRow( aTmpRange.aStart.Row() + GetDy() );
        }
        aTmpRange.aEnd.SetCol( aTmpRange.aEnd.Col() + GetDx() );
        aTmpRange.aEnd.SetRow( aTmpRange.aEnd.Row() + GetDy() );
    }

    String aRsc( ScGlobal::GetRscString( STR_CHANGED_DELETE ) );
    xub_StrLen nPos = aRsc.SearchAscii( "#1" );
    rStr += aRsc.Copy( 0, nPos );
    rStr += ScGlobal::GetRscString( nWhatId );
    rStr += ' ';
    rStr += GetRefString( aTmpRange, pDoc );
    rStr += aRsc.Copy( nPos+2 );
}


BOOL ScChangeActionDel::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) && GetType() != SC_CAT_DELETE_TABS )
        return FALSE;

    BOOL bOk = TRUE;

    if ( IsTopDelete() )
    {   // den kompletten Bereich in einem Rutsch restaurieren
        ScBigRange aTmpRange( GetOverAllRange() );
        if ( !aTmpRange.IsValid( pDoc ) )
        {
            if ( GetType() == SC_CAT_DELETE_TABS )
            {   // wird Tab angehaengt?
                if ( aTmpRange.aStart.Tab() > pDoc->GetMaxTableNumber() )
                    bOk = FALSE;
            }
            else
                bOk = FALSE;
        }
        if ( bOk )
        {
            ScRange aRange( aTmpRange.MakeRange() );
            // InDelete... fuer Formel UpdateReference in Document
            pTrack->SetInDeleteRange( aRange );
            pTrack->SetInDeleteTop( TRUE );
            pTrack->SetInDeleteUndo( TRUE );
            pTrack->SetInDelete( TRUE );
            switch ( GetType() )
            {
                case SC_CAT_DELETE_COLS :
                    if ( !(aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL) )
                    {   // nur wenn nicht TabDelete
                        if ( bOk = pDoc->CanInsertCol( aRange ) )
                            bOk = pDoc->InsertCol( aRange );
                    }
                break;
                case SC_CAT_DELETE_ROWS :
                    if ( bOk = pDoc->CanInsertRow( aRange ) )
                        bOk = pDoc->InsertRow( aRange );
                break;
                case SC_CAT_DELETE_TABS :
                {
//2do: Tabellennamen merken?
                    String aName;
                    pDoc->CreateValidTabName( aName );
                    if ( bOk = pDoc->ValidNewTabName( aName ) )
                        bOk = pDoc->InsertTab( aRange.aStart.Tab(), aName );
                }
                break;
            }
            pTrack->SetInDelete( FALSE );
            pTrack->SetInDeleteUndo( FALSE );
        }
        if ( !bOk )
        {
            pTrack->SetInDeleteTop( FALSE );
            return FALSE;
        }
        // InDeleteTop fuer UpdateReference-Undo behalten
    }

    // setzt rejected und ruft UpdateReference-Undo und DeleteCellEntries
    RejectRestoreContents( pTrack, GetDx(), GetDy() );

    pTrack->SetInDeleteTop( FALSE );
    RemoveAllLinks();
    return TRUE;
}


void ScChangeActionDel::UndoCutOffMoves()
{   // abgeschnittene Moves wiederherstellen, Entries/Links deleten
    while ( pLinkMove )
    {
        ScChangeActionMove* pMove = pLinkMove->GetMove();
        short nFrom = pLinkMove->GetCutOffFrom();
        short nTo = pLinkMove->GetCutOffTo();
        switch ( GetType() )
        {
            case SC_CAT_DELETE_COLS :
                if ( nFrom > 0 )
                    pMove->GetFromRange().aStart.IncCol( -nFrom );
                else if ( nFrom < 0 )
                    pMove->GetFromRange().aEnd.IncCol( -nFrom );
                if ( nTo > 0 )
                    pMove->GetBigRange().aStart.IncCol( -nTo );
                else if ( nTo < 0 )
                    pMove->GetBigRange().aEnd.IncCol( -nTo );
            break;
            case SC_CAT_DELETE_ROWS :
                if ( nFrom > 0 )
                    pMove->GetFromRange().aStart.IncRow( -nFrom );
                else if ( nFrom < 0 )
                    pMove->GetFromRange().aEnd.IncRow( -nFrom );
                if ( nTo > 0 )
                    pMove->GetBigRange().aStart.IncRow( -nTo );
                else if ( nTo < 0 )
                    pMove->GetBigRange().aEnd.IncRow( -nTo );
            break;
            case SC_CAT_DELETE_TABS :
                if ( nFrom > 0 )
                    pMove->GetFromRange().aStart.IncTab( -nFrom );
                else if ( nFrom < 0 )
                    pMove->GetFromRange().aEnd.IncTab( -nFrom );
                if ( nTo > 0 )
                    pMove->GetBigRange().aStart.IncTab( -nTo );
                else if ( nTo < 0 )
                    pMove->GetBigRange().aEnd.IncTab( -nTo );
            break;
        }
        delete pLinkMove;       // rueckt sich selbst hoch
    }
}

void ScChangeActionDel::UndoCutOffInsert()
{   // abgeschnittenes Insert wiederherstellen
    if ( pCutOff )
    {
        switch ( pCutOff->GetType() )
        {
            case SC_CAT_INSERT_COLS :
                if ( nCutOff < 0 )
                    pCutOff->GetBigRange().aEnd.IncCol( -nCutOff );
                else
                    pCutOff->GetBigRange().aStart.IncCol( -nCutOff );
            break;
            case SC_CAT_INSERT_ROWS :
                if ( nCutOff < 0 )
                    pCutOff->GetBigRange().aEnd.IncRow( -nCutOff );
                else
                    pCutOff->GetBigRange().aStart.IncRow( -nCutOff );
            break;
            case SC_CAT_INSERT_TABS :
                if ( nCutOff < 0 )
                    pCutOff->GetBigRange().aEnd.IncTab( -nCutOff );
                else
                    pCutOff->GetBigRange().aStart.IncTab( -nCutOff );
            break;
        }
        SetCutOffInsert( NULL, 0 );
    }
}


// --- ScChangeActionMove --------------------------------------------------

ScChangeActionMove::ScChangeActionMove( SvStream& rStrm,
            ScMultipleReadHeader& rHdr, ScChangeTrack* pTrackP )
        :
        ScChangeAction( rStrm, rHdr, pTrackP ),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        nStartLastCut(0),
        nEndLastCut(0)
{
    rStrm >> aFromRange;
}

ScChangeActionMove::ScChangeActionMove(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
                                    const ScBigRange& aToBigRange, const String& aUser, const DateTime& aDateTime, const String &sComment,
                                    const ScBigRange& aFromBigRange, ScChangeTrack* pTrackP) // wich of nDx and nDy is set is depend on the type
        :
        ScChangeAction(SC_CAT_MOVE, aToBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment),
        aFromRange(aFromBigRange),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        nStartLastCut(0),
        nEndLastCut(0)
{
}

ScChangeActionMove::~ScChangeActionMove()
{
    DeleteCellEntries();
}


BOOL ScChangeActionMove::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
    rStrm << aFromRange;
    return bOk;
}


BOOL ScChangeActionMove::StoreLinks( SvStream& rStrm ) const
{
    BOOL bOk = ScChangeAction::StoreLinks( rStrm );
    bOk &= ScChangeAction::StoreCellList( pFirstCell, rStrm );
    return bOk;
}


BOOL ScChangeActionMove::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
{
    BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
    bOk &= ScChangeAction::LoadCellList( this, pFirstCell, rStrm, pTrack );
    return bOk;
}


void ScChangeActionMove::AddContent( ScChangeActionContent* pContent )
{
    ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
        pContent, pFirstCell );
    pFirstCell = pE;
}


void ScChangeActionMove::DeleteCellEntries()
{
    pTrack->DeleteCellEntries( pFirstCell, this );
}


void ScChangeActionMove::UpdateReference( const ScChangeTrack* pTrack,
        UpdateRefMode eMode, const ScBigRange& rRange,
        INT32 nDx, INT32 nDy, INT32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aFromRange );
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
}


void ScChangeActionMove::GetDelta( INT32& nDx, INT32& nDy, INT32& nDz ) const
{
    const ScBigAddress& rToPos = GetBigRange().aStart;
    const ScBigAddress& rFromPos = GetFromRange().aStart;
    nDx = rToPos.Col() - rFromPos.Col();
    nDy = rToPos.Row() - rFromPos.Row();
    nDz = rToPos.Tab() - rFromPos.Tab();
}


void ScChangeActionMove::GetDescription( String& rStr, ScDocument* pDoc,
        BOOL bSplitRange ) const
{
    BOOL bFlag3D = ( GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab() );

    String aRsc( ScGlobal::GetRscString( STR_CHANGED_MOVE ) );

    xub_StrLen nPos = 0;
    String aTmpStr = ScChangeAction::GetRefString( GetFromRange(), pDoc, bFlag3D );
    nPos = aRsc.SearchAscii( "#1", nPos );
    aRsc.Erase( nPos, 2 );
    aRsc.Insert( aTmpStr, nPos );
    nPos += aTmpStr.Len();

    aTmpStr = ScChangeAction::GetRefString( GetBigRange(), pDoc, bFlag3D );
    nPos = aRsc.SearchAscii( "#2", nPos );
    aRsc.Erase( nPos, 2 );
    aRsc.Insert( aTmpStr, nPos );
    nPos += aTmpStr.Len();

    rStr += aRsc;
}


void ScChangeActionMove::GetRefString( String& rStr, ScDocument* pDoc,
        BOOL bFlag3D ) const
{
    if ( !bFlag3D )
        bFlag3D = ( GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab() );
    rStr = ScChangeAction::GetRefString( GetFromRange(), pDoc, bFlag3D );
    rStr += ',';
    rStr += ' ';
    rStr += ScChangeAction::GetRefString( GetBigRange(), pDoc, bFlag3D );
}


BOOL ScChangeActionMove::Reject( ScDocument* pDoc )
{
    if ( !(aBigRange.IsValid( pDoc ) && aFromRange.IsValid( pDoc )) )
        return FALSE;

    ScRange aToRange( aBigRange.MakeRange() );
    ScRange aFrmRange( aFromRange.MakeRange() );

    BOOL bOk = pDoc->IsBlockEditable( aToRange.aStart.Tab(),
        aToRange.aStart.Col(), aToRange.aStart.Row(),
        aToRange.aEnd.Col(), aToRange.aEnd.Row() );
    if ( bOk )
        bOk = pDoc->IsBlockEditable( aFrmRange.aStart.Tab(),
            aFrmRange.aStart.Col(), aFrmRange.aStart.Row(),
            aFrmRange.aEnd.Col(), aFrmRange.aEnd.Row() );
    if ( !bOk )
        return FALSE;

    pTrack->LookUpContents( aToRange, pDoc, 0, 0, 0 );  // zu movende Contents

    pDoc->DeleteAreaTab( aToRange, IDF_ALL );
    pDoc->DeleteAreaTab( aFrmRange, IDF_ALL );
    // Formeln im Dokument anpassen
    pDoc->UpdateReference( URM_MOVE,
        aFrmRange.aStart.Col(), aFrmRange.aStart.Row(), aFrmRange.aStart.Tab(),
        aFrmRange.aEnd.Col(), aFrmRange.aEnd.Row(), aFrmRange.aEnd.Tab(),
        (short) aFrmRange.aStart.Col() - aToRange.aStart.Col(),
        (short) aFrmRange.aStart.Row() - aToRange.aStart.Row(),
        (short) aFrmRange.aStart.Tab() - aToRange.aStart.Tab(), NULL );

    // LinkDependent freigeben, nachfolgendes UpdateReference-Undo setzt
    // ToRange->FromRange Dependents
    RemoveAllDependent();

    // setzt rejected und ruft UpdateReference-Undo und DeleteCellEntries
    RejectRestoreContents( pTrack, 0, 0 );

    while ( pLinkDependent )
    {
        ScChangeAction* p = pLinkDependent->GetAction();
        if ( p && p->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = (ScChangeActionContent*) p;
            if ( !pContent->IsDeletedIn() &&
                    pContent->GetBigRange().aStart.IsValid( pDoc ) )
                pContent->PutNewValueToDoc( pDoc, 0, 0 );
            // in LookUpContents generierte loeschen
            if ( pTrack->IsGenerated( pContent->GetActionNumber() ) &&
                    !pContent->IsDeletedIn() )
            {
                pLinkDependent->UnLink();       //! sonst wird der mitgeloescht
                pTrack->DeleteGeneratedDelContent( pContent );
            }
        }
        delete pLinkDependent;
    }

    RemoveAllLinks();
    return TRUE;
}


// --- ScChangeActionContent -----------------------------------------------

const USHORT nMemPoolChangeActionContent = (0x8000 - 64) / sizeof(ScChangeActionContent);
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent, nMemPoolChangeActionContent, nMemPoolChangeActionContent );


ScChangeActionContent::ScChangeActionContent( SvStream& rStrm,
            ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer,
            ScChangeTrack* pTrack )
        :
        ScChangeAction( rStrm, rHdr, pTrack ),
        pNextInSlot( NULL ),
        ppPrevInSlot( NULL )
{
    UINT32 n32;
    rStrm.ReadByteString( aOldValue, rStrm.GetStreamCharSet() );
    rStrm.ReadByteString( aNewValue, rStrm.GetStreamCharSet() );
    rStrm >> n32; pNextContent = (ScChangeActionContent*)(ULONG) n32;
    rStrm >> n32; pPrevContent = (ScChangeActionContent*)(ULONG) n32;

    {
        ScMultipleReadHeader aDataHdr( rStrm );
        pOldCell = ScChangeAction::LoadCell( rStrm, aDataHdr, pDoc, nVer );
        pNewCell = ScChangeAction::LoadCell( rStrm, aDataHdr, pDoc, nVer );
    }
}

ScChangeActionContent::ScChangeActionContent( const ULONG nActionNumber,
            const ScChangeActionState eState, const ULONG nRejectingNumber,
            const ScBigRange& aBigRange, const String& aUser,
            const DateTime& aDateTime, const String& sComment,
            ScBaseCell* pTempOldCell, ScDocument* pDoc, const String& sResult )
        :
        ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber),
        pOldCell(pTempOldCell),
        pNewCell(NULL),
        pNextContent(NULL),
        pPrevContent(NULL),
        pNextInSlot(NULL),
        ppPrevInSlot(NULL),
        aOldValue(sResult)

{
    if (pOldCell)
        ScChangeActionContent::SetCell( aOldValue, pOldCell, 0, pDoc );
}

ScChangeActionContent::ScChangeActionContent( const ULONG nActionNumber,
            ScBaseCell* pTempNewCell, const ScBigRange& aBigRange,
            ScDocument* pDoc )
        :
        ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber),
        pNewCell(pTempNewCell),
        pOldCell(NULL),
        pNextContent(NULL),
        pPrevContent(NULL),
        pNextInSlot(NULL),
        ppPrevInSlot(NULL)
{
    if (pNewCell)
        ScChangeActionContent::SetCell( aNewValue, pNewCell, 0, pDoc );
}

ScChangeActionContent::~ScChangeActionContent()
{
    ClearTrack();
}


void ScChangeActionContent::ClearTrack()
{
    RemoveFromSlot();
    if ( pPrevContent )
        pPrevContent->pNextContent = pNextContent;
    if ( pNextContent )
        pNextContent->pPrevContent = pPrevContent;
}


BOOL ScChangeActionContent::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
    rStrm.WriteByteString( aOldValue, rStrm.GetStreamCharSet() );
    rStrm.WriteByteString( aNewValue, rStrm.GetStreamCharSet() );
    rStrm << (UINT32) ( pNextContent ? pNextContent->GetActionNumber() : 0 );
    rStrm << (UINT32) ( pPrevContent ? pPrevContent->GetActionNumber() : 0 );

    {
        ScMultipleWriteHeader aDataHdr( rStrm );
        ScChangeAction::StoreCell( pOldCell, rStrm, aDataHdr );
        ScChangeAction::StoreCell( pNewCell, rStrm, aDataHdr );
    }

    return bOk;
}


BOOL ScChangeActionContent::StoreLinks( SvStream& rStrm ) const
{
    BOOL bOk = ScChangeAction::StoreLinks( rStrm );
    return bOk;
}


BOOL ScChangeActionContent::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
{
    BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
    if ( pNextContent )
    {
        pNextContent = (ScChangeActionContent*) pTrack->GetAction(
            (ULONG) pNextContent );
        DBG_ASSERT( pNextContent,
            "ScChangeActionContent::LoadLinks: missing NextContent" );
    }
    if ( pPrevContent )
    {
        pPrevContent = (ScChangeActionContent*) pTrack->GetAction(
            (ULONG) pPrevContent );
        DBG_ASSERT( pPrevContent,
            "ScChangeActionContent::LoadLinks: missing PrevContent" );
    }
    return bOk;
}


ScChangeActionContent* ScChangeActionContent::GetTopContent() const
{
    if ( pNextContent )
    {
        ScChangeActionContent* pContent = pNextContent;
        while ( pContent->pNextContent )
            pContent = pContent->pNextContent;
        return pContent;
    }
    return (ScChangeActionContent*) this;
}


ScChangeActionLinkEntry* ScChangeActionContent::GetDeletedIn() const
{
    if ( pNextContent )
        return GetTopContent()->pLinkDeletedIn;
    return pLinkDeletedIn;
}


ScChangeActionLinkEntry** ScChangeActionContent::GetDeletedInAddress()
{
    if ( pNextContent )
        return GetTopContent()->GetDeletedInAddress();
    return &pLinkDeletedIn;
}


void ScChangeActionContent::SetOldValue( const ScBaseCell* pCell,
        const ScDocument* pFromDoc, ScDocument* pToDoc, ULONG nFormat )
{
    ScChangeActionContent::SetValue( aOldValue, pOldCell,
        nFormat, pCell, pFromDoc, pToDoc );
}


void ScChangeActionContent::SetOldValue( const ScBaseCell* pCell,
        const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    ScChangeActionContent::SetValue( aOldValue, pOldCell,
        aBigRange.aStart.MakeAddress(), pCell, pFromDoc, pToDoc );
}


void ScChangeActionContent::SetNewValue( const ScBaseCell* pCell,
        ScDocument* pDoc )
{
    ScChangeActionContent::SetValue( aNewValue, pNewCell,
        aBigRange.aStart.MakeAddress(), pCell, pDoc, pDoc );
}


void ScChangeActionContent::SetOldNewCells( ScBaseCell* pOldCellP,
                        ULONG nOldFormat, ScBaseCell* pNewCellP,
                        ULONG nNewFormat, ScDocument* pDoc )
{
    pOldCell = pOldCellP;
    pNewCell = pNewCellP;
    ScChangeActionContent::SetCell( aOldValue, pOldCell, nOldFormat, pDoc );
    ScChangeActionContent::SetCell( aNewValue, pNewCell, nNewFormat, pDoc );
}

void ScChangeActionContent::SetNewCell( ScBaseCell* pCell, ScDocument* pDoc )
{
    DBG_ASSERT( !pNewCell, "ScChangeActionContent::SetNewCell: overwriting existing cell" );
    pNewCell = pCell;
    ScChangeActionContent::SetCell( aNewValue, pNewCell, 0, pDoc );
}

void ScChangeActionContent::SetValueString( String& rValue, ScBaseCell*& pCell,
        const String& rStr, ScDocument* pDoc )
{
    if ( pCell )
    {
        pCell->Delete();
        pCell = NULL;
    }
    if ( rStr.Len() > 1 && rStr.GetChar(0) == '=' )
    {
        rValue.Erase();
        pCell = new ScFormulaCell(
            pDoc, aBigRange.aStart.MakeAddress(), rStr );
        ((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
    }
    else
        rValue = rStr;
}


void ScChangeActionContent::SetOldValue( const String& rOld, ScDocument* pDoc )
{
    SetValueString( aOldValue, pOldCell, rOld, pDoc );
}


void ScChangeActionContent::SetNewValue( const String& rNew, ScDocument* pDoc )
{
    SetValueString( aNewValue, pNewCell, rNew, pDoc );
}


void ScChangeActionContent::GetOldString( String& rStr ) const
{
    GetValueString( rStr, aOldValue, pOldCell );
}


void ScChangeActionContent::GetNewString( String& rStr ) const
{
    GetValueString( rStr, aNewValue, pNewCell );
}


void ScChangeActionContent::GetDescription( String& rStr, ScDocument* pDoc,
        BOOL bSplitRange ) const
{

    String aRsc( ScGlobal::GetRscString( STR_CHANGED_CELL ) );

    String aTmpStr;
    GetRefString( aTmpStr, pDoc );

    xub_StrLen nPos = 0;
    nPos = aRsc.SearchAscii( "#1", nPos );
    aRsc.Erase( nPos, 2 );
    aRsc.Insert( aTmpStr, nPos );
    nPos += aTmpStr.Len();

    GetOldString( aTmpStr );
    if ( !aTmpStr.Len() )
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );
    nPos = aRsc.SearchAscii( "#2", nPos );
    aRsc.Erase( nPos, 2 );
    aRsc.Insert( aTmpStr, nPos );
    nPos += aTmpStr.Len();

    GetNewString( aTmpStr );
    if ( !aTmpStr.Len() )
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );
    nPos = aRsc.SearchAscii( "#3", nPos );
    aRsc.Erase( nPos, 2 );
    aRsc.Insert( aTmpStr, nPos );

    rStr += aRsc;
}


void ScChangeActionContent::GetRefString( String& rStr, ScDocument* pDoc,
        BOOL bFlag3D ) const
{
    USHORT nFlags = ( GetBigRange().IsValid( pDoc ) ? SCA_VALID : 0 );
    if ( nFlags )
    {
        const ScBaseCell* pCell = GetNewCell();
        if ( ScChangeActionContent::GetContentCellType( pCell ) == SC_CACCT_MATORG )
        {
            ScBigRange aBigRange( GetBigRange() );
            USHORT nC, nR;
            ((const ScFormulaCell*)pCell)->GetMatColsRows( nC, nR );
            aBigRange.aEnd.IncCol( nC-1 );
            aBigRange.aEnd.IncRow( nR-1 );
            rStr = ScChangeAction::GetRefString( aBigRange, pDoc, bFlag3D );

            return ;
        }

        ScAddress aTmpAddress( GetBigRange().aStart.MakeAddress() );
        if ( bFlag3D )
            nFlags |= SCA_TAB_3D;
        aTmpAddress.Format( rStr, nFlags, pDoc );
        if ( IsDeletedIn() )
        {
            rStr.Insert( '(', 0 );
            rStr += ')';
        }
    }
    else
        rStr = ScGlobal::GetRscString( STR_NOREF_STR );
}


BOOL ScChangeActionContent::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return FALSE;

    PutOldValueToDoc( pDoc, 0, 0 );

    SetState( SC_CAS_REJECTED );
    RemoveAllLinks();

    return TRUE;
}


BOOL ScChangeActionContent::Select( ScDocument* pDoc, ScChangeTrack* pTrack,
        BOOL bOldest, Stack* pRejectActions )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return FALSE;

    ScChangeActionContent* pContent = this;
    // accept previous contents
    while ( pContent = pContent->pPrevContent )
    {
        if ( pContent->IsVirgin() )
            pContent->SetState( SC_CAS_ACCEPTED );
    }
    ScChangeActionContent* pEnd = pContent = this;
    // reject subsequent contents
    while ( pContent = pContent->pNextContent )
    {
        // MatrixOrigin may have dependents, no dependency recursion needed
        const ScChangeActionLinkEntry* pL = pContent->GetFirstDependentEntry();
        while ( pL )
        {
            ScChangeAction* p = (ScChangeAction*) pL->GetAction();
            if ( p )
                p->SetRejected();
            pL = pL->GetNext();
        }
        pContent->SetRejected();
        pEnd = pContent;
    }

    if ( bOldest || pEnd != this )
    {   // wenn nicht aeltester: ist es ueberhaupt ein anderer als der letzte?
        ScRange aRange( aBigRange.aStart.MakeAddress() );
        const ScAddress& rPos = aRange.aStart;

        ScChangeActionContent* pNew = new ScChangeActionContent( aRange );
        pNew->SetOldValue( pDoc->GetCell( rPos ), pDoc, pDoc );

        if ( bOldest )
            PutOldValueToDoc( pDoc, 0, 0 );
        else
            PutNewValueToDoc( pDoc, 0, 0 );

        pNew->SetRejectAction( bOldest ? GetActionNumber() : pEnd->GetActionNumber() );
        pNew->SetState( SC_CAS_ACCEPTED );
        if ( pRejectActions )
            pRejectActions->Push( pNew );
        else
        {
            pNew->SetNewValue( pDoc->GetCell( rPos ), pDoc );
            pTrack->Append( pNew );
        }
    }

    if ( bOldest )
        SetRejected();
    else
        SetState( SC_CAS_ACCEPTED );

    return TRUE;
}


// static
void ScChangeActionContent::GetStringOfCell( String& rStr,
        const ScBaseCell* pCell, const ScDocument* pDoc, const ScAddress& rPos )
{
    if ( pCell )
    {
        if ( ScChangeActionContent::NeedsNumberFormat( pCell ) )
            GetStringOfCell( rStr, pCell, pDoc, pDoc->GetNumberFormat( rPos ) );
        else
            GetStringOfCell( rStr, pCell, pDoc, 0 );
    }
    else
        rStr.Erase();
}


// static
void ScChangeActionContent::GetStringOfCell( String& rStr,
        const ScBaseCell* pCell, const ScDocument* pDoc, ULONG nFormat )
{
    if ( ScChangeActionContent::GetContentCellType( pCell ) )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE :
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                pDoc->GetFormatTable()->GetInputLineString( nValue, nFormat,
                    rStr );
            }
            break;
            case CELLTYPE_STRING :
                ((ScStringCell*)pCell)->GetString( rStr );
            break;
            case CELLTYPE_EDIT :
                ((ScEditCell*)pCell)->GetString( rStr );
            break;
            case CELLTYPE_FORMULA :
                ((ScFormulaCell*)pCell)->GetFormula( rStr );
            break;
            default:
                rStr.Erase();
        }
    }
    else
        rStr.Erase();
}


// static
ScChangeActionContentCellType ScChangeActionContent::GetContentCellType( const ScBaseCell* pCell )
{
    if ( pCell )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE :
            case CELLTYPE_STRING :
            case CELLTYPE_EDIT :
                return SC_CACCT_NORMAL;
            break;
            case CELLTYPE_FORMULA :
                switch ( ((const ScFormulaCell*)pCell)->GetMatrixFlag() )
                {
                    case MM_NONE :
                        return SC_CACCT_NORMAL;
                    break;
                    case MM_FORMULA :
                    case MM_FAKE :
                        return SC_CACCT_MATORG;
                    break;
                    case MM_REFERENCE :
                        return SC_CACCT_MATREF;
                    break;
                }
                return SC_CACCT_NORMAL;
            break;
            default:
                return SC_CACCT_NONE;
        }
    }
    return SC_CACCT_NONE;
}


// static
BOOL ScChangeActionContent::NeedsNumberFormat( const ScBaseCell* pCell )
{
    return pCell && pCell->GetCellType() == CELLTYPE_VALUE;
}


// static
void ScChangeActionContent::SetValue( String& rStr, ScBaseCell*& pCell,
        const ScAddress& rPos, const ScBaseCell* pOrgCell,
        const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    if ( ScChangeActionContent::NeedsNumberFormat( pOrgCell ) )
        ScChangeActionContent::SetValue( rStr, pCell,
            pFromDoc->GetNumberFormat( rPos ), pOrgCell, pFromDoc, pToDoc );
    else
        ScChangeActionContent::SetValue( rStr, pCell,
            0, pOrgCell, pFromDoc, pToDoc );
}


// static
void ScChangeActionContent::SetValue( String& rStr, ScBaseCell*& pCell,
        ULONG nFormat, const ScBaseCell* pOrgCell,
        const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    rStr.Erase();
    if ( pCell )
        pCell->Delete();
    if ( ScChangeActionContent::GetContentCellType( pOrgCell ) )
    {
        pCell = pOrgCell->Clone( pToDoc );
        switch ( pOrgCell->GetCellType() )
        {
            case CELLTYPE_VALUE :
            {   // z.B. Datum auch als solches merken
                double nValue = ((ScValueCell*)pOrgCell)->GetValue();
                pFromDoc->GetFormatTable()->GetInputLineString( nValue,
                    nFormat, rStr );
            }
            break;
            case CELLTYPE_FORMULA :
                ((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
            break;
        }
    }
    else
        pCell = NULL;
}


// static
void ScChangeActionContent::SetCell( String& rStr, ScBaseCell* pCell,
        ULONG nFormat, const ScDocument* pDoc )
{
    rStr.Erase();
    if ( pCell )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE :
            {   // e.g. remember date as date string
                double nValue = ((ScValueCell*)pCell)->GetValue();
                pDoc->GetFormatTable()->GetInputLineString( nValue,
                    nFormat, rStr );
            }
            break;
            case CELLTYPE_FORMULA :
                ((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
            break;
        }
    }
}


void ScChangeActionContent::GetValueString( String& rStr,
        const String& rValue, const ScBaseCell* pCell ) const
{
    if ( !rValue.Len() )
    {
        if ( pCell )
        {
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_STRING :
                    ((ScStringCell*)pCell)->GetString( rStr );
                break;
                case CELLTYPE_EDIT :
                    ((ScEditCell*)pCell)->GetString( rStr );
                break;
                case CELLTYPE_VALUE :   // ist immer in rValue
                    rStr = rValue;
                break;
                case CELLTYPE_FORMULA :
                    GetFormulaString( rStr, (ScFormulaCell*) pCell );
                break;
            }
        }
        else
            rStr.Erase();
    }
    else
        rStr = rValue;
}


void ScChangeActionContent::GetFormulaString( String& rStr,
        const ScFormulaCell* pCell ) const
{
    ScAddress aPos( aBigRange.aStart.MakeAddress() );
    if ( aPos == pCell->aPos || IsDeletedIn() )
        pCell->GetFormula( rStr );
    else
    {
        DBG_ERROR( "ScChangeActionContent::GetFormulaString: aPos != pCell->aPos" );
        ScFormulaCell* pNew = (ScFormulaCell*) pCell->Clone(
            pCell->GetDocument(), aPos, TRUE );     // TRUE: bNoListening
        pNew->GetFormula( rStr );
        delete pNew;
    }
}


void ScChangeActionContent::PutOldValueToDoc( ScDocument* pDoc,
        short nDx, short nDy ) const
{
    PutValueToDoc( pOldCell, aOldValue, pDoc, nDx, nDy );
}


void ScChangeActionContent::PutNewValueToDoc( ScDocument* pDoc,
        short nDx, short nDy ) const
{
    PutValueToDoc( pNewCell, aNewValue, pDoc, nDx, nDy );
}


void ScChangeActionContent::PutValueToDoc( ScBaseCell* pCell,
        const String& rValue, ScDocument* pDoc, short nDx, short nDy ) const
{
    ScAddress aPos( aBigRange.aStart.MakeAddress() );
    if ( nDx )
        aPos.IncCol( nDx );
    if ( nDy )
        aPos.IncRow( nDy );
    if ( !rValue.Len() )
    {
        if ( pCell )
        {
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE :   // ist immer in rValue
                    pDoc->SetString( aPos.Col(), aPos.Row(), aPos.Tab(), rValue );
                break;
                default:
                    switch ( ScChangeActionContent::GetContentCellType( pCell ) )
                    {
                        case SC_CACCT_MATORG :
                        {
                            USHORT nC, nR;
                            ((const ScFormulaCell*)pCell)->GetMatColsRows( nC, nR );
                            DBG_ASSERT( nC>0 && nR>0, "ScChangeActionContent::PutValueToDoc: MatColsRows?" );
                            ScRange aRange( aPos );
                            if ( nC > 1 )
                                aRange.aEnd.IncCol( nC-1 );
                            if ( nR > 1 )
                                aRange.aEnd.IncRow( nR-1 );
                            ScMarkData aDestMark;
                            aDestMark.SelectOneTable( aPos.Tab() );
                            aDestMark.SetMarkArea( aRange );
                            pDoc->InsertMatrixFormula( aPos.Col(), aPos.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(),
                                aDestMark, EMPTY_STRING,
                                ((const ScFormulaCell*)pCell)->GetCode() );
                        }
                        break;
                        case SC_CACCT_MATREF :
                            // nothing
                        break;
                        default:
                            pDoc->PutCell( aPos, pCell->Clone( pDoc ) );
                    }
            }
        }
        else
            pDoc->PutCell( aPos, NULL );
    }
    else
        pDoc->SetString( aPos.Col(), aPos.Row(), aPos.Tab(), rValue );
}


void lcl_InvalidateReference( ComplRefData& rRef, const ScBigAddress& rPos )
{
    if ( rPos.Col() < 0 || MAXCOL < rPos.Col() )
    {
        rRef.Ref1.nCol = (INT16)(~0);
        rRef.Ref1.nRelCol = (INT16)(~0);
        rRef.Ref1.SetColDeleted( TRUE );
        rRef.Ref2.nCol = (INT16)(~0);
        rRef.Ref2.nRelCol = (INT16)(~0);
        rRef.Ref2.SetColDeleted( TRUE );
    }
    if ( rPos.Row() < 0 || MAXROW < rPos.Row() )
    {
        rRef.Ref1.nRow = (INT16)(~0);
        rRef.Ref1.nRelRow = (INT16)(~0);
        rRef.Ref1.SetRowDeleted( TRUE );
        rRef.Ref2.nRow = (INT16)(~0);
        rRef.Ref2.nRelRow = (INT16)(~0);
        rRef.Ref2.SetRowDeleted( TRUE );
    }
    if ( rPos.Tab() < 0 || MAXTAB < rPos.Tab() )
    {
        rRef.Ref1.nTab = (INT16)(~0);
        rRef.Ref1.nRelTab = (INT16)(~0);
        rRef.Ref1.SetTabDeleted( TRUE );
        rRef.Ref2.nTab = (INT16)(~0);
        rRef.Ref2.nRelTab = (INT16)(~0);
        rRef.Ref2.SetTabDeleted( TRUE );
    }
}


void ScChangeActionContent::UpdateReference( const ScChangeTrack* pTrack,
        UpdateRefMode eMode, const ScBigRange& rRange,
        INT32 nDx, INT32 nDy, INT32 nDz )
{
    USHORT nOldSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aBigRange );
    USHORT nNewSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
    if ( nNewSlot != nOldSlot )
    {
        RemoveFromSlot();
        InsertInSlot( &(pTrack->GetContentSlots()[nNewSlot]) );
    }

    if ( pTrack->IsInDelete() && !pTrack->IsInDeleteTop() )
        return ;        // Formeln nur kompletten Bereich updaten

    BOOL bOldFormula = ( pOldCell && pOldCell->GetCellType() == CELLTYPE_FORMULA );
    BOOL bNewFormula = ( pNewCell && pNewCell->GetCellType() == CELLTYPE_FORMULA );
    if ( bOldFormula || bNewFormula )
    {   // via ScFormulaCell UpdateReference anpassen (dort)
        if ( pTrack->IsInDelete() )
        {
            const ScRange& rDelRange = pTrack->GetInDeleteRange();
            if ( nDx > 0 )
                nDx = rDelRange.aEnd.Col() - rDelRange.aStart.Col() + 1;
            else if ( nDx < 0 )
                nDx = -(rDelRange.aEnd.Col() - rDelRange.aStart.Col() + 1);
            if ( nDy > 0 )
                nDy = rDelRange.aEnd.Row() - rDelRange.aStart.Row() + 1;
            else if ( nDy < 0 )
                nDy = -(rDelRange.aEnd.Row() - rDelRange.aStart.Row() + 1);
            if ( nDz > 0 )
                nDz = rDelRange.aEnd.Tab() - rDelRange.aStart.Tab() + 1;
            else if ( nDz < 0 )
                nDz = -(rDelRange.aEnd.Tab() - rDelRange.aStart.Tab() + 1);
        }
        ScBigRange aTmpRange( rRange );
        switch ( eMode )
        {
            case URM_INSDEL :
                if ( nDx < 0 || nDy < 0 || nDz < 0 )
                {   // Delete startet dort hinter geloeschtem Bereich,
                    // Position wird dort angepasst.
                    if ( nDx )
                        aTmpRange.aStart.IncCol( -nDx );
                    if ( nDy )
                        aTmpRange.aStart.IncRow( -nDy );
                    if ( nDz )
                        aTmpRange.aStart.IncTab( -nDz );
                }
            break;
            case URM_MOVE :
                // Move ist hier Quelle, dort Ziel,
                // Position muss vorher angepasst sein.
                if ( bOldFormula )
                    ((ScFormulaCell*)pOldCell)->aPos = aBigRange.aStart.MakeAddress();
                if ( bNewFormula )
                    ((ScFormulaCell*)pNewCell)->aPos = aBigRange.aStart.MakeAddress();
                if ( nDx )
                {
                    aTmpRange.aStart.IncCol( nDx );
                    aTmpRange.aEnd.IncCol( nDx );
                }
                if ( nDy )
                {
                    aTmpRange.aStart.IncRow( nDy );
                    aTmpRange.aEnd.IncRow( nDy );
                }
                if ( nDz )
                {
                    aTmpRange.aStart.IncTab( nDz );
                    aTmpRange.aEnd.IncTab( nDz );
                }
            break;
        }
        ScRange aRange( aTmpRange.MakeRange() );
        if ( bOldFormula )
            ((ScFormulaCell*)pOldCell)->UpdateReference( eMode, aRange,
                (short) nDx, (short) nDy, (short) nDz, NULL );
        if ( bNewFormula )
            ((ScFormulaCell*)pNewCell)->UpdateReference( eMode, aRange,
                (short) nDx, (short) nDy, (short) nDz, NULL );
        if ( !aBigRange.aStart.IsValid( pTrack->GetDocument() ) )
        {   //! HACK!
            //! UpdateReference kann nicht mit Positionen ausserhalb des
            //! Dokuments umgehen, deswegen alles auf #REF! setzen
//2do: make it possible! das bedeutet grossen Umbau von ScAddress etc.!
            const ScBigAddress& rPos = aBigRange.aStart;
            if ( bOldFormula )
            {
                ScToken* t;
                ScTokenArray* pArr = ((ScFormulaCell*)pOldCell)->GetCode();
                pArr->Reset();
                while ( t = pArr->GetNextReference() )
                    lcl_InvalidateReference( t->aRef, rPos );
                pArr->Reset();
                while ( t = pArr->GetNextReferenceRPN() )
                    lcl_InvalidateReference( t->aRef, rPos );
            }
            if ( bNewFormula )
            {
                ScToken* t;
                ScTokenArray* pArr = ((ScFormulaCell*)pNewCell)->GetCode();
                pArr->Reset();
                while ( t = pArr->GetNextReference() )
                    lcl_InvalidateReference( t->aRef, rPos );
                pArr->Reset();
                while ( t = pArr->GetNextReferenceRPN() )
                    lcl_InvalidateReference( t->aRef, rPos );
            }
        }
    }
}


// --- ScChangeActionReject ------------------------------------------------

ScChangeActionReject::ScChangeActionReject( SvStream& rStrm,
            ScMultipleReadHeader& rHdr, ScChangeTrack* pTrack )
        :
        ScChangeAction( rStrm, rHdr, pTrack )
{
}

ScChangeActionReject::ScChangeActionReject(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
                                                const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String& sComment)
        :
        ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment)
{
}

BOOL ScChangeActionReject::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
{
    BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
    return TRUE;
}


// --- ScChangeTrack -------------------------------------------------------

IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo, 16, 16 );

const USHORT ScChangeTrack::nContentRowsPerSlot = InitContentRowsPerSlot();
const USHORT ScChangeTrack::nContentSlots =
    (MAXROW+1) / InitContentRowsPerSlot() + 2;

// static
USHORT ScChangeTrack::InitContentRowsPerSlot()
{
    const USHORT nMaxSlots = 0xffe0 / sizeof( ScChangeActionContent* ) - 2;
    USHORT nRowsPerSlot = (MAXROW+1) / nMaxSlots;
    if ( nRowsPerSlot * nMaxSlots < (MAXROW+1) )
        ++nRowsPerSlot;
    return nRowsPerSlot;
}


ScChangeTrack::ScChangeTrack( ScDocument* pDocP ) :
        pDoc( pDocP )
{
    Init();
    StartListening( *SfxGetpApp() );
    ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
    memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
}

ScChangeTrack::ScChangeTrack( ScDocument* pDocP, const StrCollection& aTempUserCollection) :
        pDoc( pDocP ),
        aUserCollection(aTempUserCollection)
{
    Init();
    StartListening( *SfxGetpApp() );
    ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
    memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
}

ScChangeTrack::~ScChangeTrack()
{
    DtorClear();
    delete ppContentSlots;
}


void ScChangeTrack::Init()
{
    pFirst = NULL;
    pLast = NULL;
    pFirstGeneratedDelContent = NULL;
    pLastCutMove = NULL;
    pLinkInsertCol = NULL;
    pLinkInsertRow = NULL;
    pLinkInsertTab = NULL;
    pLinkMove = NULL;
    pBlockModifyMsg = NULL;
    nActionMax = 0;
    nGeneratedMin = SC_CHGTRACK_GENERATED_START;
    nMarkLastSaved = 0;
    nStartLastCut = 0;
    nEndLastCut = 0;
    nLastMerge = 0;
    eMergeState = SC_CTMS_NONE;
    nLoadedFileFormatVersion = SC_CHGTRACK_FILEFORMAT;
    bLoadSave = FALSE;
    bInDelete = FALSE;
    bInDeleteTop = FALSE;
    bInDeleteUndo = FALSE;
    bInPasteCut = FALSE;
    bUseFixDateTime = FALSE;

    SvtUserOptions aUserOpt;
    aUser = aUserOpt.GetFirstName();
    aUser += ' ';
    aUser += aUserOpt.GetLastName();
    aUserCollection.Insert( new StrData( aUser ) );
}


void ScChangeTrack::DtorClear()
{
    ScChangeAction* p;
    ScChangeAction* pNext;
    for ( p = GetFirst(); p; p = pNext )
    {
        pNext = p->GetNext();
        delete p;
    }
    for ( p = pFirstGeneratedDelContent; p; p = pNext )
    {
        pNext = p->GetNext();
        delete p;
    }
    for ( p = aPasteCutTable.First(); p; p = aPasteCutTable.Next() )
    {
        delete p;
    }
    delete pLastCutMove;
    ClearMsgQueue();
}


void ScChangeTrack::ClearMsgQueue()
{
    if ( pBlockModifyMsg )
    {
        delete pBlockModifyMsg;
        pBlockModifyMsg = NULL;
    }
    ScChangeTrackMsgInfo* pMsgInfo;
    while ( pMsgInfo = aMsgStackTmp.Pop() )
        delete pMsgInfo;
    while ( pMsgInfo = aMsgStackFinal.Pop() )
        delete pMsgInfo;
    while ( pMsgInfo = aMsgQueue.Get() )
        delete pMsgInfo;
}


void ScChangeTrack::Clear()
{
    DtorClear();
    aTable.Clear();
    aGeneratedTable.Clear();
    aPasteCutTable.Clear();
    aUserCollection.FreeAll();
    aUser.Erase();
    Init();
}


void __EXPORT ScChangeTrack::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( !pDoc->IsInDtorClear() )
    {
        const SfxItemSetHint* pHint = PTR_CAST( SfxItemSetHint, &rHint );
        if ( pHint )
        {
            const SfxItemSet& rSet = pHint->GetItemSet();
            const SfxPoolItem* pItem;
            if ( rSet.GetItemState(
                    rSet.GetPool()->GetWhich( SID_ATTR_ADDRESS ),
                    TRUE, &pItem ) == SFX_ITEM_SET )
            {
                String aStr( ((SvxAddressItem*)pItem)->GetFirstName() );
                aStr += ' ';
                aStr += ((SvxAddressItem*)pItem)->GetName();
                SetUser( aStr );
            }
        }
    }
}


void ScChangeTrack::SetUser( const String& rUser )
{
    if ( IsLoadSave() )
        return ;        // nicht die Collection zerschiessen

    aUser = rUser;
    StrData* pStrData = new StrData( aUser );
    if ( !aUserCollection.Insert( pStrData ) )
        delete pStrData;
}


void ScChangeTrack::StartBlockModify( ScChangeTrackMsgType eMsgType,
        ULONG nStartAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( pBlockModifyMsg )
            aMsgStackTmp.Push( pBlockModifyMsg );   // Block im Block
        pBlockModifyMsg = new ScChangeTrackMsgInfo;
        pBlockModifyMsg->eMsgType = eMsgType;
        pBlockModifyMsg->nStartAction = nStartAction;
    }
}


void ScChangeTrack::EndBlockModify( ULONG nEndAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( pBlockModifyMsg )
        {
            if ( pBlockModifyMsg->nStartAction <= nEndAction )
            {
                pBlockModifyMsg->nEndAction = nEndAction;
                // Blocks in Blocks aufgeloest
                aMsgStackFinal.Push( pBlockModifyMsg );
            }
            else
                delete pBlockModifyMsg;
            pBlockModifyMsg = aMsgStackTmp.Pop();   // evtl. Block im Block
        }
        if ( !pBlockModifyMsg )
        {
            BOOL bNew = FALSE;
            ScChangeTrackMsgInfo* pMsg;
            while ( pMsg = aMsgStackFinal.Pop() )
            {
                aMsgQueue.Put( pMsg );
                bNew = TRUE;
            }
            if ( bNew )
                aModifiedLink.Call( this );
        }
    }
}


void ScChangeTrack::NotifyModified( ScChangeTrackMsgType eMsgType,
        ULONG nStartAction, ULONG nEndAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( !pBlockModifyMsg || pBlockModifyMsg->eMsgType != eMsgType ||
                (IsGenerated( nStartAction ) &&
                (eMsgType == SC_CTM_APPEND || eMsgType == SC_CTM_REMOVE)) )
        {   // Append innerhalb von Append z.B. nicht
            StartBlockModify( eMsgType, nStartAction );
            EndBlockModify( nEndAction );
        }
    }
}


BOOL ScChangeTrack::Load( SvStream& rStrm, USHORT nVer )
{
    BOOL bOk = TRUE;
    SetLoadSave( TRUE );

    ScReadHeader aGlobalHdr( rStrm );

    BYTE n8;
    UINT16 n16;
    UINT32 n32;

    rStrm >> n16; nLoadedFileFormatVersion = n16;
    if ( (nLoadedFileFormatVersion & 0xFF00) > (SC_CHGTRACK_FILEFORMAT & 0xFF00) )
    {   // inkompatible neuere Version
        Clear();
        rStrm.SetError( SCWARN_IMPORT_INFOLOST );
        return FALSE;
    }

    aUserCollection.Load( rStrm );

    ULONG nCount, nLastAction, nGeneratedCount;
    rStrm >> n32; nCount = n32;
    rStrm >> n32; nActionMax = n32;
    rStrm >> n32; nLastAction = n32;

    rStrm >> n32; nGeneratedCount = n32;

    // GeneratedDelContents laden
    {
        ScMultipleReadHeader aHdr( rStrm );
        for ( ULONG j = 0; j < nGeneratedCount && bOk; j++ )
        {
            ScChangeActionContent* pAct;

            aHdr.StartEntry();

            ScChangeActionType eType;
            rStrm >> n8; eType = (ScChangeActionType) n8;

            switch ( eType )
            {
                case SC_CAT_CONTENT :
                    pAct = new ScChangeActionContent( rStrm, aHdr, pDoc, nVer, this );
                break;
                default:
                    DBG_ERROR( "ScChangeTrack::Load: unknown GeneratedType" );
                    pAct = NULL;
                    bOk = FALSE;
            }

            aHdr.EndEntry();

            if ( pAct )
            {
                pAct->SetType( eType );
                if ( pFirstGeneratedDelContent )
                    pFirstGeneratedDelContent->pPrev = pAct;
                pAct->pNext = pFirstGeneratedDelContent;
                pFirstGeneratedDelContent = pAct;
                aGeneratedTable.Insert( pAct->GetActionNumber(), pAct );
            }
        }
        rStrm >> n32; nGeneratedMin = n32;
    }

    if ( bOk )
        bOk = ( nGeneratedCount == aGeneratedTable.Count() );
    DBG_ASSERT( bOk, "ScChangeTrack::Load: Generated failed" );


    // erste Runde: Actions laden
    {
        ScMultipleReadHeader aHdr( rStrm );
        for ( ULONG j = 0; j < nCount && bOk; j++ )
        {
            ScChangeAction* pAct;

            aHdr.StartEntry();

            USHORT nUserIndex;
            rStrm >> n16; nUserIndex = n16;

            ScChangeActionType eType;
            rStrm >> n8; eType = (ScChangeActionType) n8;

            switch ( eType )
            {
                case SC_CAT_INSERT_COLS :
                case SC_CAT_INSERT_ROWS :
                case SC_CAT_INSERT_TABS :
                    pAct = new ScChangeActionIns( rStrm, aHdr, this );
                break;
                case SC_CAT_DELETE_COLS :
                case SC_CAT_DELETE_ROWS :
                case SC_CAT_DELETE_TABS :
                    pAct = new ScChangeActionDel( rStrm, aHdr, pDoc, nVer, this );
                break;
                case SC_CAT_MOVE :
                    pAct = new ScChangeActionMove( rStrm, aHdr, this );
                break;
                case SC_CAT_CONTENT :
                    pAct = new ScChangeActionContent( rStrm, aHdr, pDoc, nVer, this );
                break;
                case SC_CAT_REJECT :
                    pAct = new ScChangeActionReject( rStrm, aHdr, this );
                break;
                default:
                    DBG_ERROR( "ScChangeTrack::Load: unknown ScChangeActionType" );
                    pAct = NULL;
                    bOk = FALSE;
            }

            aHdr.EndEntry();

            if ( pAct )
            {
                pAct->SetType( eType );
                if ( nUserIndex != 0xffff )
                {
                    StrData* pUser = (StrData*) aUserCollection.At( nUserIndex );
                    if ( pUser )
                        pAct->SetUser( pUser->GetString() );
                }
                AppendLoaded( pAct );
            }
        }
    }

    if ( pLast )
        nMarkLastSaved = pLast->GetActionNumber();

    if ( bOk )
        bOk = ( nMarkLastSaved == nLastAction && nCount == aTable.Count() );
    DBG_ASSERT( bOk, "ScChangeTrack::Load: failed" );

    // zweite Runde: Links laden und alles verpointern
    {
        ScMultipleReadHeader aHdr( rStrm );
        for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
        {
            aHdr.StartEntry();
            bOk = p->LoadLinks( rStrm, this );
            aHdr.EndEntry();
        }
    }

    SetLoadSave( FALSE );

    // den aktuellen User erst einfuegen, wenn die Actions bereits ihre User haben
    SetUser( aUser );

    if ( !bOk )
    {
        Clear();        // eindeutiger Zustand
        rStrm.SetError( SCWARN_IMPORT_INFOLOST );
    }

    return bOk;
}


BOOL ScChangeTrack::Store( SvStream& rStrm )
{
    BOOL bOk = TRUE;
    SetLoadSave( TRUE );

    ScWriteHeader aGlobalHdr( rStrm );

    rStrm << (UINT16) SC_CHGTRACK_FILEFORMAT;

    aUserCollection.Store( rStrm );

    ULONG nCount = aTable.Count();
    ULONG nLastAction = ( pLast ? pLast->GetActionNumber() : 0 );
    ULONG nGeneratedCount = aGeneratedTable.Count();
    rStrm << (UINT32) nCount << (UINT32) nActionMax << (UINT32) nLastAction;
    rStrm << (UINT32) nGeneratedCount;

    // GeneratedDelContents speichern
    ULONG nSave = 0;
    {
        ScMultipleWriteHeader aHdr( rStrm );
        ULONG nNewGeneratedMin = SC_CHGTRACK_GENERATED_START;
        for ( ScChangeAction* p = pFirstGeneratedDelContent; p && bOk;
                p = p->GetNext() )
        {
            ++nSave;
            aHdr.StartEntry();
            rStrm << (BYTE) p->GetType();
            bOk = p->Store( rStrm, aHdr );
            aHdr.EndEntry();
            ULONG nAct = p->GetActionNumber();
            if ( nNewGeneratedMin > nAct )
                nNewGeneratedMin = nAct;
        }
        nGeneratedMin = nNewGeneratedMin;   // evtl. unbenutzten Bereich freigeben
        rStrm << (UINT32) nGeneratedMin;
    }

    if ( bOk )
        bOk = ( nGeneratedCount == nSave );
    DBG_ASSERT( bOk, "ScChangeTrack::Store: failed" );

    // erste Runde: Actions speichern
    nSave = 0;
    {
        ScMultipleWriteHeader aHdr( rStrm );
        StrData* pUserSearch = new StrData( aUser );
        USHORT nUserIndex;
        for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
        {
            ++nSave;
            aHdr.StartEntry();

            pUserSearch->SetString( p->GetUser() );
            if ( aUserCollection.Search( pUserSearch, nUserIndex ) )
                rStrm << (UINT16) nUserIndex;
            else
                rStrm << (UINT16) 0xffff;
            rStrm << (BYTE) p->GetType();

            bOk = p->Store( rStrm, aHdr );

            aHdr.EndEntry();
        }
        delete pUserSearch;
    }

    if ( pLast )
        nMarkLastSaved = pLast->GetActionNumber();

    if ( bOk )
        bOk = ( nCount == nSave );
    DBG_ASSERT( bOk, "ScChangeTrack::Store: failed" );

    // zweite Runde: Links speichern
    {
        ScMultipleWriteHeader aHdr( rStrm );
        for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
        {
            aHdr.StartEntry();
            bOk = p->StoreLinks( rStrm );
            aHdr.EndEntry();
        }
    }

    SetLoadSave( FALSE );
    return bOk;
}


void ScChangeTrack::MasterLinks( ScChangeAction* pAppend )
{
    ScChangeActionType eType = pAppend->GetType();

    if ( eType == SC_CAT_CONTENT )
    {
        if ( !IsGenerated( pAppend->GetActionNumber() ) )
        {
            USHORT nSlot = ComputeContentSlot(
                pAppend->GetBigRange().aStart.Row() );
            ((ScChangeActionContent*)pAppend)->InsertInSlot(
                &ppContentSlots[nSlot] );
        }
        return ;
    }

    if ( pAppend->IsRejecting() )
        return ;        // Rejects haben keine Abhaengigkeiten

    switch ( eType )
    {
        case SC_CAT_INSERT_COLS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertCol, pAppend );
            pAppend->AddLink( NULL, pLink );
        }
        break;
        case SC_CAT_INSERT_ROWS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertRow, pAppend );
            pAppend->AddLink( NULL, pLink );
        }
        break;
        case SC_CAT_INSERT_TABS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertTab, pAppend );
            pAppend->AddLink( NULL, pLink );
        }
        break;
        case SC_CAT_MOVE :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkMove, pAppend );
            pAppend->AddLink( NULL, pLink );
        }
        break;
    }
}


void ScChangeTrack::AppendLoaded( ScChangeAction* pAppend )
{
    aTable.Insert( pAppend->GetActionNumber(), pAppend );
    if ( !pLast )
        pFirst = pLast = pAppend;
    else
    {
        pLast->pNext = pAppend;
        pAppend->pPrev = pLast;
        pLast = pAppend;
    }
    MasterLinks( pAppend );
}


void ScChangeTrack::Append( ScChangeAction* pAppend, ULONG nAction )
{
    if ( nActionMax < nAction )
        nActionMax = nAction;
    pAppend->SetUser( aUser );
    if ( bUseFixDateTime )
        pAppend->SetDateTimeUTC( aFixDateTime );
    pAppend->SetActionNumber( nAction );
    aTable.Insert( nAction, pAppend );
    // UpdateReference Inserts vor Dependencies.
    // Delete rejectendes Insert hatte UpdateReference mit Delete-Undo.
    // UpdateReference auch wenn pLast==NULL, weil pAppend ein Delete sein
    // kann, dass DelContents generiert haben kann
    if ( pAppend->IsInsertType() && !pAppend->IsRejecting() )
        UpdateReference( pAppend, FALSE );
    if ( !pLast )
        pFirst = pLast = pAppend;
    else
    {
        pLast->pNext = pAppend;
        pAppend->pPrev = pLast;
        pLast = pAppend;
        Dependencies( pAppend );
    }
    // UpdateReference Inserts nicht nach Dependencies.
    // Move rejectendes Move hatte UpdateReference mit Move-Undo, Inhalt in
    // ToRange nicht deleten.
    if ( !pAppend->IsInsertType() &&
            !(pAppend->GetType() == SC_CAT_MOVE && pAppend->IsRejecting()) )
        UpdateReference( pAppend, FALSE );
    MasterLinks( pAppend );

    if ( aModifiedLink.IsSet() )
    {
        NotifyModified( SC_CTM_APPEND, nAction, nAction );
        if ( pAppend->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = (ScChangeActionContent*) pAppend;
            if ( pContent = pContent->GetPrevContent() )
            {
                ULONG nMod = pContent->GetActionNumber();
                NotifyModified( SC_CTM_CHANGE, nMod, nMod );
            }
        }
        else
            NotifyModified( SC_CTM_CHANGE, pFirst->GetActionNumber(),
                pLast->GetActionNumber() );
    }
}


void ScChangeTrack::Append( ScChangeAction* pAppend )
{
    Append( pAppend, ++nActionMax );
}


void ScChangeTrack::AppendDeleteRange( const ScRange& rRange,
        ScDocument* pRefDoc, ULONG& nStartAction, ULONG& nEndAction, short nDz )
{
    nStartAction = GetActionMax() + 1;
    AppendDeleteRange( rRange, pRefDoc, nDz, 0 );
    nEndAction = GetActionMax();
}


void ScChangeTrack::AppendDeleteRange( const ScRange& rRange,
        ScDocument* pRefDoc, short nDz, ULONG nRejectingInsert )
{
    SetInDeleteRange( rRange );
    StartBlockModify( SC_CTM_APPEND, GetActionMax() + 1 );
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    for ( USHORT nTab = nTab1; nTab <= nTab2; nTab++ )
    {
        if ( !pRefDoc || nTab < pRefDoc->GetTableCount() )
        {
            if ( nCol1 == 0 && nCol2 == MAXCOL )
            {   // ganze Zeilen und/oder Tabellen
                if ( nRow1 == 0 && nRow2 == MAXROW )
                {   // ganze Tabellen
//2do: geht nicht auch komplette Tabelle als ganzes?
                    ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
                    for ( USHORT nCol = nCol1; nCol <= nCol2; nCol++ )
                    {   // spaltenweise ist weniger als zeilenweise
                        aRange.aStart.SetCol( nCol );
                        aRange.aEnd.SetCol( nCol );
                        if ( nCol == nCol2 )
                            SetInDeleteTop( TRUE );
                        AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
                            nTab-nTab1 + nDz, nRejectingInsert );
                    }
                    //! immer noch InDeleteTop
                    AppendOneDeleteRange( rRange, pRefDoc, 0, 0,
                        nTab-nTab1 + nDz, nRejectingInsert );
                }
                else
                {   // ganze Zeilen
                    ScRange aRange( 0, 0, nTab, MAXCOL, 0, nTab );
                    for ( USHORT nRow = nRow1; nRow <= nRow2; nRow++ )
                    {
                        aRange.aStart.SetRow( nRow );
                        aRange.aEnd.SetRow( nRow );
                        if ( nRow == nRow2 )
                            SetInDeleteTop( TRUE );
                        AppendOneDeleteRange( aRange, pRefDoc, 0, nRow-nRow1,
                            0, nRejectingInsert );
                    }
                }
            }
            else if ( nRow1 == 0 && nRow2 == MAXROW )
            {   // ganze Spalten
                ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
                for ( USHORT nCol = nCol1; nCol <= nCol2; nCol++ )
                {
                    aRange.aStart.SetCol( nCol );
                    aRange.aEnd.SetCol( nCol );
                    if ( nCol == nCol2 )
                        SetInDeleteTop( TRUE );
                    AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
                        0, nRejectingInsert );
                }
            }
            else
                DBG_ERROR( "ScChangeTrack::AppendDeleteRange: Block not supported!" );
            SetInDeleteTop( FALSE );
        }
    }
    EndBlockModify( GetActionMax() );
}


void ScChangeTrack::AppendOneDeleteRange( const ScRange& rOrgRange,
        ScDocument* pRefDoc, short nDx, short nDy, short nDz,
        ULONG nRejectingInsert )
{
    ScRange aTrackRange( rOrgRange );
    if ( nDx )
    {
        aTrackRange.aStart.IncCol( -nDx );
        aTrackRange.aEnd.IncCol( -nDx );
    }
    if ( nDy )
    {
        aTrackRange.aStart.IncRow( -nDy );
        aTrackRange.aEnd.IncRow( -nDy );
    }
    if ( nDz )
    {
        aTrackRange.aStart.IncTab( -nDz );
        aTrackRange.aEnd.IncTab( -nDz );
    }
    ScChangeActionDel* pAct = new ScChangeActionDel( aTrackRange, nDx, nDy,
        this );
    // TabDelete keine Contents, sind in einzelnen Spalten
    if ( !(rOrgRange.aStart.Col() == 0 && rOrgRange.aStart.Row() == 0 &&
            rOrgRange.aEnd.Col() == MAXCOL && rOrgRange.aEnd.Row() == MAXROW) )
        LookUpContents( rOrgRange, pRefDoc, -nDx, -nDy, -nDz );
    if ( nRejectingInsert )
    {
        pAct->SetRejectAction( nRejectingInsert );
        pAct->SetState( SC_CAS_ACCEPTED );
    }
    Append( pAct );
}


void ScChangeTrack::LookUpContents( const ScRange& rOrgRange,
        ScDocument* pRefDoc, short nDx, short nDy, short nDz )
{
    if ( pRefDoc )
    {
        ScAddress aPos;
        ScBigAddress aBigPos;
        ScCellIterator aIter( pRefDoc, rOrgRange );
        ScBaseCell* pCell = aIter.GetFirst();
        while ( pCell )
        {
            if ( ScChangeActionContent::GetContentCellType( pCell ) )
            {
                aBigPos.Set( aIter.GetCol() + nDx, aIter.GetRow() + nDy,
                    aIter.GetTab() + nDz );
                ScChangeActionContent* pContent = SearchContentAt( aBigPos, NULL );
                if ( !pContent )
                {   // nicht getrackte Contents
                    aPos.Set( aIter.GetCol() + nDx, aIter.GetRow() + nDy,
                        aIter.GetTab() + nDz );
                    GenerateDelContent( aPos, pCell, pRefDoc );
                    //! der Content wird hier _nicht_ per AddContent hinzugefuegt,
                    //! sondern in UpdateReference, um z.B. auch kreuzende Deletes
                    //! korrekt zu erfassen
                }
            }
            pCell = aIter.GetNext();
        }
    }
}


void ScChangeTrack::AppendMove( const ScRange& rFromRange,
        const ScRange& rToRange, ScDocument* pRefDoc )
{
    ScChangeActionMove* pAct = new ScChangeActionMove( rFromRange, rToRange, this );
    LookUpContents( rToRange, pRefDoc, 0, 0, 0 );   // ueberschriebene Contents
    Append( pAct );
}


// static
BOOL ScChangeTrack::IsMatrixFormulaRangeDifferent( const ScBaseCell* pOldCell,
        const ScBaseCell* pNewCell )
{
    USHORT nC1, nR1, nC2, nR2;
    nC1 = nR1 = nC2 = nR2 = 0;
    if ( pOldCell && (pOldCell->GetCellType() == CELLTYPE_FORMULA) &&
            ((const ScFormulaCell*)pOldCell)->GetMatrixFlag() == MM_FORMULA )
        ((const ScFormulaCell*)pOldCell)->GetMatColsRows( nC1, nR1 );
    if ( pNewCell && (pNewCell->GetCellType() == CELLTYPE_FORMULA) &&
            ((const ScFormulaCell*)pNewCell)->GetMatrixFlag() == MM_FORMULA )
        ((const ScFormulaCell*)pNewCell)->GetMatColsRows( nC1, nR1 );
    return nC1 != nC2 || nR1 != nR2;
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        const String& rNewValue )
{
    ScBaseCell* pCell = pDoc->GetCell( rPos );
    AppendContent( rPos, rNewValue, pCell );
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        const String& rNewValue, ScBaseCell* pOldCell )
{
    String aOldValue;
    ScChangeActionContent::GetStringOfCell( aOldValue, pOldCell, pDoc, rPos );
    if ( aOldValue != rNewValue ||
            IsMatrixFormulaRangeDifferent( pOldCell, NULL ) )
    {   // nur wirkliche Aenderung tracken
        ScRange aRange( rPos );
        ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
        pAct->SetOldValue( pOldCell, pDoc, pDoc );
        pAct->SetNewValue( rNewValue, pDoc );
        Append( pAct );
    }
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        const ScBaseCell* pOldCell, ULONG nOldFormat, ScDocument* pRefDoc )
{
    if ( !pRefDoc )
        pRefDoc = pDoc;
    String aOldValue;
    ScChangeActionContent::GetStringOfCell( aOldValue, pOldCell, pRefDoc, nOldFormat );
    String aNewValue;
    ScBaseCell* pNewCell = pDoc->GetCell( rPos );
    ScChangeActionContent::GetStringOfCell( aNewValue, pNewCell, pDoc, rPos );
    if ( aOldValue != aNewValue ||
            IsMatrixFormulaRangeDifferent( pOldCell, pNewCell ) )
    {   // nur wirkliche Aenderung tracken
        ScRange aRange( rPos );
        ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
        pAct->SetOldValue( pOldCell, pRefDoc, pDoc, nOldFormat );
        pAct->SetNewValue( pNewCell, pDoc );
        Append( pAct );
    }
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        ScDocument* pRefDoc )
{
    String aOldValue;
    ScBaseCell* pOldCell = pRefDoc->GetCell( rPos );
    ScChangeActionContent::GetStringOfCell( aOldValue, pOldCell, pRefDoc, rPos );
    String aNewValue;
    ScBaseCell* pNewCell = pDoc->GetCell( rPos );
    ScChangeActionContent::GetStringOfCell( aNewValue, pNewCell, pDoc, rPos );
    if ( aOldValue != aNewValue ||
            IsMatrixFormulaRangeDifferent( pOldCell, pNewCell ) )
    {   // nur wirkliche Aenderung tracken
        ScRange aRange( rPos );
        ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
        pAct->SetOldValue( pOldCell, pRefDoc, pDoc );
        pAct->SetNewValue( pNewCell, pDoc );
        Append( pAct );
    }
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        const ScBaseCell* pOldCell )
{
    if ( ScChangeActionContent::NeedsNumberFormat( pOldCell ) )
        AppendContent( rPos, pOldCell, pDoc->GetNumberFormat( rPos ), pDoc );
    else
        AppendContent( rPos, pOldCell, 0, pDoc );
}


void ScChangeTrack::AppendContent( const ScAddress& rPos,
        const ScBaseCell* pOldCell, ScDocument* pRefDoc )
{
    if ( ScChangeActionContent::NeedsNumberFormat( pOldCell ) )
        AppendContent( rPos, pOldCell, pRefDoc->GetNumberFormat( rPos ), pRefDoc );
    else
        AppendContent( rPos, pOldCell, 0, pRefDoc );
}


void ScChangeTrack::SetLastCutMoveRange( const ScRange& rRange,
        ScDocument* pRefDoc )
{
    if ( pLastCutMove )
    {
        // ToRange nicht mit Deletes linken und nicht in der Groesse aendern,
        // eigentlich unnoetig, da ein Delete vorher in
        // ScViewFunc::PasteFromClip ein ResetLastCut ausloest
        ScBigRange& r = pLastCutMove->GetBigRange();
        r.aEnd.SetCol( -1 );
        r.aEnd.SetRow( -1 );
        r.aEnd.SetTab( -1 );
        r.aStart.SetCol( -1 - (rRange.aEnd.Col() - rRange.aStart.Col()) );
        r.aStart.SetRow( -1 - (rRange.aEnd.Row() - rRange.aStart.Row()) );
        r.aStart.SetTab( -1 - (rRange.aEnd.Tab() - rRange.aStart.Tab()) );
        // zu ueberschreibende Contents im FromRange
        LookUpContents( rRange, pRefDoc, 0, 0, 0 );
    }
}


void ScChangeTrack::AppendContentRange( const ScRange& rRange,
        ScDocument* pRefDoc, ULONG& nStartAction, ULONG& nEndAction,
        ScChangeActionClipMode eClipMode )
{
    if ( eClipMode == SC_CACM_CUT )
    {
        ResetLastCut();
        pLastCutMove = new ScChangeActionMove( rRange, rRange, this );
        SetLastCutMoveRange( rRange, pRefDoc );
    }
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    BOOL bDoContents;
    if ( eClipMode == SC_CACM_PASTE && HasLastCut() )
    {
        bDoContents = FALSE;
        SetInPasteCut( TRUE );
        // Paste und Cut abstimmen, Paste kann groesserer Range sein
        ScRange aRange( rRange );
        ScBigRange& r = pLastCutMove->GetBigRange();
        USHORT nTmp;
        if ( (nTmp = (USHORT) (r.aEnd.Col() - r.aStart.Col())) != (nCol2 - nCol1) )
        {
            aRange.aEnd.SetCol( aRange.aStart.Col() + nTmp );
            nCol1 += nTmp + 1;
            bDoContents = TRUE;
        }
        if ( (nTmp = (USHORT) (r.aEnd.Row() - r.aStart.Row())) != (nRow2 - nRow1) )
        {
            aRange.aEnd.SetRow( aRange.aStart.Row() + nTmp );
            nRow1 += nTmp + 1;
            bDoContents = TRUE;
        }
        if ( (nTmp = (USHORT) (r.aEnd.Tab() - r.aStart.Tab())) != (nTab2 - nTab1) )
        {
            aRange.aEnd.SetTab( aRange.aStart.Tab() + nTmp );
            nTab1 += nTmp + 1;
            bDoContents = TRUE;
        }
        r = aRange;
        Undo( nStartLastCut, nEndLastCut ); // hier werden sich die Cuts gemerkt
        //! StartAction erst nach Undo
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
        // zu ueberschreibende Contents im ToRange
        LookUpContents( aRange, pRefDoc, 0, 0, 0 );
        pLastCutMove->SetStartLastCut( nStartLastCut );
        pLastCutMove->SetEndLastCut( nEndLastCut );
        Append( pLastCutMove );
        pLastCutMove = NULL;
        ResetLastCut();
        SetInPasteCut( FALSE );
    }
    else
    {
        bDoContents = TRUE;
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
    }
    if ( bDoContents )
    {
        ScAddress aPos;
        for ( USHORT nTab = nTab1; nTab <= nTab2; nTab++ )
        {
            aPos.SetTab( nTab );
            for ( USHORT nCol = nCol1; nCol <= nCol2; nCol++ )
            {
                aPos.SetCol( nCol );
                for ( USHORT nRow = nRow1; nRow <= nRow2; nRow++ )
                {
                    aPos.SetRow( nRow );
                    AppendContent( aPos, pRefDoc );
                }
            }
        }
    }
    nEndAction = GetActionMax();
    EndBlockModify( nEndAction );
    if ( eClipMode == SC_CACM_CUT )
    {
        nStartLastCut = nStartAction;
        nEndLastCut = nEndAction;
    }
}


void ScChangeTrack::AppendContentsIfInRefDoc( ScDocument* pRefDoc,
            ULONG& nStartAction, ULONG& nEndAction )
{
    ScDocumentIterator aIter( pRefDoc, 0, MAXTAB );
    if ( aIter.GetFirst() )
    {
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
        SvNumberFormatter* pFormatter = pRefDoc->GetFormatTable();
        do
        {
            USHORT nCol, nRow, nTab;
            aIter.GetPos( nCol, nRow, nTab );
            ScAddress aPos( nCol, nRow, nTab );
            AppendContent( aPos, aIter.GetCell(),
                aIter.GetPattern()->GetNumberFormat( pFormatter ), pRefDoc );
        } while ( aIter.GetNext() );
        nEndAction = GetActionMax();
        EndBlockModify( nEndAction );
    }
    else
        nStartAction = nEndAction = 0;
}


ScChangeActionContent* ScChangeTrack::AppendContentOnTheFly(
        const ScAddress& rPos, ScBaseCell* pOldCell, ScBaseCell* pNewCell,
        ULONG nOldFormat, ULONG nNewFormat )
{
    ScRange aRange( rPos );
    ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
    pAct->SetOldNewCells( pOldCell, nOldFormat, pNewCell, nNewFormat, pDoc );
    Append( pAct );
    return pAct;
}


void ScChangeTrack::AppendInsert( const ScRange& rRange )
{
    ScChangeActionIns* pAct = new ScChangeActionIns( rRange );
    Append( pAct );
}


void ScChangeTrack::DeleteCellEntries( ScChangeActionCellListEntry*& pCellList,
        ScChangeAction* pDeletor )
{
    ScChangeActionCellListEntry* pE = pCellList;
    while ( pE )
    {
        ScChangeActionCellListEntry* pNext = pE->pNext;
        pE->pContent->RemoveDeletedIn( pDeletor );
        if ( IsGenerated( pE->pContent->GetActionNumber() ) &&
                !pE->pContent->IsDeletedIn() )
            DeleteGeneratedDelContent( pE->pContent );
        delete pE;
        pE = pNext;
    }
    pCellList = NULL;
}


ScChangeActionContent* ScChangeTrack::GenerateDelContent(
        const ScAddress& rPos, const ScBaseCell* pCell,
        const ScDocument* pFromDoc )
{
    ScChangeActionContent* pContent = new ScChangeActionContent(
        ScRange( rPos ) );
    pContent->SetActionNumber( --nGeneratedMin );
    // nur NewValue
    ScChangeActionContent::SetValue( pContent->aNewValue, pContent->pNewCell,
        rPos, pCell, pFromDoc, pDoc );
    // pNextContent und pPrevContent werden nicht gesetzt
    if ( pFirstGeneratedDelContent )
    {   // vorne reinhaengen
        pFirstGeneratedDelContent->pPrev = pContent;
        pContent->pNext = pFirstGeneratedDelContent;
    }
    pFirstGeneratedDelContent = pContent;
    aGeneratedTable.Insert( nGeneratedMin, pContent );
    NotifyModified( SC_CTM_APPEND, nGeneratedMin, nGeneratedMin );
    return pContent;
}


void ScChangeTrack::DeleteGeneratedDelContent( ScChangeActionContent* pContent )
{
    ULONG nAct = pContent->GetActionNumber();
    aGeneratedTable.Remove( nAct );
    if ( pFirstGeneratedDelContent == pContent )
        pFirstGeneratedDelContent = (ScChangeActionContent*) pContent->pNext;
    if ( pContent->pNext )
        pContent->pNext->pPrev = pContent->pPrev;
    if ( pContent->pPrev )
        pContent->pPrev->pNext = pContent->pNext;
    delete pContent;
    NotifyModified( SC_CTM_REMOVE, nAct, nAct );
    if ( nAct == nGeneratedMin )
        ++nGeneratedMin;        //! erst nach NotifyModified wg. IsGenerated
}


ScChangeActionContent* ScChangeTrack::SearchGeneratedDelContentAt(
        const ScBigAddress& rPos, ScChangeActionType eNotInDelType ) const
{
    for ( ScChangeAction* p = pFirstGeneratedDelContent; p; p = p->GetNext() )
    {
        if ( p->GetType() == SC_CAT_CONTENT && p->GetBigRange().aStart == rPos
                && !p->IsDeletedInDelType( eNotInDelType ) )
        {
            return (ScChangeActionContent*) p;
        }
    }
    return NULL;
}


ScChangeActionContent* ScChangeTrack::SearchContentAt(
        const ScBigAddress& rPos, ScChangeAction* pButNotThis ) const
{
    USHORT nSlot = ComputeContentSlot( rPos.Row() );
    for ( ScChangeActionContent* p = ppContentSlots[nSlot]; p;
            p = p->GetNextInSlot() )
    {
        if ( p != pButNotThis && !p->IsDeletedIn() &&
                p->GetBigRange().aStart == rPos )
        {
            ScChangeActionContent* pContent = p->GetTopContent();
            if ( !pContent->IsDeletedIn() )
                return pContent;
        }
    }
    return NULL;
}


void ScChangeTrack::AddDependentWithNotify( ScChangeAction* pParent,
        ScChangeAction* pDependent )
{
    ScChangeActionLinkEntry* pLink = pParent->AddDependent( pDependent );
    pDependent->AddLink( pParent, pLink );
    if ( aModifiedLink.IsSet() )
    {
        ULONG nMod = pParent->GetActionNumber();
        NotifyModified( SC_CTM_PARENT, nMod, nMod );
    }
}


void ScChangeTrack::Dependencies( ScChangeAction* pAct )
{
    // Finde die letzte Abhaengigkeit fuer jeweils Col/Row/Tab.
    // Content an gleicher Position verketten.
    // Move Abhaengigkeiten.
    ScChangeActionType eActType = pAct->GetType();
    if ( eActType == SC_CAT_REJECT ||
            (eActType == SC_CAT_MOVE && pAct->IsRejecting()) )
        return ;        // diese Rejects sind nicht abhaengig

    if ( eActType == SC_CAT_CONTENT )
    {
        if ( !(((ScChangeActionContent*)pAct)->GetNextContent() ||
            ((ScChangeActionContent*)pAct)->GetPrevContent()) )
        {   // Contents an gleicher Position verketten
            ScChangeActionContent* pContent = SearchContentAt(
                pAct->GetBigRange().aStart, pAct );
            if ( pContent )
            {
                pContent->SetNextContent( (ScChangeActionContent*) pAct );
                ((ScChangeActionContent*)pAct)->SetPrevContent( pContent );
            }
        }
        const ScBaseCell* pCell = ((ScChangeActionContent*)pAct)->GetNewCell();
        if ( ScChangeActionContent::GetContentCellType( pCell ) == SC_CACCT_MATREF )
        {
            ScAddress aOrg;
            ((const ScFormulaCell*)pCell)->GetMatrixOrigin( aOrg );
            ScChangeActionContent* pContent = SearchContentAt( aOrg, pAct );
            if ( pContent && pContent->IsMatrixOrigin() )
            {
                AddDependentWithNotify( pContent, pAct );
            }
            else
            {
                DBG_ERRORFILE( "ScChangeTrack::Dependencies: MatOrg not found" );
            }
        }
    }

    if ( !(pLinkInsertCol || pLinkInsertRow || pLinkInsertTab || pLinkMove) )
        return ;        // keine Dependencies
    if ( pAct->IsRejecting() )
        return ;        // ausser Content keine Dependencies

    // Insert in einem entsprechenden Insert haengt davon ab, sonst muesste
    // der vorherige Insert gesplittet werden.
    // Sich kreuzende Inserts und Deletes sind nicht abhaengig.
    // Alles andere ist abhaengig.

    // Der zuletzt eingelinkte Insert steht am Anfang einer Kette,
    // also genau richtig

    const ScBigRange& rRange = pAct->GetBigRange();
    BOOL bActNoInsert = !pAct->IsInsertType();
    BOOL bActColDel = ( eActType == SC_CAT_DELETE_COLS );
    BOOL bActRowDel = ( eActType == SC_CAT_DELETE_ROWS );
    BOOL bActTabDel = ( eActType == SC_CAT_DELETE_TABS );

    if ( pLinkInsertCol && (eActType == SC_CAT_INSERT_COLS ||
            (bActNoInsert && !bActRowDel && !bActTabDel)) )
    {
        for ( ScChangeActionLinkEntry* pL = pLinkInsertCol; pL; pL = pL->GetNext() )
        {
            ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
            if ( !pTest->IsRejected() &&
                    pTest->GetBigRange().Intersects( rRange ) )
            {
                AddDependentWithNotify( pTest, pAct );
                break;  // for
            }
        }
    }
    if ( pLinkInsertRow && (eActType == SC_CAT_INSERT_ROWS ||
            (bActNoInsert && !bActColDel && !bActTabDel)) )
    {
        for ( ScChangeActionLinkEntry* pL = pLinkInsertRow; pL; pL = pL->GetNext() )
        {
            ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
            if ( !pTest->IsRejected() &&
                    pTest->GetBigRange().Intersects( rRange ) )
            {
                AddDependentWithNotify( pTest, pAct );
                break;  // for
            }
        }
    }
    if ( pLinkInsertTab && (eActType == SC_CAT_INSERT_TABS ||
            (bActNoInsert && !bActColDel &&  !bActRowDel)) )
    {
        for ( ScChangeActionLinkEntry* pL = pLinkInsertTab; pL; pL = pL->GetNext() )
        {
            ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
            if ( !pTest->IsRejected() &&
                    pTest->GetBigRange().Intersects( rRange ) )
            {
                AddDependentWithNotify( pTest, pAct );
                break;  // for
            }
        }
    }

    if ( pLinkMove )
    {
        if ( eActType == SC_CAT_CONTENT )
        {   // Content ist von FromRange abhaengig
            const ScBigAddress& rPos = rRange.aStart;
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
                if ( !pTest->IsRejected() &&
                        pTest->GetFromRange().In( rPos ) )
                {
                    AddDependentWithNotify( pTest, pAct );
                }
            }
        }
        else if ( eActType == SC_CAT_MOVE )
        {   // Move FromRange ist von ToRange abhaengig
            const ScBigRange& rFromRange = ((ScChangeActionMove*)pAct)->GetFromRange();
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
                if ( !pTest->IsRejected() &&
                        pTest->GetBigRange().Intersects( rFromRange ) )
                {
                    AddDependentWithNotify( pTest, pAct );
                }
            }
        }
        else
        {   // Inserts und Deletes sind abhaengig, sobald sie FromRange oder
            // ToRange kreuzen
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
                if ( !pTest->IsRejected() &&
                        (pTest->GetFromRange().Intersects( rRange ) ||
                        pTest->GetBigRange().Intersects( rRange )) )
                {
                    AddDependentWithNotify( pTest, pAct );
                }
            }
        }
    }
}


void ScChangeTrack::Remove( ScChangeAction* pRemove )
{
    // aus Track ausklinken
    ULONG nAct = pRemove->GetActionNumber();
    aTable.Remove( nAct );
    if ( nAct == nActionMax )
        --nActionMax;
    if ( pRemove == pLast )
        pLast = pRemove->pPrev;
    if ( pRemove == pFirst )
        pFirst = pRemove->pNext;
    if ( nAct == nMarkLastSaved )
        nMarkLastSaved =
            ( pRemove->pPrev ? pRemove->pPrev->GetActionNumber() : 0 );

    // aus der globalen Kette ausklinken
    if ( pRemove->pNext )
        pRemove->pNext->pPrev = pRemove->pPrev;
    if ( pRemove->pPrev )
        pRemove->pPrev->pNext = pRemove->pNext;

    // Dependencies nicht loeschen, passiert on delete automatisch durch
    // LinkEntry, ohne Listen abzuklappern

    if ( aModifiedLink.IsSet() )
    {
        NotifyModified( SC_CTM_REMOVE, nAct, nAct );
        if ( pRemove->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = (ScChangeActionContent*) pRemove;
            if ( pContent = pContent->GetPrevContent() )
            {
                ULONG nMod = pContent->GetActionNumber();
                NotifyModified( SC_CTM_CHANGE, nMod, nMod );
            }
        }
        else if ( pLast )
            NotifyModified( SC_CTM_CHANGE, pFirst->GetActionNumber(),
                pLast->GetActionNumber() );
    }

    if ( IsInPasteCut() && pRemove->GetType() == SC_CAT_CONTENT )
    {   //! Content wird wiederverwertet
        ScChangeActionContent* pContent = (ScChangeActionContent*) pRemove;
        pContent->RemoveAllLinks();
        pContent->ClearTrack();
        pContent->pNext = pContent->pPrev = NULL;
        pContent->pNextContent = pContent->pPrevContent = NULL;
    }
}


void ScChangeTrack::Undo( ULONG nStartAction, ULONG nEndAction )
{
    if ( nStartAction == 0 )
        ++nStartAction;
    if ( nEndAction > nActionMax )
        nEndAction = nActionMax;
    if ( nEndAction && nStartAction <= nEndAction )
    {
        if ( nStartAction == nStartLastCut && nEndAction == nEndLastCut &&
                !IsInPasteCut() )
            ResetLastCut();
        StartBlockModify( SC_CTM_REMOVE, nStartAction );
        for ( ULONG j = nEndAction; j >= nStartAction; --j )
        {   // rueckwaerts um evtl. nActionMax zu recyclen und schnelleren
            // Zugriff via pLast, Deletes in richtiger Reihenfolge
            ScChangeAction* pAct = ( (j == nActionMax && pLast &&
                pLast->GetActionNumber() == j) ? pLast : GetAction( j ) );
            if ( pAct )
            {
                if ( pAct->IsDeleteType() )
                {
                    if ( j == nEndAction || (pAct != pLast &&
                            ((ScChangeActionDel*)pAct)->IsTopDelete()) )
                    {
                        SetInDeleteTop( TRUE );
                        SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                            GetOverAllRange().MakeRange() );
                    }
                }
                UpdateReference( pAct, TRUE );
                SetInDeleteTop( FALSE );
                Remove( pAct );
                if ( IsInPasteCut() )
                    aPasteCutTable.Insert( pAct->GetActionNumber(), pAct );
                else
                {
                    if ( j == nStartAction && pAct->GetType() == SC_CAT_MOVE )
                    {
                        ScChangeActionMove* pMove = (ScChangeActionMove*) pAct;
                        ULONG nStart = pMove->GetStartLastCut();
                        ULONG nEnd = pMove->GetEndLastCut();
                        if ( nStart && nStart <= nEnd )
                        {   // LastCut wiederherstellen
                            //! Links vor Cut-Append aufloesen
                            pMove->RemoveAllLinks();
                            StartBlockModify( SC_CTM_APPEND, nStart );
                            for ( ULONG nCut = nStart; nCut <= nEnd; nCut++ )
                            {
                                ScChangeAction* pCut = aPasteCutTable.Remove( nCut );
                                if ( pCut )
                                {
                                    DBG_ASSERT( !aTable.Get( nCut ), "ScChangeTrack::Undo: nCut dup" );
                                    Append( pCut, nCut );
                                }
                                else
                                    DBG_ERROR( "ScChangeTrack::Undo: nCut not found" );
                            }
                            EndBlockModify( nEnd );
                            ResetLastCut();
                            nStartLastCut = nStart;
                            nEndLastCut = nEnd;
                            pLastCutMove = pMove;
                            SetLastCutMoveRange(
                                pMove->GetFromRange().MakeRange(), pDoc );
                        }
                        else
                            delete pMove;
                    }
                    else
                        delete pAct;
                }
            }
        }
        EndBlockModify( nEndAction );
    }
}


// static
BOOL ScChangeTrack::MergeIgnore( const ScChangeAction& rAction, ULONG nFirstMerge )
{
    if ( rAction.IsRejected() )
        return TRUE;                // da kommt noch eine passende Reject-Action

    if ( rAction.IsRejecting() && rAction.GetRejectAction() >= nFirstMerge )
        return TRUE;                // da ist sie

    return FALSE;                   // alles andere
}


void ScChangeTrack::MergePrepare( ScChangeAction* pFirstMerge )
{
    SetMergeState( SC_CTMS_PREPARE );
    ULONG nFirstMerge = pFirstMerge->GetActionNumber();
    ScChangeAction* pAct = GetLast();
    if ( pAct )
    {
        SetLastMerge( pAct->GetActionNumber() );
        while ( pAct )
        {   // rueckwaerts, Deletes in richtiger Reihenfolge
            if ( !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
            {
                if ( pAct->IsDeleteType() )
                {
                    if ( ((ScChangeActionDel*)pAct)->IsTopDelete() )
                    {
                        SetInDeleteTop( TRUE );
                        SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                            GetOverAllRange().MakeRange() );
                    }
                }
                UpdateReference( pAct, TRUE );
                SetInDeleteTop( FALSE );
                pAct->DeleteCellEntries();      // sonst GPF bei Track Clear()
            }
            pAct = ( pAct == pFirstMerge ? NULL : pAct->GetPrev() );
        }
    }
    SetMergeState( SC_CTMS_OTHER );     //! nachfolgende per default MergeOther
}


void ScChangeTrack::MergeOwn( ScChangeAction* pAct, ULONG nFirstMerge )
{
    if ( !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
    {
        SetMergeState( SC_CTMS_OWN );
        if ( pAct->IsDeleteType() )
        {
            if ( ((ScChangeActionDel*)pAct)->IsTopDelete() )
            {
                SetInDeleteTop( TRUE );
                SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                    GetOverAllRange().MakeRange() );
            }
        }
        UpdateReference( pAct, FALSE );
        SetInDeleteTop( FALSE );
        SetMergeState( SC_CTMS_OTHER );     //! nachfolgende per default MergeOther
    }
}


void ScChangeTrack::UpdateReference( ScChangeAction* pAct, BOOL bUndo )
{
    ScChangeActionType eActType = pAct->GetType();
    if ( eActType == SC_CAT_CONTENT || eActType == SC_CAT_REJECT )
        return ;

    //! Formelzellen haengen nicht im Dokument
    BOOL bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( FALSE );
    BOOL bOldNoListening = pDoc->GetNoListening();
    pDoc->SetNoListening( TRUE );
    //! Formelzellen ExpandRefs synchronisiert zu denen im Dokument
    BOOL bOldExpandRefs = pDoc->IsExpandRefs();
    if ( (!bUndo && pAct->IsInsertType()) || (bUndo && pAct->IsDeleteType()) )
        pDoc->SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );

    if ( pAct->IsDeleteType() )
    {
        SetInDeleteUndo( bUndo );
        SetInDelete( TRUE );
    }
    else if ( GetMergeState() == SC_CTMS_OWN )
    {
        // Referenzen von Formelzellen wiederherstellen,
        // vorheriges MergePrepare war bei einem Insert wie ein Delete
        if ( pAct->IsInsertType() )
            SetInDeleteUndo( TRUE );
    }

    //! erst die generated, als waeren sie vorher getrackt worden
    if ( pFirstGeneratedDelContent )
        UpdateReference( (ScChangeAction**)&pFirstGeneratedDelContent, pAct,
            bUndo );
    UpdateReference( &pFirst, pAct, bUndo );

    SetInDelete( FALSE );
    SetInDeleteUndo( FALSE );

    pDoc->SetExpandRefs( bOldExpandRefs );
    pDoc->SetNoListening( bOldNoListening );
    pDoc->SetAutoCalc( bOldAutoCalc );
}


void ScChangeTrack::UpdateReference( ScChangeAction** ppFirstAction,
        ScChangeAction* pAct, BOOL bUndo )
{
    ScChangeActionType eActType = pAct->GetType();
    BOOL bGeneratedDelContents =
        ( ppFirstAction == (ScChangeAction**)&pFirstGeneratedDelContent );
    const ScBigRange& rOrgRange = pAct->GetBigRange();
    ScBigRange aRange( rOrgRange );
    ScBigRange aDelRange( rOrgRange );
    INT32 nDx, nDy, nDz;
    nDx = nDy = nDz = 0;
    UpdateRefMode eMode = URM_INSDEL;
    BOOL bDel = FALSE;
    switch ( eActType )
    {
        case SC_CAT_INSERT_COLS :
            aRange.aEnd.SetCol( nInt32Max );
            nDx = rOrgRange.aEnd.Col() - rOrgRange.aStart.Col() + 1;
        break;
        case SC_CAT_INSERT_ROWS :
            aRange.aEnd.SetRow( nInt32Max );
            nDy = rOrgRange.aEnd.Row() - rOrgRange.aStart.Row() + 1;
        break;
        case SC_CAT_INSERT_TABS :
            aRange.aEnd.SetTab( nInt32Max );
            nDz = rOrgRange.aEnd.Tab() - rOrgRange.aStart.Tab() + 1;
        break;
        case SC_CAT_DELETE_COLS :
            aRange.aEnd.SetCol( nInt32Max );
            nDx = -(rOrgRange.aEnd.Col() - rOrgRange.aStart.Col() + 1);
            aDelRange.aEnd.SetCol( aDelRange.aStart.Col() - nDx - 1 );
            bDel = TRUE;
        break;
        case SC_CAT_DELETE_ROWS :
            aRange.aEnd.SetRow( nInt32Max );
            nDy = -(rOrgRange.aEnd.Row() - rOrgRange.aStart.Row() + 1);
            aDelRange.aEnd.SetRow( aDelRange.aStart.Row() - nDy - 1 );
            bDel = TRUE;
        break;
        case SC_CAT_DELETE_TABS :
            aRange.aEnd.SetTab( nInt32Max );
            nDz = -(rOrgRange.aEnd.Tab() - rOrgRange.aStart.Tab() + 1);
            aDelRange.aEnd.SetTab( aDelRange.aStart.Tab() - nDz - 1 );
            bDel = TRUE;
        break;
        case SC_CAT_MOVE :
            eMode = URM_MOVE;
            ((ScChangeActionMove*)pAct)->GetDelta( nDx, nDy, nDz );
        break;
        default:
            DBG_ERROR( "ScChangeTrack::UpdateReference: unknown Type" );
    }
    if ( bUndo )
    {
        nDx = -nDx;
        nDy = -nDy;
        nDz = -nDz;
    }
    if ( bDel )
    {   //! fuer diesen Mechanismus gilt:
        //! es gibt nur ganze, einfache geloeschte Spalten/Zeilen
        ScChangeActionDel* pActDel = (ScChangeActionDel*) pAct;
        if ( !bUndo )
        {   // Delete
            ScChangeActionType eInsType;        // fuer Insert-Undo-"Deletes"
            switch ( eActType )
            {
                case SC_CAT_DELETE_COLS :
                    eInsType = SC_CAT_INSERT_COLS;
                break;
                case SC_CAT_DELETE_ROWS :
                    eInsType = SC_CAT_INSERT_ROWS;
                break;
                case SC_CAT_DELETE_TABS :
                    eInsType = SC_CAT_INSERT_TABS;
                break;
            }
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                BOOL bUpdate = TRUE;
                if ( GetMergeState() == SC_CTMS_OTHER &&
                        p->GetActionNumber() <= GetLastMerge() )
                {   // Delete in mergendem Dokument, Action im zu mergenden
                    if ( p->IsInsertType() )
                    {
                        // Bei Insert Referenzen nur anpassen, wenn das Delete
                        // das Insert nicht schneidet.
                        if ( !aDelRange.Intersects( p->GetBigRange() ) )
                            p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                        bUpdate = FALSE;
                    }
                    else if ( p->GetType() == SC_CAT_CONTENT &&
                            p->IsDeletedInDelType( eInsType ) )
                    {   // Content in Insert-Undo-"Delete"
                        // Nicht anpassen, wenn dieses Delete in dem
                        // Insert-"Delete" sein wuerde (ist nur verschoben).
                        if ( aDelRange.In( p->GetBigRange().aStart ) )
                            bUpdate = FALSE;
                        else
                        {
                            const ScChangeActionLinkEntry* pLink = p->GetDeletedIn();
                            while ( pLink && bUpdate )
                            {
                                const ScChangeAction* pDel = pLink->GetAction();
                                if ( pDel && pDel->GetType() == eInsType &&
                                        pDel->GetBigRange().In( aDelRange ) )
                                    bUpdate = FALSE;
                                pLink = pLink->GetNext();
                            }
                        }
                    }
                    if ( !bUpdate )
                        continue;   // for
                }
                if ( aDelRange.In( p->GetBigRange() ) )
                {
                    // Innerhalb eines gerade geloeschten Bereiches nicht
                    // anpassen, stattdessen dem Bereich zuordnen.
                    // Mehrfache geloeschte Bereiche "stapeln".
                    // Kreuzende Deletes setzen mehrfach geloescht.
                    if ( !p->IsDeletedInDelType( eActType ) )
                    {
                        p->SetDeletedIn( pActDel );
                        // GeneratedDelContent in zu loeschende Liste aufnehmen
                        if ( bGeneratedDelContents )
                            pActDel->AddContent( (ScChangeActionContent*) p );
                    }
                    bUpdate = FALSE;
                }
                else
                {
                    // Eingefuegte Bereiche abschneiden, wenn Start/End im
                    // Delete liegt, aber das Insert nicht komplett innerhalb
                    // des Delete liegt bzw. das Delete nicht komplett im
                    // Insert. Das Delete merkt sich, welchem Insert es was
                    // abgeschnitten hat, es kann auch nur ein einziges Insert
                    // sein (weil Delete einspaltig/einzeilig ist).
                    // Abgeschnittene Moves kann es viele geben.
                    //! Ein Delete ist immer einspaltig/einzeilig, deswegen 1
                    //! ohne die Ueberlappung auszurechnen.
                    switch ( p->GetType() )
                    {
                        case SC_CAT_INSERT_COLS :
                            if ( eActType == SC_CAT_DELETE_COLS )
                            {
                                if ( aDelRange.In( p->GetBigRange().aStart ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, 1 );
                                    p->GetBigRange().aStart.IncCol( 1 );
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, -1 );
                                    p->GetBigRange().aEnd.IncCol( -1 );
                                }
                            }
                        break;
                        case SC_CAT_INSERT_ROWS :
                            if ( eActType == SC_CAT_DELETE_ROWS )
                            {
                                if ( aDelRange.In( p->GetBigRange().aStart ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, 1 );
                                    p->GetBigRange().aStart.IncRow( 1 );
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, -1 );
                                    p->GetBigRange().aEnd.IncRow( -1 );
                                }
                            }
                        break;
                        case SC_CAT_INSERT_TABS :
                            if ( eActType == SC_CAT_DELETE_TABS )
                            {
                                if ( aDelRange.In( p->GetBigRange().aStart ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, 1 );
                                    p->GetBigRange().aStart.IncTab( 1 );
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        (ScChangeActionIns*) p, -1 );
                                    p->GetBigRange().aEnd.IncTab( -1 );
                                }
                            }
                        break;
                        case SC_CAT_MOVE :
                        {
                            ScChangeActionMove* pMove = (ScChangeActionMove*) p;
                            short nFrom = 0;
                            short nTo = 0;
                            if ( aDelRange.In( pMove->GetBigRange().aStart ) )
                                nTo = 1;
                            else if ( aDelRange.In( pMove->GetBigRange().aEnd ) )
                                nTo = -1;
                            if ( aDelRange.In( pMove->GetFromRange().aStart ) )
                                nFrom = 1;
                            else if ( aDelRange.In( pMove->GetFromRange().aEnd ) )
                                nFrom = -1;
                            if ( nFrom )
                            {
                                switch ( eActType )
                                {
                                    case SC_CAT_DELETE_COLS :
                                        if ( nFrom > 0 )
                                            pMove->GetFromRange().aStart.IncCol( nFrom );
                                        else
                                            pMove->GetFromRange().aEnd.IncCol( nFrom );
                                    break;
                                    case SC_CAT_DELETE_ROWS :
                                        if ( nFrom > 0 )
                                            pMove->GetFromRange().aStart.IncRow( nFrom );
                                        else
                                            pMove->GetFromRange().aEnd.IncRow( nFrom );
                                    break;
                                    case SC_CAT_DELETE_TABS :
                                        if ( nFrom > 0 )
                                            pMove->GetFromRange().aStart.IncTab( nFrom );
                                        else
                                            pMove->GetFromRange().aEnd.IncTab( nFrom );
                                    break;
                                }
                            }
                            if ( nTo )
                            {
                                switch ( eActType )
                                {
                                    case SC_CAT_DELETE_COLS :
                                        if ( nTo > 0 )
                                            pMove->GetBigRange().aStart.IncCol( nTo );
                                        else
                                            pMove->GetBigRange().aEnd.IncCol( nTo );
                                    break;
                                    case SC_CAT_DELETE_ROWS :
                                        if ( nTo > 0 )
                                            pMove->GetBigRange().aStart.IncRow( nTo );
                                        else
                                            pMove->GetBigRange().aEnd.IncRow( nTo );
                                    break;
                                    case SC_CAT_DELETE_TABS :
                                        if ( nTo > 0 )
                                            pMove->GetBigRange().aStart.IncTab( nTo );
                                        else
                                            pMove->GetBigRange().aEnd.IncTab( nTo );
                                    break;
                                }
                            }
                            if ( nFrom || nTo )
                            {
                                ScChangeActionDelMoveEntry* pLink =
                                    pActDel->AddCutOffMove( pMove, nFrom, nTo );
                                pMove->AddLink( pActDel, pLink );
                            }
                        }
                        break;
                    }
                }
                if ( bUpdate )
                {
                    p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                    if ( p->GetType() == eActType && !p->IsRejected() &&
                            !pActDel->IsDeletedIn() &&
                            p->GetBigRange().In( aDelRange ) )
                        pActDel->SetDeletedIn( p );     // "druntergerutscht"
                }
            }
        }
        else
        {   // Undo Delete
            ScChangeAction* pNextAction = NULL;
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                BOOL bUpdate = TRUE;
                if ( aDelRange.In( p->GetBigRange() ) )
                {
                    if ( p->IsDeletedInDelType( eActType ) )
                    {
                        if ( p->IsDeletedIn( pActDel ) )
                        {
                            if ( p->GetType() != SC_CAT_CONTENT ||
                                    ((ScChangeActionContent*)p)->IsTopContent() )
                            {   // erst der TopContent wird wirklich entfernt
                                p->RemoveDeletedIn( pActDel );
                                // GeneratedDelContent _nicht_ aus Liste loeschen,
                                // wir brauchen ihn evtl. noch fuer Reject,
                                // geloescht wird in DeleteCellEntries
                            }
                        }
                        bUpdate = FALSE;
                    }
                    else if ( eActType != SC_CAT_DELETE_TABS &&
                            p->IsDeletedInDelType( SC_CAT_DELETE_TABS ) )
                    {   // in geloeschten Tabellen nicht updaten,
                        // ausser wenn Tabelle verschoben wird
                        bUpdate = FALSE;
                    }
                    if ( p->GetType() == eActType && pActDel->IsDeletedIn( p ) )
                    {
                        pActDel->RemoveDeletedIn( p );  // "druntergerutscht"
                        bUpdate = TRUE;
                    }
                }
                if ( bUpdate )
                    p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
            }
            if ( !bGeneratedDelContents )
            {   // die werden sonst noch fuer das echte Undo gebraucht
                pActDel->UndoCutOffInsert();
                pActDel->UndoCutOffMoves();
            }
        }
    }
    else if ( eActType == SC_CAT_MOVE )
    {
        ScChangeActionMove* pActMove = (ScChangeActionMove*) pAct;
        BOOL bLastCutMove = ( pActMove == pLastCutMove );
        const ScBigRange& rTo = pActMove->GetBigRange();
        const ScBigRange& rFrom = pActMove->GetFromRange();
        if ( !bUndo )
        {   // Move
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // Inhalt in Ziel deleten (Inhalt in Quelle moven)
                    if ( rTo.In( p->GetBigRange() ) )
                    {
                        if ( !p->IsDeletedIn( pActMove ) )
                        {
                            p->SetDeletedIn( pActMove );
                            // GeneratedDelContent in zu loeschende Liste aufnehmen
                            if ( bGeneratedDelContents )
                                pActMove->AddContent( (ScChangeActionContent*) p );
                        }
                    }
                    else if ( bLastCutMove &&
                            p->GetActionNumber() > nEndLastCut &&
                            rFrom.In( p->GetBigRange() ) )
                    {   // Paste Cut: neuer Content nach Cut eingefuegt, bleibt.
                        // Aufsplitten der ContentChain
                        ScChangeActionContent *pHere, *pTmp;
                        pHere = (ScChangeActionContent*) p;
                        while ( (pTmp = pHere->GetPrevContent()) &&
                                pTmp->GetActionNumber() > nEndLastCut )
                            pHere = pTmp;
                        if ( pTmp )
                        {   // wird TopContent des Move
                            pTmp->SetNextContent( NULL );
                            pHere->SetPrevContent( NULL );
                        }
                        do
                        {   // Abhaengigkeit vom FromRange herstellen
                            AddDependentWithNotify( pActMove, pHere );
                        } while ( pHere = pHere->GetNextContent() );
                    }
                    else
                        p->UpdateReference( this, eMode, rFrom, nDx, nDy, nDz );
                }
            }
        }
        else
        {   // Undo Move
            BOOL bActRejected = pActMove->IsRejected();
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // Inhalt in Ziel moven, wenn nicht deleted, sonst undelete
                    if ( p->IsDeletedIn( pActMove ) )
                    {
                        if ( ((ScChangeActionContent*)p)->IsTopContent() )
                        {   // erst der TopContent wird wirklich entfernt
                            p->RemoveDeletedIn( pActMove );
                            // GeneratedDelContent _nicht_ aus Liste loeschen,
                            // wir brauchen ihn evtl. noch fuer Reject,
                            // geloescht wird in DeleteCellEntries
                        }
                    }
                    else
                        p->UpdateReference( this, eMode, rTo, nDx, nDy, nDz );
                    if ( bActRejected &&
                            ((ScChangeActionContent*)p)->IsTopContent() &&
                            rFrom.In( p->GetBigRange() ) )
                    {   // Abhaengigkeit herstellen, um Content zu schreiben
                        ScChangeActionLinkEntry* pLink =
                            pActMove->AddDependent( p );
                        p->AddLink( pActMove, pLink );
                    }
                }
            }
        }
    }
    else
    {   // Insert / Undo Insert
        switch ( GetMergeState() )
        {
            case SC_CTMS_NONE :
            case SC_CTMS_OTHER :
            {
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                        continue;   // for
                    p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                }
            }
            break;
            case SC_CTMS_PREPARE :
            {
                // in Insert-Undo "Deleten"
                const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
                while ( pLink )
                {
                    ScChangeAction* p = (ScChangeAction*) pLink->GetAction();
                    if ( p )
                        p->SetDeletedIn( pAct );
                    pLink = pLink->GetNext();
                }
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                        continue;   // for
                    if ( !p->IsDeletedIn( pAct ) )
                        p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                }
            }
            break;
            case SC_CTMS_OWN :
            {
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                        continue;   // for
                    if ( !p->IsDeletedIn( pAct ) )
                        p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                }
                // in Insert-Undo "Delete" rueckgaengig
                const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
                while ( pLink )
                {
                    ScChangeAction* p = (ScChangeAction*) pLink->GetAction();
                    if ( p )
                        p->RemoveDeletedIn( pAct );
                    pLink = pLink->GetNext();
                }
            }
            break;
        }
    }
}


void ScChangeTrack::GetDependents( ScChangeAction* pAct,
        ScChangeActionTable& rTable, BOOL bListMasterDelete, BOOL bAllFlat )
{
    //! bAllFlat==TRUE: intern aus Accept oder Reject gerufen,
    //! => Generated werden nicht aufgenommen

    BOOL bIsDelete = pAct->IsDeleteType();
    BOOL bIsMasterDelete = ( bListMasterDelete && pAct->IsMasterDelete() );

    ScChangeAction* pCur = pAct;
    ScChangeActionStack* pStack = new ScChangeActionStack;
    do
    {
        if ( pCur->IsInsertType() )
        {
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        ULONG n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rTable.Insert( n, p ) )
                            if ( p->HasDependent() )
                                pStack->Push( p );
                    }
                    else
                    {
                        if ( p->GetType() == SC_CAT_CONTENT )
                        {
                            if ( ((ScChangeActionContent*)p)->IsTopContent() )
                                rTable.Insert( p->GetActionNumber(), p );
                        }
                        else
                            rTable.Insert( p->GetActionNumber(), p );
                    }
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->IsDeleteType() )
        {
            if ( bIsDelete )
            {   // Inhalte geloeschter Bereiche interessieren nur bei Delete
                ScChangeActionDel* pDel = (ScChangeActionDel*) pCur;
                if ( !bAllFlat && bIsMasterDelete && pCur == pAct )
                {
                    // zu diesem Delete gehoerende Deletes in gleiche Ebene,
                    // wenn dieses Delete das momentan oberste einer Reihe ist,
                    ScChangeActionType eType = pDel->GetType();
                    ScChangeAction* p = pDel;
                    while ( (p = p->GetPrev()) && p->GetType() == eType &&
                            !((ScChangeActionDel*)p)->IsTopDelete() )
                        rTable.Insert( p->GetActionNumber(), p );
                    // dieses Delete auch in Table!
                    rTable.Insert( pAct->GetActionNumber(), pAct );
                }
                else
                {
                    const ScChangeActionLinkEntry* pL = pCur->GetFirstDeletedEntry();
                    while ( pL )
                    {
                        ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                        if ( p != pAct )
                        {
                            if ( bAllFlat )
                            {
                                // nur ein TopContent einer Kette ist in LinkDeleted
                                ULONG n = p->GetActionNumber();
                                if ( !IsGenerated( n ) && rTable.Insert( n, p ) )
                                    if ( p->HasDeleted() ||
                                            p->GetType() == SC_CAT_CONTENT )
                                        pStack->Push( p );
                            }
                            else
                            {
                                if ( p->IsDeleteType() )
                                {   // weiteres TopDelete in gleiche Ebene,
                                    // es ist nicht rejectable
                                    if ( ((ScChangeActionDel*)p)->IsTopDelete() )
                                        rTable.Insert( p->GetActionNumber(), p );
                                }
                                else
                                    rTable.Insert( p->GetActionNumber(), p );
                            }
                        }
                        pL = pL->GetNext();
                    }
                }
            }
        }
        else if ( pCur->GetType() == SC_CAT_MOVE )
        {
            // geloeschte Contents im ToRange
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDeletedEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p != pAct && rTable.Insert( p->GetActionNumber(), p ) )
                {
                    // nur ein TopContent einer Kette ist in LinkDeleted
                    if ( bAllFlat && (p->HasDeleted() ||
                            p->GetType() == SC_CAT_CONTENT) )
                        pStack->Push( p );
                }
                pL = pL->GetNext();
            }
            // neue Contents im FromRange oder neuer FromRange im ToRange
            // oder Inserts/Deletes in FromRange/ToRange
            pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        ULONG n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rTable.Insert( n, p ) )
                            if ( p->HasDependent() || p->HasDeleted() )
                                pStack->Push( p );
                    }
                    else
                    {
                        if ( p->GetType() == SC_CAT_CONTENT )
                        {
                            if ( ((ScChangeActionContent*)p)->IsTopContent() )
                                rTable.Insert( p->GetActionNumber(), p );
                        }
                        else
                            rTable.Insert( p->GetActionNumber(), p );
                    }
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->GetType() == SC_CAT_CONTENT )
        {   // alle Aenderungen an gleicher Position
            ScChangeActionContent* pContent = (ScChangeActionContent*) pCur;
            // alle vorherigen
            while ( pContent = pContent->GetPrevContent() )
            {
                if ( !pContent->IsRejected() )
                    rTable.Insert( pContent->GetActionNumber(), pContent );
            }
            pContent = (ScChangeActionContent*) pCur;
            // alle nachfolgenden
            while ( pContent = pContent->GetNextContent() )
            {
                if ( !pContent->IsRejected() )
                    rTable.Insert( pContent->GetActionNumber(), pContent );
            }
            // all MatrixReferences of a MatrixOrigin
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        ULONG n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rTable.Insert( n, p ) )
                            if ( p->HasDependent() )
                                pStack->Push( p );
                    }
                    else
                        rTable.Insert( p->GetActionNumber(), p );
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->GetType() == SC_CAT_REJECT )
        {
            if ( bAllFlat )
            {
                ScChangeAction* p = GetAction(
                        ((ScChangeActionReject*)pCur)->GetRejectAction() );
                if ( p != pAct && !rTable.Get( p->GetActionNumber() ) )
                    pStack->Push( p );
            }
        }
    } while ( pCur = pStack->Pop() );
    delete pStack;
}


BOOL ScChangeTrack::SelectContent( ScChangeAction* pAct, BOOL bOldest )
{
    if ( pAct->GetType() != SC_CAT_CONTENT )
        return FALSE;

    ScChangeActionContent* pContent = (ScChangeActionContent*) pAct;
    if ( bOldest )
    {
        pContent = pContent->GetTopContent();
        ScChangeActionContent* pPrevContent;
        while ( (pPrevContent = pContent->GetPrevContent()) &&
                pPrevContent->IsVirgin() )
            pContent = pPrevContent;
    }

    if ( !pContent->IsClickable() )
        return FALSE;

    ScBigRange aBigRange( pContent->GetBigRange() );
    const ScBaseCell* pCell = (bOldest ? pContent->GetOldCell() :
        pContent->GetNewCell());
    if ( ScChangeActionContent::GetContentCellType( pCell ) == SC_CACCT_MATORG )
    {
        USHORT nC, nR;
        ((const ScFormulaCell*)pCell)->GetMatColsRows( nC, nR );
        aBigRange.aEnd.IncCol( nC-1 );
        aBigRange.aEnd.IncRow( nR-1 );
    }

    if ( !aBigRange.IsValid( pDoc ) )
        return FALSE;

    ScRange aRange( aBigRange.MakeRange() );
    if ( !pDoc->IsBlockEditable( aRange.aStart.Tab(), aRange.aStart.Col(),
            aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() ) )
        return FALSE;

    if ( pContent->HasDependent() )
    {
        BOOL bOk = TRUE;
        Stack aRejectActions;
        const ScChangeActionLinkEntry* pL = pContent->GetFirstDependentEntry();
        while ( pL )
        {
            ScChangeAction* p = (ScChangeAction*) pL->GetAction();
            if ( p != pContent )
            {
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // we don't need no recursion here, do we?
                    bOk &= ((ScChangeActionContent*)p)->Select( pDoc, this,
                        bOldest, &aRejectActions );
                }
                else
                {
                    DBG_ERRORFILE( "ScChangeTrack::SelectContent: content dependent no content" );
                }
            }
            pL = pL->GetNext();
        }

        bOk &= pContent->Select( pDoc, this, bOldest, NULL );
        // now the matrix is inserted and new content values are ready

        ScChangeActionContent* pNew;
        while ( pNew = (ScChangeActionContent*) aRejectActions.Pop() )
        {
            ScAddress aPos( pNew->GetBigRange().aStart.MakeAddress() );
            pNew->SetNewValue( pDoc->GetCell( aPos ), pDoc );
            Append( pNew );
        }
        return bOk;
    }
    else
        return pContent->Select( pDoc, this, bOldest, NULL );
}


void ScChangeTrack::AcceptAll()
{
    for ( ScChangeAction* p = GetFirst(); p; p = p->GetNext() )
    {
        p->Accept();
    }
}


BOOL ScChangeTrack::Accept( ScChangeAction* pAct )
{
    if ( !pAct->IsClickable() )
        return FALSE;

    if ( pAct->IsDeleteType() || pAct->GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionTable* pTable = new ScChangeActionTable;
        GetDependents( pAct, *pTable, FALSE, TRUE );
        for ( ScChangeAction* p = pTable->First(); p; p = pTable->Next() )
        {
            p->Accept();
        }
        delete pTable;
    }
    pAct->Accept();
    return TRUE;
}


BOOL ScChangeTrack::RejectAll()
{
    BOOL bOk = TRUE;
    for ( ScChangeAction* p = GetLast(); p && bOk; p = p->GetPrev() )
    {   //! rueckwaerts, weil abhaengige hinten und RejectActions angehaengt
        if ( p->IsInternalRejectable() )
            bOk = Reject( p );
    }
    return bOk;
}


BOOL ScChangeTrack::Reject( ScChangeAction* pAct )
{
    if ( !pAct->IsRejectable() )
        return FALSE;

    ScChangeActionTable* pTable = NULL;
    if ( pAct->HasDependent() )
    {
        pTable = new ScChangeActionTable;
        GetDependents( pAct, *pTable, FALSE, TRUE );
    }
    BOOL bRejected = Reject( pAct, pTable, FALSE );
    if ( pTable )
        delete pTable;
    return bRejected;
}


BOOL ScChangeTrack::Reject( ScChangeAction* pAct, ScChangeActionTable* pTable,
        BOOL bRecursion )
{
    if ( !pAct->IsInternalRejectable() )
        return FALSE;

    BOOL bOk = TRUE;
    BOOL bRejected = FALSE;
    if ( pAct->IsInsertType() )
    {
        if ( pAct->HasDependent() && !bRecursion )
        {
            const ScBigRange& rRange = pAct->GetBigRange();
            DBG_ASSERT( pTable, "ScChangeTrack::Reject: Insert ohne Table" );
            for ( ScChangeAction* p = pTable->Last(); p && bOk; p = pTable->Prev() )
            {
                // keine Contents restoren, die eh geloescht werden wuerden
                if ( p->GetType() == SC_CAT_CONTENT )
                    p->SetRejected();
                else if ( p->IsDeleteType() )
                    p->Accept();        // geloeschtes ins Nirvana
                else
                    bOk = Reject( p, NULL, TRUE );      //! rekursiv
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) )
        {
            // pRefDoc NULL := geloeschte Zellen nicht speichern
            AppendDeleteRange( pAct->GetBigRange().MakeRange(), NULL, (short) 0,
                pAct->GetActionNumber() );
        }
    }
    else if ( pAct->IsDeleteType() )
    {
        DBG_ASSERT( !pTable, "ScChangeTrack::Reject: Delete mit Table" );
        ScBigRange aDelRange;
        ULONG nRejectAction = pAct->GetActionNumber();
        BOOL bTabDel, bTabDelOk;
        if ( pAct->GetType() == SC_CAT_DELETE_TABS )
        {
            bTabDel = TRUE;
            aDelRange = pAct->GetBigRange();
            bOk = bTabDelOk = pAct->Reject( pDoc );
            if ( bOk )
            {
                pAct = pAct->GetPrev();
                bOk = ( pAct && pAct->GetType() == SC_CAT_DELETE_COLS );
            }
        }
        else
            bTabDel = bTabDelOk = FALSE;
        ScChangeActionDel* pDel = (ScChangeActionDel*) pAct;
        if ( bOk )
        {
            aDelRange = pDel->GetOverAllRange();
            bOk = aDelRange.IsValid( pDoc );
        }
        BOOL bOneOk = FALSE;
        if ( bOk )
        {
            ScChangeActionType eActType = pAct->GetType();
            switch ( eActType )
            {
                case SC_CAT_DELETE_COLS :
                    aDelRange.aStart.SetCol( aDelRange.aEnd.Col() );
                break;
                case SC_CAT_DELETE_ROWS :
                    aDelRange.aStart.SetRow( aDelRange.aEnd.Row() );
                break;
                case SC_CAT_DELETE_TABS :
                    aDelRange.aStart.SetTab( aDelRange.aEnd.Tab() );
                break;
            }
            ScChangeAction* p = pAct;
            BOOL bLoop = TRUE;
            do
            {
                pDel = (ScChangeActionDel*) p;
                bOk = pDel->Reject( pDoc );
                if ( bOk )
                {
                    if ( bOneOk )
                    {
                        switch ( pDel->GetType() )
                        {
                            case SC_CAT_DELETE_COLS :
                                aDelRange.aStart.IncCol( -1 );
                            break;
                            case SC_CAT_DELETE_ROWS :
                                aDelRange.aStart.IncRow( -1 );
                            break;
                            case SC_CAT_DELETE_TABS :
                                aDelRange.aStart.IncTab( -1 );
                            break;
                        }
                    }
                    else
                        bOneOk = TRUE;
                }
                if ( pDel->IsBaseDelete() )
                    bLoop = FALSE;
                else
                    p = p->GetPrev();
            } while ( bOk && bLoop && p && p->GetType() == eActType &&
                !((ScChangeActionDel*)p)->IsTopDelete() );
        }
        bRejected = bOk;
        if ( bOneOk || (bTabDel && bTabDelOk) )
        {
            // Delete-Reject machte UpdateReference Undo
            ScChangeActionIns* pReject = new ScChangeActionIns(
                aDelRange.MakeRange() );
            pReject->SetRejectAction( nRejectAction );
            pReject->SetState( SC_CAS_ACCEPTED );
            Append( pReject );
        }
    }
    else if ( pAct->GetType() == SC_CAT_MOVE )
    {
        if ( pAct->HasDependent() && !bRecursion )
        {
            const ScBigRange& rRange = pAct->GetBigRange();
            DBG_ASSERT( pTable, "ScChangeTrack::Reject: Move ohne Table" );
            for ( ScChangeAction* p = pTable->Last(); p && bOk; p = pTable->Prev() )
            {
                bOk = Reject( p, NULL, TRUE );      //! rekursiv
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) )
        {
            ScChangeActionMove* pReject = new ScChangeActionMove(
                pAct->GetBigRange().MakeRange(),
                ((ScChangeActionMove*)pAct)->GetFromRange().MakeRange(), this );
            pReject->SetRejectAction( pAct->GetActionNumber() );
            pReject->SetState( SC_CAS_ACCEPTED );
            Append( pReject );
        }
    }
    else if ( pAct->GetType() == SC_CAT_CONTENT )
    {
        ScRange aRange;
        ScChangeActionContent* pReject;
        if ( bRecursion )
            pReject = NULL;
        else
        {
            aRange = pAct->GetBigRange().aStart.MakeAddress();
            pReject = new ScChangeActionContent( aRange );
            pReject->SetOldValue( pDoc->GetCell( aRange.aStart ), pDoc, pDoc );
        }
        if ( (bRejected = pAct->Reject( pDoc )) && !bRecursion )
        {
            pReject->SetNewValue( pDoc->GetCell( aRange.aStart ), pDoc );
            pReject->SetRejectAction( pAct->GetActionNumber() );
            pReject->SetState( SC_CAS_ACCEPTED );
            Append( pReject );
        }
        else if ( pReject )
            delete pReject;
    }
    else
        DBG_ERROR( "ScChangeTrack::Reject: say what?" );

    return bRejected;
}


ULONG ScChangeTrack::AddLoadedGenerated(ScBaseCell* pNewCell, const ScBigRange& aBigRange )
{
    ScChangeActionContent* pAct = new ScChangeActionContent( --nGeneratedMin, pNewCell, aBigRange, pDoc );
    if ( pAct )
    {
        if ( pFirstGeneratedDelContent )
            pFirstGeneratedDelContent->pPrev = pAct;
        pAct->pNext = pFirstGeneratedDelContent;
        pFirstGeneratedDelContent = pAct;
        aGeneratedTable.Insert( pAct->GetActionNumber(), pAct );
        return pAct->GetActionNumber();
    }
    return 0;
}

