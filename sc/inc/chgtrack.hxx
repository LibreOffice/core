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

#ifndef SC_CHGTRACK_HXX
#define SC_CHGTRACK_HXX


#include <tools/string.hxx>
#include <tools/datetime.hxx>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include <tools/queue.hxx>
#include <tools/mempool.hxx>
#include <tools/link.hxx>
#include <unotools/options.hxx>
#include "global.hxx"
#include "bigrange.hxx"
#include "collect.hxx"
#include "scdllapi.h"

#ifdef SC_CHGTRACK_CXX
// core/inc
#include "refupdat.hxx"
#endif

class ScBaseCell;
class ScDocument;


enum ScChangeActionType
{
    SC_CAT_NONE,
    SC_CAT_INSERT_COLS,
    SC_CAT_INSERT_ROWS,
    SC_CAT_INSERT_TABS,
    SC_CAT_DELETE_COLS,
    SC_CAT_DELETE_ROWS,
    SC_CAT_DELETE_TABS,
    SC_CAT_MOVE,
    SC_CAT_CONTENT,
    SC_CAT_REJECT
};


enum ScChangeActionState
{
    SC_CAS_VIRGIN,
    SC_CAS_ACCEPTED,
    SC_CAS_REJECTED
};


enum ScChangeActionClipMode
{
    SC_CACM_NONE,
    SC_CACM_CUT,
    SC_CACM_COPY,
    SC_CACM_PASTE
};

class SvStream;

// --- ScChangeActionLinkEntry ---------------------------------------------

// Inserts itself as the head of a chain (better: linked list?), or before a LinkEntry
// on delete: automatically remove of what is linked (German original was strange...)
// ppPrev == &previous->pNext oder address of pointer to head of linked list,
// *ppPrev == this

class ScChangeAction;

class ScChangeActionLinkEntry
{
                                // not implemented, prevent usage
                                ScChangeActionLinkEntry(
                                    const ScChangeActionLinkEntry& );
    ScChangeActionLinkEntry&    operator=( const ScChangeActionLinkEntry& );

protected:

    ScChangeActionLinkEntry*    pNext;
    ScChangeActionLinkEntry**   ppPrev;
    ScChangeAction*             pAction;
    ScChangeActionLinkEntry*    pLink;

public:

    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry )

                                ScChangeActionLinkEntry(
                                        ScChangeActionLinkEntry** ppPrevP,
                                        ScChangeAction* pActionP )
                                    :   pNext( *ppPrevP ),
                                        ppPrev( ppPrevP ),
                                        pAction( pActionP ),
                                        pLink( NULL )
                                    {
                                        if ( pNext )
                                            pNext->ppPrev = &pNext;
                                        *ppPrevP = this;
                                    }

    virtual                     ~ScChangeActionLinkEntry()
                                    {
                                        ScChangeActionLinkEntry* p = pLink;
                                        UnLink();
                                        Remove();
                                        if ( p )
                                            delete p;
                                    }

            void                SetLink( ScChangeActionLinkEntry* pLinkP )
                                    {
                                        UnLink();
                                        if ( pLinkP )
                                        {
                                            pLink = pLinkP;
                                            pLinkP->pLink = this;
                                        }
                                    }

            void                UnLink()
                                    {
                                        if ( pLink )
                                        {
                                            pLink->pLink = NULL;
                                            pLink = NULL;
                                        }
                                    }

            void                Remove()
                                    {
                                        if ( ppPrev )
                                        {
                                            if ( ( *ppPrev = pNext ) != NULL )
                                                pNext->ppPrev = ppPrev;
                                            ppPrev = NULL;  // not inserted
                                        }
                                    }

            void                Insert( ScChangeActionLinkEntry** ppPrevP )
                                    {
                                        if ( !ppPrev )
                                        {
                                            ppPrev = ppPrevP;
                                            if ( (pNext = *ppPrevP) )
                                                pNext->ppPrev = &pNext;
                                            *ppPrevP = this;
                                        }
                                    }

    const ScChangeActionLinkEntry*  GetLink() const     { return pLink; }
    ScChangeActionLinkEntry*        GetLink()           { return pLink; }
    const ScChangeActionLinkEntry*  GetNext() const     { return pNext; }
    ScChangeActionLinkEntry*        GetNext()           { return pNext; }
    const ScChangeAction*           GetAction() const   { return pAction; }
    ScChangeAction*                 GetAction()         { return pAction; }
};

// --- ScChangeActionCellListEntry -----------------------------------------
// this is only for the XML Export in the hxx
class ScChangeActionContent;

class ScChangeActionCellListEntry
{
    friend class ScChangeAction;
    friend class ScChangeActionDel;
    friend class ScChangeActionMove;
    friend class ScChangeTrack;

            ScChangeActionCellListEntry*    pNext;
            ScChangeActionContent*          pContent;

                                ScChangeActionCellListEntry(
                                    ScChangeActionContent* pContentP,
                                    ScChangeActionCellListEntry* pNextP )
                                    :   pNext( pNextP ),
                                        pContent( pContentP )
                                    {}

public:
    const ScChangeActionCellListEntry* GetNext() const { return pNext; } // this is only for the XML Export public
    const ScChangeActionContent* GetContent() const { return pContent; } // this is only for the XML Export public

    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry )
};

// --- ScChangeAction -------------------------------------------------------

class ScChangeTrack;
class ScChangeActionIns;
class ScChangeActionDel;
class ScChangeActionContent;

class ScChangeAction
{
    friend class ScChangeTrack;
    friend class ScChangeActionIns;
    friend class ScChangeActionDel;
    friend class ScChangeActionMove;
    friend class ScChangeActionContent;

                                // not implemented, prevent usage
                                ScChangeAction( const ScChangeAction& );
            ScChangeAction&     operator=( const ScChangeAction& );

protected:

