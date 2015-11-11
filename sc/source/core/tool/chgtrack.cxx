/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "chgtrack.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "scerrors.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "patattr.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "globstr.hrc"
#include "editutil.hxx"
#include "tokenarray.hxx"
#include "refupdatecontext.hxx"
#include <refupdat.hxx>

#include <svl/zforlist.hxx>
#include <svl/itemset.hxx>
#include <svl/isethint.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/sfxsids.hrc>
#include <memory>

IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry )
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry )

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScRange& rRange )
        :
        aBigRange( rRange ),
        aDateTime( DateTime::SYSTEM ),
        pNext( nullptr ),
        pPrev( nullptr ),
        pLinkAny( nullptr ),
        pLinkDeletedIn( nullptr ),
        pLinkDeleted( nullptr ),
        pLinkDependent( nullptr ),
        nAction( 0 ),
        nRejectAction( 0 ),
        eType( eTypeP ),
        eState( SC_CAS_VIRGIN )
{
    aDateTime.ConvertToUTC();
}

ScChangeAction::ScChangeAction(
    ScChangeActionType eTypeP, const ScBigRange& rRange,
    const sal_uLong nTempAction, const sal_uLong nTempRejectAction,
    const ScChangeActionState eTempState, const DateTime& aTempDateTime,
    const OUString& aTempUser,  const OUString& aTempComment) :
        aBigRange( rRange ),
        aDateTime( aTempDateTime ),
        aUser( aTempUser ),
        aComment( aTempComment ),
        pNext( nullptr ),
        pPrev( nullptr ),
        pLinkAny( nullptr ),
        pLinkDeletedIn( nullptr ),
        pLinkDeleted( nullptr ),
        pLinkDependent( nullptr ),
        nAction( nTempAction ),
        nRejectAction( nTempRejectAction ),
        eType( eTypeP ),
        eState( eTempState )
{
}

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScBigRange& rRange,
                        const sal_uLong nTempAction)
        :
        aBigRange( rRange ),
        aDateTime( DateTime::SYSTEM ),
        pNext( nullptr ),
        pPrev( nullptr ),
        pLinkAny( nullptr ),
        pLinkDeletedIn( nullptr ),
        pLinkDeleted( nullptr ),
        pLinkDependent( nullptr ),
        nAction( nTempAction ),
        nRejectAction( 0 ),
        eType( eTypeP ),
        eState( SC_CAS_VIRGIN )
{
    aDateTime.ConvertToUTC();
}

ScChangeAction::~ScChangeAction()
{
    RemoveAllLinks();
}

bool ScChangeAction::IsInsertType() const
{
    return eType == SC_CAT_INSERT_COLS || eType == SC_CAT_INSERT_ROWS || eType == SC_CAT_INSERT_TABS;
}

bool ScChangeAction::IsDeleteType() const
{
    return eType == SC_CAT_DELETE_COLS || eType == SC_CAT_DELETE_ROWS || eType == SC_CAT_DELETE_TABS;
}

bool ScChangeAction::IsVirgin() const
{
    return eState == SC_CAS_VIRGIN;
}

bool ScChangeAction::IsAccepted() const
{
    return eState == SC_CAS_ACCEPTED;
}

bool ScChangeAction::IsRejected() const
{
    return eState == SC_CAS_REJECTED;
}

bool ScChangeAction::IsRejecting() const
{
    return nRejectAction != 0;
}

bool ScChangeAction::IsVisible() const
{
    // sequence order of execution is significant!
    if ( IsRejected() || GetType() == SC_CAT_DELETE_TABS || IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
        return static_cast<const ScChangeActionContent*>(this)->IsTopContent();
    return true;
}

bool ScChangeAction::IsTouchable() const
{
    // sequence order of execution is significant!
    if ( IsRejected() || GetType() == SC_CAT_REJECT || IsDeletedIn() )
        return false;
    // content may reject and be touchable if on top
    if ( GetType() == SC_CAT_CONTENT )
        return static_cast<const ScChangeActionContent*>(this)->IsTopContent();
    if ( IsRejecting() )
        return false;
    return true;
}

bool ScChangeAction::IsClickable() const
{
    // sequence order of execution is significant!
    if ( !IsVirgin() )
        return false;
    if ( IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContentCellType eCCT =
            ScChangeActionContent::GetContentCellType(
            static_cast<const ScChangeActionContent*>(this)->GetNewCell() );
        if ( eCCT == SC_CACCT_MATREF )
            return false;
        if ( eCCT == SC_CACCT_MATORG )
        {   // no Accept-Select if one of the references is in a deleted col/row
            const ScChangeActionLinkEntry* pL =
                static_cast<const ScChangeActionContent*>(this)->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                if ( p && p->IsDeletedIn() )
                    return false;
                pL = pL->GetNext();
            }
        }
        return true; // for Select() a content doesn't have to be touchable
    }
    return IsTouchable(); // Accept()/Reject() only on touchables
}

bool ScChangeAction::IsRejectable() const
{
    // sequence order of execution is significant!
    if ( !IsClickable() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( static_cast<const ScChangeActionContent*>(this)->IsOldMatrixReference() )
            return false;
        ScChangeActionContent* pNextContent =
            static_cast<const ScChangeActionContent*>(this)->GetNextContent();
        if ( pNextContent == nullptr )
            return true; // *this is TopContent
        return pNextContent->IsRejected(); // *this is next rejectable
    }
    return IsTouchable();
}

bool ScChangeAction::IsInternalRejectable() const
{
    // sequence order of execution is significant!
    if ( !IsVirgin() )
        return false;
    if ( IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContent* pNextContent =
            static_cast<const ScChangeActionContent*>(this)->GetNextContent();
        if ( pNextContent == nullptr )
            return true; // *this is TopContent
        return pNextContent->IsRejected(); // *this is next rejectable
    }
    return IsTouchable();
}

bool ScChangeAction::IsDialogRoot() const
{
    return IsInternalRejectable(); // only rejectables in root
}

bool ScChangeAction::IsDialogParent() const
{
    // sequence order of execution is significant!
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( !IsDialogRoot() )
            return false;
        if ( static_cast<const ScChangeActionContent*>(this)->IsMatrixOrigin() && HasDependent() )
            return true;
        ScChangeActionContent* pPrevContent =
            static_cast<const ScChangeActionContent*>(this)->GetPrevContent();
        return pPrevContent && pPrevContent->IsVirgin();
    }
    if ( HasDependent() )
        return IsDeleteType() || !IsDeletedIn();
    if ( HasDeleted() )
    {
        if ( IsDeleteType() )
        {
            if ( IsDialogRoot() )
                return true;
            ScChangeActionLinkEntry* pL = pLinkDeleted;
            while ( pL )
            {
                ScChangeAction* p = pL->GetAction();
                if ( p && p->GetType() != eType )
                    return true;
                pL = pL->GetNext();
            }
        }
        else
            return true;
    }
    return false;
}

bool ScChangeAction::IsMasterDelete() const
{
    if ( !IsDeleteType() )
        return false;
    const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(this);
    return pDel->IsMultiDelete() && (pDel->IsTopDelete() || pDel->IsRejectable());
}

void ScChangeAction::RemoveAllLinks()
{
    RemoveAllAnyLinks();
    RemoveAllDeletedIn();
    RemoveAllDeleted();
    RemoveAllDependent();
}

void ScChangeAction::RemoveAllAnyLinks()
{
    while ( pLinkAny )
        delete pLinkAny; // Moves up by itself
}

bool ScChangeAction::RemoveDeletedIn( const ScChangeAction* p )
{
    bool bRemoved = false;
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    while ( pL )
    {
        ScChangeActionLinkEntry* pNextLink = pL->GetNext();
        if ( pL->GetAction() == p )
        {
            delete pL;
            bRemoved = true;
        }
        pL = pNextLink;
    }
    return bRemoved;
}

bool ScChangeAction::IsDeletedIn() const
{
    return GetDeletedIn() != nullptr;
}

bool ScChangeAction::IsDeletedIn( const ScChangeAction* p ) const
{
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    while ( pL )
    {
        if ( pL->GetAction() == p )
            return true;
        pL = pL->GetNext();
    }
    return false;
}

void ScChangeAction::RemoveAllDeletedIn()
{
    //TODO: Not from TopContent, but really this one
    while ( pLinkDeletedIn )
        delete pLinkDeletedIn; // Moves up by itself
}

bool ScChangeAction::IsDeletedInDelType( ScChangeActionType eDelType ) const
{
    ScChangeActionLinkEntry* pL = GetDeletedIn();
    if ( pL )
    {
        // InsertType for MergePrepare/MergeOwn
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
            ScChangeAction* p;
            if ( (p = pL->GetAction()) != nullptr &&
                    (p->GetType() == eDelType || p->GetType() == eInsType) )
                return true;
            pL = pL->GetNext();
        }
    }
    return false;
}

bool ScChangeAction::HasDependent() const
{
    return pLinkDependent != nullptr;
}

bool ScChangeAction::HasDeleted() const
{
    return pLinkDeleted != nullptr;
}

void ScChangeAction::SetDeletedIn( ScChangeAction* p )
{
    ScChangeActionLinkEntry* pLink1 = AddDeletedIn( p );
    ScChangeActionLinkEntry* pLink2;
    if ( GetType() == SC_CAT_CONTENT )
        pLink2 = p->AddDeleted( static_cast<ScChangeActionContent*>(this)->GetTopContent() );
    else
        pLink2 = p->AddDeleted( this );
    pLink1->SetLink( pLink2 );
}

void ScChangeAction::RemoveAllDeleted()
{
    while ( pLinkDeleted )
        delete pLinkDeleted; // Moves up by itself
}

void ScChangeAction::RemoveAllDependent()
{
    while ( pLinkDependent )
        delete pLinkDependent; // Moves up by itself
}

DateTime ScChangeAction::GetDateTime() const
{
    DateTime aDT( aDateTime );
    aDT.ConvertToLocalTime();
    return aDT;
}

void ScChangeAction::UpdateReference( const ScChangeTrack* /* pTrack */,
        UpdateRefMode eMode, const ScBigRange& rRange,
        sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
}

void ScChangeAction::GetDescription(
    OUString& rStr, ScDocument* /* pDoc */, bool /* bSplitRange */, bool bWarning ) const
{
    if (!IsRejecting() || !bWarning)
        return;

    // Add comment if rejection may have resulted in references
    // not properly restored in formulas. See specification at
    // http://specs.openoffice.org/calc/ease-of-use/redlining_comment.sxw

    OUStringBuffer aBuf(rStr); // Take the original string.
    if (GetType() == SC_CAT_MOVE)
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_MOVE_REJECTION_WARNING) + " ");
        rStr = aBuf.makeStringAndClear();
        return;
    }

    if (IsInsertType())
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING) + " ");
        rStr = aBuf.makeStringAndClear();
        return;
    }

    const ScChangeTrack* pCT = GetChangeTrack();
    if (!pCT)
        return;

    ScChangeAction* pReject = pCT->GetActionOrGenerated(GetRejectAction());

    if (!pReject)
        return;

    if (pReject->GetType() == SC_CAT_MOVE)
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_MOVE_REJECTION_WARNING));
        aBuf.append(' ');
        rStr = aBuf.makeStringAndClear();
        return;
    }

    if (pReject->IsDeleteType())
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING));
        aBuf.append(' ');
        rStr = aBuf.makeStringAndClear();
        return;
    }

    if (pReject->HasDependent())
    {
        ScChangeActionMap aMap;
        pCT->GetDependents( pReject, aMap, false, true );
        ScChangeActionMap::iterator itChangeAction;
        for( itChangeAction = aMap.begin(); itChangeAction != aMap.end(); ++itChangeAction )
        {
            if( itChangeAction->second->GetType() == SC_CAT_MOVE)
            {
                aBuf.append(
                    ScGlobal::GetRscString(STR_CHANGED_MOVE_REJECTION_WARNING));
                aBuf.append(' ');
                rStr = aBuf.makeStringAndClear();
                return;
            }

            if (pReject->IsDeleteType())
            {
                aBuf.append(
                    ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING));
                aBuf.append(' ');
                rStr = aBuf.makeStringAndClear();
                return;
            }
        }
    }
}

OUString ScChangeAction::GetRefString(
    const ScBigRange& rRange, ScDocument* pDoc, bool bFlag3D ) const
{
    OUStringBuffer aBuf;
    sal_uInt16 nFlags = ( rRange.IsValid( pDoc ) ? SCA_VALID : 0 );
    if ( !nFlags )
        aBuf.append(ScGlobal::GetRscString(STR_NOREF_STR));
    else
    {
        ScRange aTmpRange( rRange.MakeRange() );
        switch ( GetType() )
        {
            case SC_CAT_INSERT_COLS :
            case SC_CAT_DELETE_COLS :
                if ( bFlag3D )
                {
                    OUString aTmp;
                    pDoc->GetName( aTmpRange.aStart.Tab(), aTmp );
                    aBuf.append(aTmp);
                    aBuf.append('.');
                }
                aBuf.append(ScColToAlpha(aTmpRange.aStart.Col()));
                aBuf.append(':');
                aBuf.append(ScColToAlpha(aTmpRange.aEnd.Col()));
            break;
            case SC_CAT_INSERT_ROWS :
            case SC_CAT_DELETE_ROWS :
                if ( bFlag3D )
                {
                    OUString aTmp;
                    pDoc->GetName( aTmpRange.aStart.Tab(), aTmp );
                    aBuf.append(aTmp);
                    aBuf.append('.');
                }
                aBuf.append(static_cast<sal_Int32>(aTmpRange.aStart.Row()+1));
                aBuf.append(':');
                aBuf.append(static_cast<sal_Int32>(aTmpRange.aEnd.Row()+1));
            break;
            default:
            {
                if ( bFlag3D || GetType() == SC_CAT_INSERT_TABS )
                    nFlags |= SCA_TAB_3D;

                aBuf.append(aTmpRange.Format(nFlags, pDoc, pDoc->GetAddressConvention()));
            }
        }
        if ( (bFlag3D && IsDeleteType()) || IsDeletedIn() )
        {
            aBuf.insert(0, '(');
            aBuf.append(')');
        }
    }
    return aBuf.makeStringAndClear();
}

void ScChangeAction::SetUser( const OUString& r )
{
    aUser = r;
}

void ScChangeAction::SetComment( const OUString& rStr )
{
    aComment = rStr;
}

void ScChangeAction::GetRefString(
    OUString& rStr, ScDocument* pDoc, bool bFlag3D ) const
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
        SCsCOL nDx, SCsROW nDy )
{
    // Construct list of Contents
    ScChangeActionCellListEntry* pListContents = nullptr;
    for ( ScChangeActionLinkEntry* pL = pLinkDeleted; pL; pL = pL->GetNext() )
    {
        ScChangeAction* p = pL->GetAction();
        if ( p && p->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
                static_cast<ScChangeActionContent*>(p), pListContents );
            pListContents = pE;
        }
    }
    SetState( SC_CAS_REJECTED ); // Before UpdateReference for Move
    pTrack->UpdateReference( this, true ); // Free LinkDeleted
    OSL_ENSURE( !pLinkDeleted, "ScChangeAction::RejectRestoreContents: pLinkDeleted != NULL" );

    // Work through list of Contents and delete
    ScDocument* pDoc = pTrack->GetDocument();
    ScChangeActionCellListEntry* pE = pListContents;
    while ( pE )
    {
        if ( !pE->pContent->IsDeletedIn() &&
                pE->pContent->GetBigRange().aStart.IsValid( pDoc ) )
            pE->pContent->PutNewValueToDoc( pDoc, nDx, nDy );
        ScChangeActionCellListEntry* pNextEntry;
        pNextEntry = pE->pNext;
        delete pE;
        pE = pNextEntry;
    }
    DeleteCellEntries(); // Remove generated ones
}

void ScChangeAction::SetDeletedInThis( sal_uLong nActionNumber,
        const ScChangeTrack* pTrack )
{
    if ( nActionNumber )
    {
        ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
        OSL_ENSURE( pAct, "ScChangeAction::SetDeletedInThis: missing Action" );
        if ( pAct )
            pAct->SetDeletedIn( this );
    }
}

void ScChangeAction::AddDependent( sal_uLong nActionNumber,
        const ScChangeTrack* pTrack )
{
    if ( nActionNumber )
    {
        ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
        OSL_ENSURE( pAct, "ScChangeAction::AddDependent: missing Action" );
        if ( pAct )
        {
            ScChangeActionLinkEntry* pLink = AddDependent( pAct );
            pAct->AddLink( this, pLink );
        }
    }
}

