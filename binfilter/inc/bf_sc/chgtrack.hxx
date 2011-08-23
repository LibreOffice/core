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


#include <tools/datetime.hxx>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include <tools/queue.hxx>
#include <tools/mempool.hxx>
#include <bf_svtools/lstner.hxx>

#include "bigrange.hxx"
#include "collect.hxx"

#ifdef SC_CHGTRACK_CXX
// core/inc
#endif
/*N*/#include <tools/debug.hxx>
/*N*/
class SvStream;
/*N*/class Stack;
/*N*/
namespace binfilter {

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

class ScMultipleReadHeader;
class ScMultipleWriteHeader;

// --- ScChangeActionLinkEntry ---------------------------------------------

// Fuegt sich selbst am Beginn einer Kette ein, bzw. vor einem anderen
// LinkEntry, on delete selbstaendiges ausklinken auch des gelinkten.
// ppPrev == &previous->pNext oder Adresse des Pointers auf Beginn der Kette,
// *ppPrev == this

class ScChangeAction;

class ScChangeActionLinkEntry
{
                                // not implemented, prevent usage
                                ScChangeActionLinkEntry(
                                    const ScChangeActionLinkEntry& );
/*N*/ 	ScChangeActionLinkEntry&	operator=( const ScChangeActionLinkEntry& );

protected:

    ScChangeActionLinkEntry*	pNext;
    ScChangeActionLinkEntry**	ppPrev;
    ScChangeAction*				pAction;
    ScChangeActionLinkEntry*	pLink;

public:

    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry )

                                ScChangeActionLinkEntry(
                                        ScChangeActionLinkEntry** ppPrevP,
                                        ScChangeAction* pActionP )
                                    :	ppPrev( ppPrevP ),
                                        pNext( *ppPrevP ),
                                        pAction( pActionP ),
                                        pLink( NULL )
                                    {
                                        if ( pNext )
                                            pNext->ppPrev = &pNext;
                                        *ppPrevP = this;
                                    }

    virtual						~ScChangeActionLinkEntry()
                                    {
                                        ScChangeActionLinkEntry* p = pLink;
                                        UnLink();
                                        Remove();
                                        if ( p )
                                            delete p;
                                    }

            void				SetLink( ScChangeActionLinkEntry* pLinkP )
                                    {
                                        UnLink();
                                        if ( pLinkP )
                                        {
                                            pLink = pLinkP;
                                            pLinkP->pLink = this;
                                        }
                                    }

            void				UnLink()
                                    {
                                        if ( pLink )
                                        {
                                            pLink->pLink = NULL;
                                            pLink = NULL;
                                        }
                                    }

            void				Remove()
                                    {
                                        if ( ppPrev )
                                        {
                                            if ( *ppPrev = pNext )
                                                pNext->ppPrev = ppPrev;
                                            ppPrev = NULL;	// not inserted
                                        }
                                    }

            void				Insert( ScChangeActionLinkEntry** ppPrevP )
                                    {
                                        if ( !ppPrev )
                                        {
                                            ppPrev = ppPrevP;
                                            if ( pNext = *ppPrevP )
                                                pNext->ppPrev = &pNext;
                                            *ppPrevP = this;
                                        }
                                    }

    const ScChangeActionLinkEntry*	GetLink() const		{ return pLink; }
    ScChangeActionLinkEntry*		GetLink()			{ return pLink; }
    const ScChangeActionLinkEntry*	GetNext() const		{ return pNext; }
    ScChangeActionLinkEntry*		GetNext()			{ return pNext; }
    const ScChangeAction*			GetAction() const	{ return pAction; }
    ScChangeAction*					GetAction()			{ return pAction; }
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

            ScChangeActionCellListEntry*	pNext;
            ScChangeActionContent*			pContent;

                                ScChangeActionCellListEntry(
                                    ScChangeActionContent* pContentP,
                                    ScChangeActionCellListEntry* pNextP )
                                    :	pContent( pContentP ),
                                        pNext( pNextP )
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
/*N*/ 
/*N*/ 								// not implemented, prevent usage
/*N*/ 								ScChangeAction( const ScChangeAction& );
            ScChangeAction&		operator=( const ScChangeAction& );
/*N*/ 
protected:
 
             ScBigRange	  		aBigRange;		 	// Ins/Del/MoveTo/ContentPos
             DateTime			aDateTime;			//! UTC
             String				aUser;				// wer war's
             String				aComment;			// Benutzerkommentar
             ScChangeAction*		pNext;				// naechster in Kette
             ScChangeAction*		pPrev;				// vorheriger in Kette
             ScChangeActionLinkEntry*	pLinkAny;	// irgendwelche Links
             ScChangeActionLinkEntry*	pLinkDeletedIn;	// Zuordnung zu
                                                     // geloeschten oder
                                                     // druebergemoveten oder
                                                     // rejecteten Insert
                                                     // Bereichen
             ScChangeActionLinkEntry*	pLinkDeleted;	// Links zu geloeschten
             ScChangeActionLinkEntry*	pLinkDependent;	// Links zu abhaengigen
             ULONG				nAction;
             ULONG				nRejectAction;
             ScChangeActionType	eType;
             ScChangeActionState	eState;
 
 
                            ScChangeAction( ScChangeActionType,
                                                const ScRange& );
 
                                 // only to be used in the XML import
                                ScChangeAction( ScChangeActionType,
                                                const ScBigRange&,
                                                const ULONG nAction,
                                                const ULONG nRejectAction,
                                                const ScChangeActionState eState,
                                                const DateTime& aDateTime,
                                                const String& aUser,
                                                const String& aComment );
/*N*/ 								// only to be used in the XML import
                                ScChangeAction( ScChangeActionType,
                                                const ScBigRange&,
                                                const ULONG nAction);