            ScBigRange          aBigRange;          // Ins/Del/MoveTo/ContentPos
            DateTime            aDateTime;          //! UTC
            String              aUser;              // who?
            String              aComment;           // user comment
            ScChangeAction*     pNext;              // next in linked list
            ScChangeAction*     pPrev;              // previous in linked list
            ScChangeActionLinkEntry*    pLinkAny;   // arbitrary links
            ScChangeActionLinkEntry*    pLinkDeletedIn; // access to insert areas which were
                                                    // deleted or moved or rejected
            ScChangeActionLinkEntry*    pLinkDeleted;   // links to deleted
            ScChangeActionLinkEntry*    pLinkDependent; // links to dependent
            sal_uLong               nAction;
            sal_uLong               nRejectAction;
            ScChangeActionType  eType;
            ScChangeActionState eState;


                                ScChangeAction( ScChangeActionType,
                                                const ScRange& );

                                // only to be used in the XML import
                                ScChangeAction( ScChangeActionType,
                                                const ScBigRange&,
                                                const sal_uLong nAction,
                                                const sal_uLong nRejectAction,
                                                const ScChangeActionState eState,
                                                const DateTime& aDateTime,
                                                const String& aUser,
                                                const String& aComment );
                                // only to be used in the XML import
                                ScChangeAction( ScChangeActionType,
                                                const ScBigRange&,
                                                const sal_uLong nAction);

    virtual                     ~ScChangeAction();

            String              GetRefString( const ScBigRange&,
                                    ScDocument*, sal_Bool bFlag3D = false ) const;

            void                SetActionNumber( sal_uLong n ) { nAction = n; }
            void                SetRejectAction( sal_uLong n ) { nRejectAction = n; }
            void                SetUser( const String& r ) { aUser = r; }
            void                SetType( ScChangeActionType e ) { eType = e; }
            void                SetState( ScChangeActionState e ) { eState = e; }
            void                SetRejected();

            ScBigRange&         GetBigRange() { return aBigRange; }

            ScChangeActionLinkEntry*    AddLink( ScChangeAction* p,
                                            ScChangeActionLinkEntry* pL )
                                    {
                                        ScChangeActionLinkEntry* pLnk =
                                            new ScChangeActionLinkEntry(
                                            &pLinkAny, p );
                                        pLnk->SetLink( pL );
                                        return pLnk;
                                    }
            void                RemoveAllAnyLinks();

    virtual ScChangeActionLinkEntry*    GetDeletedIn() const
                                            { return pLinkDeletedIn; }
    virtual ScChangeActionLinkEntry**   GetDeletedInAddress()
                                            { return &pLinkDeletedIn; }
            ScChangeActionLinkEntry*    AddDeletedIn( ScChangeAction* p )
                                    {
                                        return new ScChangeActionLinkEntry(
                                            GetDeletedInAddress(), p );
                                    }
            sal_Bool                RemoveDeletedIn( const ScChangeAction* );
            void                SetDeletedIn( ScChangeAction* );

            ScChangeActionLinkEntry*    AddDeleted( ScChangeAction* p )
                                    {
                                        return new ScChangeActionLinkEntry(
                                            &pLinkDeleted, p );
                                    }
            void                RemoveAllDeleted();

            ScChangeActionLinkEntry*    AddDependent( ScChangeAction* p )
                                    {
                                        return new ScChangeActionLinkEntry(
                                            &pLinkDependent, p );
                                    }
            void                RemoveAllDependent();

            void                RemoveAllLinks();

    virtual void                AddContent( ScChangeActionContent* ) = 0;
    virtual void                DeleteCellEntries() = 0;

    virtual void                UpdateReference( const ScChangeTrack*,
                                    UpdateRefMode, const ScBigRange&,
                                    sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

            void                Accept();
    virtual sal_Bool                Reject( ScDocument* ) = 0;
            void                RejectRestoreContents( ScChangeTrack*,
                                    SCsCOL nDx, SCsROW nDy );

                                // used in Reject() instead of IsRejectable()
            sal_Bool                IsInternalRejectable() const;

                                // Derived classes that hold a pointer to the
                                // ChangeTrack must return that. Otherwise NULL.
    virtual const ScChangeTrack*    GetChangeTrack() const = 0;

public:

            sal_Bool                IsInsertType() const
                                    {
                                        return eType == SC_CAT_INSERT_COLS ||
                                            eType == SC_CAT_INSERT_ROWS ||
                                            eType == SC_CAT_INSERT_TABS;
                                    }
            sal_Bool                IsDeleteType() const
                                    {
                                        return eType == SC_CAT_DELETE_COLS ||
                                            eType == SC_CAT_DELETE_ROWS ||
                                            eType == SC_CAT_DELETE_TABS;
                                    }
            sal_Bool                IsVirgin() const
                                    { return eState == SC_CAS_VIRGIN; }
            sal_Bool                IsAccepted() const
                                    { return eState == SC_CAS_ACCEPTED; }
            sal_Bool                IsRejected() const
                                    { return eState == SC_CAS_REJECTED; }

                                // Action rejects another Action
            sal_Bool                IsRejecting() const
                                    { return nRejectAction != 0; }

                                // if action is visible in the document
            sal_Bool                IsVisible() const;

                                // if action if touchable
            sal_Bool                IsTouchable() const;

                                // if action is an entry in dialog root
            sal_Bool                IsDialogRoot() const;

                                // if an entry in a dialog shall be a drop down entry
            sal_Bool                IsDialogParent() const;

                                // if action is a delete with subdeletes (aufgeklappt = open ?)
            sal_Bool                IsMasterDelete() const;

                                // if action is acceptable/selectable/rejectable
            sal_Bool                IsClickable() const;

                                // if action is rejectable
            sal_Bool                IsRejectable() const;

            const ScBigRange&   GetBigRange() const { return aBigRange; }
            SC_DLLPUBLIC DateTime           GetDateTime() const;        // local time
            const DateTime&     GetDateTimeUTC() const      // UTC time
                                    { return aDateTime; }
            const String&       GetUser() const { return aUser; }
            const String&       GetComment() const { return aComment; }
            ScChangeActionType  GetType() const { return eType; }
            ScChangeActionState GetState() const { return eState; }
            sal_uLong               GetActionNumber() const { return nAction; }
            sal_uLong               GetRejectAction() const { return nRejectAction; }

            ScChangeAction*     GetNext() const { return pNext; }
            ScChangeAction*     GetPrev() const { return pPrev; }

            sal_Bool                IsDeletedIn() const
                                    { return GetDeletedIn() != NULL; }
            sal_Bool                IsDeleted() const
                                    { return IsDeleteType() || IsDeletedIn(); }
            sal_Bool                IsDeletedIn( const ScChangeAction* ) const;
            sal_Bool                IsDeletedInDelType( ScChangeActionType ) const;
            void                RemoveAllDeletedIn();

            const ScChangeActionLinkEntry* GetFirstDeletedEntry() const
                                    { return pLinkDeleted; }
            const ScChangeActionLinkEntry* GetFirstDependentEntry() const
                                    { return pLinkDependent; }
            sal_Bool                HasDependent() const
                                    { return pLinkDependent != NULL; }
            sal_Bool                HasDeleted() const
                                    { return pLinkDeleted != NULL; }

                                // description will be appended to string
                                // with bSplitRange only one column/row will be considered for delete
                                // (for a listing of entries)
    virtual void                GetDescription( String&, ScDocument*,
                                    sal_Bool bSplitRange = false, bool bWarning = true ) const;

    virtual void                GetRefString( String&, ScDocument*,
                                    sal_Bool bFlag3D = false ) const;

                                // for DocumentMerge set old date of the other
                                // action, fetched by GetDateTimeUTC
            void                SetDateTimeUTC( const DateTime& rDT )
                                    { aDateTime = rDT; }

                                // set user comment
            void                SetComment( const String& rStr )
                                    { aComment = rStr; }

                                // only to be used in the XML import
            void                SetDeletedInThis( sal_uLong nActionNumber,
                                        const ScChangeTrack* pTrack );
                                // only to be used in the XML import
            void                AddDependent( sal_uLong nActionNumber,
                                        const ScChangeTrack* pTrack );
};


// --- ScChangeActionIns ----------------------------------------------------

class ScChangeActionIns : public ScChangeAction
{
    friend class ScChangeTrack;

