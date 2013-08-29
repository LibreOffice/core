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
#include "scmod.hxx"        // SC_MOD
#include "inputopt.hxx"     // GetExpandRefs
#include "patattr.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "globstr.hrc"
#include "editutil.hxx"
#include "tokenarray.hxx"
#include "refupdatecontext.hxx"

#include <tools/shl.hxx>        // SHL_CALC
#include <tools/rtti.hxx>
#include <svl/zforlist.hxx>
#include <svl/itemset.hxx>
#include <svl/isethint.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/sfxsids.hrc>

IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry )
IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry )

ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScRange& rRange )
        :
        aBigRange( rRange ),
        aDateTime( DateTime::SYSTEM ),
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

ScChangeAction::ScChangeAction(
    ScChangeActionType eTypeP, const ScBigRange& rRange,
    const sal_uLong nTempAction, const sal_uLong nTempRejectAction,
    const ScChangeActionState eTempState, const DateTime& aTempDateTime,
    const OUString& aTempUser,  const OUString& aTempComment) :
        aBigRange( rRange ),
        aDateTime( aTempDateTime ),
        aUser( aTempUser ),
        aComment( aTempComment ),
        pNext( NULL ),
        pPrev( NULL ),
        pLinkAny( NULL ),
        pLinkDeletedIn( NULL ),
        pLinkDeleted( NULL ),
        pLinkDependent( NULL ),
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
    //! sequence order of execution is significant
    if ( IsRejected() || GetType() == SC_CAT_DELETE_TABS || IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
        return ((ScChangeActionContent*)this)->IsTopContent();
    return true;
}


bool ScChangeAction::IsTouchable() const
{
    //! sequence order of execution is significant
    if ( IsRejected() || GetType() == SC_CAT_REJECT || IsDeletedIn() )
        return false;
    // content may reject and be touchable if on top
    if ( GetType() == SC_CAT_CONTENT )
        return ((ScChangeActionContent*)this)->IsTopContent();
    if ( IsRejecting() )
        return false;
    return true;
}


bool ScChangeAction::IsClickable() const
{
    //! sequence order of execution is significant
    if ( !IsVirgin() )
        return false;
    if ( IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContentCellType eCCT =
            ScChangeActionContent::GetContentCellType(
            ((ScChangeActionContent*)this)->GetNewCell() );
        if ( eCCT == SC_CACCT_MATREF )
            return false;
        if ( eCCT == SC_CACCT_MATORG )
        {   // no Accept-Select if one of the references is in a deleted col/row
            const ScChangeActionLinkEntry* pL =
                ((ScChangeActionContent*)this)->GetFirstDependentEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p && p->IsDeletedIn() )
                    return false;
                pL = pL->GetNext();
            }
        }
        return true;    // for Select() a content doesn't have to be touchable
    }
    return IsTouchable();   // Accept()/Reject() only on touchables
}


bool ScChangeAction::IsRejectable() const
{
    //! sequence order of execution is significant
    if ( !IsClickable() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( ((ScChangeActionContent*)this)->IsOldMatrixReference() )
            return false;
        ScChangeActionContent* pNextContent =
            ((ScChangeActionContent*)this)->GetNextContent();
        if ( pNextContent == NULL )
            return true;        // *this is TopContent
        return pNextContent->IsRejected();      // *this is next rejectable
    }
    return IsTouchable();
}


bool ScChangeAction::IsInternalRejectable() const
{
    //! sequence order of execution is significant
    if ( !IsVirgin() )
        return false;
    if ( IsDeletedIn() )
        return false;
    if ( GetType() == SC_CAT_CONTENT )
    {
        ScChangeActionContent* pNextContent =
            ((ScChangeActionContent*)this)->GetNextContent();
        if ( pNextContent == NULL )
            return true;        // *this is TopContent
        return pNextContent->IsRejected();      // *this is next rejectable
    }
    return IsTouchable();
}


bool ScChangeAction::IsDialogRoot() const
{
    return IsInternalRejectable();      // only rejectables in root
}


bool ScChangeAction::IsDialogParent() const
{
    //! sequence order of execution is significant
    if ( GetType() == SC_CAT_CONTENT )
    {
        if ( !IsDialogRoot() )
            return false;
        if ( ((ScChangeActionContent*)this)->IsMatrixOrigin() && HasDependent() )
            return true;
        ScChangeActionContent* pPrevContent =
            ((ScChangeActionContent*)this)->GetPrevContent();
        return pPrevContent && pPrevContent->IsVirgin();
    }
    if ( HasDependent() )
        return IsDeleteType() ? true : !IsDeletedIn();
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


void ScChangeAction::RemoveAllAnyLinks()
{
    while ( pLinkAny )
        delete pLinkAny;        // rueckt sich selbst hoch
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
    return GetDeletedIn() != NULL;
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
    //! nicht vom evtl. TopContent sondern wirklich dieser
    while ( pLinkDeletedIn )
        delete pLinkDeletedIn;      // rueckt sich selbst hoch
}

bool ScChangeAction::IsDeletedInDelType( ScChangeActionType eDelType ) const
{
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
        ScChangeAction* p;
        while ( pL )
        {
            if ( (p = pL->GetAction()) != NULL &&
                    (p->GetType() == eDelType || p->GetType() == eInsType) )
                return true;
            pL = pL->GetNext();
        }
    }
    return false;
}

bool ScChangeAction::HasDependent() const
{
    return pLinkDependent != NULL;
}