//  ScChangeActionIns
ScChangeActionIns::ScChangeActionIns( const ScRange& rRange, bool bEndOfList ) :
    ScChangeAction(SC_CAT_NONE, rRange),
    mbEndOfList(bEndOfList)
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
    {
        OSL_FAIL( "ScChangeActionIns: Block not supported!" );
    }
}

ScChangeActionIns::ScChangeActionIns(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber, const ScBigRange& aBigRangeP,
    const OUString& aUserP, const DateTime& aDateTimeP,
    const OUString& sComment, const ScChangeActionType eTypeP,
    bool bEndOfList ) :
    ScChangeAction(eTypeP, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
    mbEndOfList(bEndOfList)
{
}

ScChangeActionIns::~ScChangeActionIns()
{
}

void ScChangeActionIns::GetDescription(
    OUString& rStr, ScDocument* pDoc, bool bSplitRange, bool bWarning ) const
{
    ScChangeAction::GetDescription( rStr, pDoc, bSplitRange, bWarning );

    sal_uInt16 nWhatId;
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

    OUString aRsc = ScGlobal::GetRscString(STR_CHANGED_INSERT);
    sal_Int32 nPos = aRsc.indexOf("#1");
    if (nPos >= 0)
    {
        // Construct a range string to replace '#1' first.
        OUStringBuffer aBuf(ScGlobal::GetRscString(nWhatId));
        aBuf.append(' ');
        aBuf.append(GetRefString(GetBigRange(), pDoc));
        OUString aRangeStr = aBuf.makeStringAndClear();

        aRsc = aRsc.replaceAt(nPos, 2, aRangeStr); // replace '#1' with the range string.

        aBuf.append(rStr).append(aRsc);
        rStr = aBuf.makeStringAndClear();
    }
}

bool ScChangeActionIns::IsEndOfList() const
{
    return mbEndOfList;
}

bool ScChangeActionIns::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return false;

    ScRange aRange( aBigRange.MakeRange() );
    if ( !pDoc->IsBlockEditable( aRange.aStart.Tab(), aRange.aStart.Col(),
            aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() ) )
        return false;

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
        default:
        {
            // added to avoid warnings
        }
    }
    SetState( SC_CAS_REJECTED );
    RemoveAllLinks();
    return true;
}

//  ScChangeActionDel
ScChangeActionDel::ScChangeActionDel( const ScRange& rRange,
            SCsCOL nDxP, SCsROW nDyP, ScChangeTrack* pTrackP )
        :
        ScChangeAction( SC_CAT_NONE, rRange ),
        pTrack( pTrackP ),
        pFirstCell( nullptr ),
        pCutOff( nullptr ),
        nCutOff( 0 ),
        pLinkMove( nullptr ),
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
    {
        OSL_FAIL( "ScChangeActionDel: Block not supported!" );
    }
}

ScChangeActionDel::ScChangeActionDel(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber, const ScBigRange& aBigRangeP,
    const OUString& aUserP, const DateTime& aDateTimeP, const OUString &sComment,
    const ScChangeActionType eTypeP, const SCsCOLROW nD, ScChangeTrack* pTrackP) : // which of nDx and nDy is set depends on the type
    ScChangeAction(eTypeP, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
    pTrack( pTrackP ),
    pFirstCell( nullptr ),
    pCutOff( nullptr ),
    nCutOff( 0 ),
    pLinkMove( nullptr ),
    nDx( 0 ),
    nDy( 0 )
{
    if (eType == SC_CAT_DELETE_COLS)
        nDx = static_cast<SCsCOL>(nD);
    else if (eType == SC_CAT_DELETE_ROWS)
        nDy = static_cast<SCsROW>(nD);
}

ScChangeActionDel::~ScChangeActionDel()
{
    DeleteCellEntries();
    while ( pLinkMove )
        delete pLinkMove;
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

bool ScChangeActionDel::IsBaseDelete() const
{
    return !GetDx() && !GetDy();
}

bool ScChangeActionDel::IsTopDelete() const
{
    const ScChangeAction* p = GetNext();
    if ( !p || p->GetType() != GetType() )
        return true;
    return static_cast<const ScChangeActionDel*>(p)->IsBaseDelete();
}

bool ScChangeActionDel::IsMultiDelete() const
{
    if ( GetDx() || GetDy() )
        return true;
    const ScChangeAction* p = GetNext();
    if ( !p || p->GetType() != GetType() )
        return false;
    const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(p);
    if ( (pDel->GetDx() > GetDx() || pDel->GetDy() > GetDy()) &&
            pDel->GetBigRange() == aBigRange )
        return true;
    return false;
}

bool ScChangeActionDel::IsTabDeleteCol() const
{
    if ( GetType() != SC_CAT_DELETE_COLS )
        return false;
    const ScChangeAction* p = this;
    while ( p && p->GetType() == SC_CAT_DELETE_COLS &&
            !static_cast<const ScChangeActionDel*>(p)->IsTopDelete() )
        p = p->GetNext();
    return p && p->GetType() == SC_CAT_DELETE_TABS;
}

ScChangeActionDelMoveEntry* ScChangeActionDel::AddCutOffMove(
    ScChangeActionMove* pMove, short nFrom, short nTo )
{
    return new ScChangeActionDelMoveEntry(&pLinkMove, pMove, nFrom, nTo);
}

void ScChangeActionDel::UpdateReference( const ScChangeTrack* /* pTrack */,
        UpdateRefMode eMode, const ScBigRange& rRange,
        sal_Int32 nDxP, sal_Int32 nDyP, sal_Int32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDxP, nDyP, nDz, GetBigRange() );

    if ( !IsDeletedIn() )
        return ;

    // Correct in the ones who slipped through
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
                default:
                {
                    // added to avoid warnings
                }
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

void ScChangeActionDel::GetDescription(
    OUString& rStr, ScDocument* pDoc, bool bSplitRange, bool bWarning ) const
{
    ScChangeAction::GetDescription( rStr, pDoc, bSplitRange, bWarning );

    sal_uInt16 nWhatId;
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

    OUString aRsc = ScGlobal::GetRscString(STR_CHANGED_DELETE);
    sal_Int32 nPos = aRsc.indexOf("#1");
    if (nPos >= 0)
    {
        // Build a string to replace with.
        OUStringBuffer aBuf;
        aBuf.append(ScGlobal::GetRscString(nWhatId));
        aBuf.append(' ');
        aBuf.append(GetRefString(aTmpRange, pDoc));
        OUString aRangeStr = aBuf.makeStringAndClear();
        aRsc = aRsc.replaceAt(nPos, 2, aRangeStr); // replace '#1' with the string.

        aBuf.append(rStr).append(aRsc);
        rStr = aBuf.makeStringAndClear(); // append to the original.
    }
}

bool ScChangeActionDel::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) && GetType() != SC_CAT_DELETE_TABS )
        return false;

    if ( IsTopDelete() )
    {   // Restore whole section in one go
        bool bOk = true;
        ScBigRange aTmpRange( GetOverAllRange() );
        if ( !aTmpRange.IsValid( pDoc ) )
        {
            if ( GetType() == SC_CAT_DELETE_TABS )
            {   // Do we attach a Tab?
                if ( aTmpRange.aStart.Tab() > pDoc->GetMaxTableNumber() )
                    bOk = false;
            }
            else
                bOk = false;
        }
        if ( bOk )
        {
            ScRange aRange( aTmpRange.MakeRange() );
            // InDelete... for formula UpdateReference in Document
            pTrack->SetInDeleteRange( aRange );
            pTrack->SetInDeleteTop( true );
            pTrack->SetInDeleteUndo( true );
            pTrack->SetInDelete( true );
            switch ( GetType() )
            {
                case SC_CAT_DELETE_COLS :
                    if ( !(aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL) )
                    {   // Only if not TabDelete
                        if ( ( bOk = pDoc->CanInsertCol( aRange ) ) )
                            bOk = pDoc->InsertCol( aRange );
                    }
                break;
                case SC_CAT_DELETE_ROWS :
                    if ( ( bOk = pDoc->CanInsertRow( aRange ) ) )
                        bOk = pDoc->InsertRow( aRange );
                break;
                case SC_CAT_DELETE_TABS :
                {
                    //TODO: Remember table names?
                    OUString aName;
                    pDoc->CreateValidTabName( aName );
                    if ( ( bOk = pDoc->ValidNewTabName( aName ) ) )
                        bOk = pDoc->InsertTab( aRange.aStart.Tab(), aName );
                }
                break;
                default:
                {
                    // added to avoid warnings
                }
            }
            pTrack->SetInDelete( false );
            pTrack->SetInDeleteUndo( false );
        }
        if ( !bOk )
        {
            pTrack->SetInDeleteTop( false );
            return false;
        }
        // Keep InDeleteTop for UpdateReference Undo
    }

    // Sets rejected and calls UpdateReference-Undo and DeleteCellEntries
    RejectRestoreContents( pTrack, GetDx(), GetDy() );

    pTrack->SetInDeleteTop( false );
    RemoveAllLinks();
    return true;
}

void ScChangeActionDel::UndoCutOffMoves()
{   // Restore cut off Moves; delete Entries/Links
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
            default:
            {
                // added to avoid warnings
            }
        }
        delete pLinkMove; // Moves up by itself
    }
}

void ScChangeActionDel::UndoCutOffInsert()
{   //Restore cut off Insert
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
            default:
            {
                // added to avoid warnings
            }
        }
        SetCutOffInsert( nullptr, 0 );
    }
}

//  ScChangeActionMove
ScChangeActionMove::ScChangeActionMove(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber, const ScBigRange& aToBigRange,
    const OUString& aUserP, const DateTime& aDateTimeP,
    const OUString &sComment, const ScBigRange& aFromBigRange,
    ScChangeTrack* pTrackP) : // which of nDx and nDy is set depends on the type
    ScChangeAction(SC_CAT_MOVE, aToBigRange, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
    aFromRange(aFromBigRange),
    pTrack( pTrackP ),
    pFirstCell( nullptr ),
    nStartLastCut(0),
    nEndLastCut(0)
{
}

ScChangeActionMove::~ScChangeActionMove()
{
    DeleteCellEntries();
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

void ScChangeActionMove::UpdateReference( const ScChangeTrack* /* pTrack */,
        UpdateRefMode eMode, const ScBigRange& rRange,
        sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz )
{
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aFromRange );
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
}

void ScChangeActionMove::GetDelta( sal_Int32& nDx, sal_Int32& nDy, sal_Int32& nDz ) const
{
    const ScBigAddress& rToPos = GetBigRange().aStart;
    const ScBigAddress& rFromPos = GetFromRange().aStart;
    nDx = rToPos.Col() - rFromPos.Col();
    nDy = rToPos.Row() - rFromPos.Row();
    nDz = rToPos.Tab() - rFromPos.Tab();
}

void ScChangeActionMove::GetDescription(
    OUString& rStr, ScDocument* pDoc, bool bSplitRange, bool bWarning ) const
{
    ScChangeAction::GetDescription( rStr, pDoc, bSplitRange, bWarning );

    bool bFlag3D = GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab();

    OUString aRsc = ScGlobal::GetRscString(STR_CHANGED_MOVE);

    OUString aTmpStr = ScChangeAction::GetRefString(GetFromRange(), pDoc, bFlag3D);
    sal_Int32 nPos = aRsc.indexOf("#1");
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    aTmpStr = ScChangeAction::GetRefString(GetBigRange(), pDoc, bFlag3D);
    nPos = nPos >= 0 ? aRsc.indexOf("#2", nPos) : -1;
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    OUStringBuffer aBuf(rStr); // append to the original string.
    aBuf.append(aRsc);
    rStr = aBuf.makeStringAndClear();
}

void ScChangeActionMove::GetRefString(
    OUString& rStr, ScDocument* pDoc, bool bFlag3D ) const
{
    if ( !bFlag3D )
        bFlag3D = ( GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab() );

    // overwrite existing string value.
    rStr = ScChangeAction::GetRefString(GetFromRange(), pDoc, bFlag3D)
        + ", "
        + ScChangeAction::GetRefString(GetBigRange(), pDoc, bFlag3D);
}

bool ScChangeActionMove::Reject( ScDocument* pDoc )
{
    if ( !(aBigRange.IsValid( pDoc ) && aFromRange.IsValid( pDoc )) )
        return false;

    ScRange aToRange( aBigRange.MakeRange() );
    ScRange aFrmRange( aFromRange.MakeRange() );

    bool bOk = pDoc->IsBlockEditable( aToRange.aStart.Tab(),
        aToRange.aStart.Col(), aToRange.aStart.Row(),
        aToRange.aEnd.Col(), aToRange.aEnd.Row() );
    if ( bOk )
        bOk = pDoc->IsBlockEditable( aFrmRange.aStart.Tab(),
            aFrmRange.aStart.Col(), aFrmRange.aStart.Row(),
            aFrmRange.aEnd.Col(), aFrmRange.aEnd.Row() );
    if ( !bOk )
        return false;

    pTrack->LookUpContents( aToRange, pDoc, 0, 0, 0 );  // Contents to be moved

    pDoc->DeleteAreaTab( aToRange, InsertDeleteFlags::ALL );
    pDoc->DeleteAreaTab( aFrmRange, InsertDeleteFlags::ALL );
    // Adjust formula in the Document
    sc::RefUpdateContext aCxt(*pDoc);
    aCxt.meMode = URM_MOVE;
    aCxt.maRange = aFrmRange;
    aCxt.mnColDelta = aFrmRange.aStart.Col() - aToRange.aStart.Col();
    aCxt.mnRowDelta = aFrmRange.aStart.Row() - aToRange.aStart.Row();
    aCxt.mnTabDelta = aFrmRange.aStart.Tab() - aToRange.aStart.Tab();
    pDoc->UpdateReference(aCxt);

    // Free LinkDependent, set succeeding UpdateReference Undo
    // ToRange->FromRange Dependents
    RemoveAllDependent();

    // Sets rejected and calls UpdateReference Undo and DeleteCellEntries
    RejectRestoreContents( pTrack, 0, 0 );

    while ( pLinkDependent )
    {
        ScChangeAction* p = pLinkDependent->GetAction();
        if ( p && p->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = static_cast<ScChangeActionContent*>(p);
            if ( !pContent->IsDeletedIn() &&
                    pContent->GetBigRange().aStart.IsValid( pDoc ) )
                pContent->PutNewValueToDoc( pDoc, 0, 0 );
            // Delete the ones created in LookUpContents
            if ( pTrack->IsGenerated( pContent->GetActionNumber() ) &&
                    !pContent->IsDeletedIn() )
            {
                pLinkDependent->UnLink(); // Else this one is also deleted!
                pTrack->DeleteGeneratedDelContent( pContent );
            }
        }
        delete pLinkDependent;
    }

    RemoveAllLinks();
    return true;
}

//  ScChangeActionContent
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent )

ScChangeActionContent::ScChangeActionContent( const ScRange& rRange ) :
    ScChangeAction(SC_CAT_CONTENT, rRange),
    pNextContent(nullptr),
    pPrevContent(nullptr),
    pNextInSlot(nullptr),
    ppPrevInSlot(nullptr)
{}