                                ScChangeActionIns( const ScRange& rRange );
    virtual                     ~ScChangeActionIns();

    virtual void                AddContent( ScChangeActionContent* ) {}
    virtual void                DeleteCellEntries() {}

    virtual sal_Bool                Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

public:
                                ScChangeActionIns(const sal_uLong nActionNumber,
                                        const ScChangeActionState eState,
                                        const sal_uLong nRejectingNumber,
                                        const ScBigRange& aBigRange,
                                        const String& aUser,
                                        const DateTime& aDateTime,
                                        const String &sComment,
                                        const ScChangeActionType eType); // only to use in the XML import

    virtual void                GetDescription( String&, ScDocument*,
                                    sal_Bool bSplitRange = false, bool bWarning = true ) const;
};


// --- ScChangeActionDel ----------------------------------------------------

class ScChangeActionMove;

class ScChangeActionDelMoveEntry : public ScChangeActionLinkEntry
{
    friend class ScChangeActionDel;
    friend class ScChangeTrack;

            short               nCutOffFrom;
            short               nCutOffTo;


                                ScChangeActionDelMoveEntry(
                                    ScChangeActionDelMoveEntry** ppPrevP,
                                    ScChangeActionMove* pMove,
                                    short nFrom, short nTo )
                                    :   ScChangeActionLinkEntry(
                                            (ScChangeActionLinkEntry**)
                                                ppPrevP,
                                            (ScChangeAction*) pMove ),
                                        nCutOffFrom( nFrom ),
                                        nCutOffTo( nTo )
                                    {}

            ScChangeActionDelMoveEntry* GetNext()
                                    {
                                        return (ScChangeActionDelMoveEntry*)
                                        ScChangeActionLinkEntry::GetNext();
                                    }
            ScChangeActionMove* GetMove()
                                    {
                                        return (ScChangeActionMove*)
                                        ScChangeActionLinkEntry::GetAction();
                                    }

public:
            const ScChangeActionDelMoveEntry*   GetNext() const
                                    {
                                        return (const ScChangeActionDelMoveEntry*)
                                        ScChangeActionLinkEntry::GetNext();
                                    }
            const ScChangeActionMove*   GetMove() const
                                    {
                                        return (const ScChangeActionMove*)
                                        ScChangeActionLinkEntry::GetAction();
                                    }
            short               GetCutOffFrom() const { return nCutOffFrom; }
            short               GetCutOffTo() const { return nCutOffTo; }
};


class ScChangeActionDel : public ScChangeAction
{
    friend class ScChangeTrack;
    friend void ScChangeAction::Accept();

            ScChangeTrack*      pTrack;
            ScChangeActionCellListEntry* pFirstCell;
            ScChangeActionIns*  pCutOff;        // cut insert
            short               nCutOff;        // +: start  -: end
            ScChangeActionDelMoveEntry* pLinkMove;
            SCsCOL              nDx;
            SCsROW              nDy;

                                ScChangeActionDel( const ScRange& rRange,
                                    SCsCOL nDx, SCsROW nDy, ScChangeTrack* );
    virtual                     ~ScChangeActionDel();

            ScChangeActionIns*  GetCutOffInsert() { return pCutOff; }

    virtual void                AddContent( ScChangeActionContent* );
    virtual void                DeleteCellEntries();

            void                UndoCutOffMoves();
            void                UndoCutOffInsert();

    virtual void                UpdateReference( const ScChangeTrack*,
                                    UpdateRefMode, const ScBigRange&,
                                    sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

    virtual sal_Bool                Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return pTrack; }

public:
                                ScChangeActionDel(const sal_uLong nActionNumber,
                                                const ScChangeActionState eState,
                                                const sal_uLong nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                const ScChangeActionType eType,
                                                const SCsCOLROW nD,
                                                ScChangeTrack* pTrack); // only to use in the XML import
                                                                        // which of nDx and nDy is set is dependend on the type

                                // is the last in a row (or single)
            sal_Bool                IsBaseDelete() const;

                                // is the first in a row (or single)
            sal_Bool                IsTopDelete() const;

                                // is part of a row
            sal_Bool                IsMultiDelete() const;

                                // is col, belonging to a TabDelete
            sal_Bool                IsTabDeleteCol() const;

            SCsCOL              GetDx() const { return nDx; }
            SCsROW              GetDy() const { return nDy; }
            ScBigRange          GetOverAllRange() const;    // BigRange + (nDx, nDy)