                                ScChangeAction( SvStream&,
                                    ScMultipleReadHeader&, ScChangeTrack* );
    virtual						~ScChangeAction();
/*N*/ 
/*N*/ 	static	void				StoreCell( ScBaseCell*, SvStream&,
/*N*/ 									ScMultipleWriteHeader& );
/*N*/ 	static ScBaseCell*			LoadCell( SvStream&, ScMultipleReadHeader&,
/*N*/ 									ScDocument*, USHORT nVer );
/*N*/ 
/*N*/ 	static	BOOL				StoreLinkChain( ScChangeActionLinkEntry*,
/*N*/ 									SvStream& );
/*N*/ 	static	BOOL				LoadLinkChain( ScChangeAction*,
/*N*/ 									ScChangeActionLinkEntry**,
/*N*/ 									SvStream&, ScChangeTrack*,
/*N*/ 									BOOL bLinkDeleted );
/*N*/ 
/*N*/ 	static	BOOL				StoreCellList( ScChangeActionCellListEntry*,
/*N*/ 									SvStream& );
/*N*/ 	static	BOOL				LoadCellList( ScChangeAction* pOfAction,
/*N*/ 									ScChangeActionCellListEntry*&, SvStream&,
/*N*/ 									ScChangeTrack* );
/*N*/ 
/*N*/ 			String				GetRefString( const ScBigRange&,
/*N*/ 									ScDocument*, BOOL bFlag3D = FALSE ) const;
/*N*/ 
/*N*/ 			void				SetActionNumber( ULONG n ) { nAction = n; }
/*N*/ 			void				SetRejectAction( ULONG n ) { nRejectAction = n; }
/*N*/ 			void				SetUser( const String& r ) { aUser = r; }
/*N*/ 			void				SetType( ScChangeActionType e ) { eType = e; }
/*N*/ 			void				SetState( ScChangeActionState e ) { eState = e; }
/*N*/ 			void				SetRejected();
/*N*/ 
            ScBigRange& 		GetBigRange() { return aBigRange; }
/*N*/ 
/*N*/ 			ScChangeActionLinkEntry*	AddLink( ScChangeAction* p,
/*N*/ 											ScChangeActionLinkEntry* pL )
/*N*/ 									{
/*N*/ 										ScChangeActionLinkEntry* pLnk =
/*N*/ 											new ScChangeActionLinkEntry(
/*N*/ 											&pLinkAny, p );
/*N*/ 										pLnk->SetLink( pL );
/*N*/ 										return pLnk;
/*N*/ 									}
/*N*/ 			void				RemoveAllAnyLinks();
/*N*/ 
     virtual	ScChangeActionLinkEntry*	GetDeletedIn() const
                                             { return pLinkDeletedIn; }
/*N*/ 	virtual	ScChangeActionLinkEntry**	GetDeletedInAddress()
/*N*/ 											{ return &pLinkDeletedIn; }
/*N*/ 			ScChangeActionLinkEntry*	AddDeletedIn( ScChangeAction* p )
/*N*/ 									{
/*N*/ 										return new ScChangeActionLinkEntry(
/*N*/ 											GetDeletedInAddress(), p );
/*N*/ 									}
/*N*/ 			BOOL				RemoveDeletedIn( const ScChangeAction* );
/*N*/ 			void				RemoveAllDeletedIn();
/*N*/ 			void				SetDeletedIn( ScChangeAction* );
/*N*/ 
/*N*/ 			ScChangeActionLinkEntry*	AddDeleted( ScChangeAction* p )
/*N*/ 									{
/*N*/ 										return new ScChangeActionLinkEntry(
/*N*/ 											&pLinkDeleted, p );
/*N*/ 									}
/*N*/ 			void				RemoveAllDeleted();
/*N*/ 
            ScChangeActionLinkEntry*	AddDependent( ScChangeAction* p )
                                    {
                                        return new ScChangeActionLinkEntry(
                                            &pLinkDependent, p );
                                    }
/*N*/ 			void				RemoveAllDependent();
/*N*/ 
            void				RemoveAllLinks();
/*N*/ 
/*N*/ 	virtual	void				AddContent( ScChangeActionContent* ) = 0;
/*N*/ 	virtual	void				DeleteCellEntries() = 0;
/*N*/ 
/*N*/ 	virtual	void 				UpdateReference( const ScChangeTrack*,
/*N*/ 									UpdateRefMode, const ScBigRange&,
/*N*/ 									INT32 nDx, INT32 nDy, INT32 nDz );
/*N*/ 
/*N*/ 			void				Accept();
/*N*/ 	virtual	BOOL				Reject( ScDocument* ) = 0;
/*N*/ 			void				RejectRestoreContents( ScChangeTrack*,
/*N*/ 									short nDx, short nDy );
/*N*/ 
/*N*/ 								// used in Reject() instead of IsRejectable()
/*N*/ 			BOOL				IsInternalRejectable() const;
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 	virtual	BOOL				StoreLinks( SvStream& ) const;
/*N*/ 	virtual	BOOL				LoadLinks( SvStream&, ScChangeTrack* );
/*N*/ 
public:
/*N*/ 
             BOOL				IsInsertType() const
                                     {
                                         return eType == SC_CAT_INSERT_COLS ||
                                             eType == SC_CAT_INSERT_ROWS ||
                                             eType == SC_CAT_INSERT_TABS;
                                     }
             BOOL				IsDeleteType() const
                                     {
                                         return eType == SC_CAT_DELETE_COLS ||
                                             eType == SC_CAT_DELETE_ROWS ||
                                             eType == SC_CAT_DELETE_TABS;
                                     }
/*N*/ 			BOOL				IsVirgin() const
/*N*/ 									{ return eState == SC_CAS_VIRGIN; }
             BOOL				IsAccepted() const
                                     { return eState == SC_CAS_ACCEPTED; }
             BOOL				IsRejected() const
                                     { return eState == SC_CAS_REJECTED; }
/*N*/ 
/*N*/ 								// Action rejects another Action
             BOOL				IsRejecting() const
                                     { return nRejectAction != 0; }
/*N*/ 
/*N*/ 								// ob Action anfassbar ist
/*N*/ 			BOOL				IsTouchable() const;
/*N*/ 