ScChangeActionContent::ScChangeActionContent( const sal_uLong nActionNumber,
            const ScChangeActionState eStateP, const sal_uLong nRejectingNumber,
            const ScBigRange& aBigRangeP, const OUString& aUserP,
            const DateTime& aDateTimeP, const OUString& sComment,
            const ScCellValue& rOldCell, ScDocument* pDoc, const OUString& sOldValue ) :
    ScChangeAction(SC_CAT_CONTENT, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
    maOldCell(rOldCell),
    maOldValue(sOldValue),
    pNextContent(nullptr),
    pPrevContent(nullptr),
    pNextInSlot(nullptr),
    ppPrevInSlot(nullptr)
{
    if (!maOldCell.isEmpty())
        SetCell(maOldValue, maOldCell, 0, pDoc);

    if (!sOldValue.isEmpty()) // #i40704# don't overwrite SetCell result with empty string
        maOldValue = sOldValue; // set again, because SetCell removes it
}

ScChangeActionContent::ScChangeActionContent( const sal_uLong nActionNumber,
            const ScCellValue& rNewCell, const ScBigRange& aBigRangeP,
            ScDocument* pDoc, const OUString& sNewValue ) :
    ScChangeAction(SC_CAT_CONTENT, aBigRangeP, nActionNumber),
    maNewCell(rNewCell),
    maNewValue(sNewValue),
    pNextContent(nullptr),
    pPrevContent(nullptr),
    pNextInSlot(nullptr),
    ppPrevInSlot(nullptr)
{
    if (!maNewCell.isEmpty())
        SetCell(maNewValue, maNewCell, 0, pDoc);

    if (!sNewValue.isEmpty()) // #i40704# don't overwrite SetCell result with empty string
        maNewValue = sNewValue; // set again, because SetCell removes it
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

ScChangeActionContent* ScChangeActionContent::GetTopContent() const
{
    if ( pNextContent )
    {
        ScChangeActionContent* pContent = pNextContent;
        while ( pContent->pNextContent && pContent != pContent->pNextContent )
            pContent = pContent->pNextContent;
        return pContent;
    }
    return const_cast<ScChangeActionContent*>(this);
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

void ScChangeActionContent::SetOldValue(
    const ScCellValue& rCell, const ScDocument* pFromDoc, ScDocument* pToDoc, sal_uLong nFormat )
{
    SetValue(maOldValue, maOldCell, nFormat, rCell, pFromDoc, pToDoc);
}

void ScChangeActionContent::SetOldValue(
    const ScCellValue& rCell, const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    SetValue(maOldValue, maOldCell, aBigRange.aStart.MakeAddress(), rCell, pFromDoc, pToDoc);
}

void ScChangeActionContent::SetNewValue( const ScCellValue& rCell, ScDocument* pDoc )
{
    SetValue(maNewValue, maNewCell, aBigRange.aStart.MakeAddress(), rCell, pDoc, pDoc);
}

void ScChangeActionContent::SetOldNewCells(
    const ScCellValue& rOldCell, sal_uLong nOldFormat, const ScCellValue& rNewCell,
    sal_uLong nNewFormat, ScDocument* pDoc )
{
    maOldCell = rOldCell;
    maNewCell = rNewCell;
    SetCell(maOldValue, maOldCell, nOldFormat, pDoc);
    SetCell(maNewValue, maNewCell, nNewFormat, pDoc);
}

void ScChangeActionContent::SetNewCell(
    const ScCellValue& rCell, ScDocument* pDoc, const OUString& rFormatted )
{
    maNewCell = rCell;
    SetCell(maNewValue, maNewCell, 0, pDoc);

    // #i40704# allow to set formatted text here - don't call SetNewValue with string from XML filter
    if (!rFormatted.isEmpty())
        maNewValue = rFormatted;
}

void ScChangeActionContent::SetValueString(
    OUString& rValue, ScCellValue& rCell, const OUString& rStr, ScDocument* pDoc )
{
    rCell.clear();
    if ( rStr.getLength() > 1 && rStr[0] == '=' )
    {
        rValue = EMPTY_OUSTRING;
        rCell.meType = CELLTYPE_FORMULA;
        rCell.mpFormula = new ScFormulaCell(
            pDoc, aBigRange.aStart.MakeAddress(), rStr,
            formula::FormulaGrammar::GRAM_DEFAULT, pDoc->GetGrammar() );
        rCell.mpFormula->SetInChangeTrack(true);
    }
    else
        rValue = rStr;
}

void ScChangeActionContent::SetOldValue( const OUString& rOld, ScDocument* pDoc )
{
    SetValueString(maOldValue, maOldCell, rOld, pDoc);
}

void ScChangeActionContent::GetOldString( OUString& rStr, const ScDocument* pDoc ) const
{
    GetValueString(rStr, maOldValue, maOldCell, pDoc);
}

void ScChangeActionContent::GetNewString( OUString& rStr, const ScDocument* pDoc ) const
{
    GetValueString(rStr, maNewValue, maNewCell, pDoc);
}

void ScChangeActionContent::GetDescription(
    OUString& rStr, ScDocument* pDoc, bool bSplitRange, bool bWarning ) const
{
    ScChangeAction::GetDescription( rStr, pDoc, bSplitRange, bWarning );

    OUString aRsc = ScGlobal::GetRscString(STR_CHANGED_CELL);

    OUString aTmpStr;
    GetRefString(aTmpStr, pDoc);

    sal_Int32 nPos = 0;
    nPos = aRsc.indexOf("#1", nPos);
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    GetOldString( aTmpStr, pDoc );
    if (aTmpStr.isEmpty())
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );

    nPos = nPos >= 0 ? aRsc.indexOf("#2", nPos) : -1;
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    GetNewString( aTmpStr, pDoc );
    if (aTmpStr.isEmpty())
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );

    nPos = nPos >= 0 ? aRsc.indexOf("#3", nPos) : -1;
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    OUStringBuffer aBuf(rStr); // append to the original string.
    aBuf.append(aRsc);
    rStr = aBuf.makeStringAndClear();
}

void ScChangeActionContent::GetRefString(
    OUString& rStr, ScDocument* pDoc, bool bFlag3D ) const
{
    sal_uInt16 nFlags = ( GetBigRange().IsValid( pDoc ) ? SCA_VALID : 0 );
    if ( nFlags )
    {
        const ScCellValue& rCell = GetNewCell();
        if ( GetContentCellType(rCell) == SC_CACCT_MATORG )
        {
            ScBigRange aLocalBigRange( GetBigRange() );
            SCCOL nC;
            SCROW nR;
            rCell.mpFormula->GetMatColsRows( nC, nR );
            aLocalBigRange.aEnd.IncCol( nC-1 );
            aLocalBigRange.aEnd.IncRow( nR-1 );
            rStr = ScChangeAction::GetRefString( aLocalBigRange, pDoc, bFlag3D );

            return ;
        }

        ScAddress aTmpAddress( GetBigRange().aStart.MakeAddress() );
        if ( bFlag3D )
            nFlags |= SCA_TAB_3D;
        rStr = aTmpAddress.Format(nFlags, pDoc, pDoc->GetAddressConvention());
        if ( IsDeletedIn() )
        {
            // Insert the parentheses.
            OUStringBuffer aBuf;
            aBuf.append('(');
            aBuf.append(rStr);
            aBuf.append(')');
            rStr = aBuf.makeStringAndClear();
        }
    }
    else
        rStr = ScGlobal::GetRscString( STR_NOREF_STR );
}

bool ScChangeActionContent::Reject( ScDocument* pDoc )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return false;

    PutOldValueToDoc( pDoc, 0, 0 );

    SetState( SC_CAS_REJECTED );
    RemoveAllLinks();

    return true;
}

bool ScChangeActionContent::Select( ScDocument* pDoc, ScChangeTrack* pTrack,
        bool bOldest, ::std::stack<ScChangeActionContent*>* pRejectActions )
{
    if ( !aBigRange.IsValid( pDoc ) )
        return false;

    ScChangeActionContent* pContent = this;
    // accept previous contents
    while ( ( pContent = pContent->pPrevContent ) != nullptr )
    {
        if ( pContent->IsVirgin() )
            pContent->SetState( SC_CAS_ACCEPTED );
    }
    ScChangeActionContent* pEnd = pContent = this;
    // reject subsequent contents
    while ( ( pContent = pContent->pNextContent ) != nullptr )
    {
        // MatrixOrigin may have dependents, no dependency recursion needed
        const ScChangeActionLinkEntry* pL = pContent->GetFirstDependentEntry();
        while ( pL )
        {
            ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
            if ( p )
                p->SetRejected();
            pL = pL->GetNext();
        }
        pContent->SetRejected();
        pEnd = pContent;
    }

    // If not oldest: Is it anyone else than the last one?
    if ( bOldest || pEnd != this )
    {   ScRange aRange( aBigRange.aStart.MakeAddress() );
        const ScAddress& rPos = aRange.aStart;

        ScChangeActionContent* pNew = new ScChangeActionContent( aRange );
        ScCellValue aCell;
        aCell.assign(*pDoc, rPos);
        pNew->SetOldValue(aCell, pDoc, pDoc);

        if ( bOldest )
            PutOldValueToDoc( pDoc, 0, 0 );
        else
            PutNewValueToDoc( pDoc, 0, 0 );

        pNew->SetRejectAction( bOldest ? GetActionNumber() : pEnd->GetActionNumber() );
        pNew->SetState( SC_CAS_ACCEPTED );
        if ( pRejectActions )
            pRejectActions->push( pNew );
        else
        {
            aCell.assign(*pDoc, rPos);
            pNew->SetNewValue(aCell, pDoc);
            pTrack->Append( pNew );
        }
    }

    if ( bOldest )
        SetRejected();
    else
        SetState( SC_CAS_ACCEPTED );

    return true;
}

void ScChangeActionContent::GetStringOfCell(
    OUString& rStr, const ScCellValue& rCell, const ScDocument* pDoc, const ScAddress& rPos )
{
    if (NeedsNumberFormat(rCell))
        GetStringOfCell(rStr, rCell, pDoc, pDoc->GetNumberFormat(rPos));
    else
        GetStringOfCell(rStr, rCell, pDoc, 0);
}

void ScChangeActionContent::GetStringOfCell(
    OUString& rStr, const ScCellValue& rCell, const ScDocument* pDoc, sal_uLong nFormat )
{
    rStr = EMPTY_OUSTRING;

    if (!GetContentCellType(rCell))
        return;

    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            pDoc->GetFormatTable()->GetInputLineString(rCell.mfValue, nFormat, rStr);
        break;
        case CELLTYPE_STRING:
            rStr = rCell.mpString->getString();
        break;
        case CELLTYPE_EDIT:
            if (rCell.mpEditText)
                rStr = ScEditUtil::GetString(*rCell.mpEditText, pDoc);
        break;
        case CELLTYPE_FORMULA:
            rCell.mpFormula->GetFormula(rStr);
        break;
        default:
            ;
    }
}

ScChangeActionContentCellType ScChangeActionContent::GetContentCellType( const ScCellValue& rCell )
{
    switch (rCell.meType)
    {
        case CELLTYPE_VALUE :
        case CELLTYPE_STRING :
        case CELLTYPE_EDIT :
            return SC_CACCT_NORMAL;
        case CELLTYPE_FORMULA :
            switch (rCell.mpFormula->GetMatrixFlag())
            {
                case MM_NONE :
                    return SC_CACCT_NORMAL;
                case MM_FORMULA :
                    return SC_CACCT_MATORG;
                case MM_REFERENCE :
                    return SC_CACCT_MATREF;
            }
            return SC_CACCT_NORMAL;
        default:
            return SC_CACCT_NONE;
    }
}

ScChangeActionContentCellType ScChangeActionContent::GetContentCellType( const ScRefCellValue& rCell )
{
    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return SC_CACCT_NORMAL;
        case CELLTYPE_FORMULA:
        {
            const ScFormulaCell* pCell = rCell.mpFormula;
            switch (pCell->GetMatrixFlag())
            {
                case MM_NONE :
                    return SC_CACCT_NORMAL;
                case MM_FORMULA :
                    return SC_CACCT_MATORG;
                case MM_REFERENCE :
                    return SC_CACCT_MATREF;
            }
            return SC_CACCT_NORMAL;
        }
        default:
            ;
    }

    return SC_CACCT_NONE;
}

bool ScChangeActionContent::NeedsNumberFormat( const ScCellValue& rVal )
{
    return rVal.meType == CELLTYPE_VALUE;
}

void ScChangeActionContent::SetValue(
    OUString& rStr, ScCellValue& rCell, const ScAddress& rPos, const ScCellValue& rOrgCell,
    const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    sal_uLong nFormat = NeedsNumberFormat(rOrgCell) ? pFromDoc->GetNumberFormat(rPos) : 0;
    SetValue(rStr, rCell, nFormat, rOrgCell, pFromDoc, pToDoc);
}

void ScChangeActionContent::SetValue(
    OUString& rStr, ScCellValue& rCell, sal_uLong nFormat, const ScCellValue& rOrgCell,
    const ScDocument* pFromDoc, ScDocument* pToDoc )
{
    rStr.clear();

    if (GetContentCellType(rOrgCell))
    {
        rCell.assign(rOrgCell, *pToDoc);
        switch (rOrgCell.meType)
        {
            case CELLTYPE_VALUE :
            {   // E.g.: Remember date as such
                pFromDoc->GetFormatTable()->GetInputLineString(
                    rOrgCell.mfValue, nFormat, rStr);
            }
            break;
            case CELLTYPE_FORMULA :
                rCell.mpFormula->SetInChangeTrack(true);
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    else
        rCell.clear();
}

void ScChangeActionContent::SetCell( OUString& rStr, ScCellValue& rCell, sal_uLong nFormat, const ScDocument* pDoc )
{
    rStr.clear();
    if (rCell.isEmpty())
        return;

    switch (rCell.meType)
    {
        case CELLTYPE_VALUE :
            // e.g. remember date as date string
            pDoc->GetFormatTable()->GetInputLineString(rCell.mfValue, nFormat, rStr);
        break;
        case CELLTYPE_FORMULA :
            rCell.mpFormula->SetInChangeTrack(true);
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

void ScChangeActionContent::GetValueString(
    OUString& rStr, const OUString& rValue, const ScCellValue& rCell, const ScDocument* pDoc ) const
{
    if (!rValue.isEmpty())
    {
        rStr = rValue;
        return;
    }

    switch (rCell.meType)
    {
        case CELLTYPE_STRING :
            rStr = rCell.mpString->getString();
        break;
        case CELLTYPE_EDIT :
            if (rCell.mpEditText)
                rStr = ScEditUtil::GetString(*rCell.mpEditText, pDoc);
        break;
        case CELLTYPE_VALUE : // Is always in rValue
            rStr = rValue;
        break;
        case CELLTYPE_FORMULA :
            GetFormulaString(rStr, rCell.mpFormula);
        break;
        case CELLTYPE_NONE:
        default:
            rStr.clear();
    }
}

void ScChangeActionContent::GetFormulaString(
    OUString& rStr, const ScFormulaCell* pCell ) const
{
    ScAddress aPos( aBigRange.aStart.MakeAddress() );
    if ( aPos == pCell->aPos || IsDeletedIn() )
        pCell->GetFormula( rStr );
    else
    {
        OSL_FAIL( "ScChangeActionContent::GetFormulaString: aPos != pCell->aPos" );
        std::unique_ptr<ScFormulaCell> pNew(new ScFormulaCell( *pCell, *pCell->GetDocument(), aPos ));
        pNew->GetFormula( rStr );
    }
}

void ScChangeActionContent::PutOldValueToDoc( ScDocument* pDoc,
        SCsCOL nDx, SCsROW nDy ) const
{
    PutValueToDoc(maOldCell, maOldValue, pDoc, nDx, nDy);
}

void ScChangeActionContent::PutNewValueToDoc( ScDocument* pDoc,
        SCsCOL nDx, SCsROW nDy ) const
{
    PutValueToDoc(maNewCell, maNewValue, pDoc, nDx, nDy);
}

void ScChangeActionContent::PutValueToDoc(
    const ScCellValue& rCell, const OUString& rValue, ScDocument* pDoc,
    SCsCOL nDx, SCsROW nDy ) const
{
    ScAddress aPos( aBigRange.aStart.MakeAddress() );
    if ( nDx )
        aPos.IncCol( nDx );
    if ( nDy )
        aPos.IncRow( nDy );

    if (!rValue.isEmpty())
    {
        pDoc->SetString(aPos, rValue);
        return;
    }

    if (rCell.isEmpty())
    {
        pDoc->SetEmptyCell(aPos);
        return;
    }

    if (rCell.meType == CELLTYPE_VALUE)
    {
        pDoc->SetString( aPos.Col(), aPos.Row(), aPos.Tab(), rValue );
        return;
    }

    switch (GetContentCellType(rCell))
    {
        case SC_CACCT_MATORG :
        {
            SCCOL nC;
            SCROW nR;
            rCell.mpFormula->GetMatColsRows(nC, nR);
            OSL_ENSURE( nC>0 && nR>0, "ScChangeActionContent::PutValueToDoc: MatColsRows?" );
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
                aDestMark, EMPTY_OUSTRING, rCell.mpFormula->GetCode());
        }
        break;
        case SC_CACCT_MATREF :
            // nothing
        break;
        default:
            rCell.commit(*pDoc, aPos);
    }
}

static void lcl_InvalidateReference( formula::FormulaToken& rTok, const ScBigAddress& rPos )
{
    ScSingleRefData& rRef1 = *rTok.GetSingleRef();
    if ( rPos.Col() < 0 || MAXCOL < rPos.Col() )
    {
        rRef1.SetColDeleted( true );
    }
    if ( rPos.Row() < 0 || MAXROW < rPos.Row() )
    {
        rRef1.SetRowDeleted( true );
    }
    if ( rPos.Tab() < 0 || MAXTAB < rPos.Tab() )
    {
        rRef1.SetTabDeleted( true );
    }
    if ( rTok.GetType() == formula::svDoubleRef )
    {
        ScSingleRefData& rRef2 = rTok.GetDoubleRef()->Ref2;
        if ( rPos.Col() < 0 || MAXCOL < rPos.Col() )
        {
            rRef2.SetColDeleted( true );
        }
        if ( rPos.Row() < 0 || MAXROW < rPos.Row() )
        {
            rRef2.SetRowDeleted( true );
        }
        if ( rPos.Tab() < 0 || MAXTAB < rPos.Tab() )
        {
            rRef2.SetTabDeleted( true );
        }
    }
}

void ScChangeActionContent::UpdateReference( const ScChangeTrack* pTrack,
        UpdateRefMode eMode, const ScBigRange& rRange,
        sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz )
{
    SCSIZE nOldSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
    ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aBigRange );
    SCSIZE nNewSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
    if ( nNewSlot != nOldSlot )
    {
        RemoveFromSlot();
        InsertInSlot( &(pTrack->GetContentSlots()[nNewSlot]) );
    }

    if ( pTrack->IsInDelete() && !pTrack->IsInDeleteTop() )
        return ; // Formula only update whole range

    bool bOldFormula = maOldCell.meType == CELLTYPE_FORMULA;
    bool bNewFormula = maNewCell.meType == CELLTYPE_FORMULA;
    if ( bOldFormula || bNewFormula )
    {   // Adjust UpdateReference via ScFormulaCell (there)
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
                {   // Delete starts there after removed range
                    // Position is changed there
                    if ( nDx )
                        aTmpRange.aStart.IncCol( -nDx );
                    if ( nDy )
                        aTmpRange.aStart.IncRow( -nDy );
                    if ( nDz )
                        aTmpRange.aStart.IncTab( -nDz );
                }
            break;
            case URM_MOVE :
                // Move is Source here and Target there
                // Position needs to be adjusted before that
                if ( bOldFormula )
                    maOldCell.mpFormula->aPos = aBigRange.aStart.MakeAddress();
                if ( bNewFormula )
                    maNewCell.mpFormula->aPos = aBigRange.aStart.MakeAddress();
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
            default:
            {
                // added to avoid warnings
            }
        }
        ScRange aRange( aTmpRange.MakeRange() );

        sc::RefUpdateContext aRefCxt(*pTrack->GetDocument());
        aRefCxt.meMode = eMode;
        aRefCxt.maRange = aRange;
        aRefCxt.mnColDelta = nDx;
        aRefCxt.mnRowDelta = nDy;
        aRefCxt.mnTabDelta = nDz;

        if ( bOldFormula )
            maOldCell.mpFormula->UpdateReference(aRefCxt);
        if ( bNewFormula )
            maNewCell.mpFormula->UpdateReference(aRefCxt);

        if ( !aBigRange.aStart.IsValid( pTrack->GetDocument() ) )
        {   //FIXME:
            // UpdateReference cannot handle positions outside of the Document.
            // Therefore set everything to #REF!
            //TODO: Remove the need for this hack! This means big changes to ScAddress etc.!
            const ScBigAddress& rPos = aBigRange.aStart;
            if ( bOldFormula )
            {
                formula::FormulaToken* t;
                ScTokenArray* pArr = maOldCell.mpFormula->GetCode();
                pArr->Reset();
                while ( ( t = pArr->GetNextReference() ) != nullptr )
                    lcl_InvalidateReference( *t, rPos );
                pArr->Reset();
                while ( ( t = pArr->GetNextReferenceRPN() ) != nullptr )
                    lcl_InvalidateReference( *t, rPos );
            }
            if ( bNewFormula )
            {
                formula::FormulaToken* t;
                ScTokenArray* pArr = maNewCell.mpFormula->GetCode();
                pArr->Reset();
                while ( ( t = pArr->GetNextReference() ) != nullptr )
                    lcl_InvalidateReference( *t, rPos );
                pArr->Reset();
                while ( ( t = pArr->GetNextReferenceRPN() ) != nullptr )
                    lcl_InvalidateReference( *t, rPos );
            }
        }
    }
}

bool ScChangeActionContent::IsMatrixOrigin() const
{
    return GetContentCellType(GetNewCell()) == SC_CACCT_MATORG;
}

bool ScChangeActionContent::IsOldMatrixReference() const
{
    return GetContentCellType(GetOldCell()) == SC_CACCT_MATREF;
}

//  ScChangeActionReject
ScChangeActionReject::ScChangeActionReject(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber,
    const ScBigRange& aBigRangeP, const OUString& aUserP,
    const DateTime& aDateTimeP, const OUString& sComment) :
    ScChangeAction(SC_CAT_CONTENT, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment)
{
}

bool ScChangeActionReject::Reject(ScDocument* /*pDoc*/)
{
    return false;
}

//  ScChangeTrack
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo )

const SCROW ScChangeTrack::nContentRowsPerSlot = InitContentRowsPerSlot();
const SCSIZE ScChangeTrack::nContentSlots =
    (MAXROWCOUNT) / InitContentRowsPerSlot() + 2;

SCROW ScChangeTrack::InitContentRowsPerSlot()
{
    const SCSIZE nMaxSlots = 0xffe0 / sizeof( ScChangeActionContent* ) - 2;
    SCROW nRowsPerSlot = (MAXROWCOUNT) / nMaxSlots;
    if ( nRowsPerSlot * nMaxSlots < sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
        ++nRowsPerSlot;
    return nRowsPerSlot;
}

ScChangeTrack::ScChangeTrack( ScDocument* pDocP ) :
        aFixDateTime( DateTime::SYSTEM ),
        pDoc( pDocP )
{
    Init();
    SC_MOD()->GetUserOptions().AddListener(this);

    ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
    memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
}

ScChangeTrack::ScChangeTrack( ScDocument* pDocP, const std::set<OUString>& aTempUserCollection) :
        maUserCollection(aTempUserCollection),
        aFixDateTime( DateTime::SYSTEM ),
        pDoc( pDocP )
{
    Init();
    SC_MOD()->GetUserOptions().AddListener(this);
    ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
    memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
}

ScChangeTrack::~ScChangeTrack()
{
    SC_MOD()->GetUserOptions().RemoveListener(this);
    DtorClear();
    delete [] ppContentSlots;
}

void ScChangeTrack::Init()
{
    pFirst = nullptr;
    pLast = nullptr;
    pFirstGeneratedDelContent = nullptr;
    pLastCutMove = nullptr;
    pLinkInsertCol = nullptr;
    pLinkInsertRow = nullptr;
    pLinkInsertTab = nullptr;
    pLinkMove = nullptr;
    pBlockModifyMsg = nullptr;
    nActionMax = 0;
    nGeneratedMin = SC_CHGTRACK_GENERATED_START;
    nMarkLastSaved = 0;
    nStartLastCut = 0;
    nEndLastCut = 0;
    nLastMerge = 0;
    eMergeState = SC_CTMS_NONE;
    bLoadSave = false;
    bInDelete = false;
    bInDeleteTop = false;
    bInDeleteUndo = false;
    bInPasteCut = false;
    bUseFixDateTime = false;
    bTimeNanoSeconds = true;

    const SvtUserOptions& rUserOpt = SC_MOD()->GetUserOptions();
    OUStringBuffer aBuf;
    aBuf.append(rUserOpt.GetFirstName());
    aBuf.append(' ');
    aBuf.append(rUserOpt.GetLastName());
    maUser = aBuf.makeStringAndClear();
    maUserCollection.insert(maUser);
}

void ScChangeTrack::DtorClear()
{
    ScChangeAction* p;
    ScChangeAction* pNext;
    ScChangeActionMap::iterator itChangeAction;
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
    for( itChangeAction = aPasteCutMap.begin(); itChangeAction != aPasteCutMap.end(); ++itChangeAction )
    {
        delete itChangeAction->second;
    }
    delete pLastCutMove;
    ClearMsgQueue();
}

void ScChangeTrack::ClearMsgQueue()
{
    if ( pBlockModifyMsg )
    {
        delete pBlockModifyMsg;
        pBlockModifyMsg = nullptr;
    }
    while ( !aMsgStackTmp.empty() )
    {
        delete aMsgStackTmp.top();
        aMsgStackTmp.pop();
    }
    while ( !aMsgStackFinal.empty() )
    {
        delete aMsgStackFinal.top();
        aMsgStackFinal.pop();
    }

    ScChangeTrackMsgQueue::iterator itQueue;
    for ( itQueue = aMsgQueue.begin(); itQueue != aMsgQueue.end(); ++itQueue)
        delete *itQueue;

    aMsgQueue.clear();
}

void ScChangeTrack::Clear()
{
    DtorClear();
    aMap.clear();
    aGeneratedMap.clear();
    aPasteCutMap.clear();
    maUserCollection.clear();
    maUser.clear();
    Init();
}

bool ScChangeTrack::IsGenerated( sal_uLong nAction ) const
{
    return nAction >= nGeneratedMin;
}

ScChangeAction* ScChangeTrack::GetAction( sal_uLong nAction ) const
{
    ScChangeActionMap::const_iterator it = aMap.find( nAction );
    if( it != aMap.end() )
        return it->second;
    else
        return nullptr;
}

ScChangeAction* ScChangeTrack::GetGenerated( sal_uLong nGenerated ) const
{
    ScChangeActionMap::const_iterator it = aGeneratedMap.find( nGenerated );
    if( it != aGeneratedMap.end() )
        return it->second;
    else
        return nullptr;
}

ScChangeAction* ScChangeTrack::GetActionOrGenerated( sal_uLong nAction ) const
{
    return IsGenerated( nAction ) ?
        GetGenerated( nAction ) :
        GetAction( nAction );
}
sal_uLong ScChangeTrack::GetLastSavedActionNumber() const
{
    return nMarkLastSaved;
}

void ScChangeTrack::SetLastSavedActionNumber(sal_uLong nNew)
{
    nMarkLastSaved = nNew;
}

ScChangeAction* ScChangeTrack::GetLastSaved() const
{
    ScChangeActionMap::const_iterator it = aMap.find( nMarkLastSaved );
    if( it != aMap.end() )
        return it->second;
    else
        return nullptr;
}

void ScChangeTrack::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 )
{
    if ( !pDoc->IsInDtorClear() )
    {
        const SvtUserOptions& rUserOptions = SC_MOD()->GetUserOptions();
        size_t nOldCount = maUserCollection.size();

        OUStringBuffer aBuf;
        aBuf.append(rUserOptions.GetFirstName());
        aBuf.append(' ');
        aBuf.append(rUserOptions.GetLastName());
        SetUser(aBuf.makeStringAndClear());

        if ( maUserCollection.size() != nOldCount )
        {
            //  New user in collection -> have to repaint because
            //  colors may be different now (#106697#).
            //  (Has to be done in the Notify handler, to be sure
            //  the user collection has already been updated)

            SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
            if (pDocSh)
                pDocSh->Broadcast( ScPaintHint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB), PAINT_GRID ) );
        }
    }
}

void ScChangeTrack::SetUser( const OUString& rUser )
{
    if ( IsLoadSave() )
        return ; // Do not destroy the Collection

    maUser = rUser;
    maUserCollection.insert(maUser);
}

void ScChangeTrack::StartBlockModify( ScChangeTrackMsgType eMsgType,
        sal_uLong nStartAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( pBlockModifyMsg )
            aMsgStackTmp.push( pBlockModifyMsg ); // Block in Block
        pBlockModifyMsg = new ScChangeTrackMsgInfo;
        pBlockModifyMsg->eMsgType = eMsgType;
        pBlockModifyMsg->nStartAction = nStartAction;
    }
}

void ScChangeTrack::EndBlockModify( sal_uLong nEndAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( pBlockModifyMsg )
        {
            if ( pBlockModifyMsg->nStartAction <= nEndAction )
            {
                pBlockModifyMsg->nEndAction = nEndAction;
                // Blocks dissolved in Blocks
                aMsgStackFinal.push( pBlockModifyMsg );
            }
            else
                delete pBlockModifyMsg;
            if (aMsgStackTmp.empty())
                pBlockModifyMsg = nullptr;
            else
            {
                pBlockModifyMsg = aMsgStackTmp.top(); // Maybe Block in Block
                aMsgStackTmp.pop();
            }
        }
        if ( !pBlockModifyMsg )
        {
            bool bNew = false;
            while ( !aMsgStackFinal.empty() )
            {
                aMsgQueue.push_back( aMsgStackFinal.top() );
                aMsgStackFinal.pop();
                bNew = true;
            }
            if ( bNew )
                aModifiedLink.Call( *this );
        }
    }
}

ScChangeTrackMsgQueue& ScChangeTrack::GetMsgQueue()
{
    return aMsgQueue;
}

void ScChangeTrack::NotifyModified( ScChangeTrackMsgType eMsgType,
        sal_uLong nStartAction, sal_uLong nEndAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( !pBlockModifyMsg || pBlockModifyMsg->eMsgType != eMsgType ||
                (IsGenerated( nStartAction ) &&
                (eMsgType == SC_CTM_APPEND || eMsgType == SC_CTM_REMOVE)) )
        {   // Append within Append e.g. not
            StartBlockModify( eMsgType, nStartAction );
            EndBlockModify( nEndAction );
        }
    }
}

void ScChangeTrack::MasterLinks( ScChangeAction* pAppend )
{
    ScChangeActionType eType = pAppend->GetType();

    if ( eType == SC_CAT_CONTENT )
    {
        if ( !IsGenerated( pAppend->GetActionNumber() ) )
        {
            SCSIZE nSlot = ComputeContentSlot(
                pAppend->GetBigRange().aStart.Row() );
            static_cast<ScChangeActionContent*>(pAppend)->InsertInSlot(
                &ppContentSlots[nSlot] );
        }
        return ;
    }

    if ( pAppend->IsRejecting() )
        return ; // Rejects do not have dependencies

    switch ( eType )
    {
        case SC_CAT_INSERT_COLS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertCol, pAppend );
            pAppend->AddLink( nullptr, pLink );
        }
        break;
        case SC_CAT_INSERT_ROWS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertRow, pAppend );
            pAppend->AddLink( nullptr, pLink );
        }
        break;
        case SC_CAT_INSERT_TABS :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkInsertTab, pAppend );
            pAppend->AddLink( nullptr, pLink );
        }
        break;
        case SC_CAT_MOVE :
        {
            ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
                &pLinkMove, pAppend );
            pAppend->AddLink( nullptr, pLink );
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