            const ScChangeActionCellListEntry* GetFirstCellEntry() const
                                    { return pFirstCell; }
            const ScChangeActionDelMoveEntry* GetFirstMoveEntry() const
                                    { return pLinkMove; }
            const ScChangeActionIns*    GetCutOffInsert() const { return pCutOff; }
            short               GetCutOffCount() const { return nCutOff; }

    virtual void                GetDescription( String&, ScDocument*,
                                    sal_Bool bSplitRange = false, bool bWarning = true ) const;
            void                SetCutOffInsert( ScChangeActionIns* p, short n )
                                    { pCutOff = p; nCutOff = n; }   // only to use in the XML import
                                                                    // this should be protected, but for the XML import it is public
            // only to use in the XML import
            // this should be protected, but for the XML import it is public
            ScChangeActionDelMoveEntry* AddCutOffMove( ScChangeActionMove* pMove,
                                        short nFrom, short nTo )
                                    {
                                        return new ScChangeActionDelMoveEntry(
                                        &pLinkMove, pMove, nFrom, nTo );
                                    }
};


// --- ScChangeActionMove ---------------------------------------------------

class ScChangeActionMove : public ScChangeAction
{
    friend class ScChangeTrack;
    friend class ScChangeActionDel;

            ScBigRange          aFromRange;
            ScChangeTrack*      pTrack;
            ScChangeActionCellListEntry* pFirstCell;
            sal_uLong               nStartLastCut;  // fuer PasteCut Undo
            sal_uLong               nEndLastCut;

                                ScChangeActionMove( const ScRange& rFromRange,
                                    const ScRange& rToRange,
                                    ScChangeTrack* pTrackP )
                                    : ScChangeAction( SC_CAT_MOVE, rToRange ),
                                        aFromRange( rFromRange ),
                                        pTrack( pTrackP ),
                                        pFirstCell( NULL ),
                                        nStartLastCut(0),
                                        nEndLastCut(0)
                                    {}
    virtual                     ~ScChangeActionMove();

    virtual void                AddContent( ScChangeActionContent* );
    virtual void                DeleteCellEntries();

            ScBigRange&         GetFromRange() { return aFromRange; }

            void                SetStartLastCut( sal_uLong nVal ) { nStartLastCut = nVal; }
            sal_uLong               GetStartLastCut() const { return nStartLastCut; }
            void                SetEndLastCut( sal_uLong nVal ) { nEndLastCut = nVal; }
            sal_uLong               GetEndLastCut() const { return nEndLastCut; }

    virtual void                UpdateReference( const ScChangeTrack*,
                                    UpdateRefMode, const ScBigRange&,
                                    sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

    virtual sal_Bool                Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return pTrack; }

protected:
    using ScChangeAction::GetRefString;

public:
                                ScChangeActionMove(const sal_uLong nActionNumber,
                                                const ScChangeActionState eState,
                                                const sal_uLong nRejectingNumber,
                                                const ScBigRange& aToBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                const ScBigRange& aFromBigRange,
                                                ScChangeTrack* pTrack); // only to use in the XML import
            const ScChangeActionCellListEntry* GetFirstCellEntry() const
                                    { return pFirstCell; } // only to use in the XML export

            const ScBigRange&   GetFromRange() const { return aFromRange; }
    SC_DLLPUBLIC        void                GetDelta( sal_Int32& nDx, sal_Int32& nDy, sal_Int32& nDz ) const;

    virtual void                GetDescription( String&, ScDocument*,
                                    sal_Bool bSplitRange = false, bool bWarning = true ) const;

    virtual void                GetRefString( String&, ScDocument*,
                                    sal_Bool bFlag3D = false ) const;
};


// --- ScChangeActionContent ------------------------------------------------

enum ScChangeActionContentCellType
{
    SC_CACCT_NONE = 0,
    SC_CACCT_NORMAL,
    SC_CACCT_MATORG,
    SC_CACCT_MATREF
};

class Stack;

class ScChangeActionContent : public ScChangeAction
{
    friend class ScChangeTrack;

            String              aOldValue;
            String              aNewValue;
            ScBaseCell*         pOldCell;
            ScBaseCell*         pNewCell;
        ScChangeActionContent*  pNextContent;   // at the same position
        ScChangeActionContent*  pPrevContent;
        ScChangeActionContent*  pNextInSlot;    // in the same slot
        ScChangeActionContent** ppPrevInSlot;

            void                InsertInSlot( ScChangeActionContent** pp )
                                    {
                                        if ( !ppPrevInSlot )
                                        {
                                            ppPrevInSlot = pp;
                                            if ( ( pNextInSlot = *pp ) != NULL )
                                                pNextInSlot->ppPrevInSlot = &pNextInSlot;
                                            *pp = this;
                                        }
                                    }
            void                RemoveFromSlot()
                                    {
                                        if ( ppPrevInSlot )
                                        {
                                            if ( ( *ppPrevInSlot = pNextInSlot ) != NULL )
                                                pNextInSlot->ppPrevInSlot = ppPrevInSlot;
                                            ppPrevInSlot = NULL;    // not inserted
                                        }
                                    }
        ScChangeActionContent*  GetNextInSlot() { return pNextInSlot; }

            void                ClearTrack();

    static  void                GetStringOfCell( String& rStr,
                                    const ScBaseCell* pCell,
                                    const ScDocument* pDoc,
                                    const ScAddress& rPos );

    static  void                GetStringOfCell( String& rStr,
                                    const ScBaseCell* pCell,
                                    const ScDocument* pDoc,
                                    sal_uLong nFormat );

    static  void                SetValue( String& rStr, ScBaseCell*& pCell,
                                    const ScAddress& rPos,
                                    const ScBaseCell* pOrgCell,
                                    const ScDocument* pFromDoc,
                                    ScDocument* pToDoc );

    static  void                SetValue( String& rStr, ScBaseCell*& pCell,
                                    sal_uLong nFormat,
                                    const ScBaseCell* pOrgCell,
                                    const ScDocument* pFromDoc,
                                    ScDocument* pToDoc );

    static  void                SetCell( String& rStr, ScBaseCell* pCell,
                                    sal_uLong nFormat, const ScDocument* pDoc );