            const ScBigRange& 	GetBigRange() const { return aBigRange; }
             const DateTime&		GetDateTimeUTC() const		// UTC time
                                     { return aDateTime; }
            const String&		GetUser() const { return aUser; }
            const String&		GetComment() const { return aComment; }
            ScChangeActionType	GetType() const { return eType; }
/*N*/ 			ScChangeActionState	GetState() const { return eState; }
            ULONG				GetActionNumber() const { return nAction; }
            ULONG				GetRejectAction() const { return nRejectAction; }
/*N*/ 
            ScChangeAction*		GetNext() const { return pNext; }
/*N*/ 			ScChangeAction*		GetPrev() const { return pPrev; }
/*N*/ 
             BOOL				IsDeletedIn() const
                                     { return GetDeletedIn() != NULL; }
/*N*/ 			BOOL				IsDeleted() const
/*N*/ 									{ return IsDeleteType() || IsDeletedIn(); }
/*N*/ 			BOOL				IsDeletedIn( const ScChangeAction* ) const;
/*N*/ 			BOOL				IsDeletedInDelType( ScChangeActionType ) const;
/*N*/ 
             const ScChangeActionLinkEntry* GetFirstDeletedEntry() const
                                     { return pLinkDeleted; }
             const ScChangeActionLinkEntry* GetFirstDependentEntry() const
                                     { return pLinkDependent; }
             BOOL				HasDependent() const
                                     { return pLinkDependent != NULL; }
             BOOL				HasDeleted() const
                                     { return pLinkDeleted != NULL; }
/*N*/ 
/*N*/ 								// Description wird an String angehaengt.
/*N*/ 								// Mit bSplitRange wird bei Delete nur
/*N*/ 								// eine Spalte/Zeile beruecksichtigt (fuer
/*N*/ 								// Auflistung der einzelnen Eintraege).
/*N*/ 	virtual	void				GetDescription( String&, ScDocument*,
/*N*/ 									BOOL bSplitRange = FALSE ) const {}
/*N*/ 
/*N*/ 	virtual void				GetRefString( String&, ScDocument*,
/*N*/ 									BOOL bFlag3D = FALSE ) const;
/*N*/ 
/*N*/ 								// fuer DocumentMerge altes Datum einer anderen
/*N*/ 								// Action setzen, mit GetDateTimeUTC geholt
/*N*/ 			void				SetDateTimeUTC( const DateTime& rDT )
/*N*/ 									{ aDateTime = rDT; }
/*N*/ 
/*N*/ 								// Benutzerkommentar setzen
/*N*/ 			void				SetComment( const String& rStr )
/*N*/ 									{ aComment = rStr; }
/*N*/ 
/*N*/ 								// only to be used in the XML import
            void				SetDeletedInThis( ULONG nActionNumber,
                                        const ScChangeTrack* pTrack );
/*N*/ 								// only to be used in the XML import
            void				AddDependent( ULONG nActionNumber,
                                        const ScChangeTrack* pTrack );
};


// --- ScChangeActionIns ----------------------------------------------------

class ScChangeActionIns : public ScChangeAction
{
    friend class ScChangeTrack;

                                ScChangeActionIns( const ScRange& rRange );
                                ScChangeActionIns( SvStream&,
                                    ScMultipleReadHeader&, ScChangeTrack* );
    virtual						~ScChangeActionIns();
/*N*/ 
/*N*/ 	virtual	void				AddContent( ScChangeActionContent* ) {}
/*N*/ 	virtual	void				DeleteCellEntries() {}
/*N*/ 
/*N*/ 	virtual	BOOL				Reject( ScDocument* );
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 
public:
                                ScChangeActionIns(const ULONG nActionNumber,
                                        const ScChangeActionState eState,
                                        const ULONG nRejectingNumber,
                                        const ScBigRange& aBigRange,
                                        const String& aUser,
                                        const DateTime& aDateTime,
                                        const String &sComment,
                                        const ScChangeActionType eType); // only to use in the XML import

/*N*/ 	virtual	void				GetDescription( String&, ScDocument*,
/*N*/ 									BOOL bSplitRange = FALSE ) const;
};


// --- ScChangeActionDel ----------------------------------------------------

class ScChangeActionMove;

class ScChangeActionDelMoveEntry : public ScChangeActionLinkEntry
{
    friend class ScChangeActionDel;
    friend class ScChangeTrack;

            short		   		nCutOffFrom;
            short		   		nCutOffTo;


                                ScChangeActionDelMoveEntry(
                                    ScChangeActionDelMoveEntry** ppPrev,
                                    ScChangeActionMove* pMove,
                                    short nFrom, short nTo )
                                    :	ScChangeActionLinkEntry(
                                            (ScChangeActionLinkEntry**)
                                                ppPrev,
                                            (ScChangeAction*) pMove ),
                                        nCutOffFrom( nFrom ),
                                        nCutOffTo( nTo )
/*N*/									{};

/*N*/ 			ScChangeActionDelMoveEntry*	GetNext()
/*N*/ 									{
/*N*/ 										return (ScChangeActionDelMoveEntry*)
/*N*/ 										ScChangeActionLinkEntry::GetNext();
/*N*/ 									}
/*N*/ 			ScChangeActionMove*	GetMove()
/*N*/ 									{
/*N*/ 										return (ScChangeActionMove*)
/*N*/ 										ScChangeActionLinkEntry::GetAction();
/*N*/ 									}
/*N*/ 
public:
             const ScChangeActionDelMoveEntry*	GetNext() const
                                     {
                                         return (const ScChangeActionDelMoveEntry*)
                                         ScChangeActionLinkEntry::GetNext();
                                     }
/*N*/ 			const ScChangeActionMove*	GetMove() const
/*N*/ 									{
/*N*/ 										return (const ScChangeActionMove*)
/*N*/ 										ScChangeActionLinkEntry::GetAction();
/*N*/ 									}
            short				GetCutOffFrom() const { return nCutOffFrom; }
            short				GetCutOffTo() const { return nCutOffTo; }
};


class ScChangeActionDel : public ScChangeAction
{
    friend class ScChangeTrack;
/*N*/ 	friend class ScChangeAction;

            ScChangeTrack*		pTrack;
            ScChangeActionCellListEntry* pFirstCell;
            ScChangeActionIns*	pCutOff;		// abgeschnittener Insert
            short				nCutOff;		// +: Start  -: End
            ScChangeActionDelMoveEntry* pLinkMove;
            short				nDx;
            short				nDy;