void ScChangeTrack::AppendLoaded( ScChangeAction* pAppend )
{
    aMap.insert( ::std::make_pair( pAppend->GetActionNumber(), pAppend ) );
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

void ScChangeTrack::Append( ScChangeAction* pAppend, sal_uLong nAction )
{
    if ( nActionMax < nAction )
        nActionMax = nAction;
    pAppend->SetUser( maUser );
    if ( bUseFixDateTime )
        pAppend->SetDateTimeUTC( aFixDateTime );
    pAppend->SetActionNumber( nAction );
    aMap.insert( ::std::make_pair( nAction, pAppend ) );
    // UpdateReference Inserts before Dependencies.
    // Delete rejecting Insert which had UpdateReference with Delete Undo.
    // UpdateReference also with pLast==NULL, as pAppend can be a Delete,
    // which could have generated DelContents.
    if ( pAppend->IsInsertType() && !pAppend->IsRejecting() )
        UpdateReference( pAppend, false );
    if ( !pLast )
        pFirst = pLast = pAppend;
    else
    {
        pLast->pNext = pAppend;
        pAppend->pPrev = pLast;
        pLast = pAppend;
        Dependencies( pAppend );
    }
    // UpdateReference does not Insert() after Dependencies.
    // Move rejecting Move, which had UpdateReference with Move Undo.
    // Do not delete content in ToRange.
    if ( !pAppend->IsInsertType() &&
            !(pAppend->GetType() == SC_CAT_MOVE && pAppend->IsRejecting()) )
        UpdateReference( pAppend, false );
    MasterLinks( pAppend );

    if ( aModifiedLink.IsSet() )
    {
        NotifyModified( SC_CTM_APPEND, nAction, nAction );
        if ( pAppend->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = static_cast<ScChangeActionContent*>(pAppend);
            if ( ( pContent = pContent->GetPrevContent() ) != nullptr )
            {
                sal_uLong nMod = pContent->GetActionNumber();
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
        ScDocument* pRefDoc, sal_uLong& nStartAction, sal_uLong& nEndAction, SCsTAB nDz )
{
    nStartAction = GetActionMax() + 1;
    AppendDeleteRange( rRange, pRefDoc, nDz, 0 );
    nEndAction = GetActionMax();
}

void ScChangeTrack::AppendDeleteRange( const ScRange& rRange,
        ScDocument* pRefDoc, SCsTAB nDz, sal_uLong nRejectingInsert )
{
    SetInDeleteRange( rRange );
    StartBlockModify( SC_CTM_APPEND, GetActionMax() + 1 );
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    for ( SCTAB nTab = nTab1; nTab <= nTab2; nTab++ )
    {
        if ( !pRefDoc || nTab < pRefDoc->GetTableCount() )
        {
            if ( nCol1 == 0 && nCol2 == MAXCOL )
            {   // Whole Row and/or Tables
                if ( nRow1 == 0 && nRow2 == MAXROW )
                {   // Whole Table
                    // TODO: Can't we do the whole Table as a whole?
                    ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
                    for ( SCCOL nCol = nCol1; nCol <= nCol2; nCol++ )
                    {   // Column by column is less than row by row
                        aRange.aStart.SetCol( nCol );
                        aRange.aEnd.SetCol( nCol );
                        if ( nCol == nCol2 )
                            SetInDeleteTop( true );
                        AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
                            nTab-nTab1 + nDz, nRejectingInsert );
                    }
                    // Still InDeleteTop!
                    AppendOneDeleteRange( rRange, pRefDoc, 0, 0,
                        nTab-nTab1 + nDz, nRejectingInsert );
                }
                else
                {   // Whole rows
                    ScRange aRange( 0, 0, nTab, MAXCOL, 0, nTab );
                    for ( SCROW nRow = nRow1; nRow <= nRow2; nRow++ )
                    {
                        aRange.aStart.SetRow( nRow );
                        aRange.aEnd.SetRow( nRow );
                        if ( nRow == nRow2 )
                            SetInDeleteTop( true );
                        AppendOneDeleteRange( aRange, pRefDoc, 0, nRow-nRow1,
                            0, nRejectingInsert );
                    }
                }
            }
            else if ( nRow1 == 0 && nRow2 == MAXROW )
            {   // Whole columns
                ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
                for ( SCCOL nCol = nCol1; nCol <= nCol2; nCol++ )
                {
                    aRange.aStart.SetCol( nCol );
                    aRange.aEnd.SetCol( nCol );
                    if ( nCol == nCol2 )
                        SetInDeleteTop( true );
                    AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
                        0, nRejectingInsert );
                }
            }
            else
            {
                OSL_FAIL( "ScChangeTrack::AppendDeleteRange: Block not supported!" );
            }
            SetInDeleteTop( false );
        }
    }
    EndBlockModify( GetActionMax() );
}

void ScChangeTrack::AppendOneDeleteRange( const ScRange& rOrgRange,
        ScDocument* pRefDoc, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
        sal_uLong nRejectingInsert )
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
    // TabDelete not Contents; they are in separate columns
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
        ScDocument* pRefDoc, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if (!pRefDoc)
        return;

    ScAddress aPos;
    ScBigAddress aBigPos;
    ScCellIterator aIter( pRefDoc, rOrgRange );
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (!ScChangeActionContent::GetContentCellType(aIter.getRefCellValue()))
            continue;

        aBigPos.Set( aIter.GetPos().Col() + nDx, aIter.GetPos().Row() + nDy,
            aIter.GetPos().Tab() + nDz );
        ScChangeActionContent* pContent = SearchContentAt( aBigPos, nullptr );
        if (pContent)
            continue;

        // Untracked Contents
        aPos.Set( aIter.GetPos().Col() + nDx, aIter.GetPos().Row() + nDy,
            aIter.GetPos().Tab() + nDz );

        GenerateDelContent(aPos, aIter.getCellValue(), pRefDoc);
        // The Content is _not_ added with AddContent here, but in UpdateReference.
        // We do this in order to e.g. handle intersecting Deletes correctly
    }
}

void ScChangeTrack::AppendMove( const ScRange& rFromRange,
        const ScRange& rToRange, ScDocument* pRefDoc )
{
    ScChangeActionMove* pAct = new ScChangeActionMove( rFromRange, rToRange, this );
    LookUpContents( rToRange, pRefDoc, 0, 0, 0 ); // Overwritten Contents
    Append( pAct );
}

bool ScChangeTrack::IsMatrixFormulaRangeDifferent(
    const ScCellValue& rOldCell, const ScCellValue& rNewCell )
{
    SCCOL nC1, nC2;
    SCROW nR1, nR2;
    nC1 = nC2 = 0;
    nR1 = nR2 = 0;

    if (rOldCell.meType == CELLTYPE_FORMULA && rOldCell.mpFormula->GetMatrixFlag() == MM_FORMULA)
        rOldCell.mpFormula->GetMatColsRows(nC1, nR1);

    if (rNewCell.meType == CELLTYPE_FORMULA && rNewCell.mpFormula->GetMatrixFlag() == MM_FORMULA)
        rNewCell.mpFormula->GetMatColsRows(nC1, nR1);

    return nC1 != nC2 || nR1 != nR2;
}

void ScChangeTrack::AppendContent(
    const ScAddress& rPos, const ScCellValue& rOldCell, sal_uLong nOldFormat, ScDocument* pRefDoc )
{
    if ( !pRefDoc )
        pRefDoc = pDoc;

    OUString aOldValue;
    ScChangeActionContent::GetStringOfCell(aOldValue, rOldCell, pRefDoc, nOldFormat);

    OUString aNewValue;
    ScCellValue aNewCell;
    aNewCell.assign(*pDoc, rPos);
    ScChangeActionContent::GetStringOfCell(aNewValue, aNewCell, pDoc, rPos);

    if (!aOldValue.equals(aNewValue) || IsMatrixFormulaRangeDifferent(rOldCell, aNewCell))
    {   // Only track real changes
        ScRange aRange( rPos );
        ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
        pAct->SetOldValue(rOldCell, pRefDoc, pDoc, nOldFormat);
        pAct->SetNewValue(aNewCell, pDoc);
        Append( pAct );
    }
}

void ScChangeTrack::AppendContent( const ScAddress& rPos,
        ScDocument* pRefDoc )
{
    OUString aOldValue;
    ScCellValue aOldCell;
    aOldCell.assign(*pRefDoc, rPos);
    ScChangeActionContent::GetStringOfCell(aOldValue, aOldCell, pRefDoc, rPos);

    OUString aNewValue;
    ScCellValue aNewCell;
    aNewCell.assign(*pDoc, rPos);
    ScChangeActionContent::GetStringOfCell(aNewValue, aNewCell, pDoc, rPos);

    if (!aOldValue.equals(aNewValue) || IsMatrixFormulaRangeDifferent(aOldCell, aNewCell))
    {   // Only track real changes
        ScRange aRange( rPos );
        ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
        pAct->SetOldValue(aOldCell, pRefDoc, pDoc);
        pAct->SetNewValue(aNewCell, pDoc);
        Append( pAct );
    }
}

void ScChangeTrack::AppendContent( const ScAddress& rPos, const ScCellValue& rOldCell )
{
    if (ScChangeActionContent::NeedsNumberFormat(rOldCell))
        AppendContent(rPos, rOldCell, pDoc->GetNumberFormat(rPos), pDoc);
    else
        AppendContent(rPos, rOldCell, 0, pDoc);
}

void ScChangeTrack::SetLastCutMoveRange( const ScRange& rRange,
        ScDocument* pRefDoc )
{
    if ( pLastCutMove )
    {
        // Do not link ToRange with Deletes and don't change its size
        // This is actually unnecessary, as a delete triggers a ResetLastCut
        // in ScViewFunc::PasteFromClip before that
        ScBigRange& r = pLastCutMove->GetBigRange();
        r.aEnd.SetCol( -1 );
        r.aEnd.SetRow( -1 );
        r.aEnd.SetTab( -1 );
        r.aStart.SetCol( -1 - (rRange.aEnd.Col() - rRange.aStart.Col()) );
        r.aStart.SetRow( -1 - (rRange.aEnd.Row() - rRange.aStart.Row()) );
        r.aStart.SetTab( -1 - (rRange.aEnd.Tab() - rRange.aStart.Tab()) );
        // Contents in FromRange we should overwrite
        LookUpContents( rRange, pRefDoc, 0, 0, 0 );
    }
}