bool ScChangeAction::HasDeleted() const
{
    return pLinkDeleted != NULL;
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


void ScChangeAction::RemoveAllDeleted()
{
    while ( pLinkDeleted )
        delete pLinkDeleted;        // rueckt sich selbst hoch
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
            ScGlobal::GetRscString(STR_CHANGED_MOVE_REJECTION_WARNING));
        aBuf.append(sal_Unicode(' '));
        rStr = aBuf.makeStringAndClear();
        return;
    }

    if (IsInsertType())
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING));
        aBuf.append(sal_Unicode(' '));
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
        aBuf.append(sal_Unicode(' '));
        rStr = aBuf.makeStringAndClear();
        return;
    }

    if (pReject->IsDeleteType())
    {
        aBuf.append(
            ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING));
        aBuf.append(sal_Unicode(' '));
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
                aBuf.append(sal_Unicode(' '));
                rStr = aBuf.makeStringAndClear();
                return;
            }

            if (pReject->IsDeleteType())
            {
                aBuf.append(
                    ScGlobal::GetRscString(STR_CHANGED_DELETE_REJECTION_WARNING));
                aBuf.append(sal_Unicode(' '));
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
                    aBuf.append(sal_Unicode('.'));
                }
                aBuf.append(ScColToAlpha(aTmpRange.aStart.Col()));
                aBuf.append(sal_Unicode(':'));
                aBuf.append(ScColToAlpha(aTmpRange.aEnd.Col()));
            break;
            case SC_CAT_INSERT_ROWS :
            case SC_CAT_DELETE_ROWS :
                if ( bFlag3D )
                {
                    OUString aTmp;
                    pDoc->GetName( aTmpRange.aStart.Tab(), aTmp );
                    aBuf.append(aTmp);
                    aBuf.append(sal_Unicode('.'));
                }
                aBuf.append(static_cast<sal_Int32>(aTmpRange.aStart.Row()+1));
                aBuf.append(sal_Unicode(':'));
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
            aBuf.insert(0, sal_Unicode('('));
            aBuf.append(sal_Unicode(')'));
        }
    }
    return aBuf.makeStringAndClear();
}

const OUString& ScChangeAction::GetUser() const
{
    return aUser;
}

void ScChangeAction::SetUser( const OUString& r )
{
    aUser = r;
}

const OUString& ScChangeAction::GetComment() const
{
    return aComment;
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
    pTrack->UpdateReference( this, true );      // LinkDeleted freigeben
    OSL_ENSURE( !pLinkDeleted, "ScChangeAction::RejectRestoreContents: pLinkDeleted != NULL" );
    // Liste der Contents abarbeiten und loeschen
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
    DeleteCellEntries();        // weg mit den generierten
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
    {
        OSL_FAIL( "ScChangeActionIns: Block not supported!" );
    }
}


ScChangeActionIns::ScChangeActionIns(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber, const ScBigRange& aBigRangeP,
    const OUString& aUserP, const DateTime& aDateTimeP,
    const OUString& sComment, const ScChangeActionType eTypeP) :
    ScChangeAction(eTypeP, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment)
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
    sal_Int32 nPos = aRsc.indexOfAsciiL("#1", 2);
    if (nPos >= 0)
    {
        // Construct a range string to replace '#1' first.
        OUStringBuffer aBuf(ScGlobal::GetRscString(nWhatId));
        aBuf.append(sal_Unicode(' '));
        aBuf.append(GetRefString(GetBigRange(), pDoc));
        OUString aRangeStr = aBuf.makeStringAndClear();

        aRsc = aRsc.replaceAt(nPos, 2, aRangeStr); // replace '#1' with the range string.

        aBuf.append(rStr).append(aRsc);
        rStr = aBuf.makeStringAndClear();
    }
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


// --- ScChangeActionDel ---------------------------------------------------

ScChangeActionDel::ScChangeActionDel( const ScRange& rRange,
            SCsCOL nDxP, SCsROW nDyP, ScChangeTrack* pTrackP )
        :
        ScChangeAction( SC_CAT_NONE, rRange ),
        pTrack( pTrackP ),
        pFirstCell( NULL ),
        pCutOff( NULL ),
        nCutOff( 0 ),
        pLinkMove( NULL ),
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
    pFirstCell( NULL ),
    pCutOff( NULL ),
    nCutOff( 0 ),
    pLinkMove( NULL ),
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
    return ((ScChangeActionDel*)p)->IsBaseDelete();
}


bool ScChangeActionDel::IsMultiDelete() const
{
    if ( GetDx() || GetDy() )
        return true;
    const ScChangeAction* p = GetNext();
    if ( !p || p->GetType() != GetType() )
        return false;
    const ScChangeActionDel* pDel = (const ScChangeActionDel*) p;
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
            !((const ScChangeActionDel*)p)->IsTopDelete() )
        p = p->GetNext();
    return p && p->GetType() == SC_CAT_DELETE_TABS;
}