    static  sal_Bool                NeedsNumberFormat( const ScBaseCell* );

            void                SetValueString( String& rValue,
                                    ScBaseCell*& pCell, const String& rStr,
                                    ScDocument* pDoc );

            void                GetValueString( String& rStr,
                                    const String& rValue,
                                    const ScBaseCell* pCell ) const;

            void                GetFormulaString( String& rStr,
                                    const ScFormulaCell* pCell ) const;

    virtual void                AddContent( ScChangeActionContent* ) {}
    virtual void                DeleteCellEntries() {}

    virtual void                UpdateReference( const ScChangeTrack*,
                                    UpdateRefMode, const ScBigRange&,
                                    sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

    virtual sal_Bool                Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

                                // pRejectActions!=NULL: reject actions get
                                // stacked, no SetNewValue, no Append
            sal_Bool                Select( ScDocument*, ScChangeTrack*,
                                    sal_Bool bOldest, Stack* pRejectActions );

            void                PutValueToDoc( ScBaseCell*, const String&,
                                    ScDocument*, SCsCOL nDx, SCsROW nDy ) const;

protected:
    using ScChangeAction::GetRefString;

public:

    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent )

                                ScChangeActionContent( const ScRange& rRange )
                                    : ScChangeAction( SC_CAT_CONTENT, rRange ),
                                        pOldCell( NULL ),
                                        pNewCell( NULL ),
                                        pNextContent( NULL ),
                                        pPrevContent( NULL ),
                                        pNextInSlot( NULL ),
                                        ppPrevInSlot( NULL )
                                    {}
                                ScChangeActionContent(const sal_uLong nActionNumber,
                                                const ScChangeActionState eState,
                                                const sal_uLong nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                ScBaseCell* pOldCell,
                                                ScDocument* pDoc,
                                                const String& sOldValue); // to use for XML Import
                                ScChangeActionContent(const sal_uLong nActionNumber,
                                                ScBaseCell* pNewCell,
                                                const ScBigRange& aBigRange,
                                                ScDocument* pDoc,
                                                const String& sNewValue); // to use for XML Import of Generated Actions
    virtual                     ~ScChangeActionContent();

        ScChangeActionContent*  GetNextContent() const { return pNextContent; }
        ScChangeActionContent*  GetPrevContent() const { return pPrevContent; }
        ScChangeActionContent*  GetTopContent() const;
            sal_Bool                IsTopContent() const
                                    { return pNextContent == NULL; }

    virtual ScChangeActionLinkEntry*    GetDeletedIn() const;
    virtual ScChangeActionLinkEntry**   GetDeletedInAddress();

            void                PutOldValueToDoc( ScDocument*,
                                    SCsCOL nDx, SCsROW nDy ) const;
            void                PutNewValueToDoc( ScDocument*,
                                    SCsCOL nDx, SCsROW nDy ) const;

            void                SetOldValue( const ScBaseCell*,
                                    const ScDocument* pFromDoc,
                                    ScDocument* pToDoc,
                                    sal_uLong nFormat );
            void                SetOldValue( const ScBaseCell*,
                                    const ScDocument* pFromDoc,
                                    ScDocument* pToDoc );
            void                SetNewValue( const ScBaseCell*, ScDocument* );

                                // Used in import filter AppendContentOnTheFly,
                                // takes ownership of cells.
            void                SetOldNewCells( ScBaseCell* pOldCell,
                                    sal_uLong nOldFormat, ScBaseCell* pNewCell,
                                    sal_uLong nNewFormat, ScDocument* pDoc );

                                // Use this only in the XML import,
                                // takes ownership of cell.
            void                SetNewCell( ScBaseCell* pCell, ScDocument* pDoc, const String& rFormatted );

                                // These functions should be protected but for
                                // the XML import they are public.
            void                SetNextContent( ScChangeActionContent* p )
                                    { pNextContent = p; }
            void                SetPrevContent( ScChangeActionContent* p )
                                    { pPrevContent = p; }

                                // don't use:
                                // assigns String / creates forumula cell
            void                SetOldValue( const String& rOld, ScDocument* );
            void                SetNewValue( const String& rNew, ScDocument* );

            void                GetOldString( String& ) const;
            void                GetNewString( String& ) const;
            const ScBaseCell*   GetOldCell() const { return pOldCell; }
            const ScBaseCell*   GetNewCell() const { return pNewCell; }
    virtual void                GetDescription( String&, ScDocument*,
                                    sal_Bool bSplitRange = false, bool bWarning = true ) const;
    virtual void                GetRefString( String&, ScDocument*,
                                    sal_Bool bFlag3D = false ) const;

    static  ScChangeActionContentCellType   GetContentCellType( const ScBaseCell* );

                                // NewCell
            sal_Bool                IsMatrixOrigin() const
                                    {
                                        return GetContentCellType( GetNewCell() )
                                            == SC_CACCT_MATORG;
                                    }
            sal_Bool                IsMatrixReference() const
                                    {
                                        return GetContentCellType( GetNewCell() )
                                            == SC_CACCT_MATREF;
                                    }
                                // OldCell
            sal_Bool                IsOldMatrixOrigin() const
                                    {
                                        return GetContentCellType( GetOldCell() )
                                            == SC_CACCT_MATORG;
                                    }
            sal_Bool                IsOldMatrixReference() const
                                    {
                                        return GetContentCellType( GetOldCell() )
                                            == SC_CACCT_MATREF;
                                    }

};


// --- ScChangeActionReject -------------------------------------------------

class Stack;

class ScChangeActionReject : public ScChangeAction
{
    friend class ScChangeTrack;
    friend class ScChangeActionContent;

                                ScChangeActionReject( sal_uLong nReject )
                                    : ScChangeAction( SC_CAT_REJECT, ScRange() )
                                    {
                                        SetRejectAction( nReject );
                                        SetState( SC_CAS_ACCEPTED );
                                    }

    virtual void                AddContent( ScChangeActionContent* ) {}
    virtual void                DeleteCellEntries() {}