void ScChangeTrack::AppendContentRange( const ScRange& rRange,
        ScDocument* pRefDoc, sal_uLong& nStartAction, sal_uLong& nEndAction,
        ScChangeActionClipMode eClipMode )
{
    if ( eClipMode == SC_CACM_CUT )
    {
        ResetLastCut();
        pLastCutMove = new ScChangeActionMove( rRange, rRange, this );
        SetLastCutMoveRange( rRange, pRefDoc );
    }
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    bool bDoContents;
    if ( eClipMode == SC_CACM_PASTE && HasLastCut() )
    {
        bDoContents = false;
        SetInPasteCut( true );
        // Adjust Paste and Cut; Paste can be larger a Range
        ScRange aRange( rRange );
        ScBigRange& r = pLastCutMove->GetBigRange();
        SCCOL nTmpCol;
        if ( (nTmpCol = (SCCOL) (r.aEnd.Col() - r.aStart.Col())) != (nCol2 - nCol1) )
        {
            aRange.aEnd.SetCol( aRange.aStart.Col() + nTmpCol );
            nCol1 += nTmpCol + 1;
            bDoContents = true;
        }
        SCROW nTmpRow;
        if ( (nTmpRow = (SCROW) (r.aEnd.Row() - r.aStart.Row())) != (nRow2 - nRow1) )
        {
            aRange.aEnd.SetRow( aRange.aStart.Row() + nTmpRow );
            nRow1 += nTmpRow + 1;
            bDoContents = true;
        }
        SCTAB nTmpTab;
        if ( (nTmpTab = (SCTAB) (r.aEnd.Tab() - r.aStart.Tab())) != (nTab2 - nTab1) )
        {
            aRange.aEnd.SetTab( aRange.aStart.Tab() + nTmpTab );
            nTab1 += nTmpTab + 1;
            bDoContents = true;
        }
        r = aRange;
        Undo( nStartLastCut, nEndLastCut ); // Remember Cuts here
        // StartAction only after Undo!
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
        // Contents to overwrite in ToRange
        LookUpContents( aRange, pRefDoc, 0, 0, 0 );
        pLastCutMove->SetStartLastCut( nStartLastCut );
        pLastCutMove->SetEndLastCut( nEndLastCut );
        Append( pLastCutMove );
        pLastCutMove = nullptr;
        ResetLastCut();
        SetInPasteCut( false );
    }
    else
    {
        bDoContents = true;
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
    }
    if ( bDoContents )
    {
        ScAddress aPos;
        for ( SCTAB nTab = nTab1; nTab <= nTab2; nTab++ )
        {
            aPos.SetTab( nTab );
            for ( SCCOL nCol = nCol1; nCol <= nCol2; nCol++ )
            {
                aPos.SetCol( nCol );
                for ( SCROW nRow = nRow1; nRow <= nRow2; nRow++ )
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
            sal_uLong& nStartAction, sal_uLong& nEndAction )
{
    ScCellIterator aIter(pRefDoc, ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB));
    if (aIter.first())
    {
        nStartAction = GetActionMax() + 1;
        StartBlockModify( SC_CTM_APPEND, nStartAction );
        SvNumberFormatter* pFormatter = pRefDoc->GetFormatTable();
        do
        {
            const ScAddress& rPos = aIter.GetPos();
            const ScPatternAttr* pPat = pRefDoc->GetPattern(rPos);
            AppendContent(
                rPos, aIter.getCellValue(), pPat->GetNumberFormat(pFormatter), pRefDoc);
        }
        while (aIter.next());

        nEndAction = GetActionMax();
        EndBlockModify( nEndAction );
    }
    else
        nStartAction = nEndAction = 0;
}

ScChangeActionContent* ScChangeTrack::AppendContentOnTheFly(
    const ScAddress& rPos, const ScCellValue& rOldCell, const ScCellValue& rNewCell,
    sal_uLong nOldFormat, sal_uLong nNewFormat )
{
    ScRange aRange( rPos );
    ScChangeActionContent* pAct = new ScChangeActionContent( aRange );
    pAct->SetOldNewCells(rOldCell, nOldFormat, rNewCell, nNewFormat, pDoc);
    Append( pAct );
    return pAct;
}

void ScChangeTrack::AppendInsert( const ScRange& rRange, bool bEndOfList )
{
    ScChangeActionIns* pAct = new ScChangeActionIns(rRange, bEndOfList);
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
    pCellList = nullptr;
}

ScChangeActionContent* ScChangeTrack::GenerateDelContent(
        const ScAddress& rPos, const ScCellValue& rCell, const ScDocument* pFromDoc )
{
    ScChangeActionContent* pContent = new ScChangeActionContent(
        ScRange( rPos ) );
    pContent->SetActionNumber( --nGeneratedMin );
    // Only NewValue
    ScChangeActionContent::SetValue( pContent->maNewValue, pContent->maNewCell,
        rPos, rCell, pFromDoc, pDoc );
    // pNextContent and pPrevContent are not set
    if ( pFirstGeneratedDelContent )
    {   // Insert at front
        pFirstGeneratedDelContent->pPrev = pContent;
        pContent->pNext = pFirstGeneratedDelContent;
    }
    pFirstGeneratedDelContent = pContent;
    aGeneratedMap.insert( std::make_pair( nGeneratedMin, pContent ) );
    NotifyModified( SC_CTM_APPEND, nGeneratedMin, nGeneratedMin );
    return pContent;
}

void ScChangeTrack::DeleteGeneratedDelContent( ScChangeActionContent* pContent )
{
    sal_uLong nAct = pContent->GetActionNumber();
    aGeneratedMap.erase( nAct );
    if ( pFirstGeneratedDelContent == pContent )
        pFirstGeneratedDelContent = static_cast<ScChangeActionContent*>(pContent->pNext);
    if ( pContent->pNext )
        pContent->pNext->pPrev = pContent->pPrev;
    if ( pContent->pPrev )
        pContent->pPrev->pNext = pContent->pNext;
    delete pContent;
    NotifyModified( SC_CTM_REMOVE, nAct, nAct );
    if ( nAct == nGeneratedMin )
        ++nGeneratedMin; // Only after NotifyModified due to IsGenerated!
}

ScChangeActionContent* ScChangeTrack::SearchContentAt(
        const ScBigAddress& rPos, ScChangeAction* pButNotThis ) const
{
    SCSIZE nSlot = ComputeContentSlot( rPos.Row() );
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
    return nullptr;
}

void ScChangeTrack::AddDependentWithNotify( ScChangeAction* pParent,
        ScChangeAction* pDependent )
{
    ScChangeActionLinkEntry* pLink = pParent->AddDependent( pDependent );
    pDependent->AddLink( pParent, pLink );
    if ( aModifiedLink.IsSet() )
    {
        sal_uLong nMod = pParent->GetActionNumber();
        NotifyModified( SC_CTM_PARENT, nMod, nMod );
    }
}

void ScChangeTrack::Dependencies( ScChangeAction* pAct )
{
    // Find the last dependency for Col/Row/Tab each
    // Concatenate Content at the same position
    // Move dependencies
    ScChangeActionType eActType = pAct->GetType();
    if ( eActType == SC_CAT_REJECT ||
            (eActType == SC_CAT_MOVE && pAct->IsRejecting()) )
        return ; // These Rejects are not dependent

    if ( eActType == SC_CAT_CONTENT )
    {
        if ( !(static_cast<ScChangeActionContent*>(pAct)->GetNextContent() ||
            static_cast<ScChangeActionContent*>(pAct)->GetPrevContent()) )
        {   // Concatenate Contents at same position
            ScChangeActionContent* pContent = SearchContentAt(
                pAct->GetBigRange().aStart, pAct );
            if ( pContent )
            {
                pContent->SetNextContent( static_cast<ScChangeActionContent*>(pAct) );
                static_cast<ScChangeActionContent*>(pAct)->SetPrevContent( pContent );
            }
        }
        const ScCellValue& rCell = static_cast<ScChangeActionContent*>(pAct)->GetNewCell();
        if ( ScChangeActionContent::GetContentCellType(rCell) == SC_CACCT_MATREF )
        {
            ScAddress aOrg;
            bool bOrgFound = rCell.mpFormula->GetMatrixOrigin(aOrg);
            ScChangeActionContent* pContent = (bOrgFound ? SearchContentAt( aOrg, pAct ) : nullptr);
            if ( pContent && pContent->IsMatrixOrigin() )
            {
                AddDependentWithNotify( pContent, pAct );
            }
            else
            {
                OSL_FAIL( "ScChangeTrack::Dependencies: MatOrg not found" );
            }
        }
    }

    if ( !(pLinkInsertCol || pLinkInsertRow || pLinkInsertTab || pLinkMove) )
        return ; // No Dependencies
    if ( pAct->IsRejecting() )
        return ; // Except for Content no Dependencies

    // Insert in a corresponding Insert depends on it or else we would need
    // to split the preceding one.
    // Intersecting Inserts and Deletes are not dependent, everything else
    // is dependent.
    // The Insert last linked in is at the beginning of a chain, just the way we need it

    const ScBigRange& rRange = pAct->GetBigRange();
    bool bActNoInsert = !pAct->IsInsertType();
    bool bActColDel = ( eActType == SC_CAT_DELETE_COLS );
    bool bActRowDel = ( eActType == SC_CAT_DELETE_ROWS );
    bool bActTabDel = ( eActType == SC_CAT_DELETE_TABS );

    if ( pLinkInsertCol && (eActType == SC_CAT_INSERT_COLS ||
            (bActNoInsert && !bActRowDel && !bActTabDel)) )
    {
        for ( ScChangeActionLinkEntry* pL = pLinkInsertCol; pL; pL = pL->GetNext() )
        {
            ScChangeActionIns* pTest = static_cast<ScChangeActionIns*>(pL->GetAction());
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
            ScChangeActionIns* pTest = static_cast<ScChangeActionIns*>(pL->GetAction());
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
            ScChangeActionIns* pTest = static_cast<ScChangeActionIns*>(pL->GetAction());
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
        {   // Content is depending on FromRange
            const ScBigAddress& rPos = rRange.aStart;
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = static_cast<ScChangeActionMove*>(pL->GetAction());
                if ( !pTest->IsRejected() &&
                        pTest->GetFromRange().In( rPos ) )
                {
                    AddDependentWithNotify( pTest, pAct );
                }
            }
        }
        else if ( eActType == SC_CAT_MOVE )
        {   // Move FromRange is depending on ToRange
            const ScBigRange& rFromRange = static_cast<ScChangeActionMove*>(pAct)->GetFromRange();
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = static_cast<ScChangeActionMove*>(pL->GetAction());
                if ( !pTest->IsRejected() &&
                        pTest->GetBigRange().Intersects( rFromRange ) )
                {
                    AddDependentWithNotify( pTest, pAct );
                }
            }
        }
        else
        {   // Inserts and Deletes are depending as soon as they cross FromRange or
            // ToRange
            for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
            {
                ScChangeActionMove* pTest = static_cast<ScChangeActionMove*>(pL->GetAction());
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
    // Remove from Track
    sal_uLong nAct = pRemove->GetActionNumber();
    aMap.erase( nAct );
    if ( nAct == nActionMax )
        --nActionMax;
    if ( pRemove == pLast )
        pLast = pRemove->pPrev;
    if ( pRemove == pFirst )
        pFirst = pRemove->pNext;
    if ( nAct == nMarkLastSaved )
        nMarkLastSaved =
            ( pRemove->pPrev ? pRemove->pPrev->GetActionNumber() : 0 );

    // Remove from global chain
    if ( pRemove->pNext )
        pRemove->pNext->pPrev = pRemove->pPrev;
    if ( pRemove->pPrev )
        pRemove->pPrev->pNext = pRemove->pNext;

    // Don't delete Dependencies
    // That happens automatically on delete by LinkEntry without traversing lists
    if ( aModifiedLink.IsSet() )
    {
        NotifyModified( SC_CTM_REMOVE, nAct, nAct );
        if ( pRemove->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = static_cast<ScChangeActionContent*>(pRemove);
            if ( ( pContent = pContent->GetPrevContent() ) != nullptr )
            {
                sal_uLong nMod = pContent->GetActionNumber();
                NotifyModified( SC_CTM_CHANGE, nMod, nMod );
            }
        }
        else if ( pLast )
            NotifyModified( SC_CTM_CHANGE, pFirst->GetActionNumber(),
                pLast->GetActionNumber() );
    }

    if ( IsInPasteCut() && pRemove->GetType() == SC_CAT_CONTENT )
    {   // Content is reused!
        ScChangeActionContent* pContent = static_cast<ScChangeActionContent*>(pRemove);
        pContent->RemoveAllLinks();
        pContent->ClearTrack();
        pContent->pNext = pContent->pPrev = nullptr;
        pContent->pNextContent = pContent->pPrevContent = nullptr;
    }
}

void ScChangeTrack::Undo( sal_uLong nStartAction, sal_uLong nEndAction, bool bMerge )
{
    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    if ( bMerge )
    {
        SetMergeState( SC_CTMS_UNDO );
    }

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
        for ( sal_uLong j = nEndAction; j >= nStartAction; --j )
        {   // Traverse backwards to recycle nActionMax and for faster access via pLast
            // Deletes are in right order
            ScChangeAction* pAct = IsLastAction(j) ? pLast : GetAction(j);

            if (!pAct)
                continue;

            if ( pAct->IsDeleteType() )
            {
                if (j == nEndAction || (pAct != pLast && static_cast<ScChangeActionDel*>(pAct)->IsTopDelete()))
                {
                    SetInDeleteTop( true );
                    SetInDeleteRange( static_cast<ScChangeActionDel*>(pAct)->GetOverAllRange().MakeRange() );
                }
            }
            UpdateReference( pAct, true );
            SetInDeleteTop( false );
            Remove( pAct );
            if ( IsInPasteCut() )
            {
                aPasteCutMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
                continue;
            }

            if ( j == nStartAction && pAct->GetType() == SC_CAT_MOVE )
            {
                ScChangeActionMove* pMove = static_cast<ScChangeActionMove*>(pAct);
                sal_uLong nStart = pMove->GetStartLastCut();
                sal_uLong nEnd = pMove->GetEndLastCut();
                if ( nStart && nStart <= nEnd )
                {   // Recover LastCut
                    // Break Links before Cut Append!
                    pMove->RemoveAllLinks();
                    StartBlockModify( SC_CTM_APPEND, nStart );
                    for ( sal_uLong nCut = nStart; nCut <= nEnd; nCut++ )
                    {
                        ScChangeActionMap::iterator itCut = aPasteCutMap.find( nCut );

                        if ( itCut != aPasteCutMap.end() )
                        {
                            OSL_ENSURE( aMap.find( nCut ) == aMap.end(), "ScChangeTrack::Undo: nCut dup" );
                            Append( itCut->second, nCut );
                            aPasteCutMap.erase( itCut );
                        }
                        else
                        {
                            OSL_FAIL( "ScChangeTrack::Undo: nCut not found" );
                        }
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
        EndBlockModify( nEndAction );
    }

    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    if ( bMerge )
    {
        SetMergeState( SC_CTMS_OTHER );
    }
}

bool ScChangeTrack::MergeIgnore( const ScChangeAction& rAction, sal_uLong nFirstMerge )
{
    if ( rAction.IsRejected() )
        return true; // There's still a suitable Reject Action coming

    if ( rAction.IsRejecting() && rAction.GetRejectAction() >= nFirstMerge )
        return true; // There it is

    return false; // Everything else
}

void ScChangeTrack::MergePrepare( ScChangeAction* pFirstMerge, bool bShared )
{
    SetMergeState( SC_CTMS_PREPARE );
    sal_uLong nFirstMerge = pFirstMerge->GetActionNumber();
    ScChangeAction* pAct = GetLast();
    if ( pAct )
    {
        SetLastMerge( pAct->GetActionNumber() );
        while ( pAct )
        {   // Traverse backwards; Deletes in right order
            // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
            if ( bShared || !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
            {
                if ( pAct->IsDeleteType() )
                {
                    if ( static_cast<ScChangeActionDel*>(pAct)->IsTopDelete() )
                    {
                        SetInDeleteTop( true );
                        SetInDeleteRange( static_cast<ScChangeActionDel*>(pAct)->
                            GetOverAllRange().MakeRange() );
                    }
                }
                UpdateReference( pAct, true );
                SetInDeleteTop( false );
                pAct->DeleteCellEntries(); // Else segfault in Track Clear()
            }
            pAct = ( pAct == pFirstMerge ? nullptr : pAct->GetPrev() );
        }
    }
    SetMergeState( SC_CTMS_OTHER ); // Preceding by default MergeOther!
}

void ScChangeTrack::MergeOwn( ScChangeAction* pAct, sal_uLong nFirstMerge, bool bShared )
{
    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    if ( bShared || !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
    {
        SetMergeState( SC_CTMS_OWN );
        if ( pAct->IsDeleteType() )
        {
            if ( static_cast<ScChangeActionDel*>(pAct)->IsTopDelete() )
            {
                SetInDeleteTop( true );
                SetInDeleteRange( static_cast<ScChangeActionDel*>(pAct)->
                    GetOverAllRange().MakeRange() );
            }
        }
        UpdateReference( pAct, false );
        SetInDeleteTop( false );
        SetMergeState( SC_CTMS_OTHER ); // Preceding by default MergeOther!
    }
}

void ScChangeTrack::UpdateReference( ScChangeAction* pAct, bool bUndo )
{
    ScChangeActionType eActType = pAct->GetType();
    if ( eActType == SC_CAT_CONTENT || eActType == SC_CAT_REJECT )
        return ;

    // Formula cells are not in the Document!
    bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( false );
    bool bOldNoListening = pDoc->GetNoListening();
    pDoc->SetNoListening( true );

    // Formula cells ExpandRefs synchronized to the ones in the Document!
    bool bOldExpandRefs = pDoc->IsExpandRefs();
    if ( (!bUndo && pAct->IsInsertType()) || (bUndo && pAct->IsDeleteType()) )
        pDoc->SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );

    if ( pAct->IsDeleteType() )
    {
        SetInDeleteUndo( bUndo );
        SetInDelete( true );
    }
    else if ( GetMergeState() == SC_CTMS_OWN )
    {
        // Recover references of formula cells
        // Previous MergePrepare behaved like a Delete when Inserting
        if ( pAct->IsInsertType() )
            SetInDeleteUndo( true );
    }

    // First the generated ones, as if they were tracked previously!
    if ( pFirstGeneratedDelContent )
        UpdateReference( reinterpret_cast<ScChangeAction**>(&pFirstGeneratedDelContent), pAct,
            bUndo );
    UpdateReference( &pFirst, pAct, bUndo );

    SetInDelete( false );
    SetInDeleteUndo( false );

    pDoc->SetExpandRefs( bOldExpandRefs );
    pDoc->SetNoListening( bOldNoListening );
    pDoc->SetAutoCalc( bOldAutoCalc );
}

void ScChangeTrack::UpdateReference( ScChangeAction** ppFirstAction,
        ScChangeAction* pAct, bool bUndo )
{
    ScChangeActionType eActType = pAct->GetType();
    bool bGeneratedDelContents =
        ( ppFirstAction == reinterpret_cast<ScChangeAction**>(&pFirstGeneratedDelContent) );
    const ScBigRange& rOrgRange = pAct->GetBigRange();
    ScBigRange aRange( rOrgRange );
    ScBigRange aDelRange( rOrgRange );
    sal_Int32 nDx, nDy, nDz;
    nDx = nDy = nDz = 0;
    UpdateRefMode eMode = URM_INSDEL;
    bool bDel = false;
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
            bDel = true;
        break;
        case SC_CAT_DELETE_ROWS :
            aRange.aEnd.SetRow( nInt32Max );
            nDy = -(rOrgRange.aEnd.Row() - rOrgRange.aStart.Row() + 1);
            aDelRange.aEnd.SetRow( aDelRange.aStart.Row() - nDy - 1 );
            bDel = true;
        break;
        case SC_CAT_DELETE_TABS :
            aRange.aEnd.SetTab( nInt32Max );
            nDz = -(rOrgRange.aEnd.Tab() - rOrgRange.aStart.Tab() + 1);
            aDelRange.aEnd.SetTab( aDelRange.aStart.Tab() - nDz - 1 );
            bDel = true;
        break;
        case SC_CAT_MOVE :
            eMode = URM_MOVE;
            static_cast<ScChangeActionMove*>(pAct)->GetDelta( nDx, nDy, nDz );
        break;
        default:
            OSL_FAIL( "ScChangeTrack::UpdateReference: unknown Type" );
    }
    if ( bUndo )
    {
        nDx = -nDx;
        nDy = -nDy;
        nDz = -nDz;
    }
    if ( bDel )
    {   // For this mechanism we assume:
        // There's only a whole, simple deleted row/column
        ScChangeActionDel* pActDel = static_cast<ScChangeActionDel*>(pAct);
        if ( !bUndo )
        {   // Delete
            ScChangeActionType eInsType = SC_CAT_NONE; // for Insert Undo "Deletes"
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
                default:
                {
                    // added to avoid warnings
                }
            }
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                bool bUpdate = true;
                if ( GetMergeState() == SC_CTMS_OTHER &&
                        p->GetActionNumber() <= GetLastMerge() )
                {   // Delete in merged Document, Action in the one to be merged
                    if ( p->IsInsertType() )
                    {
                        // On Insert only adjust references if the Delete does
                        // not intersect the Insert
                        if ( !aDelRange.Intersects( p->GetBigRange() ) )
                            p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                        bUpdate = false;
                    }
                    else if ( p->GetType() == SC_CAT_CONTENT &&
                            p->IsDeletedInDelType( eInsType ) )
                    {   // Content in Insert Undo "Delete"
                        // Do not adjust if this Delete would be in the Insert "Delete" (was just moved)
                        if ( aDelRange.In( p->GetBigRange().aStart ) )
                            bUpdate = false;
                        else
                        {
                            const ScChangeActionLinkEntry* pLink = p->GetDeletedIn();
                            while ( pLink && bUpdate )
                            {
                                const ScChangeAction* pDel = pLink->GetAction();
                                if ( pDel && pDel->GetType() == eInsType &&
                                        pDel->GetBigRange().In( aDelRange ) )
                                    bUpdate = false;
                                pLink = pLink->GetNext();
                            }
                        }
                    }
                    if ( !bUpdate )
                        continue; // for
                }
                if ( aDelRange.In( p->GetBigRange() ) )
                {
                    // Do not adjust within a just deleted range,
                    // instead assign the range.
                    // Stack up ranges that have been deleted multiple times.
                    // Intersecting Deletes cause "multiple delete" to be set.
                    if ( !p->IsDeletedInDelType( eActType ) )
                    {
                        p->SetDeletedIn( pActDel );
                        // Add GeneratedDelContent to the to-be-deleted list
                        if ( bGeneratedDelContents )
                            pActDel->AddContent( static_cast<ScChangeActionContent*>(p) );
                    }
                    bUpdate = false;
                }
                else
                {
                    // Cut off inserted ranges, if Start/End is within the Delete,
                    // but the Insert is not completely within the Delete or
                    // the Delete is not completelty within the Insert.
                    // The Delete remembers which Insert it has cut off from;
                    // it can also just be a single Insert (because Delete has
                    // a single column/is a single row).
                    // There can be a lot of cut-off Moves.
                    //
                    // ! A Delete is always a single column/a single row, therefore
                    // ! 1 without calculating the intersection.
                    switch ( p->GetType() )
                    {
                        case SC_CAT_INSERT_COLS :
                            if ( eActType == SC_CAT_DELETE_COLS )
                            {
                                if ( aDelRange.In( p->GetBigRange().aStart ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        static_cast<ScChangeActionIns*>(p), 1 );
                                    p->GetBigRange().aStart.IncCol();
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        static_cast<ScChangeActionIns*>(p), -1 );
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
                                        static_cast<ScChangeActionIns*>(p), 1 );
                                    p->GetBigRange().aStart.IncRow();
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        static_cast<ScChangeActionIns*>(p), -1 );
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
                                        static_cast<ScChangeActionIns*>(p), 1 );
                                    p->GetBigRange().aStart.IncTab();
                                }
                                else if ( aDelRange.In( p->GetBigRange().aEnd ) )
                                {
                                    pActDel->SetCutOffInsert(
                                        static_cast<ScChangeActionIns*>(p), -1 );
                                    p->GetBigRange().aEnd.IncTab( -1 );
                                }
                            }
                        break;
                        case SC_CAT_MOVE :
                        {
                            ScChangeActionMove* pMove = static_cast<ScChangeActionMove*>(p);
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
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
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
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
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
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
                if ( bUpdate )
                {
                    p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                    if ( p->GetType() == eActType && !p->IsRejected() &&
                            !pActDel->IsDeletedIn() &&
                            p->GetBigRange().In( aDelRange ) )
                        pActDel->SetDeletedIn( p ); // Slipped underneath it
                }
            }
        }
        else
        {   // Undo Delete
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                bool bUpdate = true;
                if ( aDelRange.In( p->GetBigRange() ) )
                {
                    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
                    if ( GetMergeState() == SC_CTMS_UNDO && !p->IsDeletedIn( pAct ) && pAct->IsDeleteType() &&
                         ( p->GetType() == SC_CAT_CONTENT ||
                           p->GetType() == SC_CAT_DELETE_ROWS || p->GetType() == SC_CAT_DELETE_COLS ||
                           p->GetType() == SC_CAT_INSERT_ROWS || p->GetType() == SC_CAT_INSERT_COLS ) )
                    {
                        p->SetDeletedIn( pAct );
                    }

                    if ( p->IsDeletedInDelType( eActType ) )
                    {
                        if ( p->IsDeletedIn( pActDel ) )
                        {
                            if ( p->GetType() != SC_CAT_CONTENT ||
                                    static_cast<ScChangeActionContent*>(p)->IsTopContent() )
                            {   // First really remove the TopContent
                                p->RemoveDeletedIn( pActDel );
                                // Do NOT delete GeneratedDelContent from the list, we might need
                                // it later on for Reject; we delete in DeleteCellEntries
                            }
                        }
                        bUpdate = false;
                    }
                    else if ( eActType != SC_CAT_DELETE_TABS &&
                            p->IsDeletedInDelType( SC_CAT_DELETE_TABS ) )
                    {   // Do not update in deleted Tables except for when moving Tables
                        bUpdate = false;
                    }
                    if ( p->GetType() == eActType && pActDel->IsDeletedIn( p ) )
                    {
                        pActDel->RemoveDeletedIn( p );// Slipped underneath
                        bUpdate = true;
                    }
                }
                if ( bUpdate )
                    p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
            }
            if ( !bGeneratedDelContents )
            {   // These are else also needed for the real Undo
                pActDel->UndoCutOffInsert();
                pActDel->UndoCutOffMoves();
            }
        }
    }
    else if ( eActType == SC_CAT_MOVE )
    {
        ScChangeActionMove* pActMove = static_cast<ScChangeActionMove*>(pAct);
        bool bLastCutMove = ( pActMove == pLastCutMove );
        const ScBigRange& rTo = pActMove->GetBigRange();
        const ScBigRange& rFrom = pActMove->GetFromRange();
        if ( !bUndo )
        {   // Move
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue; // for
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // Delete content in Target (Move Content to Source)
                    if ( rTo.In( p->GetBigRange() ) )
                    {
                        if ( !p->IsDeletedIn( pActMove ) )
                        {
                            p->SetDeletedIn( pActMove );
                            // Add GeneratedDelContent to the to-be-deleted list
                            if ( bGeneratedDelContents )
                                pActMove->AddContent( static_cast<ScChangeActionContent*>(p) );
                        }
                    }
                    else if ( bLastCutMove &&
                            p->GetActionNumber() > nEndLastCut &&
                            rFrom.In( p->GetBigRange() ) )
                    {   // Paste Cut: insert new Content inserted after stays
                        // Split up the ContentChain
                        ScChangeActionContent *pHere, *pTmp;
                        pHere = static_cast<ScChangeActionContent*>(p);
                        while ( (pTmp = pHere->GetPrevContent()) != nullptr &&
                                pTmp->GetActionNumber() > nEndLastCut )
                            pHere = pTmp;
                        if ( pTmp )
                        {   // Becomes TopContent of the Move
                            pTmp->SetNextContent( nullptr );
                            pHere->SetPrevContent( nullptr );
                        }
                        do
                        {   // Recover dependency from FromRange
                            AddDependentWithNotify( pActMove, pHere );
                        } while ( ( pHere = pHere->GetNextContent() ) != nullptr );
                    }
                    // #i87003# [Collaboration] Move range and insert content in FromRange is not merged correctly
                    else if ( ( GetMergeState() != SC_CTMS_PREPARE && GetMergeState() != SC_CTMS_OWN ) || p->GetActionNumber() <= pAct->GetActionNumber() )
                        p->UpdateReference( this, eMode, rFrom, nDx, nDy, nDz );
                }
            }
        }
        else
        {   // Undo Move
            bool bActRejected = pActMove->IsRejected();
            for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
            {
                if ( p == pAct )
                    continue;   // for
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // Move Content into Target if not deleted else to delete (FIXME: What?)
                    if ( p->IsDeletedIn( pActMove ) )
                    {
                        if ( static_cast<ScChangeActionContent*>(p)->IsTopContent() )
                        {   // First really remove the TopContent
                            p->RemoveDeletedIn( pActMove );
                            // Do NOT delete GeneratedDelContent from the list, we might need
                            // it later on for Reject; we delete in DeleteCellEntries
                        }
                    }
                    // #i87003# [Collaboration] Move range and insert content in FromRange is not merged correctly
                    else if ( ( GetMergeState() != SC_CTMS_PREPARE && GetMergeState() != SC_CTMS_OWN ) || p->GetActionNumber() <= pAct->GetActionNumber() )
                        p->UpdateReference( this, eMode, rTo, nDx, nDy, nDz );
                    if ( bActRejected &&
                            static_cast<ScChangeActionContent*>(p)->IsTopContent() &&
                            rFrom.In( p->GetBigRange() ) )
                    {   // Recover dependency to write Content
                        ScChangeActionLinkEntry* pLink =
                            pActMove->AddDependent( p );
                        p->AddLink( pActMove, pLink );
                    }
                }
            }
        }
    }
    else
    {   // Insert/Undo Insert
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
                // "Delete" in Insert-Undo
                const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
                while ( pLink )
                {
                    ScChangeAction* p = const_cast<ScChangeAction*>(pLink->GetAction());
                    if ( p )
                        p->SetDeletedIn( pAct );
                    pLink = pLink->GetNext();
                }

                // #i87049# [Collaboration] Conflict between delete row and insert content is not merged correctly
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( !p->IsDeletedIn( pAct ) && pAct->IsInsertType() &&
                         // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
                         ( p->GetType() == SC_CAT_CONTENT ||
                           p->GetType() == SC_CAT_DELETE_ROWS || p->GetType() == SC_CAT_DELETE_COLS ||
                           p->GetType() == SC_CAT_INSERT_ROWS || p->GetType() == SC_CAT_INSERT_COLS ) &&
                         pAct->GetBigRange().Intersects( p->GetBigRange() ) )
                    {
                        p->SetDeletedIn( pAct );
                    }
                }

                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                        continue;   // for
                    if ( !p->IsDeletedIn( pAct )
                         // #i95212# [Collaboration] Bad handling of row insertion in shared spreadsheet
                         && p->GetActionNumber() <= pAct->GetActionNumber() )
                    {
                        p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                    }
                }
            }
            break;
            case SC_CTMS_OWN :
            {
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                        continue;   // for
                    if ( !p->IsDeletedIn( pAct )
                         // #i95212# [Collaboration] Bad handling of row insertion in shared spreadsheet
                         && p->GetActionNumber() <= pAct->GetActionNumber() )
                    {
                        p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                    }
                }
                // Undo "Delete" in Insert-Undo
                const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
                while ( pLink )
                {
                    ScChangeAction* p = const_cast<ScChangeAction*>(pLink->GetAction());
                    if ( p )
                        p->RemoveDeletedIn( pAct );
                    pLink = pLink->GetNext();
                }

                // #i87049# [Collaboration] Conflict between delete row and insert content is not merged correctly
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p->IsDeletedIn( pAct ) && pAct->IsInsertType() &&
                         // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
                         ( p->GetType() == SC_CAT_CONTENT ||
                           p->GetType() == SC_CAT_DELETE_ROWS || p->GetType() == SC_CAT_DELETE_COLS ||
                           p->GetType() == SC_CAT_INSERT_ROWS || p->GetType() == SC_CAT_INSERT_COLS ) &&
                         pAct->GetBigRange().Intersects( p->GetBigRange() ) )
                    {
                        p->RemoveDeletedIn( pAct );
                    }
                }
            }
            break;
            // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
            case SC_CTMS_UNDO :
            {
                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( !p->IsDeletedIn( pAct ) && pAct->IsInsertType() &&
                         ( p->GetType() == SC_CAT_CONTENT ||
                           p->GetType() == SC_CAT_DELETE_ROWS || p->GetType() == SC_CAT_DELETE_COLS ||
                           p->GetType() == SC_CAT_INSERT_ROWS || p->GetType() == SC_CAT_INSERT_COLS ) &&
                         pAct->GetBigRange().Intersects( p->GetBigRange() ) )
                    {
                        p->SetDeletedIn( pAct );
                    }
                }

                for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
                {
                    if ( p == pAct )
                    {
                        continue;
                    }
                    if ( !p->IsDeletedIn( pAct ) && p->GetActionNumber() <= pAct->GetActionNumber() )
                    {
                        p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                    }
                }
            }
            break;
        }
    }
}