                                ScChangeActionDel( const ScRange& rRange,
                                    short nDx, short nDy, ScChangeTrack* );
                                ScChangeActionDel( SvStream&,
                                    ScMultipleReadHeader&, ScDocument*,
                                    USHORT nVer, ScChangeTrack* );
/*N*/ 	virtual						~ScChangeActionDel();
/*N*/ 
            ScChangeActionIns*	GetCutOffInsert() { return pCutOff; }
/*N*/ 
/*N*/ 	virtual	void				AddContent( ScChangeActionContent* );
/*N*/ 	virtual	void				DeleteCellEntries();
/*N*/ 
/*N*/ 			void				UndoCutOffMoves();
/*N*/ 			void				UndoCutOffInsert();
/*N*/ 
/*N*/ 	virtual	void 				UpdateReference( const ScChangeTrack*,
/*N*/ 									UpdateRefMode, const ScBigRange&,
/*N*/ 									INT32 nDx, INT32 nDy, INT32 nDz );
/*N*/ 
/*N*/ 	virtual	BOOL				Reject( ScDocument* );
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 	virtual	BOOL				StoreLinks( SvStream& ) const;
/*N*/ 	virtual	BOOL				LoadLinks( SvStream&, ScChangeTrack* );
/*N*/ 
public:
                                ScChangeActionDel(const ULONG nActionNumber,
                                                const ScChangeActionState eState,
                                                const ULONG nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                const ScChangeActionType eType,
                                                const short nD,
                                                ScChangeTrack* pTrack); // only to use in the XML import
                                                                        // wich of nDx and nDy is set is depend on the type

                                // ob dieses das unterste einer Reihe (oder
                                // auch einzeln) ist
/*N*/ 			BOOL				IsBaseDelete() const;

                                // ob dieses das oberste einer Reihe (oder
                                // auch einzeln) ist
/*N*/ 			BOOL				IsTopDelete() const;

                                // ob dieses ein Teil einer Reihe ist
/*N*/			BOOL				IsMultiDelete() const;


            short				GetDx() const { return nDx; }
            short				GetDy() const { return nDy; }
/*N*/ 			ScBigRange			GetOverAllRange() const;	// BigRange + (nDx, nDy)

/*N*/ 			const ScChangeActionCellListEntry* GetFirstCellEntry() const
/*N*/ 									{ return pFirstCell; }
             const ScChangeActionDelMoveEntry* GetFirstMoveEntry() const
                                     { return pLinkMove; }
            const ScChangeActionIns*	GetCutOffInsert() const { return pCutOff; }
        short				GetCutOffCount() const { return nCutOff; }
/*N*/ 
/*N*/ 	virtual	void				GetDescription( String&, ScDocument*,
/*N*/ 									BOOL bSplitRange = FALSE ) const;
            void				SetCutOffInsert( ScChangeActionIns* p, short n )
                                    { pCutOff = p; nCutOff = n; }	// only to use in the XML import
/*N*/ 																	// this should be protected, but for the XML import it is public
/*N*/ 			// only to use in the XML import
/*N*/ 			// this should be protected, but for the XML import it is public
            ScChangeActionDelMoveEntry*	AddCutOffMove( ScChangeActionMove* pMove,
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
/*N*/ 	friend class ScChangeActionDel;

            ScBigRange			aFromRange;
            ScChangeTrack*		pTrack;
            ScChangeActionCellListEntry* pFirstCell;
            ULONG				nStartLastCut;	// fuer PasteCut Undo
            ULONG				nEndLastCut;

                                ScChangeActionMove( const ScRange& rFromRange,
                                    const ScRange& rToRange,
                                    ScChangeTrack* pTrackP )
                                    : ScChangeAction( SC_CAT_MOVE, rToRange ),
                                        aFromRange( rFromRange ),
                                        pTrack( pTrackP ),
                                        pFirstCell( NULL ),
                                        nStartLastCut(0),
                                        nEndLastCut(0)
/*N*/									{};
                                ScChangeActionMove( SvStream&,
                                    ScMultipleReadHeader&, ScChangeTrack* );
/*N*/ 	virtual						~ScChangeActionMove();

/*N*/ 	virtual	void				AddContent( ScChangeActionContent* );
/*N*/ 	virtual	void				DeleteCellEntries();
/*N*/ 
            ScBigRange&			GetFromRange() { return aFromRange; }
/*N*/ 
/*N*/ 			void				SetStartLastCut( ULONG nVal ) { nStartLastCut = nVal; }
/*N*/ 			ULONG				GetStartLastCut() const { return nStartLastCut; }
/*N*/ 			void				SetEndLastCut( ULONG nVal )	{ nEndLastCut = nVal; }
/*N*/ 			ULONG				GetEndLastCut() const { return nEndLastCut; }
/*N*/ 
/*N*/ 	virtual	void 				UpdateReference( const ScChangeTrack*,
/*N*/ 									UpdateRefMode, const ScBigRange&,
/*N*/ 									INT32 nDx, INT32 nDy, INT32 nDz );
/*N*/ 
/*N*/ 	virtual	BOOL				Reject( ScDocument* );
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 	virtual	BOOL				StoreLinks( SvStream& ) const;
/*N*/ 	virtual	BOOL				LoadLinks( SvStream&, ScChangeTrack* );
/*N*/ 
public:
                                ScChangeActionMove(const ULONG nActionNumber,
                                                const ScChangeActionState eState,
                                                const ULONG nRejectingNumber,
                                                const ScBigRange& aToBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                const ScBigRange& aFromBigRange,
                                                ScChangeTrack* pTrack); // only to use in the XML import
/*N*/ 			const ScChangeActionCellListEntry* GetFirstCellEntry() const
/*N*/ 									{ return pFirstCell; } // only to use in the XML export
/*N*/ 
            const ScBigRange&	GetFromRange() const { return aFromRange; }
/*N*/ 			void				GetDelta( INT32& nDx, INT32& nDy, INT32& nDz ) const;
/*N*/ 
/*N*/ 	virtual	void				GetDescription( String&, ScDocument*,
/*N*/ 									BOOL bSplitRange = FALSE ) const;
/*N*/ 
/*N*/ 	virtual void				GetRefString( String&, ScDocument*,
/*N*/ 									BOOL bFlag3D = FALSE ) const;
};


// --- ScChangeActionContent ------------------------------------------------

enum ScChangeActionContentCellType
{
    SC_CACCT_NONE = 0,
    SC_CACCT_NORMAL,
    SC_CACCT_MATORG,
    SC_CACCT_MATREF
};

class ScChangeActionContent : public ScChangeAction
{
    friend class ScChangeTrack;