    virtual sal_Bool                Reject( ScDocument* ) { return false; }

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

public:
                                ScChangeActionReject(const sal_uLong nActionNumber,
                                                const ScChangeActionState eState,
                                                const sal_uLong nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment); // only to use in the XML import
};


// --- ScChangeTrack --------------------------------------------------------

enum ScChangeTrackMsgType
{
    SC_CTM_NONE,
    SC_CTM_APPEND,      // Actions appended
    SC_CTM_REMOVE,      // Actions removed
    SC_CTM_CHANGE,      // Actions changed
    SC_CTM_PARENT       // became a parent (and wasn't before)
};

struct ScChangeTrackMsgInfo
{
    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo )

    ScChangeTrackMsgType    eMsgType;
    sal_uLong                   nStartAction;
    sal_uLong                   nEndAction;
};

// MsgQueue for notification via ModifiedLink
DECLARE_QUEUE( ScChangeTrackMsgQueue, ScChangeTrackMsgInfo* )
DECLARE_STACK( ScChangeTrackMsgStack, ScChangeTrackMsgInfo* )

enum ScChangeTrackMergeState
{
    SC_CTMS_NONE,
    SC_CTMS_PREPARE,
    SC_CTMS_OWN,
    SC_CTMS_UNDO,
    SC_CTMS_OTHER
};

// Table, additionally to pFirst/pNext/pLast/pPrev, to enable fast access by ActionNumber and by list
DECLARE_TABLE( ScChangeActionTable, ScChangeAction* )

// Internally generated actions start at this value (nearly all bits set)
// and are decremented, to keep values in a table seperated from "normal" actions.
#define SC_CHGTRACK_GENERATED_START ((sal_uInt32) 0xfffffff0)

class ScChangeTrack : public utl::ConfigurationListener
{
    friend void ScChangeAction::RejectRestoreContents( ScChangeTrack*, SCsCOL, SCsROW );
    friend sal_Bool ScChangeActionDel::Reject( ScDocument* pDoc );
    friend void ScChangeActionDel::DeleteCellEntries();
    friend void ScChangeActionMove::DeleteCellEntries();
    friend sal_Bool ScChangeActionMove::Reject( ScDocument* pDoc );

    static  const SCROW         nContentRowsPerSlot;
    static  const SCSIZE        nContentSlots;

    com::sun::star::uno::Sequence< sal_Int8 >   aProtectPass;
            ScChangeActionTable aTable;
            ScChangeActionTable aGeneratedTable;
            ScChangeActionTable aPasteCutTable;
        ScChangeTrackMsgQueue   aMsgQueue;
        ScChangeTrackMsgStack   aMsgStackTmp;
        ScChangeTrackMsgStack   aMsgStackFinal;
            ScStrCollection     aUserCollection;
            String              aUser;
            Link                aModifiedLink;
            ScRange             aInDeleteRange;
            DateTime            aFixDateTime;
            ScChangeAction*     pFirst;
            ScChangeAction*     pLast;
        ScChangeActionContent*  pFirstGeneratedDelContent;
        ScChangeActionContent** ppContentSlots;
        ScChangeActionMove*     pLastCutMove;
    ScChangeActionLinkEntry*    pLinkInsertCol;
    ScChangeActionLinkEntry*    pLinkInsertRow;
    ScChangeActionLinkEntry*    pLinkInsertTab;
    ScChangeActionLinkEntry*    pLinkMove;
        ScChangeTrackMsgInfo*   pBlockModifyMsg;
            ScDocument*         pDoc;
            sal_uLong               nActionMax;
            sal_uLong               nGeneratedMin;
            sal_uLong               nMarkLastSaved;
            sal_uLong               nStartLastCut;
            sal_uLong               nEndLastCut;
            sal_uLong               nLastMerge;
        ScChangeTrackMergeState eMergeState;
            sal_uInt16              nLoadedFileFormatVersion;
            sal_Bool                bLoadSave;
            sal_Bool                bInDelete;
            sal_Bool                bInDeleteUndo;
            sal_Bool                bInDeleteTop;
            sal_Bool                bInPasteCut;
            sal_Bool                bUseFixDateTime;
            sal_Bool                bTime100thSeconds;

                                // not implemented, prevent usage
                                ScChangeTrack( const ScChangeTrack& );
            ScChangeTrack&      operator=( const ScChangeTrack& );

#ifdef SC_CHGTRACK_CXX
    static  SCROW               InitContentRowsPerSlot();

                                // sal_True if one is MM_FORMULA and the other is
                                // not, or if both are and range differs
    static  sal_Bool                IsMatrixFormulaRangeDifferent(
                                    const ScBaseCell* pOldCell,
                                    const ScBaseCell* pNewCell );

            void                Init();
            void                DtorClear();
            void                SetLoadSave( sal_Bool bVal ) { bLoadSave = bVal; }
            void                SetInDeleteRange( const ScRange& rRange )
                                    { aInDeleteRange = rRange; }
            void                SetInDelete( sal_Bool bVal )
                                    { bInDelete = bVal; }
            void                SetInDeleteTop( sal_Bool bVal )
                                    { bInDeleteTop = bVal; }
            void                SetInDeleteUndo( sal_Bool bVal )
                                    { bInDeleteUndo = bVal; }
            void                SetInPasteCut( sal_Bool bVal )
                                    { bInPasteCut = bVal; }
            void                SetMergeState( ScChangeTrackMergeState eState )
                                    { eMergeState = eState; }
        ScChangeTrackMergeState GetMergeState() const { return eMergeState; }
            void                SetLastMerge( sal_uLong nVal ) { nLastMerge = nVal; }
            sal_uLong               GetLastMerge() const { return nLastMerge; }

            void                SetLastCutMoveRange( const ScRange&, ScDocument* );

                                // create block of ModifyMsg
            void                StartBlockModify( ScChangeTrackMsgType,
                                    sal_uLong nStartAction );
            void                EndBlockModify( sal_uLong nEndAction );

            void                AddDependentWithNotify( ScChangeAction* pParent,
                                    ScChangeAction* pDependent );