void ScChangeTrack::GetDependents( ScChangeAction* pAct,
        ScChangeActionMap& rMap, bool bListMasterDelete, bool bAllFlat ) const
{
    //TODO: bAllFlat==TRUE: called internally from Accept or Reject
    //TODO:                 => Generated will not be added
    bool bIsDelete = pAct->IsDeleteType();
    bool bIsMasterDelete = ( bListMasterDelete && pAct->IsMasterDelete() );

    const ScChangeAction* pCur = nullptr;
    ::std::stack<ScChangeAction*> cStack;
    cStack.push(pAct);

    while ( !cStack.empty() )
    {
        pCur = cStack.top();
        cStack.pop();

        if ( pCur->IsInsertType() )
        {
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        sal_uLong n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                            if ( p->HasDependent() )
                                cStack.push( p );
                    }
                    else
                    {
                        if ( p->GetType() == SC_CAT_CONTENT )
                        {
                            if ( static_cast<ScChangeActionContent*>(p)->IsTopContent() )
                                rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                        }
                        else
                            rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                    }
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->IsDeleteType() )
        {
            if ( bIsDelete )
            {   // Contents of deleted Ranges are only of interest on Delete
                ScChangeActionDel* pDel = const_cast<ScChangeActionDel*>(static_cast<const ScChangeActionDel*>(pCur));
                if ( !bAllFlat && bIsMasterDelete && pCur == pAct )
                {
                    // Corresponding Deletes to this Delete to the same level,
                    // if this Delete is at the top of a Row
                    ScChangeActionType eType = pDel->GetType();
                    ScChangeAction* p = pDel;
                    while ( (p = p->GetPrev()) != nullptr && p->GetType() == eType &&
                            !static_cast<ScChangeActionDel*>(p)->IsTopDelete() )
                        rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                    // delete this in the map too
                    rMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
                }
                else
                {
                    const ScChangeActionLinkEntry* pL = pCur->GetFirstDeletedEntry();
                    while ( pL )
                    {
                        ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                        if ( p != pAct )
                        {
                            if ( bAllFlat )
                            {
                                // Only a TopContent of a chain is in LinkDeleted
                                sal_uLong n = p->GetActionNumber();
                                if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                                    if ( p->HasDeleted() ||
                                            p->GetType() == SC_CAT_CONTENT )
                                        cStack.push( p );
                            }
                            else
                            {
                                if ( p->IsDeleteType() )
                                {   // Further TopDeletes to same level: it's not rejectable
                                    if ( static_cast<ScChangeActionDel*>(p)->IsTopDelete() )
                                        rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                                }
                                else
                                    rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                            }
                        }
                        pL = pL->GetNext();
                    }
                }
            }
        }
        else if ( pCur->GetType() == SC_CAT_MOVE )
        {
            // Deleted Contents in ToRange
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDeletedEntry();
            while ( pL )
            {
                ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                if ( p != pAct && rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) ).second )
                {
                    // Only one TopContent of a chain is in LinkDeleted
                    if ( bAllFlat && (p->HasDeleted() ||
                            p->GetType() == SC_CAT_CONTENT) )
                        cStack.push( p );
                }
                pL = pL->GetNext();
            }
            // New Contents in FromRange or new FromRange in ToRange
            // or Inserts/Deletes in FromRange/ToRange
            pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        sal_uLong n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                            if ( p->HasDependent() || p->HasDeleted() )
                                cStack.push( p );
                    }
                    else
                    {
                        if ( p->GetType() == SC_CAT_CONTENT )
                        {
                            if ( static_cast<ScChangeActionContent*>(p)->IsTopContent() )
                                rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                        }
                        else
                            rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                    }
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->GetType() == SC_CAT_CONTENT )
        {   // All changes at same position
            ScChangeActionContent* pContent = const_cast<ScChangeActionContent*>(static_cast<const ScChangeActionContent*>(pCur));
            // All preceding ones
            while ( ( pContent = pContent->GetPrevContent() ) != nullptr )
            {
                if ( !pContent->IsRejected() )
                    rMap.insert( ::std::make_pair( pContent->GetActionNumber(), pContent ) );
            }
            pContent = const_cast<ScChangeActionContent*>(static_cast<const ScChangeActionContent*>(pCur));
            // All succeeding ones
            while ( ( pContent = pContent->GetNextContent() ) != nullptr )
            {
                if ( !pContent->IsRejected() )
                    rMap.insert( ::std::make_pair( pContent->GetActionNumber(), pContent ) );
            }
            // all MatrixReferences of a MatrixOrigin
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
                if ( p != pAct )
                {
                    if ( bAllFlat )
                    {
                        sal_uLong n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                            if ( p->HasDependent() )
                                cStack.push( p );
                    }
                    else
                        rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                }
                pL = pL->GetNext();
            }
        }
        else if ( pCur->GetType() == SC_CAT_REJECT )
        {
            if ( bAllFlat )
            {
                ScChangeAction* p = GetAction(
                        static_cast<const ScChangeActionReject*>(pCur)->GetRejectAction() );
                if (p != pAct && rMap.find( p->GetActionNumber() ) == rMap.end())
                    cStack.push( p );
            }
        }
    }
}

bool ScChangeTrack::SelectContent( ScChangeAction* pAct, bool bOldest )
{
    if ( pAct->GetType() != SC_CAT_CONTENT )
        return false;

    ScChangeActionContent* pContent = static_cast<ScChangeActionContent*>(pAct);
    if ( bOldest )
    {
        pContent = pContent->GetTopContent();
        ScChangeActionContent* pPrevContent;
        while ( (pPrevContent = pContent->GetPrevContent()) != nullptr &&
                pPrevContent->IsVirgin() )
            pContent = pPrevContent;
    }

    if ( !pContent->IsClickable() )
        return false;

    ScBigRange aBigRange( pContent->GetBigRange() );
    const ScCellValue& rCell = (bOldest ? pContent->GetOldCell() : pContent->GetNewCell());
    if ( ScChangeActionContent::GetContentCellType(rCell) == SC_CACCT_MATORG )
    {
        SCCOL nC;
        SCROW nR;
        rCell.mpFormula->GetMatColsRows(nC, nR);
        aBigRange.aEnd.IncCol( nC-1 );
        aBigRange.aEnd.IncRow( nR-1 );
    }

    if ( !aBigRange.IsValid( pDoc ) )
        return false;

    ScRange aRange( aBigRange.MakeRange() );
    if ( !pDoc->IsBlockEditable( aRange.aStart.Tab(), aRange.aStart.Col(),
            aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() ) )
        return false;

    if ( pContent->HasDependent() )
    {
        bool bOk = true;
        ::std::stack<ScChangeActionContent*> aRejectActions;
        const ScChangeActionLinkEntry* pL = pContent->GetFirstDependentEntry();
        while ( pL )
        {
            ScChangeAction* p = const_cast<ScChangeAction*>(pL->GetAction());
            if ( p != pContent )
            {
                if ( p->GetType() == SC_CAT_CONTENT )
                {
                    // we don't need no recursion here, do we?
                    bOk &= static_cast<ScChangeActionContent*>(p)->Select( pDoc, this,
                        bOldest, &aRejectActions );
                }
                else
                {
                    OSL_FAIL( "ScChangeTrack::SelectContent: content dependent no content" );
                }
            }
            pL = pL->GetNext();
        }

        bOk &= pContent->Select( pDoc, this, bOldest, nullptr );
        // now the matrix is inserted and new content values are ready

        while ( !aRejectActions.empty() )
        {
            ScChangeActionContent* pNew = aRejectActions.top();
            aRejectActions.pop();
            ScAddress aPos( pNew->GetBigRange().aStart.MakeAddress() );
            ScCellValue aCell;
            aCell.assign(*pDoc, aPos);
            pNew->SetNewValue(aCell, pDoc);
            Append( pNew );
        }
        return bOk;
    }
    else
        return pContent->Select( pDoc, this, bOldest, nullptr );
}