            String				aOldValue;
            String				aNewValue;
            ScBaseCell*			pOldCell;
            ScBaseCell*			pNewCell;
        ScChangeActionContent*	pNextContent;	// an gleicher Position
        ScChangeActionContent*	pPrevContent;
        ScChangeActionContent*	pNextInSlot;	// in gleichem Slot
        ScChangeActionContent**	ppPrevInSlot;

                                ScChangeActionContent( SvStream&,
                                    ScMultipleReadHeader&, ScDocument*,
                                    USHORT nVer, ScChangeTrack* );

/*N*/ 			void				InsertInSlot( ScChangeActionContent** pp )
/*N*/ 									{
/*N*/ 										if ( !ppPrevInSlot )
/*N*/ 										{
/*N*/ 											ppPrevInSlot = pp;
/*N*/ 											if ( pNextInSlot = *pp )
/*N*/ 												pNextInSlot->ppPrevInSlot = &pNextInSlot;
/*N*/ 											*pp = this;
/*N*/ 										}
/*N*/ 									}
/*N*/ 			void				RemoveFromSlot()
/*N*/ 									{
/*N*/ 										if ( ppPrevInSlot )
/*N*/ 										{
/*N*/ 											if ( *ppPrevInSlot = pNextInSlot )
/*N*/ 												pNextInSlot->ppPrevInSlot = ppPrevInSlot;
/*N*/ 											ppPrevInSlot = NULL;	// not inserted
/*N*/ 										}
/*N*/ 									}
/*N*/ 		ScChangeActionContent*	GetNextInSlot() { return pNextInSlot; }
/*N*/ 
/*N*/ 			void				ClearTrack();
/*N*/ 
/*N*/ 	static	void				SetValue( String& rStr, ScBaseCell*& pCell,
/*N*/ 									const ScAddress& rPos,
/*N*/ 									const ScBaseCell* pOrgCell,
/*N*/ 									const ScDocument* pFromDoc,
/*N*/ 									ScDocument* pToDoc );
/*N*/ 
/*N*/ 	static	void				SetValue( String& rStr, ScBaseCell*& pCell,
/*N*/ 									ULONG nFormat,
/*N*/ 									const ScBaseCell* pOrgCell,
/*N*/ 									const ScDocument* pFromDoc,
/*N*/ 									ScDocument* pToDoc );
/*N*/ 
/*N*/ 	static	void				SetCell( String& rStr, ScBaseCell* pCell,
/*N*/ 									ULONG nFormat, const ScDocument* pDoc );
/*N*/ 
/*N*/ 	static	BOOL				NeedsNumberFormat( const ScBaseCell* );
/*N*/ 
/*N*/ 			void				GetValueString( String& rStr,
/*N*/ 									const String& rValue,
/*N*/ 									const ScBaseCell* pCell ) const;
/*N*/ 
/*N*/ 			void				GetFormulaString( String& rStr,
/*N*/ 									const ScFormulaCell* pCell ) const;
/*N*/ 
/*N*/ 	virtual	void				AddContent( ScChangeActionContent* ) {}
/*N*/ 	virtual	void				DeleteCellEntries() {}
/*N*/ 
/*N*/ 	virtual	void 				UpdateReference( const ScChangeTrack*,
/*N*/ 									UpdateRefMode, const ScBigRange&,
/*N*/ 									INT32 nDx, INT32 nDy, INT32 nDz );
/*N*/ 
/*N*/ 	virtual	BOOL				Reject( ScDocument* );
/*N*/ 
/*N*/ 			void				PutValueToDoc( ScBaseCell*, const String&,
/*N*/ 									ScDocument*, short nDx, short nDy ) const;
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 	virtual	BOOL				StoreLinks( SvStream& ) const;
/*N*/ 	virtual	BOOL				LoadLinks( SvStream&, ScChangeTrack* );
/*N*/ 
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
/*N*/									{};
                                ScChangeActionContent(const ULONG nActionNumber,
                                                const ScChangeActionState eState,
                                                const ULONG nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment,
                                                ScBaseCell* pOldCell,
                                                ScDocument* pDoc,
                                                const String& sResult); // to use for XML Import
                                ScChangeActionContent(const ULONG nActionNumber,
                                                ScBaseCell* pNewCell,
                                                const ScBigRange& aBigRange,
                                                ScDocument* pDoc); // to use for XML Import of Generated Actions
/*N*/ 	virtual						~ScChangeActionContent();
/*N*/ 
/*N*/ 		ScChangeActionContent*	GetNextContent() const { return pNextContent; }
        ScChangeActionContent*	GetPrevContent() const { return pPrevContent; }
/*N*/ 		ScChangeActionContent*	GetTopContent() const;
             BOOL				IsTopContent() const
                                     { return pNextContent == NULL; }
/*N*/ 
/*N*/ 	virtual	ScChangeActionLinkEntry*  	GetDeletedIn() const;
/*N*/ 	virtual	ScChangeActionLinkEntry**	GetDeletedInAddress();
/*N*/ 
/*N*/ 			void				PutOldValueToDoc( ScDocument*,
/*N*/ 									short nDx, short nDy ) const;
/*N*/ 			void				PutNewValueToDoc( ScDocument*,
/*N*/ 									short nDx, short nDy ) const;
/*N*/ 
/*N*/ 			void				SetOldValue( const ScBaseCell*,
/*N*/ 									const ScDocument* pFromDoc,
/*N*/ 									ScDocument* pToDoc );
/*N*/ 			void				SetNewValue( const ScBaseCell*,	ScDocument* );
/*N*/ 
/*N*/ 								// Use this only in the XML import,
/*N*/ 								// takes ownership of cell.
            void				SetNewCell( ScBaseCell* pNewCell, ScDocument* pDoc );
/*N*/ 
/*N*/ 								// These functions should be protected but for
/*N*/ 								// the XML import they are public.
            void				SetNextContent( ScChangeActionContent* p )
                                    { pNextContent = p; }
            void				SetPrevContent( ScChangeActionContent* p )
                                    { pPrevContent = p; }
/*N*/ 
/*N*/			void				GetOldString( String& ) const;
/*N*/			void				GetNewString( String& ) const;
            const ScBaseCell*	GetOldCell() const { return pOldCell; }
            const ScBaseCell*	GetNewCell() const { return pNewCell; }
/*N*/ 	virtual	void				GetDescription( String&, ScDocument*,
/*N*/ 									BOOL bSplitRange = FALSE ) const;
/*N*/ 	virtual void				GetRefString( String&, ScDocument*,
/*N*/ 									BOOL bFlag3D = FALSE ) const;
/*N*/ 
/*N*/ 	static	ScChangeActionContentCellType	GetContentCellType( const ScBaseCell* );
/*N*/ 
/*N*/ 								// NewCell
/*N*/ 			BOOL				IsMatrixOrigin() const
/*N*/ 									{
/*N*/ 										return GetContentCellType( GetNewCell() )
/*N*/ 											== SC_CACCT_MATORG;
/*N*/ 									}
/*N*/ 			BOOL				IsMatrixReference() const
/*N*/ 									{
/*N*/ 										return GetContentCellType( GetNewCell() )
/*N*/ 											== SC_CACCT_MATREF;
/*N*/ 									}
/*N*/ 								// OldCell
/*N*/ 			BOOL				IsOldMatrixOrigin() const
/*N*/ 									{
/*N*/ 										return GetContentCellType( GetOldCell() )
/*N*/ 											== SC_CACCT_MATORG;
/*N*/ 									}
/*N*/ 			BOOL				IsOldMatrixReference() const
/*N*/ 									{
/*N*/ 										return GetContentCellType( GetOldCell() )
/*N*/ 											== SC_CACCT_MATREF;
/*N*/ 									}
/*N*/ 
};


// --- ScChangeActionReject -------------------------------------------------

class ScChangeActionReject : public ScChangeAction
{
    friend class ScChangeTrack;
/*N*/ 	friend class ScChangeActionContent;