SCsCOL ScChangeActionDel::GetDx() const { return nDx; }
SCsROW ScChangeActionDel::GetDy() const { return nDy; }

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
    sal_Int32 nPos = aRsc.indexOfAsciiL("#1", 2);
    if (nPos >= 0)
    {
        // Build a string to replace with.
        OUStringBuffer aBuf;
        aBuf.append(ScGlobal::GetRscString(nWhatId));
        aBuf.append(sal_Unicode(' '));
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
    {   // den kompletten Bereich in einem Rutsch restaurieren
        bool bOk = true;
        ScBigRange aTmpRange( GetOverAllRange() );
        if ( !aTmpRange.IsValid( pDoc ) )
        {
            if ( GetType() == SC_CAT_DELETE_TABS )
            {   // wird Tab angehaengt?
                if ( aTmpRange.aStart.Tab() > pDoc->GetMaxTableNumber() )
                    bOk = false;
            }
            else
                bOk = false;
        }
        if ( bOk )
        {
            ScRange aRange( aTmpRange.MakeRange() );
            // InDelete... fuer Formel UpdateReference in Document
            pTrack->SetInDeleteRange( aRange );
            pTrack->SetInDeleteTop( true );
            pTrack->SetInDeleteUndo( true );
            pTrack->SetInDelete( true );
            switch ( GetType() )
            {
                case SC_CAT_DELETE_COLS :
                    if ( !(aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL) )
                    {   // nur wenn nicht TabDelete
                        if ( ( bOk = pDoc->CanInsertCol( aRange ) ) != false )
                            bOk = pDoc->InsertCol( aRange );
                    }
                break;
                case SC_CAT_DELETE_ROWS :
                    if ( ( bOk = pDoc->CanInsertRow( aRange ) ) != false )
                        bOk = pDoc->InsertRow( aRange );
                break;
                case SC_CAT_DELETE_TABS :
                {
//2do: Tabellennamen merken?
                    OUString aName;
                    pDoc->CreateValidTabName( aName );
                    if ( ( bOk = pDoc->ValidNewTabName( aName ) ) != false )
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
        // InDeleteTop fuer UpdateReference-Undo behalten
    }

    // setzt rejected und ruft UpdateReference-Undo und DeleteCellEntries
    RejectRestoreContents( pTrack, GetDx(), GetDy() );

    pTrack->SetInDeleteTop( false );
    RemoveAllLinks();
    return true;
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
            default:
            {
                // added to avoid warnings
            }
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
            default:
            {
                // added to avoid warnings
            }
        }
        SetCutOffInsert( NULL, 0 );
    }
}


// --- ScChangeActionMove --------------------------------------------------

ScChangeActionMove::ScChangeActionMove(
    const sal_uLong nActionNumber, const ScChangeActionState eStateP,
    const sal_uLong nRejectingNumber, const ScBigRange& aToBigRange,
    const OUString& aUserP, const DateTime& aDateTimeP,
    const OUString &sComment, const ScBigRange& aFromBigRange,
    ScChangeTrack* pTrackP) : // which of nDx and nDy is set depends on the type
    ScChangeAction(SC_CAT_MOVE, aToBigRange, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
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
    sal_Int32 nPos = aRsc.indexOfAsciiL("#1", 2);
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    aTmpStr = ScChangeAction::GetRefString(GetBigRange(), pDoc, bFlag3D);
    nPos = aRsc.indexOfAsciiL("#2", 2, nPos);
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

    OUStringBuffer aBuf;
    aBuf.append(ScChangeAction::GetRefString(GetFromRange(), pDoc, bFlag3D));
    aBuf.append(sal_Unicode(','));
    aBuf.append(sal_Unicode(' '));
    aBuf.append(ScChangeAction::GetRefString(GetBigRange(), pDoc, bFlag3D));
    rStr = aBuf.makeStringAndClear(); // overwrite existing string value.
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

    pTrack->LookUpContents( aToRange, pDoc, 0, 0, 0 );  // zu movende Contents

    pDoc->DeleteAreaTab( aToRange, IDF_ALL );
    pDoc->DeleteAreaTab( aFrmRange, IDF_ALL );
    // Formeln im Dokument anpassen
    sc::RefUpdateContext aCxt(*pDoc);
    aCxt.meMode = URM_MOVE;
    aCxt.maRange = aFrmRange;
    aCxt.mnColDelta = aFrmRange.aStart.Col() - aToRange.aStart.Col();
    aCxt.mnRowDelta = aFrmRange.aStart.Row() - aToRange.aStart.Row();
    aCxt.mnTabDelta = aFrmRange.aStart.Tab() - aToRange.aStart.Tab();
    pDoc->UpdateReference(aCxt, NULL);

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
    return true;
}


// --- ScChangeActionContent -----------------------------------------------

IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent )

ScChangeActionContent::ScChangeActionContent( const ScRange& rRange ) :
    ScChangeAction(SC_CAT_CONTENT, rRange),
    pNextContent(NULL),
    pPrevContent(NULL),
    pNextInSlot(NULL),
    ppPrevInSlot(NULL)
{}

ScChangeActionContent::ScChangeActionContent( const sal_uLong nActionNumber,
            const ScChangeActionState eStateP, const sal_uLong nRejectingNumber,
            const ScBigRange& aBigRangeP, const OUString& aUserP,
            const DateTime& aDateTimeP, const OUString& sComment,
            const ScCellValue& rOldCell, ScDocument* pDoc, const OUString& sOldValue ) :
    ScChangeAction(SC_CAT_CONTENT, aBigRangeP, nActionNumber, nRejectingNumber, eStateP, aDateTimeP, aUserP, sComment),
    maOldCell(rOldCell),
    maOldValue(sOldValue),
    pNextContent(NULL),
    pPrevContent(NULL),
    pNextInSlot(NULL),
    ppPrevInSlot(NULL)
{
    if (!maOldCell.isEmpty())
        SetCell(maOldValue, maOldCell, 0, pDoc);

    if (!sOldValue.isEmpty())     // #i40704# don't overwrite SetCell result with empty string
        maOldValue = sOldValue; // set again, because SetCell removes it
}