            void                Dependencies( ScChangeAction* );
            void                UpdateReference( ScChangeAction*, sal_Bool bUndo );
            void                UpdateReference( ScChangeAction** ppFirstAction,
                                    ScChangeAction* pAct, sal_Bool bUndo );
            void                Append( ScChangeAction* pAppend, sal_uLong nAction );
    SC_DLLPUBLIC        void                AppendDeleteRange( const ScRange&,
                                    ScDocument* pRefDoc, SCsTAB nDz,
                                    sal_uLong nRejectingInsert );
            void                AppendOneDeleteRange( const ScRange& rOrgRange,
                                    ScDocument* pRefDoc,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    sal_uLong nRejectingInsert );
            void                LookUpContents( const ScRange& rOrgRange,
                                    ScDocument* pRefDoc,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
            void                Remove( ScChangeAction* );
            void                MasterLinks( ScChangeAction* );

                                // Content on top an Position
        ScChangeActionContent*  SearchContentAt( const ScBigAddress&,
                                    ScChangeAction* pButNotThis ) const;
            void                DeleteGeneratedDelContent(
                                    ScChangeActionContent* );
        ScChangeActionContent*  GenerateDelContent( const ScAddress&,
                                    const ScBaseCell*,
                                    const ScDocument* pFromDoc );
            void                DeleteCellEntries(
                                    ScChangeActionCellListEntry*&,
                                    ScChangeAction* pDeletor );

                                // Reject action and all dependent actions,
                                // Table stems from previous GetDependents,
                                // only needed for Insert and Move (MasterType),
                                // is NULL otherwise.
                                // bRecursion == called from reject with table
            sal_Bool                Reject( ScChangeAction*,
                                    ScChangeActionTable*, sal_Bool bRecursion );

#endif  // SC_CHGTRACK_CXX

            void                ClearMsgQueue();
    virtual void                ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

public:

    static  SCSIZE              ComputeContentSlot( sal_Int32 nRow )
                                    {
                                        if ( nRow < 0 || nRow > MAXROW )
                                            return nContentSlots - 1;
                                        return static_cast< SCSIZE >( nRow / nContentRowsPerSlot );
                                    }

            SC_DLLPUBLIC        ScChangeTrack( ScDocument* );
                                ScChangeTrack( ScDocument*,
                                            const ScStrCollection& ); // only to use in the XML import
            SC_DLLPUBLIC virtual ~ScChangeTrack();
            void                Clear();

            ScChangeActionContent*  GetFirstGenerated() const { return pFirstGeneratedDelContent; }
            ScChangeAction*     GetFirst() const { return pFirst; }
            ScChangeAction*     GetLast() const { return pLast; }
            sal_uLong               GetActionMax() const { return nActionMax; }
            sal_Bool                IsGenerated( sal_uLong nAction ) const
                                    { return nAction >= nGeneratedMin; }
            ScChangeAction*     GetAction( sal_uLong nAction ) const
                                    { return aTable.Get( nAction ); }
            ScChangeAction*     GetGenerated( sal_uLong nGenerated ) const
                                    { return aGeneratedTable.Get( nGenerated ); }
            ScChangeAction*     GetActionOrGenerated( sal_uLong nAction ) const
                                    {
                                        return IsGenerated( nAction ) ?
                                            GetGenerated( nAction ) :
                                            GetAction( nAction );
                                    }
            sal_uLong               GetLastSavedActionNumber() const
                                    { return nMarkLastSaved; }
            void                SetLastSavedActionNumber(sal_uLong nNew)
                                    { nMarkLastSaved = nNew; }
            ScChangeAction*     GetLastSaved() const
                                    { return aTable.Get( nMarkLastSaved ); }
        ScChangeActionContent** GetContentSlots() const { return ppContentSlots; }

            sal_Bool                IsLoadSave() const { return bLoadSave; }
            const ScRange&      GetInDeleteRange() const
                                    { return aInDeleteRange; }
            sal_Bool                IsInDelete() const { return bInDelete; }
            sal_Bool                IsInDeleteTop() const { return bInDeleteTop; }
            sal_Bool                IsInDeleteUndo() const { return bInDeleteUndo; }
            sal_Bool                IsInPasteCut() const { return bInPasteCut; }
    SC_DLLPUBLIC        void                SetUser( const String& );
            const String&       GetUser() const { return aUser; }
            const ScStrCollection&  GetUserCollection() const
                                    { return aUserCollection; }
            ScDocument*         GetDocument() const { return pDoc; }
                                // for import filter
            const DateTime&     GetFixDateTime() const { return aFixDateTime; }

                                // set this if the date/time set with
                                // SetFixDateTime...() shall be applied to
                                // appended actions
            void                SetUseFixDateTime( sal_Bool bVal )
                                    { bUseFixDateTime = bVal; }
                                // for MergeDocument, apply original date/time as UTC
            void                SetFixDateTimeUTC( const DateTime& rDT )
                                    { aFixDateTime = rDT; }
                                // for import filter, apply original date/time as local time
            void                SetFixDateTimeLocal( const DateTime& rDT )
                                    { aFixDateTime = rDT; aFixDateTime.ConvertToUTC(); }

            void                Append( ScChangeAction* );

                                // pRefDoc may be NULL => no lookup of contents
                                // => no generation of deleted contents
    SC_DLLPUBLIC        void                AppendDeleteRange( const ScRange&,
                                    ScDocument* pRefDoc,
                                    sal_uLong& nStartAction, sal_uLong& nEndAction,
                                    SCsTAB nDz = 0 );
                                    // nDz: multi TabDel, LookUpContent must be searched
                                    // with an offset of -nDz

                                // after new value was set in the document,
                                // old value from RefDoc/UndoDoc
            void                AppendContent( const ScAddress& rPos,
                                    ScDocument* pRefDoc );
                                // after new values were set in the document,
                                // old values from RefDoc/UndoDoc
            void                AppendContentRange( const ScRange& rRange,
                                    ScDocument* pRefDoc,
                                    sal_uLong& nStartAction, sal_uLong& nEndAction,
                                    ScChangeActionClipMode eMode = SC_CACM_NONE );
                                // after new value was set in the document,
                                // old value from pOldCell, nOldFormat,
                                // RefDoc==NULL => Doc
            void                AppendContent( const ScAddress& rPos,
                                    const ScBaseCell* pOldCell,
                                    sal_uLong nOldFormat, ScDocument* pRefDoc = NULL );
                                // after new value was set in the document,
                                // old value from pOldCell, format from Doc
            void                AppendContent( const ScAddress& rPos,
                                    const ScBaseCell* pOldCell );
                                // after new values were set in the document,
                                // old values from RefDoc/UndoDoc.
                                // All contents with a cell in RefDoc
            void                AppendContentsIfInRefDoc( ScDocument* pRefDoc,
                                    sal_uLong& nStartAction, sal_uLong& nEndAction );

                                // Meant for import filter, creates and inserts
                                // an unconditional content action of the two
                                // cells without querying the document, not
                                // even for number formats (though the number
                                // formatter of the document may be used).
                                // The action is returned and may be used to
                                // set user name, description, date/time et al.
                                // Takes ownership of the cells!
    SC_DLLPUBLIC    ScChangeActionContent*  AppendContentOnTheFly( const ScAddress& rPos,
                                    ScBaseCell* pOldCell,
                                    ScBaseCell* pNewCell,
                                    sal_uLong nOldFormat = 0,
                                    sal_uLong nNewFormat = 0 );

                                // Only use the following two if there is no different solution!
                                // (Assign String for NewValue or creation of a formula respectively)

    SC_DLLPUBLIC        void                AppendInsert( const ScRange& );

                                // pRefDoc may be NULL => no lookup of contents
                                // => no generation of deleted contents
    SC_DLLPUBLIC        void                AppendMove( const ScRange& rFromRange,
                                    const ScRange& rToRange,
                                    ScDocument* pRefDoc );

                                // Cut to Clipboard
            void                ResetLastCut()
                                    {
                                        nStartLastCut = nEndLastCut = 0;
                                        if ( pLastCutMove )
                                        {
                                            delete pLastCutMove;
                                            pLastCutMove = NULL;
                                        }
                                    }
            sal_Bool                HasLastCut() const
                                    {
                                        return nEndLastCut > 0 &&
                                            nStartLastCut <= nEndLastCut &&
                                            pLastCutMove;
                                    }

    SC_DLLPUBLIC        void                Undo( sal_uLong nStartAction, sal_uLong nEndAction, bool bMerge = false );

                                // adjust references for MergeDocument
                                //! may only be used in a temporary opened document.
                                //! the Track (?) is unclean afterwards
            void                MergePrepare( ScChangeAction* pFirstMerge, bool bShared = false );
            void                MergeOwn( ScChangeAction* pAct, sal_uLong nFirstMerge, bool bShared = false );
    static  sal_Bool                MergeIgnore( const ScChangeAction&, sal_uLong nFirstMerge );

                                // This comment was already really strange in German.
                                // Tried to structure it a little. Hope no information got lost...
                                //
                                // Insert dependents into table.
                                // ScChangeAction is
                                // - "Insert": really dependents
                                // - "Move": dependent contents in FromRange /
                                //           deleted contents in ToRange
                                //      OR   inserts in FromRange or ToRange
                                // - "Delete": a list of deleted (what?)
                                //      OR     for content, different contents at the same position
                                //      OR     MatrixReferences belonging to MatrixOrigin
                                //
                                // With bListMasterDelete (==TRUE ?) all Deletes of a row belonging
                                // to a MasterDelete are listed (possibly it is
                                // "all Deletes belonging...are listed in a row?)
                                //
                                // With bAllFlat (==TRUE ?) all dependents of dependents
                                // will be inserted flatly.

    SC_DLLPUBLIC        void                GetDependents( ScChangeAction*,
                                    ScChangeActionTable&,
                                    sal_Bool bListMasterDelete = false,
                                    sal_Bool bAllFlat = false ) const;

                                // Reject visible action (and dependents)
            sal_Bool                Reject( ScChangeAction*, bool bShared = false );

                                // Accept visible action (and dependents)
    SC_DLLPUBLIC        sal_Bool                Accept( ScChangeAction* );

            void                AcceptAll();    // all Virgins
            sal_Bool                RejectAll();    // all Virgins

                                // Selects a content of several contents at the same
                                // position and accepts this one and
                                // the older ones, rejects the more recent ones.
                                // If bOldest==TRUE then the first OldValue
                                // of a Virgin-Content-List will be restored.
            sal_Bool                SelectContent( ScChangeAction*,
                                    sal_Bool bOldest = false );

                                // If ModifiedLink is set, changes go to
                                // ScChangeTrackMsgQueue
            void                SetModifiedLink( const Link& r )
                                    { aModifiedLink = r; ClearMsgQueue(); }
            const Link&         GetModifiedLink() const { return aModifiedLink; }
            ScChangeTrackMsgQueue& GetMsgQueue() { return aMsgQueue; }

            void                NotifyModified( ScChangeTrackMsgType eMsgType,
                                    sal_uLong nStartAction, sal_uLong nEndAction );

            sal_uInt16              GetLoadedFileFormatVersion() const
                                    { return nLoadedFileFormatVersion; }

            sal_uLong               AddLoadedGenerated(ScBaseCell* pOldCell,
                                                const ScBigRange& aBigRange, const String& sNewValue ); // only to use in the XML import
            void                AppendLoaded( ScChangeAction* pAppend ); // this is only for the XML import public, it should be protected
            void                SetActionMax(sal_uLong nTempActionMax)
                                    { nActionMax = nTempActionMax; } // only to use in the XML import

            void                SetProtection( const com::sun::star::uno::Sequence< sal_Int8 >& rPass )
                                    { aProtectPass = rPass; }
    com::sun::star::uno::Sequence< sal_Int8 >   GetProtection() const
                                    { return aProtectPass; }
            sal_Bool                IsProtected() const
                                    { return aProtectPass.getLength() != 0; }

                                // If time stamps of actions of this
                                // ChangeTrack and a second one are to be
                                // compared including 100th seconds.
            void                SetTime100thSeconds( sal_Bool bVal )
                                    { bTime100thSeconds = bVal; }
            sal_Bool                IsTime100thSeconds() const
                                    { return bTime100thSeconds; }

            void                AppendCloned( ScChangeAction* pAppend );
    SC_DLLPUBLIC ScChangeTrack* Clone( ScDocument* pDocument ) const;
            void                MergeActionState( ScChangeAction* pAct, const ScChangeAction* pOtherAct );
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