                                ScChangeActionReject( ULONG nReject )
                                    : ScChangeAction( SC_CAT_REJECT, ScRange() )
                                    {
/*N*/										SetRejectAction( nReject );
/*N*/ 										SetState( SC_CAS_ACCEPTED );
                                    }
                                ScChangeActionReject( SvStream&,
                                    ScMultipleReadHeader&, ScChangeTrack* );

/*N*/ 	virtual	void				AddContent( ScChangeActionContent* ) {}
/*N*/ 	virtual	void				DeleteCellEntries() {}
/*N*/ 
/*N*/ 	virtual	BOOL				Reject( ScDocument* p ) { return FALSE; }
/*N*/ 
/*N*/ 	virtual	BOOL				Store( SvStream&, ScMultipleWriteHeader& ) const;
/*N*/ 
public:
                                ScChangeActionReject(const ULONG nActionNumber,
                                                const ScChangeActionState eState,
                                                const ULONG nRejectingNumber,
                                                const ScBigRange& aBigRange,
                                                const String& aUser,
                                                const DateTime& aDateTime,
                                                const String &sComment); // only to use in the XML import
};


// --- ScChangeTrack --------------------------------------------------------

enum ScChangeTrackMsgType
{
    SC_CTM_NONE,
    SC_CTM_APPEND,		// Actions angehaengt
    SC_CTM_REMOVE,		// Actions weggenommen
    SC_CTM_CHANGE,		// Actions geaendert
    SC_CTM_PARENT		// war kein Parent und ist jetzt einer
};

struct ScChangeTrackMsgInfo
{
    DECL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo )

    ScChangeTrackMsgType	eMsgType;
    ULONG					nStartAction;
    ULONG					nEndAction;
};

// MsgQueue fuer Benachrichtigung via ModifiedLink
/*N*/ DECLARE_QUEUE( ScChangeTrackMsgQueue, ScChangeTrackMsgInfo* )
/*N*/ DECLARE_STACK( ScChangeTrackMsgStack, ScChangeTrackMsgInfo* )

enum ScChangeTrackMergeState
{
    SC_CTMS_NONE,
    SC_CTMS_PREPARE,
    SC_CTMS_OWN,
    SC_CTMS_OTHER
};

// zusaetzlich zu pFirst/pNext/pLast/pPrev eine Table, um schnell sowohl
// per ActionNumber als auch ueber Liste zugreifen zu koennen
/*N*/DECLARE_TABLE( ScChangeActionTable, ScChangeAction* )

// Intern generierte Actions beginnen bei diesem Wert (fast alle Bits gesetzt)
// und werden runtergezaehlt, um sich in einer Table wertemaessig nicht mit den
// "normalen" Actions in die Quere zu kommen.
#define SC_CHGTRACK_GENERATED_START	((UINT32) 0xfffffff0)

// SfxListener an der Applikation, um Aenderungen des Usernamens mitzubekommen

class ScChangeTrack : public SfxListener
{
/*N*/ 	friend void ScChangeAction::RejectRestoreContents( ScChangeTrack*, short, short );
/*N*/ 	friend BOOL ScChangeActionDel::Reject( ScDocument* pDoc );
/*N*/ 	friend void ScChangeActionDel::DeleteCellEntries();
/*N*/ 	friend void ScChangeActionMove::DeleteCellEntries();
/*N*/ 	friend BOOL ScChangeActionMove::Reject( ScDocument* pDoc );

    static	const USHORT		nContentRowsPerSlot;
    static	const USHORT		nContentSlots;

     ::com::sun::star::uno::Sequence< sal_Int8 >	aProtectPass;
             ScChangeActionTable	aTable;
             ScChangeActionTable	aGeneratedTable;
             ScChangeActionTable	aPasteCutTable;
/*N*/  		ScChangeTrackMsgQueue	aMsgQueue;
/*N*/  		ScChangeTrackMsgStack	aMsgStackTmp;
/*N*/  		ScChangeTrackMsgStack	aMsgStackFinal;
             StrCollection		aUserCollection;
             String				aUser;
             Link				aModifiedLink;
             ScRange				aInDeleteRange;
             DateTime			aFixDateTime;
             ScChangeAction*		pFirst;
             ScChangeAction*		pLast;
         ScChangeActionContent*	pFirstGeneratedDelContent;
         ScChangeActionContent**	ppContentSlots;
         ScChangeActionMove*		pLastCutMove;
     ScChangeActionLinkEntry*	pLinkInsertCol;
     ScChangeActionLinkEntry*	pLinkInsertRow;
     ScChangeActionLinkEntry*	pLinkInsertTab;
     ScChangeActionLinkEntry*	pLinkMove;
         ScChangeTrackMsgInfo*	pBlockModifyMsg;
             ScDocument*			pDoc;
             ULONG				nActionMax;
             ULONG				nGeneratedMin;
             ULONG				nMarkLastSaved;
             ULONG				nStartLastCut;
             ULONG				nEndLastCut;
             ULONG				nLastMerge;
         ScChangeTrackMergeState	eMergeState;
             USHORT				nLoadedFileFormatVersion;
             BOOL				bLoadSave;
             BOOL				bInDelete;
             BOOL				bInDeleteUndo;
             BOOL				bInDeleteTop;
             BOOL				bInPasteCut;
             BOOL				bUseFixDateTime;
             BOOL                bTime100thSeconds;
 