ScChangeActionContent::ScChangeActionContent( const sal_uLong nActionNumber,
            const ScCellValue& rNewCell, const ScBigRange& aBigRangeP,
            ScDocument* pDoc, const OUString& sNewValue ) :
    ScChangeAction(SC_CAT_CONTENT, aBigRangeP, nActionNumber),
    maNewCell(rNewCell),
    maNewValue(sNewValue),
    pNextContent(NULL),
    pPrevContent(NULL),
    pNextInSlot(NULL),
    ppPrevInSlot(NULL)
{
    if (!maNewCell.isEmpty())
        SetCell(maNewValue, maNewCell, 0, pDoc);

    if (!sNewValue.isEmpty())     // #i40704# don't overwrite SetCell result with empty string
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
    if ( rStr.getLength() > 1 && rStr.getStr()[0] == '=' )
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

const ScCellValue& ScChangeActionContent::GetOldCell() const
{
    return maOldCell;
}

const ScCellValue& ScChangeActionContent::GetNewCell() const
{
    return maNewCell;
}

void ScChangeActionContent::GetDescription(
    OUString& rStr, ScDocument* pDoc, bool bSplitRange, bool bWarning ) const
{
    ScChangeAction::GetDescription( rStr, pDoc, bSplitRange, bWarning );

    OUString aRsc = ScGlobal::GetRscString(STR_CHANGED_CELL);

    OUString aTmpStr;
    GetRefString(aTmpStr, pDoc);

    sal_Int32 nPos = 0;
    nPos = aRsc.indexOfAsciiL("#1", 2, nPos);
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    GetOldString( aTmpStr, pDoc );
    if (aTmpStr.isEmpty())
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );

    nPos = aRsc.indexOfAsciiL("#2", 2, nPos);
    if (nPos >= 0)
    {
        aRsc = aRsc.replaceAt(nPos, 2, aTmpStr);
        nPos += aTmpStr.getLength();
    }

    GetNewString( aTmpStr, pDoc );
    if (aTmpStr.isEmpty())
        aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );

    nPos = aRsc.indexOfAsciiL("#3", 2, nPos);
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
            aBuf.append(sal_Unicode('('));
            aBuf.append(rStr);
            aBuf.append(sal_Unicode(')'));
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
    while ( ( pContent = pContent->pPrevContent ) != NULL )
    {
        if ( pContent->IsVirgin() )
            pContent->SetState( SC_CAS_ACCEPTED );
    }
    ScChangeActionContent* pEnd = pContent = this;
    // reject subsequent contents
    while ( ( pContent = pContent->pNextContent ) != NULL )
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
            rStr = *rCell.mpString;
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
                case MM_FAKE :
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
                case MM_FAKE :
                    return SC_CACCT_MATORG;
                case MM_REFERENCE :
                    return SC_CACCT_MATREF;
                default:
                    ;
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
    rStr = OUString();

    if (GetContentCellType(rOrgCell))
    {
        rCell.assign(rOrgCell, *pToDoc);
        switch (rOrgCell.meType)
        {
            case CELLTYPE_VALUE :
            {   // z.B. Datum auch als solches merken
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
    rStr = OUString();
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
            rStr = *rCell.mpString;
        break;
        case CELLTYPE_EDIT :
            if (rCell.mpEditText)
                rStr = ScEditUtil::GetString(*rCell.mpEditText, pDoc);
        break;
        case CELLTYPE_VALUE :   // ist immer in rValue
            rStr = rValue;
        break;
        case CELLTYPE_FORMULA :
            GetFormulaString(rStr, rCell.mpFormula);
        break;
        default:
        {
            // added to avoid warnings
        }
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
        ScFormulaCell* pNew = new ScFormulaCell( *pCell, *pCell->GetDocument(), aPos );
        pNew->GetFormula( rStr );
        delete pNew;
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


static void lcl_InvalidateReference( ScToken& rTok, const ScBigAddress& rPos )
{
    ScSingleRefData& rRef1 = rTok.GetSingleRef();
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
        ScSingleRefData& rRef2 = rTok.GetDoubleRef().Ref2;
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
        return ;        // Formeln nur kompletten Bereich updaten

    bool bOldFormula = maOldCell.meType == CELLTYPE_FORMULA;
    bool bNewFormula = maNewCell.meType == CELLTYPE_FORMULA;
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
            maOldCell.mpFormula->UpdateReference(aRefCxt, NULL);
        if ( bNewFormula )
            maNewCell.mpFormula->UpdateReference(aRefCxt, NULL);

        if ( !aBigRange.aStart.IsValid( pTrack->GetDocument() ) )
        {   //! HACK!
            //! UpdateReference kann nicht mit Positionen ausserhalb des
            //! Dokuments umgehen, deswegen alles auf #REF! setzen
//2do: make it possible! das bedeutet grossen Umbau von ScAddress etc.!
            const ScBigAddress& rPos = aBigRange.aStart;
            if ( bOldFormula )
            {
                ScToken* t;
                ScTokenArray* pArr = maOldCell.mpFormula->GetCode();
                pArr->Reset();
                while ( ( t = static_cast<ScToken*>(pArr->GetNextReference()) ) != NULL )
                    lcl_InvalidateReference( *t, rPos );
                pArr->Reset();
                while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
                    lcl_InvalidateReference( *t, rPos );
            }
            if ( bNewFormula )
            {
                ScToken* t;
                ScTokenArray* pArr = maNewCell.mpFormula->GetCode();
                pArr->Reset();
                while ( ( t = static_cast<ScToken*>(pArr->GetNextReference()) ) != NULL )
                    lcl_InvalidateReference( *t, rPos );
                pArr->Reset();
                while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
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

// --- ScChangeActionReject ------------------------------------------------

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


// --- ScChangeTrack -------------------------------------------------------

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
    aBuf.append(sal_Unicode(' '));
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
        pBlockModifyMsg = NULL;
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
    maUser = OUString();
    Init();
}

const std::set<OUString>& ScChangeTrack::GetUserCollection() const
{
    return maUserCollection;
}

void ScChangeTrack::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 )
{
    if ( !pDoc->IsInDtorClear() )
    {
        const SvtUserOptions& rUserOptions = SC_MOD()->GetUserOptions();
        size_t nOldCount = maUserCollection.size();

        OUStringBuffer aBuf;
        aBuf.append(rUserOptions.GetFirstName());
        aBuf.append(sal_Unicode(' '));
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
        return ;        // nicht die Collection zerschiessen

    maUser = rUser;
    maUserCollection.insert(maUser);
}

const OUString& ScChangeTrack::GetUser() const
{
    return maUser;
}

void ScChangeTrack::StartBlockModify( ScChangeTrackMsgType eMsgType,
        sal_uLong nStartAction )
{
    if ( aModifiedLink.IsSet() )
    {
        if ( pBlockModifyMsg )
            aMsgStackTmp.push( pBlockModifyMsg );   // Block im Block
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
                // Blocks in Blocks aufgeloest
                aMsgStackFinal.push( pBlockModifyMsg );
            }
            else
                delete pBlockModifyMsg;
            if (aMsgStackTmp.empty())
                pBlockModifyMsg = NULL;
            else
            {
                pBlockModifyMsg = aMsgStackTmp.top();   // evtl. Block im Block
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
                aModifiedLink.Call( this );
        }
    }
}


void ScChangeTrack::NotifyModified( ScChangeTrackMsgType eMsgType,
        sal_uLong nStartAction, sal_uLong nEndAction )
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


void ScChangeTrack::MasterLinks( ScChangeAction* pAppend )
{
    ScChangeActionType eType = pAppend->GetType();

    if ( eType == SC_CAT_CONTENT )
    {
        if ( !IsGenerated( pAppend->GetActionNumber() ) )
        {
            SCSIZE nSlot = ComputeContentSlot(
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
    // UpdateReference Inserts vor Dependencies.
    // Delete rejectendes Insert hatte UpdateReference mit Delete-Undo.
    // UpdateReference auch wenn pLast==NULL, weil pAppend ein Delete sein
    // kann, dass DelContents generiert haben kann
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
    // UpdateReference Inserts nicht nach Dependencies.
    // Move rejectendes Move hatte UpdateReference mit Move-Undo, Inhalt in
    // ToRange nicht deleten.
    if ( !pAppend->IsInsertType() &&
            !(pAppend->GetType() == SC_CAT_MOVE && pAppend->IsRejecting()) )
        UpdateReference( pAppend, false );
    MasterLinks( pAppend );

    if ( aModifiedLink.IsSet() )
    {
        NotifyModified( SC_CTM_APPEND, nAction, nAction );
        if ( pAppend->GetType() == SC_CAT_CONTENT )
        {
            ScChangeActionContent* pContent = (ScChangeActionContent*) pAppend;
            if ( ( pContent = pContent->GetPrevContent() ) != NULL )
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
            {   // ganze Zeilen und/oder Tabellen
                if ( nRow1 == 0 && nRow2 == MAXROW )
                {   // ganze Tabellen
//2do: geht nicht auch komplette Tabelle als ganzes?
                    ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
                    for ( SCCOL nCol = nCol1; nCol <= nCol2; nCol++ )
                    {   // spaltenweise ist weniger als zeilenweise
                        aRange.aStart.SetCol( nCol );
                        aRange.aEnd.SetCol( nCol );
                        if ( nCol == nCol2 )
                            SetInDeleteTop( true );
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
            {   // ganze Spalten
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
        ScChangeActionContent* pContent = SearchContentAt( aBigPos, NULL );
        if (pContent)
            continue;

        // nicht getrackte Contents
        aPos.Set( aIter.GetPos().Col() + nDx, aIter.GetPos().Row() + nDy,
            aIter.GetPos().Tab() + nDz );

        GenerateDelContent(aPos, aIter.getCellValue(), pRefDoc);
        //! der Content wird hier _nicht_ per AddContent hinzugefuegt,
        //! sondern in UpdateReference, um z.B. auch kreuzende Deletes
        //! korrekt zu erfassen
    }
}


void ScChangeTrack::AppendMove( const ScRange& rFromRange,
        const ScRange& rToRange, ScDocument* pRefDoc )
{
    ScChangeActionMove* pAct = new ScChangeActionMove( rFromRange, rToRange, this );
    LookUpContents( rToRange, pRefDoc, 0, 0, 0 );   // ueberschriebene Contents
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
    {   // nur wirkliche Aenderung tracken
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
    {   // nur wirkliche Aenderung tracken
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
        // Paste und Cut abstimmen, Paste kann groesserer Range sein
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
        const ScAddress& rPos, const ScCellValue& rCell, const ScDocument* pFromDoc )
{
    ScChangeActionContent* pContent = new ScChangeActionContent(
        ScRange( rPos ) );
    pContent->SetActionNumber( --nGeneratedMin );
    // nur NewValue
    ScChangeActionContent::SetValue( pContent->maNewValue, pContent->maNewCell,
        rPos, rCell, pFromDoc, pDoc );
    // pNextContent und pPrevContent werden nicht gesetzt
    if ( pFirstGeneratedDelContent )
    {   // vorne reinhaengen
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
    return NULL;
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
        const ScCellValue& rCell = static_cast<ScChangeActionContent*>(pAct)->GetNewCell();
        if ( ScChangeActionContent::GetContentCellType(rCell) == SC_CACCT_MATREF )
        {
            ScAddress aOrg;
            rCell.mpFormula->GetMatrixOrigin(aOrg);
            ScChangeActionContent* pContent = SearchContentAt( aOrg, pAct );
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
    bool bActNoInsert = !pAct->IsInsertType();
    bool bActColDel = ( eActType == SC_CAT_DELETE_COLS );
    bool bActRowDel = ( eActType == SC_CAT_DELETE_ROWS );
    bool bActTabDel = ( eActType == SC_CAT_DELETE_TABS );

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
            if ( ( pContent = pContent->GetPrevContent() ) != NULL )
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
    {   //! Content wird wiederverwertet
        ScChangeActionContent* pContent = (ScChangeActionContent*) pRemove;
        pContent->RemoveAllLinks();
        pContent->ClearTrack();
        pContent->pNext = pContent->pPrev = NULL;
        pContent->pNextContent = pContent->pPrevContent = NULL;
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
                        SetInDeleteTop( true );
                        SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                            GetOverAllRange().MakeRange() );
                    }
                }
                UpdateReference( pAct, true );
                SetInDeleteTop( false );
                Remove( pAct );
                if ( IsInPasteCut() )
                    aPasteCutMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
                else
                {
                    if ( j == nStartAction && pAct->GetType() == SC_CAT_MOVE )
                    {
                        ScChangeActionMove* pMove = (ScChangeActionMove*) pAct;
                        sal_uLong nStart = pMove->GetStartLastCut();
                        sal_uLong nEnd = pMove->GetEndLastCut();
                        if ( nStart && nStart <= nEnd )
                        {   // LastCut wiederherstellen
                            //! Links vor Cut-Append aufloesen
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
            }
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
        return true;                // da kommt noch eine passende Reject-Action

    if ( rAction.IsRejecting() && rAction.GetRejectAction() >= nFirstMerge )
        return true;                // da ist sie

    return false;                   // alles andere
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
        {   // rueckwaerts, Deletes in richtiger Reihenfolge
            // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
            if ( bShared || !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
            {
                if ( pAct->IsDeleteType() )
                {
                    if ( ((ScChangeActionDel*)pAct)->IsTopDelete() )
                    {
                        SetInDeleteTop( true );
                        SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                            GetOverAllRange().MakeRange() );
                    }
                }
                UpdateReference( pAct, true );
                SetInDeleteTop( false );
                pAct->DeleteCellEntries();      // sonst GPF bei Track Clear()
            }
            pAct = ( pAct == pFirstMerge ? NULL : pAct->GetPrev() );
        }
    }
    SetMergeState( SC_CTMS_OTHER );     //! nachfolgende per default MergeOther
}


void ScChangeTrack::MergeOwn( ScChangeAction* pAct, sal_uLong nFirstMerge, bool bShared )
{
    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    if ( bShared || !ScChangeTrack::MergeIgnore( *pAct, nFirstMerge ) )
    {
        SetMergeState( SC_CTMS_OWN );
        if ( pAct->IsDeleteType() )
        {
            if ( ((ScChangeActionDel*)pAct)->IsTopDelete() )
            {
                SetInDeleteTop( true );
                SetInDeleteRange( ((ScChangeActionDel*)pAct)->
                    GetOverAllRange().MakeRange() );
            }
        }
        UpdateReference( pAct, false );
        SetInDeleteTop( false );
        SetMergeState( SC_CTMS_OTHER );     //! nachfolgende per default MergeOther
    }
}


void ScChangeTrack::UpdateReference( ScChangeAction* pAct, bool bUndo )
{
    ScChangeActionType eActType = pAct->GetType();
    if ( eActType == SC_CAT_CONTENT || eActType == SC_CAT_REJECT )
        return ;

    //! Formelzellen haengen nicht im Dokument
    bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( false );
    bool bOldNoListening = pDoc->GetNoListening();
    pDoc->SetNoListening( true );
    //! Formelzellen ExpandRefs synchronisiert zu denen im Dokument
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
        // Referenzen von Formelzellen wiederherstellen,
        // vorheriges MergePrepare war bei einem Insert wie ein Delete
        if ( pAct->IsInsertType() )
            SetInDeleteUndo( true );
    }

    //! erst die generated, als waeren sie vorher getrackt worden
    if ( pFirstGeneratedDelContent )
        UpdateReference( (ScChangeAction**)&pFirstGeneratedDelContent, pAct,
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
        ( ppFirstAction == (ScChangeAction**)&pFirstGeneratedDelContent );
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
            ((ScChangeActionMove*)pAct)->GetDelta( nDx, nDy, nDz );
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
    {   //! fuer diesen Mechanismus gilt:
        //! es gibt nur ganze, einfache geloeschte Spalten/Zeilen
        ScChangeActionDel* pActDel = (ScChangeActionDel*) pAct;
        if ( !bUndo )
        {   // Delete
            ScChangeActionType eInsType = SC_CAT_NONE;      // for Insert-Undo-"Deletes"
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
                {   // Delete in mergendem Dokument, Action im zu mergenden
                    if ( p->IsInsertType() )
                    {
                        // Bei Insert Referenzen nur anpassen, wenn das Delete
                        // das Insert nicht schneidet.
                        if ( !aDelRange.Intersects( p->GetBigRange() ) )
                            p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
                        bUpdate = false;
                    }
                    else if ( p->GetType() == SC_CAT_CONTENT &&
                            p->IsDeletedInDelType( eInsType ) )
                    {   // Content in Insert-Undo-"Delete"
                        // Nicht anpassen, wenn dieses Delete in dem
                        // Insert-"Delete" sein wuerde (ist nur verschoben).
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
                    bUpdate = false;
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
                        pActDel->SetDeletedIn( p );     // "druntergerutscht"
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
                                    ((ScChangeActionContent*)p)->IsTopContent() )
                            {   // erst der TopContent wird wirklich entfernt
                                p->RemoveDeletedIn( pActDel );
                                // GeneratedDelContent _nicht_ aus Liste loeschen,
                                // wir brauchen ihn evtl. noch fuer Reject,
                                // geloescht wird in DeleteCellEntries
                            }
                        }
                        bUpdate = false;
                    }
                    else if ( eActType != SC_CAT_DELETE_TABS &&
                            p->IsDeletedInDelType( SC_CAT_DELETE_TABS ) )
                    {   // in geloeschten Tabellen nicht updaten,
                        // ausser wenn Tabelle verschoben wird
                        bUpdate = false;
                    }
                    if ( p->GetType() == eActType && pActDel->IsDeletedIn( p ) )
                    {
                        pActDel->RemoveDeletedIn( p );  // "druntergerutscht"
                        bUpdate = true;
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
        bool bLastCutMove = ( pActMove == pLastCutMove );
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
                        while ( (pTmp = pHere->GetPrevContent()) != NULL &&
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
                        } while ( ( pHere = pHere->GetNextContent() ) != NULL );
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
                    // #i87003# [Collaboration] Move range and insert content in FromRange is not merged correctly
                    else if ( ( GetMergeState() != SC_CTMS_PREPARE && GetMergeState() != SC_CTMS_OWN ) || p->GetActionNumber() <= pAct->GetActionNumber() )
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
                // in Insert-Undo "Delete" rueckgaengig
                const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
                while ( pLink )
                {
                    ScChangeAction* p = (ScChangeAction*) pLink->GetAction();
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
    //! bAllFlat==TRUE: intern aus Accept oder Reject gerufen,
    //! => Generated werden nicht aufgenommen

    bool bIsDelete = pAct->IsDeleteType();
    bool bIsMasterDelete = ( bListMasterDelete && pAct->IsMasterDelete() );

    const ScChangeAction* pCur = NULL;
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
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
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
                            if ( ((ScChangeActionContent*)p)->IsTopContent() )
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
            {   // Inhalte geloeschter Bereiche interessieren nur bei Delete
                ScChangeActionDel* pDel = (ScChangeActionDel*) pCur;
                if ( !bAllFlat && bIsMasterDelete && pCur == pAct )
                {
                    // zu diesem Delete gehoerende Deletes in gleiche Ebene,
                    // wenn dieses Delete das momentan oberste einer Reihe ist,
                    ScChangeActionType eType = pDel->GetType();
                    ScChangeAction* p = pDel;
                    while ( (p = p->GetPrev()) != NULL && p->GetType() == eType &&
                            !((ScChangeActionDel*)p)->IsTopDelete() )
                        rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) );
                    // delete this in the map too
                    rMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
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
                                sal_uLong n = p->GetActionNumber();
                                if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                                    if ( p->HasDeleted() ||
                                            p->GetType() == SC_CAT_CONTENT )
                                        cStack.push( p );
                            }
                            else
                            {
                                if ( p->IsDeleteType() )
                                {   // weiteres TopDelete in gleiche Ebene,
                                    // es ist nicht rejectable
                                    if ( ((ScChangeActionDel*)p)->IsTopDelete() )
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
            // geloeschte Contents im ToRange
            const ScChangeActionLinkEntry* pL = pCur->GetFirstDeletedEntry();
            while ( pL )
            {
                ScChangeAction* p = (ScChangeAction*) pL->GetAction();
                if ( p != pAct && rMap.insert( ::std::make_pair( p->GetActionNumber(), p ) ).second )
                {
                    // nur ein TopContent einer Kette ist in LinkDeleted
                    if ( bAllFlat && (p->HasDeleted() ||
                            p->GetType() == SC_CAT_CONTENT) )
                        cStack.push( p );
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
                        sal_uLong n = p->GetActionNumber();
                        if ( !IsGenerated( n ) && rMap.insert( ::std::make_pair( n, p ) ).second )
                            if ( p->HasDependent() || p->HasDeleted() )
                                cStack.push( p );
                    }
                    else
                    {
                        if ( p->GetType() == SC_CAT_CONTENT )
                        {
                            if ( ((ScChangeActionContent*)p)->IsTopContent() )
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
        {   // alle Aenderungen an gleicher Position
            ScChangeActionContent* pContent = (ScChangeActionContent*) pCur;
            // alle vorherigen
            while ( ( pContent = pContent->GetPrevContent() ) != NULL )
            {
                if ( !pContent->IsRejected() )
                    rMap.insert( ::std::make_pair( pContent->GetActionNumber(), pContent ) );
            }
            pContent = (ScChangeActionContent*) pCur;
            // alle nachfolgenden
            while ( ( pContent = pContent->GetNextContent() ) != NULL )
            {
                if ( !pContent->IsRejected() )
                    rMap.insert( ::std::make_pair( pContent->GetActionNumber(), pContent ) );
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
                        ((ScChangeActionReject*)pCur)->GetRejectAction() );
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

    ScChangeActionContent* pContent = (ScChangeActionContent*) pAct;
    if ( bOldest )
    {
        pContent = pContent->GetTopContent();
        ScChangeActionContent* pPrevContent;
        while ( (pPrevContent = pContent->GetPrevContent()) != NULL &&
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
                    OSL_FAIL( "ScChangeTrack::SelectContent: content dependent no content" );
                }
            }
            pL = pL->GetNext();
        }

        bOk &= pContent->Select( pDoc, this, bOldest, NULL );
        // now the matrix is inserted and new content values are ready

        ScChangeActionContent* pNew;
        while ( !aRejectActions.empty() )
        {
            pNew = aRejectActions.top();
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
        return pContent->Select( pDoc, this, bOldest, NULL );
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
    {   //! rueckwaerts, weil abhaengige hinten und RejectActions angehaengt
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

    ScChangeActionMap* pMap = NULL;
    if ( pAct->HasDependent() )
    {
        pMap = new ScChangeActionMap;
        GetDependents( pAct, *pMap, false, true );
    }
    bool bRejected = Reject( pAct, pMap, false );
    if ( pMap )
        delete pMap;
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
                // keine Contents restoren, die eh geloescht werden wuerden
                if ( itChangeAction->second->GetType() == SC_CAT_CONTENT )
                    itChangeAction->second->SetRejected();
                else if ( itChangeAction->second->IsDeleteType() )
                    itChangeAction->second->Accept();        // geloeschtes ins Nirvana
                else
                    bOk = Reject( itChangeAction->second, NULL, true );      //! rekursiv
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) != false )
        {
            // pRefDoc NULL := geloeschte Zellen nicht speichern
            AppendDeleteRange( pAct->GetBigRange().MakeRange(), NULL, (short) 0,
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
        ScChangeActionDel* pDel = (ScChangeActionDel*) pAct;
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
            OSL_ENSURE( pMap, "ScChangeTrack::Reject: Move without Map" );
            ScChangeActionMap::reverse_iterator itChangeAction;

            for( itChangeAction = pMap->rbegin(); itChangeAction != pMap->rend() && bOk; ++itChangeAction )
            {
                bOk = Reject( itChangeAction->second, NULL, true );      //! rekursiv
            }
        }
        if ( bOk && (bRejected = pAct->Reject( pDoc )) != false )
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
            ScCellValue aCell;
            aCell.assign(*pDoc, aRange.aStart);
            pReject->SetOldValue(aCell, pDoc, pDoc);
        }
        if ( (bRejected = pAct->Reject( pDoc )) != false && !bRecursion )
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


sal_uLong ScChangeTrack::AddLoadedGenerated(
    const ScCellValue& rNewCell, const ScBigRange& aBigRange, const OUString& sNewValue )
{
    ScChangeActionContent* pAct = new ScChangeActionContent( --nGeneratedMin, rNewCell, aBigRange, pDoc, sNewValue );
    if ( pAct )
    {
        if ( pFirstGeneratedDelContent )
            pFirstGeneratedDelContent->pPrev = pAct;
        pAct->pNext = pFirstGeneratedDelContent;
        pFirstGeneratedDelContent = pAct;
        aGeneratedMap.insert( ::std::make_pair( pAct->GetActionNumber(), pAct ) );
        return pAct->GetActionNumber();
    }
    return 0;
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
        return NULL;
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
        const ScChangeActionContent* pContent = dynamic_cast< const ScChangeActionContent* >( pGenerated );
        OSL_ENSURE( pContent, "ScChangeTrack::Clone: pContent is null!" );
        const ScCellValue& rNewCell = pContent->GetNewCell();
        if (!rNewCell.isEmpty())
        {
            ScCellValue aClonedNewCell;
            aClonedNewCell.assign(rNewCell, *pDocument);
            OUString aNewValue;
            pContent->GetNewString( aNewValue, pDocument );
            pClonedTrack->nGeneratedMin = pGenerated->GetActionNumber() + 1;
            pClonedTrack->AddLoadedGenerated(aClonedNewCell, pGenerated->GetBigRange(), aNewValue);
        }
    }

    // clone actions
    const ScChangeAction* pAction = GetFirst();
    while ( pAction )
    {
        ScChangeAction* pClonedAction = NULL;

        switch ( pAction->GetType() )
        {
            case SC_CAT_INSERT_COLS:
            case SC_CAT_INSERT_ROWS:
            case SC_CAT_INSERT_TABS:
                {
                    pClonedAction = new ScChangeActionIns(
                        pAction->GetActionNumber(),
                        pAction->GetState(),
                        pAction->GetRejectAction(),
                        pAction->GetBigRange(),
                        pAction->GetUser(),
                        pAction->GetDateTimeUTC(),
                        pAction->GetComment(),
                        pAction->GetType() );
                }
                break;
            case SC_CAT_DELETE_COLS:
            case SC_CAT_DELETE_ROWS:
            case SC_CAT_DELETE_TABS:
                {
                    const ScChangeActionDel* pDelete = dynamic_cast< const ScChangeActionDel* >( pAction );
                    OSL_ENSURE( pDelete, "ScChangeTrack::Clone: pDelete is null!" );

                    SCsCOLROW nD = 0;
                    ScChangeActionType eType = pAction->GetType();
                    if ( eType == SC_CAT_DELETE_COLS )
                    {
                        nD = static_cast< SCsCOLROW >( pDelete->GetDx() );
                    }
                    else if ( eType == SC_CAT_DELETE_ROWS )
                    {
                        nD = static_cast< SCsCOLROW >( pDelete->GetDy() );
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
                    const ScChangeActionContent* pContent = dynamic_cast< const ScChangeActionContent* >( pAction );
                    OSL_ENSURE( pContent, "ScChangeTrack::Clone: pContent is null!" );
                    const ScCellValue& rOldCell = pContent->GetOldCell();
                    ScCellValue aClonedOldCell;
                    aClonedOldCell.assign(rOldCell, *pDocument);
                    OUString aOldValue;
                    pContent->GetOldString( aOldValue, pDocument );

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

                    const ScCellValue& rNewCell = pContent->GetNewCell();
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