void ScChangeTrack::AcceptAll()
{
    for ( ScChangeAction* p = GetFirst(); p; p = p->GetNext() )
    {
        p->Accept();
    }
}

bool ScChangeTrack::Accept( ScChangeAction* pAct )
{
    if ( !pAct->IsClickable() )
        return false;

    if ( pAct->IsDeleteType() || pAct->GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionMap aActionMap;
        ScChangeActionMap::iterator itChangeAction;

        GetDependents( pAct, aActionMap, false, true );

        for( itChangeAction = aActionMap.begin(); itChangeAction != aActionMap.end(); ++itChangeAction )
        {
            itChangeAction->second->Accept();
        }
    }
    pAct->Accept();
    return true;
}

bool ScChangeTrack::RejectAll()
{
    bool bOk = true;
    for ( ScChangeAction* p = GetLast(); p && bOk; p = p->GetPrev() )
    {   //TODO: Traverse backwards as dependencies attached to RejectActions
        if ( p->IsInternalRejectable() )
            bOk = Reject( p );
    }
    return bOk;
}

bool ScChangeTrack::Reject( ScChangeAction* pAct, bool bShared )
{
    // #i100895# When collaboration changes are reversed, it must be possible
    // to reject a deleted row above another deleted row.
    if ( bShared && pAct->IsDeletedIn() )
        pAct->RemoveAllDeletedIn();

    if ( !pAct->IsRejectable() )
        return false;

    std::unique_ptr<ScChangeActionMap> pMap;
    if ( pAct->HasDependent() )
    {
        pMap.reset(new ScChangeActionMap);
        GetDependents( pAct, *pMap, false, true );
    }
    bool bRejected = Reject( pAct, pMap.get(), false );
    return bRejected;
}

bool ScChangeTrack::Reject(
    ScChangeAction* pAct, ScChangeActionMap* pMap, bool bRecursion )
{
    if ( !pAct->IsInternalRejectable() )
        return false;

    bool bOk = true;
    bool bRejected = false;
    if ( pAct->IsInsertType() )
    {
        if ( pAct->HasDependent() && !bRecursion )
        {
            OSL_ENSURE( pMap, "ScChangeTrack::Reject: Insert without map" );
            ScChangeActionMap::reverse_iterator itChangeAction;
            for (itChangeAction = pMap->rbegin();
                 itChangeAction != pMap->rend() && bOk; ++itChangeAction)
            {
                // Do not restore Contents which would end up being deleted anyways
                if ( itChangeAction->second->GetType() == SC_CAT_CONTENT )
                    itChangeAction->second->SetRejected();
                else if ( itChangeAction->second->IsDeleteType() )
                    itChangeAction->second->Accept(); // Deleted to Nirvana
                else
                    bOk = Reject( itChangeAction->second, nullptr, true ); // Recursion!
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) )
        {
            // pRefDoc NULL := Do not save deleted Cells
            AppendDeleteRange( pAct->GetBigRange().MakeRange(), nullptr, (short) 0,
                pAct->GetActionNumber() );
        }
    }
    else if ( pAct->IsDeleteType() )
    {
        OSL_ENSURE( !pMap, "ScChangeTrack::Reject: Delete with map" );
        ScBigRange aDelRange;
        sal_uLong nRejectAction = pAct->GetActionNumber();
        bool bTabDel, bTabDelOk;
        if ( pAct->GetType() == SC_CAT_DELETE_TABS )
        {
            bTabDel = true;
            aDelRange = pAct->GetBigRange();
            bTabDelOk = pAct->Reject( pDoc );
            bOk = bTabDelOk;
            if ( bOk )
            {
                pAct = pAct->GetPrev();
                bOk = ( pAct && pAct->GetType() == SC_CAT_DELETE_COLS );
            }
        }
        else
            bTabDel = bTabDelOk = false;
        ScChangeActionDel* pDel = static_cast<ScChangeActionDel*>(pAct);
        if ( bOk )
        {
            aDelRange = pDel->GetOverAllRange();
            bOk = aDelRange.IsValid( pDoc );
        }
        bool bOneOk = false;
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
                default:
                {
                    // added to avoid warnings
                }
            }
            ScChangeAction* p = pAct;
            bool bLoop = true;
            do
            {
                pDel = static_cast<ScChangeActionDel*>(p);
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
                            default:
                            {
                                // added to avoid warnings
                            }
                        }
                    }
                    else
                        bOneOk = true;
                }
                if ( pDel->IsBaseDelete() )
                    bLoop = false;
                else
                    p = p->GetPrev();
            } while ( bOk && bLoop && p && p->GetType() == eActType &&
                !static_cast<ScChangeActionDel*>(p)->IsTopDelete() );
        }
        bRejected = bOk;
        if ( bOneOk || (bTabDel && bTabDelOk) )
        {
            // Delete Reject made UpdateReference Undo
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
            OSL_ENSURE( pMap, "ScChangeTrack::Reject: Move without Map" );
            ScChangeActionMap::reverse_iterator itChangeAction;

            for( itChangeAction = pMap->rbegin(); itChangeAction != pMap->rend() && bOk; ++itChangeAction )
            {
                bOk = Reject( itChangeAction->second, nullptr, true ); // Recursion!
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) )
        {
            ScChangeActionMove* pReject = new ScChangeActionMove(
                pAct->GetBigRange().MakeRange(),
                static_cast<ScChangeActionMove*>(pAct)->GetFromRange().MakeRange(), this );
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
            pReject = nullptr;
        else
        {
            aRange = pAct->GetBigRange().aStart.MakeAddress();
            pReject = new ScChangeActionContent( aRange );
            ScCellValue aCell;
            aCell.assign(*pDoc, aRange.aStart);
            pReject->SetOldValue(aCell, pDoc, pDoc);
        }
        if ( (bRejected = pAct->Reject( pDoc )) && !bRecursion )
        {
            ScCellValue aCell;
            aCell.assign(*pDoc, aRange.aStart);
            pReject->SetNewValue(aCell, pDoc);
            pReject->SetRejectAction( pAct->GetActionNumber() );
            pReject->SetState( SC_CAS_ACCEPTED );
            Append( pReject );
        }
        else if ( pReject )
            delete pReject;
    }
    else
    {
        OSL_FAIL( "ScChangeTrack::Reject: say what?" );
    }

    return bRejected;
}

bool ScChangeTrack::IsLastAction( sal_uLong nNum ) const
{
    return nNum == nActionMax && pLast && pLast->GetActionNumber() == nNum;
}

sal_uLong ScChangeTrack::AddLoadedGenerated(
    const ScCellValue& rNewCell, const ScBigRange& aBigRange, const OUString& sNewValue )
{
    ScChangeActionContent* pAct = new ScChangeActionContent( --nGeneratedMin, rNewCell, aBigRange, pDoc, sNewValue );
    if ( pFirstGeneratedDelContent )
        pFirstGeneratedDelContent->pPrev = pAct;
    pAct->pNext = pFirstGeneratedDelContent;
    pFirstGeneratedDelContent = pAct;
    aGeneratedMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
    return pAct->GetActionNumber();
}

void ScChangeTrack::AppendCloned( ScChangeAction* pAppend )
{
    aMap.insert( ::std::make_pair( pAppend->GetActionNumber(), pAppend ) );
    if ( !pLast )
        pFirst = pLast = pAppend;
    else
    {
        pLast->pNext = pAppend;
        pAppend->pPrev = pLast;
        pLast = pAppend;
    }
}

ScChangeTrack* ScChangeTrack::Clone( ScDocument* pDocument ) const
{
    if ( !pDocument )
    {
        return nullptr;
    }

    ScChangeTrack* pClonedTrack = new ScChangeTrack( pDocument );
    pClonedTrack->SetTimeNanoSeconds( IsTimeNanoSeconds() );

    // clone generated actions
    ::std::stack< const ScChangeAction* > aGeneratedStack;
    const ScChangeAction* pGenerated = GetFirstGenerated();
    while ( pGenerated )
    {
        aGeneratedStack.push( pGenerated );
        pGenerated = pGenerated->GetNext();
    }
    while ( !aGeneratedStack.empty() )
    {
        pGenerated = aGeneratedStack.top();
        aGeneratedStack.pop();
        const ScChangeActionContent& rContent = dynamic_cast<const ScChangeActionContent&>(*pGenerated);
        const ScCellValue& rNewCell = rContent.GetNewCell();
        if (!rNewCell.isEmpty())
        {
            ScCellValue aClonedNewCell;
            aClonedNewCell.assign(rNewCell, *pDocument);
            OUString aNewValue;
            rContent.GetNewString( aNewValue, pDocument );
            pClonedTrack->nGeneratedMin = pGenerated->GetActionNumber() + 1;
            pClonedTrack->AddLoadedGenerated(aClonedNewCell, pGenerated->GetBigRange(), aNewValue);
        }
    }

    // clone actions
    const ScChangeAction* pAction = GetFirst();
    while ( pAction )
    {
        ScChangeAction* pClonedAction = nullptr;

        switch ( pAction->GetType() )
        {
            case SC_CAT_INSERT_COLS:
            case SC_CAT_INSERT_ROWS:
            case SC_CAT_INSERT_TABS:
            {
                bool bEndOfList = static_cast<const ScChangeActionIns*>(pAction)->IsEndOfList();
                pClonedAction = new ScChangeActionIns(
                    pAction->GetActionNumber(),
                    pAction->GetState(),
                    pAction->GetRejectAction(),
                    pAction->GetBigRange(),
                    pAction->GetUser(),
                    pAction->GetDateTimeUTC(),
                    pAction->GetComment(),
                    pAction->GetType(),
                    bEndOfList );
            }
            break;
            case SC_CAT_DELETE_COLS:
            case SC_CAT_DELETE_ROWS:
            case SC_CAT_DELETE_TABS:
                {
                    const ScChangeActionDel& rDelete = dynamic_cast<const ScChangeActionDel&>(*pAction);

                    SCsCOLROW nD = 0;
                    ScChangeActionType eType = pAction->GetType();
                    if ( eType == SC_CAT_DELETE_COLS )
                    {
                        nD = static_cast< SCsCOLROW >( rDelete.GetDx() );
                    }
                    else if ( eType == SC_CAT_DELETE_ROWS )
                    {
                        nD = static_cast< SCsCOLROW >( rDelete.GetDy() );
                    }

                    pClonedAction = new ScChangeActionDel(
                        pAction->GetActionNumber(),
                        pAction->GetState(),
                        pAction->GetRejectAction(),
                        pAction->GetBigRange(),
                        pAction->GetUser(),
                        pAction->GetDateTimeUTC(),
                        pAction->GetComment(),
                        eType,
                        nD,
                        pClonedTrack );
                }
                break;
            case SC_CAT_MOVE:
                {
                    const ScChangeActionMove* pMove = dynamic_cast< const ScChangeActionMove* >( pAction );
                    OSL_ENSURE( pMove, "ScChangeTrack::Clone: pMove is null!" );

                    pClonedAction = new ScChangeActionMove(
                        pAction->GetActionNumber(),
                        pAction->GetState(),
                        pAction->GetRejectAction(),
                        pAction->GetBigRange(),
                        pAction->GetUser(),
                        pAction->GetDateTimeUTC(),
                        pAction->GetComment(),
                        pMove->GetFromRange(),
                        pClonedTrack );
                }
                break;
            case SC_CAT_CONTENT:
                {
                    const ScChangeActionContent& rContent = dynamic_cast<const ScChangeActionContent&>(*pAction);
                    const ScCellValue& rOldCell = rContent.GetOldCell();
                    ScCellValue aClonedOldCell;
                    aClonedOldCell.assign(rOldCell, *pDocument);
                    OUString aOldValue;
                    rContent.GetOldString( aOldValue, pDocument );

                    ScChangeActionContent* pClonedContent = new ScChangeActionContent(
                        pAction->GetActionNumber(),
                        pAction->GetState(),
                        pAction->GetRejectAction(),
                        pAction->GetBigRange(),
                        pAction->GetUser(),
                        pAction->GetDateTimeUTC(),
                        pAction->GetComment(),
                        aClonedOldCell,
                        pDocument,
                        aOldValue );

                    const ScCellValue& rNewCell = rContent.GetNewCell();
                    if (!rNewCell.isEmpty())
                    {
                        ScCellValue aClonedNewCell;
                        aClonedNewCell.assign(rNewCell, *pDocument);
                        pClonedContent->SetNewValue(aClonedNewCell, pDocument);
                    }

                    pClonedAction = pClonedContent;
                }
                break;
            case SC_CAT_REJECT:
                {
                    pClonedAction = new ScChangeActionReject(
                        pAction->GetActionNumber(),
                        pAction->GetState(),
                        pAction->GetRejectAction(),
                        pAction->GetBigRange(),
                        pAction->GetUser(),
                        pAction->GetDateTimeUTC(),
                        pAction->GetComment() );
                }
                break;
            default:
                {
                }
                break;
        }

        if ( pClonedAction )
        {
            pClonedTrack->AppendCloned( pClonedAction );
        }

        pAction = pAction->GetNext();
    }

    if ( pClonedTrack->GetLast() )
    {
        pClonedTrack->SetActionMax( pClonedTrack->GetLast()->GetActionNumber() );
    }

    // set dependencies for Deleted/DeletedIn
    pAction = GetFirst();
    while ( pAction )
    {
        if ( pAction->HasDeleted() )
        {
            ::std::stack< sal_uLong > aStack;
            const ScChangeActionLinkEntry* pL = pAction->GetFirstDeletedEntry();
            while ( pL )
            {
                const ScChangeAction* pDeleted = pL->GetAction();
                if ( pDeleted )
                {
                    aStack.push( pDeleted->GetActionNumber() );
                }
                pL = pL->GetNext();
            }
            ScChangeAction* pClonedAction = pClonedTrack->GetAction( pAction->GetActionNumber() );
            if ( pClonedAction )
            {
                while ( !aStack.empty() )
                {
                    ScChangeAction* pClonedDeleted = pClonedTrack->GetActionOrGenerated( aStack.top() );
                    aStack.pop();
                    if ( pClonedDeleted )
                    {
                        pClonedDeleted->SetDeletedIn( pClonedAction );
                    }
                }
            }
        }
        pAction = pAction->GetNext();
    }

    // set dependencies for Dependent/Any
    pAction = GetLast();
    while ( pAction )
    {
        if ( pAction->HasDependent() )
        {
            ::std::stack< sal_uLong > aStack;
            const ScChangeActionLinkEntry* pL = pAction->GetFirstDependentEntry();
            while ( pL )
            {
                const ScChangeAction* pDependent = pL->GetAction();
                if ( pDependent )
                {
                    aStack.push( pDependent->GetActionNumber() );
                }
                pL = pL->GetNext();
            }
            ScChangeAction* pClonedAction = pClonedTrack->GetAction( pAction->GetActionNumber() );
            if ( pClonedAction )
            {
                while ( !aStack.empty() )
                {
                    ScChangeAction* pClonedDependent = pClonedTrack->GetActionOrGenerated( aStack.top() );
                    aStack.pop();
                    if ( pClonedDependent )
                    {
                        ScChangeActionLinkEntry* pLink = pClonedAction->AddDependent( pClonedDependent );
                        pClonedDependent->AddLink( pClonedAction, pLink );
                    }
                }
            }
        }
        pAction = pAction->GetPrev();
    }

    // masterlinks
    ScChangeAction* pClonedAction = pClonedTrack->GetFirst();
    while ( pClonedAction )
    {
        pClonedTrack->MasterLinks( pClonedAction );
        pClonedAction = pClonedAction->GetNext();
    }

    if ( IsProtected() )
    {
        pClonedTrack->SetProtection( GetProtection() );
    }

    if ( pClonedTrack->GetLast() )
    {
        pClonedTrack->SetLastSavedActionNumber( pClonedTrack->GetLast()->GetActionNumber() );
    }

    pDocument->SetChangeTrack( pClonedTrack );

    return pClonedTrack;
}

void ScChangeTrack::MergeActionState( ScChangeAction* pAct, const ScChangeAction* pOtherAct )
{
    if ( pAct->IsVirgin() )
    {
        if ( pOtherAct->IsAccepted() )
        {
            pAct->Accept();
            if ( pOtherAct->IsRejecting() )
            {
                pAct->SetRejectAction( pOtherAct->GetRejectAction() );
            }
        }
        else if ( pOtherAct->IsRejected() )
        {
            pAct->SetRejected();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