                                // not implemented, prevent usage
/*N*/								ScChangeTrack( const ScChangeTrack& );
            ScChangeTrack&		operator=( const ScChangeTrack& );

#ifdef SC_CHGTRACK_CXX
    static	USHORT				InitContentRowsPerSlot();
/*N*/
/*N*/ 	virtual	void				Notify( SfxBroadcaster&, const SfxHint& );
/*N*/ 			void				Init();
/*N*/ 			void				DtorClear();
/*N*/ 			void				SetLoadSave( BOOL bVal ) { bLoadSave = bVal; }
/*N*/ 			void				SetInDeleteRange( const ScRange& rRange )
/*N*/ 									{ aInDeleteRange = rRange; }
/*N*/ 			void				SetInDelete( BOOL bVal )
/*N*/ 									{ bInDelete = bVal; }
/*N*/ 			void				SetInDeleteTop( BOOL bVal )
/*N*/ 									{ bInDeleteTop = bVal; }
/*N*/ 			void				SetInDeleteUndo( BOOL bVal )
/*N*/ 									{ bInDeleteUndo = bVal; }
/*N*/ 			void				SetInPasteCut( BOOL bVal )
/*N*/ 									{ bInPasteCut = bVal; }
/*N*/ 			void				SetMergeState( ScChangeTrackMergeState eState )
/*N*/ 									{ eMergeState = eState; }
/*N*/ 		ScChangeTrackMergeState	GetMergeState() const { return eMergeState; }
/*N*/ 			void				SetLastMerge( ULONG nVal ) { nLastMerge = nVal; }
/*N*/ 			ULONG				GetLastMerge() const { return nLastMerge; }
/*N*/ 
/*N*/ 								// ModifyMsg blockweise und nicht einzeln erzeugen
/*N*/ 			void				StartBlockModify( ScChangeTrackMsgType,
/*N*/ 									ULONG nStartAction );
/*N*/ 			void				EndBlockModify( ULONG nEndAction );
/*N*/ 
/*N*/ 			void				AddDependentWithNotify( ScChangeAction* pParent,
/*N*/ 									ScChangeAction* pDependent );
/*N*/ 
/*N*/ 			void				Dependencies( ScChangeAction* );
/*N*/ 			void				UpdateReference( ScChangeAction*, BOOL bUndo );
/*N*/ 			void				UpdateReference( ScChangeAction** ppFirstAction,
/*N*/ 									ScChangeAction* pAct, BOOL bUndo );
/*N*/ 			void				Append( ScChangeAction* pAppend, ULONG nAction );
/*N*/ 			void				AppendDeleteRange( const ScRange&,
/*N*/ 									ScDocument* pRefDoc, short nDz,
/*N*/ 									ULONG nRejectingInsert );
/*N*/ 			void				AppendOneDeleteRange( const ScRange& rOrgRange,
/*N*/ 									ScDocument* pRefDoc,
/*N*/ 									short nDx, short nDy, short nDz,
/*N*/ 									ULONG nRejectingInsert );
/*N*/ 			void				LookUpContents( const ScRange& rOrgRange,
/*N*/ 									ScDocument* pRefDoc,
/*N*/ 									short nDx, short nDy, short nDz );
/*N*/ 			void				Remove( ScChangeAction* );
/*N*/ 			void				MasterLinks( ScChangeAction* );
/*N*/                                                          // Content on top an Position
/*N*/          ScChangeActionContent*  SearchContentAt( const ScBigAddress&,
/*N*/                                                                  ScChangeAction* pButNotThis ) const;
/*N*/ 
/*N*/ 								// das gleiche fuer generierte Del-Eintraege,
/*N*/ 								// wobei der Content nicht in der angegebenen
/*N*/ 								// Richtung geloescht sein darf
/*N*/ 			void				DeleteGeneratedDelContent(
/*N*/ 									ScChangeActionContent* );
/*N*/ 		ScChangeActionContent*	GenerateDelContent( const ScAddress&,
/*N*/ 									const ScBaseCell*,
/*N*/ 									const ScDocument* pFromDoc );
/*N*/ 			void				DeleteCellEntries(
/*N*/ 									ScChangeActionCellListEntry*&,
/*N*/ 									ScChangeAction* pDeletor );
/*N*/ 
/*N*/ 								// Action und alle abhaengigen rejecten,
/*N*/ 								// Table stammt aus vorherigem GetDependents,
/*N*/ 								// ist nur bei Insert und Move (MasterType)
/*N*/ 								// noetig, kann ansonsten NULL sein.
/*N*/ 								// bRecursion == Aufruf aus Reject mit Table
/*N*/ 			BOOL				Reject( ScChangeAction*,
/*N*/ 									ScChangeActionTable*, BOOL bRecursion );
/*N*/ 
#endif	// SC_CHGTRACK_CXX
/*N*/ 
/*N*/ 			void				ClearMsgQueue();
/*N*/ 
public:

/*N*/ 	static	USHORT				ComputeContentSlot( INT32 nRow )
/*N*/ 									{
/*N*/ 										if ( nRow < 0 || nRow > MAXROW )
/*N*/ 											return nContentSlots - 1;
/*N*/ 										return nRow / nContentRowsPerSlot;
/*N*/ 									}

/*N*/								ScChangeTrack( ScDocument* );
                                ScChangeTrack( ScDocument*,
/*N*/											const StrCollection& ); // only to use in the XML import
/*N*/ 	virtual						~ScChangeTrack();
/*N*/ 			void				Clear();
/*N*/ 
            ScChangeActionContent*	GetFirstGenerated() const { return pFirstGeneratedDelContent; }
            ScChangeAction*		GetFirst() const { return pFirst; }
            ScChangeAction*		GetLast() const	{ return pLast; }
            ULONG				GetActionMax() const { return nActionMax; }
             BOOL				IsGenerated( ULONG nAction ) const
/*N*/ 									{ return nAction >= nGeneratedMin;}
            ScChangeAction*		GetAction( ULONG nAction ) const
                                    { return aTable.Get( nAction ); }
/*N*/ 			ScChangeAction*		GetGenerated( ULONG nGenerated ) const
/*N*/ 									{ return aGeneratedTable.Get( nGenerated ); }
/*N*/ 			ScChangeAction*		GetActionOrGenerated( ULONG nAction ) const
/*N*/ 									{
/*N*/ 										return IsGenerated( nAction ) ?
/*N*/ 											GetGenerated( nAction ) :
/*N*/ 											GetAction( nAction );
/*N*/ 									}
/*N*/ 			ULONG				GetLastSavedActionNumber() const
/*N*/ 									{ return nMarkLastSaved; }
            void                SetLastSavedActionNumber(ULONG nNew)
                                    { nMarkLastSaved = nNew; }
/*N*/ 			ScChangeAction*		GetLastSaved() const
/*N*/ 									{ return aTable.Get( nMarkLastSaved ); }
/*N*/ 		ScChangeActionContent**	GetContentSlots() const { return ppContentSlots; }
/*N*/ 
/*N*/ 			BOOL				IsLoadSave() const { return bLoadSave; }
/*N*/ 			const ScRange&		GetInDeleteRange() const
/*N*/ 									{ return aInDeleteRange; }
/*N*/ 			BOOL				IsInDelete() const { return bInDelete; }
/*N*/ 			BOOL				IsInDeleteTop() const { return bInDeleteTop; }
            BOOL				IsInDeleteUndo() const { return bInDeleteUndo; }
/*N*/ 			BOOL				IsInPasteCut() const { return bInPasteCut; }
/*N*/ 			void				SetUser( const String& );
/*N*/ 			const String&		GetUser() const { return aUser; }
            const StrCollection&	GetUserCollection() const
                                    { return aUserCollection; }
/*N*/ 			ScDocument*			GetDocument() const { return pDoc; }
/*N*/ 								// for import filter
/*N*/ 			const DateTime&		GetFixDateTime() const { return aFixDateTime; }
/*N*/ 
/*N*/ 								// set this if the date/time set with
/*N*/ 								// SetFixDateTime...() shall be applied to
/*N*/ 								// appended actions
/*N*/ 			void				SetUseFixDateTime( BOOL bVal )
/*N*/ 									{ bUseFixDateTime = bVal; }
/*N*/ 								// for MergeDocument, apply original date/time as UTC
/*N*/ 			void				SetFixDateTimeUTC( const DateTime& rDT )
/*N*/ 									{ aFixDateTime = rDT; }
/*N*/ 								// for import filter, apply original date/time as local time
/*N*/ 			void				SetFixDateTimeLocal( const DateTime& rDT )
/*N*/ 									{ aFixDateTime = rDT; aFixDateTime.ConvertToUTC(); }
/*N*/ 
/*N*/ 			void				Append( ScChangeAction* );
/*N*/ 
/*N*/ 								// Cut to Clipboard
/*N*/ 			void				ResetLastCut()
/*N*/ 									{
/*N*/ 										nStartLastCut = nEndLastCut = 0;
/*N*/ 										if ( pLastCutMove )
/*N*/ 										{
/*N*/ 											delete pLastCutMove;
/*N*/ 											pLastCutMove = NULL;
/*N*/ 										}
/*N*/ 									}
/*N*/ 			BOOL				HasLastCut() const
/*N*/ 									{
/*N*/ 										return nEndLastCut > 0 &&
/*N*/ 											nStartLastCut <= nEndLastCut &&
/*N*/ 											pLastCutMove;
/*N*/ 									}
/*N*/ 
/*N*/ 
/*N*/ 								// wenn ModifiedLink gesetzt, landen
/*N*/ 								// Aenderungen in ScChangeTrackMsgQueue
/*N*/ 			void				SetModifiedLink( const Link& r )
/*N*/ 									{ aModifiedLink = r; ClearMsgQueue(); }
/*N*/ 			const Link&			GetModifiedLink() const { return aModifiedLink; }
/*N*/ 			ScChangeTrackMsgQueue& GetMsgQueue() { return aMsgQueue; }
/*N*/ 
/*N*/ 			void				NotifyModified( ScChangeTrackMsgType eMsgType,
/*N*/ 									ULONG nStartAction, ULONG nEndAction );
/*N*/ 
/*N*/ 			BOOL				Load( SvStream& rStrm, USHORT nVer );
/*N*/ 			BOOL				Store( SvStream& rStrm );
/*N*/ 			USHORT				GetLoadedFileFormatVersion() const
/*N*/ 									{ return nLoadedFileFormatVersion; }
/*N*/ 
            ULONG				AddLoadedGenerated(ScBaseCell* pOldCell,
                                                const ScBigRange& aBigRange ); // only to use in the XML import
             void				AppendLoaded( ScChangeAction* pAppend ); // this is only for the XML import public, it should be protected
             void				SetActionMax(ULONG nTempActionMax)
                                    { nActionMax = nTempActionMax; } // only to use in the XML import
/*N*/ 
            void                SetProtection( const ::com::sun::star::uno::Sequence< sal_Int8 >& rPass )
                                    { aProtectPass = rPass; }
     ::com::sun::star::uno::Sequence< sal_Int8 >   GetProtection() const
                                     { return aProtectPass; }
             BOOL                IsProtected() const
                                     { return aProtectPass.getLength() != 0; }
/*N*/ 
/*N*/                                 // If time stamps of actions of this
/*N*/                                 // ChangeTrack and a second one are to be
/*N*/                                 // compared including 100th seconds.
            void                SetTime100thSeconds( BOOL bVal )
                                    { bTime100thSeconds = bVal; }
/*N*/             BOOL                IsTime100thSeconds() const
/*N*/                                     { return bTime100thSeconds; }
};


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
