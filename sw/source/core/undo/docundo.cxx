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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <UndoManager.hxx>

#include <unotools/undoopt.hxx>

#include <vcl/wrkwin.hxx>

#include <svx/svdmodel.hxx>

#include <swmodule.hxx>
#include <doc.hxx>
#include <ndarr.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <undobj.hxx>
#include <rolbck.hxx>
#include <docary.hxx>
#include <undo.hrc>


using namespace ::com::sun::star;


// the undo array should never grow beyond this limit:
#define UNDO_ACTION_LIMIT (USHRT_MAX - 1000)


SV_IMPL_PTRARR( SwUndoIds, SwUndoIdAndNamePtr )

//#define _SHOW_UNDORANGE
#ifdef _SHOW_UNDORANGE


class UndoArrStatus : public WorkWindow
{
    USHORT nUndo, nUndoNds;
    virtual void Paint( const Rectangle& );
public:
    UndoArrStatus();
    void Set( USHORT, USHORT );
};
static UndoArrStatus* pUndoMsgWin = 0;


UndoArrStatus::UndoArrStatus()
    : WorkWindow( APP_GETAPPWINDOW() ), nUndo(0), nUndoNds(0)
{
    SetSizePixel( Size( 200, 100 ));
    SetFont( Font( "Courier", Size( 0, 10 )) );
    Show();
}


void UndoArrStatus::Set( USHORT n1, USHORT n2 )
{
    nUndo = n1; nUndoNds = n2;
    Invalidate();
}


void UndoArrStatus::Paint( const Rectangle& )
{
    String s;
    DrawRect( Rectangle( Point(0,0), GetOutputSize() ));
    ( s = "Undos: " ) += nUndo;
    DrawText( Point( 0, 0 ), s );
    ( s = "UndoNodes: " ) += nUndoNds;
    DrawText( Point( 0, 15 ), s );
}

#endif


// UndoManager ///////////////////////////////////////////////////////////

namespace sw {

UndoManager::UndoManager(::std::auto_ptr<SwNodes> pUndoNodes,
            IDocumentDrawModelAccess & rDrawModelAccess,
            IDocumentRedlineAccess & rRedlineAccess,
            IDocumentState & rState)
    :   m_rDrawModelAccess(rDrawModelAccess)
    ,   m_rRedlineAccess(rRedlineAccess)
    ,   m_rState(rState)
    ,   m_pUndoNodes(pUndoNodes)
    ,   m_pUndos( new SwUndos( 0, 20 ) )
    ,   m_nUndoPos(0)
    ,   m_nUndoSavePos(0)
    ,   m_nUndoActions(0)
    ,   m_nNestingDepth(0)
    ,   m_bUndo(false)
    ,   m_bGroupUndo(true)
    ,   m_bDrawUndo(true)
    ,   m_bLockUndoNoModifiedPosition(false)
{
    OSL_ASSERT(m_pUndoNodes.get());
}

SwNodes const& UndoManager::GetUndoNodes() const
{
    return *m_pUndoNodes;
}

SwNodes      & UndoManager::GetUndoNodes()
{
    return *m_pUndoNodes;
}

bool UndoManager::IsUndoNodes(SwNodes const& rNodes) const
{
    return & rNodes == m_pUndoNodes.get();
}

void UndoManager::DoUndo(bool const bDoUndo)
{
    m_bUndo = bDoUndo;

    SdrModel *const pSdrModel = m_rDrawModelAccess.GetDrawModel();
    if( pSdrModel )
    {
        pSdrModel->EnableUndo(bDoUndo);
    }
}

bool UndoManager::DoesUndo() const
{
    return m_bUndo;
}

void UndoManager::DoGroupUndo(bool const bDoUndo)
{
    m_bGroupUndo = bDoUndo;
}

bool UndoManager::DoesGroupUndo() const
{
    return m_bGroupUndo;
}

void UndoManager::DoDrawUndo(bool const bDoUndo)
{
    m_bDrawUndo = bDoUndo;
}

bool UndoManager::DoesDrawUndo() const
{
    return m_bDrawUndo;
}


bool UndoManager::IsUndoNoResetModified() const
{
    return USHRT_MAX == m_nUndoSavePos;
}

void UndoManager::SetUndoNoResetModified()
{
    m_nUndoSavePos = USHRT_MAX;
}

void UndoManager::SetUndoNoModifiedPosition()
{
    if (!m_bLockUndoNoModifiedPosition)
    {
        m_nUndoSavePos = (m_pUndos->Count())
            ?   m_nUndoPos
            :   USHRT_MAX;
    }
}

void UndoManager::LockUndoNoModifiedPosition()
{
    m_bLockUndoNoModifiedPosition = true;
}

void UndoManager::UnLockUndoNoModifiedPosition()
{
    m_bLockUndoNoModifiedPosition = false;
}


SwUndo* UndoManager::GetLastUndo()
{
    return (0 == m_nUndoPos)
        ?   0
        :   (*m_pUndos)[m_nUndoPos-1];
}

void UndoManager::AppendUndo(SwUndo *const pUndo)
{
    if( nsRedlineMode_t::REDLINE_NONE == pUndo->GetRedlineMode() )
    {
        pUndo->SetRedlineMode( m_rRedlineAccess.GetRedlineMode() );
    }

    // Unfortunately, the silly SvPtrArr can only store a little less than
    // USHRT_MAX elements. Of course it doesn't see any necessity for asserting
    // or even doing error handling. pUndos should definitely be replaced by an
    // STL container that doesn't have this problem. cf #95884#
    OSL_ENSURE( m_pUndos->Count() < USHRT_MAX - 16,
                "Writer will crash soon. I apologize for the inconvenience." );

    m_pUndos->Insert(pUndo, m_nUndoPos);
    ++m_nUndoPos;
    switch( pUndo->GetId() )
    {
        case UNDO_START:
            ++m_nNestingDepth;
        break;

        case UNDO_END:
            OSL_ENSURE(m_nNestingDepth, "AppendUndo(): Undo-End without Start");
            --m_nNestingDepth;
            // no break !

        default:
            if ((m_pUndos->Count() != m_nUndoPos) &&
                (UNDO_END != pUndo->GetId()))
            {
                ClearRedo();
            }
            OSL_ENSURE( m_pUndos->Count() == m_nUndoPos
                    ||  UNDO_END == pUndo->GetId(),
                        "AppendUndo(): Redo history not deleted!" );
            if (!m_nNestingDepth)
            {
                ++m_nUndoActions;
            }
        break;
    }

#ifdef _SHOW_UNDORANGE
    // zur Anzeige der aktuellen Undo-Groessen
    if( !pUndoMsgWin )
            pUndoMsgWin = new UndoArrStatus;
    pUndoMsgWin->Set( m_pUndos->Count(), GetUndoNodes()->Count() );
#endif

    // if the bracketing is still open, nothing more to do here
    if (m_nNestingDepth)
    {
        return;
    }

    // folgende Array-Grenzen muessen ueberwacht werden:
    //  - Undo,             Grenze: fester Wert oder USHRT_MAX - 1000
    //  - UndoNodes,        Grenze:  USHRT_MAX - 1000
    //  - AttrHistory       Grenze:  USHRT_MAX - 1000
    // (defined in UNDO_ACTION_LIMIT at the top of this file)

    USHORT nEnde = UNDO_ACTION_LIMIT;

#ifdef DBG_UTIL
{
    USHORT nUndosCnt = 0, nSttEndCnt = 0;
    for (USHORT nCnt = 0; nCnt < m_nUndoPos; ++nCnt)
    {
        SwUndoId const nId = (*m_pUndos)[ nCnt ]->GetId();
        if (UNDO_START == nId)
        {
            ++nSttEndCnt;
        }
        else if( UNDO_END == nId )
            --nSttEndCnt;
        if( !nSttEndCnt )
            ++nUndosCnt;
    }
    OSL_ENSURE(nSttEndCnt == m_nNestingDepth,
            "AppendUndo(): nesting depth is wrong");
    OSL_ENSURE(nUndosCnt == m_nUndoActions,
            "AppendUndo(): Undo action count is wrong");
}
#endif

    sal_Int32 const nActions(SW_MOD()->GetUndoOptions().GetUndoCount());
    if (nActions < m_nUndoActions)
    {
        // immer 1/10 loeschen
        //JP 23.09.95: oder wenn neu eingestellt wurde um die Differenz
        //JP 29.5.2001: Task #83891#: remove only the overlapping actions
        DelUndoObj( m_nUndoActions - nActions );
    }
    else
    {
        USHORT const nUndosCnt = m_nUndoActions;
            // immer 1/10 loeschen bis der "Ausloeser" behoben ist
        while (nEnde < GetUndoNodes().Count())
        {
            DelUndoObj( nUndosCnt / 10 );
        }
    }
}


void UndoManager::ClearRedo()
{
    if (m_nUndoPos != m_pUndos->Count())
    {
        // update m_nUndoActions
        for (USHORT nCnt = m_pUndos->Count(); m_nUndoPos < nCnt;
                --m_nUndoActions)
        {
            // skip Start/End bracketing
            SwUndo *const pUndo = (*m_pUndos)[ --nCnt ];
            if (UNDO_END == pUndo->GetId())
            {
                nCnt = nCnt - static_cast<SwUndoEnd*>(pUndo)->GetSttOffset();
            }
        }

        // delete Undo actions in reverse order!
        m_pUndos->DeleteAndDestroy(m_nUndoPos, m_pUndos->Count() - m_nUndoPos);
    }
}


void UndoManager::DelAllUndoObj()
{
    ::sw::UndoGuard const undoGuard(*this);

    ClearRedo();

    // retain open Start bracketing!
    USHORT nSize = m_pUndos->Count();
    while( nSize )
    {
        SwUndo *const pUndo = (*m_pUndos)[ --nSize ];
        if ((UNDO_START != pUndo->GetId()) ||
            // bracketing closed with End?
            static_cast<SwUndoStart*>(pUndo)->GetEndOffset())
        {
            m_pUndos->DeleteAndDestroy( nSize, 1 );
        }
    }

    m_nUndoActions = 0;
    m_nUndoPos = m_pUndos->Count();

    m_nUndoSavePos = USHRT_MAX;
}


bool UndoManager::DelUndoObj( sal_uInt16 nEnd )
{
    if (0 == nEnd)  // in case 0 is passed in
    {
        if (!m_pUndos->Count())
        {
            return false;
        }
        ++nEnd;     // correct it to 1 // FIXME  why ???
    }

    ::sw::UndoGuard const undoGuard(*this);

    // check where the end is
    USHORT nSttEndCnt = 0;
    USHORT nCnt;

    for (nCnt = 0; nEnd && nCnt < m_nUndoPos; ++nCnt)
    {
        SwUndoId const nId = (*m_pUndos)[ nCnt ]->GetId();
        if (UNDO_START == nId)
        {
            ++nSttEndCnt;
        }
        else if( UNDO_END == nId )
        {
            --nSttEndCnt;
        }
        if( !nSttEndCnt )
        {
            --nEnd, --m_nUndoActions;
        }
    }

    OSL_ENSURE( nCnt < m_nUndoPos || m_nUndoPos == m_pUndos->Count(),
            "DelUndoObj(): end inside of Redo actions!" );

    // update positions
    nSttEndCnt = nCnt;
    if (m_nUndoSavePos < nSttEndCnt) // abandon SavePos
    {
        m_nUndoSavePos = USHRT_MAX;
    }
    else if (m_nUndoSavePos != USHRT_MAX)
    {
        m_nUndoSavePos = m_nUndoSavePos - nSttEndCnt;
    }

    while( nSttEndCnt )
    {
        m_pUndos->DeleteAndDestroy( --nSttEndCnt, 1 );
    }
    m_nUndoPos = m_pUndos->Count();

    return true;
}

/**************** UNDO ******************/

bool UndoManager::HasUndoId(SwUndoId const eId) const
{
    USHORT nSize = m_nUndoPos;
    while( nSize-- )
    {
        SwUndo *const pUndo = (*m_pUndos)[nSize];
        if ((pUndo->GetId() == eId) ||
            ( UNDO_START == pUndo->GetId() &&
                (static_cast<SwUndoStart*>(pUndo)->GetUserId() == eId))
            || ( UNDO_END == pUndo->GetId() &&
                (static_cast<SwUndoEnd*>(pUndo)->GetUserId() == eId)))
        {
            return true;
        }
    }

    return false;
}


bool UndoManager::Undo( SwUndoIter& rUndoIter )
{
    if ( (rUndoIter.GetId()!=0) && (!HasUndoId(rUndoIter.GetId())) )
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }
    if (!m_nUndoPos)
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }

    SwUndo * pUndo = (*m_pUndos)[ --m_nUndoPos ];

    RedlineMode_t const eOld = m_rRedlineAccess.GetRedlineMode();
    RedlineMode_t eTmpMode = (RedlineMode_t)pUndo->GetRedlineMode();
    if( (nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
    {
        m_rRedlineAccess.SetRedlineMode( eTmpMode );
    }
    m_rRedlineAccess.SetRedlineMode_intern(
        static_cast<RedlineMode_t>(eTmpMode | nsRedlineMode_t::REDLINE_IGNORE));

    SwUndoId const nAktId = pUndo->GetId();
    //JP 11.05.98: FlyFormate ueber die EditShell selektieren, nicht aus dem
    //              Undo heraus
    switch( nAktId )
    {
    case UNDO_START:
    case UNDO_END:
    case UNDO_INSDRAWFMT:
        break;

    default:
        rUndoIter.ClearSelections();
    }

    pUndo->Undo( rUndoIter );

    m_rRedlineAccess.SetRedlineMode( eOld );

    // special treatment for Undo Replace: internal history
    if ((UNDO_REPLACE == nAktId) && static_cast<SwUndoReplace*>(pUndo)->nAktPos)
    {
        ++m_nUndoPos;
        return true;
    }

    if (m_nUndoPos && !rUndoIter.bWeiter)
    {
        pUndo = (*m_pUndos)[ m_nUndoPos-1 ];
        if (UNDO_START == pUndo->GetId())
        {
            --m_nUndoPos;
        }
    }

    // if we are at the "last save" position, the document is not modified
    if (m_nUndoSavePos == m_nUndoPos)
    {
        m_rState.ResetModified();
    }
    // JP 29.10.96: Start und End setzen kein Modify-Flag.
    //              Sonst gibt es Probleme mit der autom. Aufnahme von Ausnahmen
    //              bei der Autokorrektur
    else if ((UNDO_START != nAktId) && (UNDO_END != nAktId))
    {
        m_rState.SetModified();
    }

    return true;
}


SwUndoId
UndoManager::StartUndo(SwUndoId const i_eUndoId,
        SwRewriter const*const pRewriter)
{
    if (!m_bUndo)
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( (0 == i_eUndoId) ? UNDO_START : i_eUndoId );

    SwUndoStart * pUndo = new SwUndoStart( eUndoId );

    if (pRewriter)
        pUndo->SetRewriter(*pRewriter);

    AppendUndo(pUndo);

    return eUndoId;
}


SwUndoId
UndoManager::EndUndo(SwUndoId const i_eUndoId, SwRewriter const*const pRewriter)
{
    USHORT nSize = m_nUndoPos;
    if (!m_bUndo || !nSize--)
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( ((0 == i_eUndoId) || (UNDO_START == i_eUndoId))
            ? UNDO_END : i_eUndoId );

    SwUndo * pUndo = (*m_pUndos)[ nSize ];
    if( UNDO_START == pUndo->GetId() )
    {
        // empty Start/End bracketing?
        m_pUndos->DeleteAndDestroy( nSize );
        --m_nUndoPos;
        --m_nNestingDepth;
        return UNDO_EMPTY;
    }

    // exist above any redo objects? If yes, delete them
    if (m_nUndoPos != m_pUndos->Count())
    {
        // update UndoCnt
        for (USHORT nCnt = m_pUndos->Count(); m_nUndoPos < nCnt;
                --m_nUndoActions)
        {
            // skip bracketing
            pUndo = (*m_pUndos)[ --nCnt ];
            if (UNDO_END == pUndo->GetId())
            {
                nCnt -= static_cast<SwUndoEnd*>(pUndo)->GetSttOffset();
            }
        }

        m_pUndos->DeleteAndDestroy(m_nUndoPos, m_pUndos->Count() - m_nUndoPos);
    }

    // search for Start of this bracketing
    SwUndoStart * pUndoStart(0);
    while( nSize )
    {
        SwUndo *const pTmpUndo = (*m_pUndos)[ --nSize ];
        if ((UNDO_START == pTmpUndo->GetId()) &&
            !static_cast<SwUndoStart*>(pTmpUndo)->GetEndOffset())
        {
            pUndoStart = static_cast<SwUndoStart *>(pTmpUndo);
            break;      // found start
        }
    }

    if (!pUndoStart)
    {
        // kann eigentlich nur beim Abspielen von Macros passieren, die
        // Undo/Redo/Repeat benutzen und die eine exitierende Selection
        // durch Einfuegen loeschen
        OSL_ENSURE(false , "EndUndo(): corresponding UNDO_START not found");
        // not found => do not insert end and reset members

        m_nNestingDepth = 0;
        m_nUndoActions = 0;
        // update m_nUndoActions
        for (USHORT nCnt = 0; nCnt < m_pUndos->Count();
                ++nCnt, ++m_nUndoActions)
        {
            // skip bracketing
            SwUndo *const pTmpUndo = (*m_pUndos)[ nCnt ];
            if (UNDO_START == pTmpUndo->GetId())
            {
                nCnt += static_cast<SwUndoStart*>(pTmpUndo)->GetEndOffset();
            }
        }
        return UNDO_EMPTY;
    }

    // bracketing around single Undo action is unnecessary!
    // except if there is a custom user ID.
    if ((2 == m_pUndos->Count() - nSize) &&
        ((UNDO_END == eUndoId) || (eUndoId == (*m_pUndos)[ nSize+1 ]->GetId())))
    {
        m_pUndos->DeleteAndDestroy( nSize );
        m_nUndoPos = m_pUndos->Count();
        if (!--m_nNestingDepth)
        {
            ++m_nUndoActions;
            sal_Int32 const nActions(SW_MOD()->GetUndoOptions().GetUndoCount());
            if (nActions < m_nUndoActions)
            {
                // immer 1/10 loeschen
                //JP 23.09.95: oder wenn neu eingestellt wurde um die Differenz
                //JP 29.5.2001: Task #83891#: remove only the overlapping actions
                DelUndoObj( m_nUndoActions - nActions );
            }
            else
            {
                USHORT nEnde = USHRT_MAX - 1000;
                USHORT const nUndosCnt = m_nUndoActions;
                    // immer 1/10 loeschen bis der "Ausloeser" behoben ist
                while (nEnde < GetUndoNodes().Count())
                {
                    DelUndoObj( nUndosCnt / 10 );
                }
            }
        }
        return eUndoId;
    }

    // set offset for Start/End bracketing
    nSize = m_pUndos->Count() - nSize;
    pUndoStart->SetEndOffset( nSize );

    SwUndoEnd *const pUndoEnd = new SwUndoEnd( eUndoId );
    pUndoEnd->SetSttOffset( nSize );

// nur zum Testen der Start/End-Verpointerung vom Start/End Undo
#ifdef DBG_UTIL
    {
        USHORT nEndCnt = 1;
        USHORT nCnt = m_pUndos->Count();
        SwUndoId nTmpId = UNDO_EMPTY;
        while( nCnt )
        {
            nTmpId = (*m_pUndos)[ --nCnt ]->GetId();
            if (UNDO_START == nTmpId)
            {
                if( !nEndCnt ) // falls mal ein Start ohne Ende vorhanden ist
                    continue;
                --nEndCnt;
                if( !nEndCnt )      // hier ist der Anfang
                    break;
            }
            else if( UNDO_END == nTmpId )
                ++nEndCnt;
            else if( !nEndCnt )
                break;
        }
        OSL_ENSURE( nCnt == m_pUndos->Count() - nSize,
                "EndUndo(): Start-End bracketing wrong" );
    }
#endif

    if (pRewriter)
    {
        pUndoStart->SetRewriter(*pRewriter);
        pUndoEnd->SetRewriter(*pRewriter);
    }
    else
    {
        pUndoEnd->SetRewriter(pUndoStart->GetRewriter());
    }

    AppendUndo( pUndoEnd );
    return eUndoId;
}

/*-- 24.11.2004 16:11:21---------------------------------------------------

  -----------------------------------------------------------------------*/


/**
   Returns id and comment for a certain undo object in an undo stack.

   Remark: In the following the object type referred to is always the
   effective object type. If an UNDO_START or UNDO_END has a user type
   it is referred to as this type.

   If the queried object is an UNDO_END and has no user id the result
   is taken from the first object that is not an UNDO_END nor an
   UNDO_START preceeding the queried object.

   If the queried object is an UNDO_START and has no user id the
   result is taken from the first object that is not an UNDO_END nor
   an UNDO_START preceeding the UNDO_END object belonging to the
   queried object.

   In all other cases the result is taken from the queried object.

   @param rUndos           the undo stack
   @param nPos             position of the undo object to query

   @return SwUndoIdAndName object containing the query result
 */
SwUndoIdAndName * lcl_GetUndoIdAndName(const SwUndos & rUndos, sal_uInt16 nPos )
{
    SwUndo * pUndo = rUndos[ nPos ];
    SwUndoId nId = UNDO_EMPTY;
    String sStr("??", RTL_TEXTENCODING_ASCII_US);

    OSL_ENSURE( nPos < rUndos.Count(), "nPos out of range");

    switch (pUndo->GetId())
    {
    case UNDO_START:
        {
            SwUndoStart * pUndoStart = (SwUndoStart *) pUndo;
            nId = pUndoStart->GetUserId();

            if (nId <= UNDO_END)
            {
                /**
                   Start at the according UNDO_END.  Search backwards
                   for first objects that is not a UNDO_END.
                 */
                int nTmpPos = nPos + pUndoStart->GetEndOffset();
                int nSubstitute = -1;

                // --> OD 2009-09-30 #i105457#
                if ( nTmpPos > 0 )
                // <--
                {
                    SwUndo * pTmpUndo;
                    do
                    {
                        nTmpPos--;
                        pTmpUndo = rUndos[ static_cast<USHORT>(nTmpPos) ];

                        if (pTmpUndo->GetEffectiveId() > UNDO_END)
                            nSubstitute = nTmpPos;
                    }
                    while (nSubstitute < 0 && nTmpPos > nPos);

                    if (nSubstitute >= 0)
                    {
                        SwUndo * pSubUndo = rUndos[ static_cast<USHORT>(nSubstitute) ];
                        nId = pSubUndo->GetEffectiveId();
                        sStr = pSubUndo->GetComment();
                    }
                }
            }
            else
                sStr = pUndo->GetComment();
        }

        break;

    case UNDO_END:
        {
            SwUndoEnd * pUndoEnd = (SwUndoEnd *) pUndo;
            nId = pUndoEnd->GetUserId();

            if (nId <= UNDO_END)
            {
                /**
                   Start at this UNDO_END.  Search backwards
                   for first objects that is not a UNDO_END.
                 */

                int nTmpPos = nPos;
                int nUndoStart = nTmpPos - pUndoEnd->GetSttOffset();
                int nSubstitute = -1;

                if (nTmpPos > 0)
                {
                    SwUndo * pTmpUndo;

                    do
                    {
                        nTmpPos--;
                        pTmpUndo = rUndos[ static_cast<USHORT>(nTmpPos) ];

                        if (pTmpUndo->GetEffectiveId() > UNDO_END)
                            nSubstitute = nTmpPos;
                    }
                    while (nSubstitute < 0 && nTmpPos > nUndoStart);

                    if (nSubstitute >= 0)
                    {
                        SwUndo * pSubUndo = rUndos[ static_cast<USHORT>(nSubstitute) ];
                        nId = pSubUndo->GetEffectiveId();
                        sStr = pSubUndo->GetComment();
                    }
                }
            }
            else
                sStr = pUndo->GetComment();
        }

        break;

    default:
        nId = pUndo->GetId();
        sStr = pUndo->GetComment();
    }

    return new SwUndoIdAndName(nId, &sStr);
}

SwUndoId
UndoManager::GetUndoIds(String *const o_pStr, SwUndoIds *const o_pUndoIds) const
{
    int nTmpPos = m_nUndoPos - 1;
    SwUndoId nId = UNDO_EMPTY;

    while (nTmpPos >= 0)
    {
        SwUndo *const pUndo = (*m_pUndos)[ static_cast<USHORT>(nTmpPos) ];

        SwUndoIdAndName *const pIdAndName =
            lcl_GetUndoIdAndName( *m_pUndos, static_cast<sal_uInt16>(nTmpPos) );

        if (nTmpPos == m_nUndoPos - 1)
        {
            nId = pIdAndName->GetUndoId();

            if (o_pStr)
            {
                *o_pStr = *pIdAndName->GetUndoStr();
            }
        }

        if (o_pUndoIds)
        {
            o_pUndoIds->Insert(pIdAndName, o_pUndoIds->Count());
        }
        else
            break;

        if (pUndo->GetId() == UNDO_END)
        {
            nTmpPos -= static_cast<SwUndoEnd *>(pUndo)->GetSttOffset();
        }

        nTmpPos--;
    }

    return nId;
}

bool UndoManager::HasTooManyUndos() const
{
    // AppendUndo checks the UNDO_ACTION_LIMIT, unless there's a nested undo.
    // So HasTooManyUndos() may only occur when undos are nested; else
    // AppendUndo has some sort of bug.
    OSL_ENSURE(
        (m_nNestingDepth != 0) || (m_pUndos->Count() < UNDO_ACTION_LIMIT),
                "non-nested undos should have been handled in AppendUndo" );
    return (m_pUndos->Count() >= UNDO_ACTION_LIMIT);
}


/**************** REDO ******************/


bool UndoManager::Redo(SwUndoIter & rUndoIter)
{
    if( rUndoIter.GetId() && !HasUndoId( rUndoIter.GetId() ) )
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }
    if (m_nUndoPos == m_pUndos->Count())
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }

    SwUndo *const pUndo = (*m_pUndos)[ m_nUndoPos++ ];

    RedlineMode_t const eOld = m_rRedlineAccess.GetRedlineMode();
    RedlineMode_t eTmpMode = (RedlineMode_t)pUndo->GetRedlineMode();
    if( (nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
    {
        m_rRedlineAccess.SetRedlineMode(eTmpMode);
    }
    m_rRedlineAccess.SetRedlineMode_intern(
        static_cast<RedlineMode_t>(eTmpMode | nsRedlineMode_t::REDLINE_IGNORE));

    //JP 11.05.98: FlyFormate ueber die EditShell selektieren, nicht aus dem
    //              Undo heraus
    if( UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        rUndoIter.ClearSelections();

    pUndo->Redo( rUndoIter );

    m_rRedlineAccess.SetRedlineMode(eOld);

    // special treatment for Undo Replace: internal history
    if ((UNDO_REPLACE == pUndo->GetId()) &&
        (USHRT_MAX != static_cast<SwUndoReplace*>(pUndo)->nAktPos))
    {
        --m_nUndoPos;
        return true;
    }

    if (rUndoIter.bWeiter && (m_nUndoPos >= m_pUndos->Count()))
    {
        rUndoIter.bWeiter = FALSE;
    }

    // if we are at the "last save" position, the document is not modified
    if (m_nUndoSavePos == m_nUndoPos)
    {
        m_rState.ResetModified();
    }
    else
    {
        m_rState.SetModified();
    }
    return true;
}


SwUndoId
UndoManager::GetRedoIds(String *const o_pStr, SwUndoIds *const o_pRedoIds) const
{
    sal_uInt16 nTmpPos = m_nUndoPos;
    SwUndoId nId = UNDO_EMPTY;

    while (nTmpPos < m_pUndos->Count())
    {
        SwUndo *const pUndo = (*m_pUndos)[nTmpPos];

        SwUndoIdAndName *const pIdAndName =
            lcl_GetUndoIdAndName(*m_pUndos, nTmpPos);

        if (nTmpPos == m_nUndoPos)
        {
            nId = pIdAndName->GetUndoId();

            if (o_pStr)
            {
                *o_pStr = *pIdAndName->GetUndoStr();
            }
        }

        if (o_pRedoIds)
        {
            o_pRedoIds->Insert(pIdAndName, o_pRedoIds->Count());
        }
        else
            break;

        if (pUndo->GetId() == UNDO_START)
        {
            nTmpPos =
                nTmpPos + static_cast<SwUndoStart *>(pUndo)->GetEndOffset();
        }

        nTmpPos++;
    }

    return nId;
}

/**************** REPEAT ******************/


bool UndoManager::Repeat(SwUndoIter & rUndoIter, sal_uInt16 const nRepeatCnt)
{
    if( rUndoIter.GetId() && !HasUndoId( rUndoIter.GetId() ) )
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }
    USHORT nCurrentRepeat = m_nUndoPos;
    if( !nCurrentRepeat )
    {
        rUndoIter.bWeiter = FALSE;
        return false;
    }

    // look for current Repeat action, considering Start/End bracketing
    SwUndo *const pUndo = (*m_pUndos)[ --nCurrentRepeat ];
    if( UNDO_END == pUndo->GetId() )
    {
        nCurrentRepeat -= static_cast<SwUndoEnd*>(pUndo)->GetSttOffset();
    }

    USHORT const nEndCnt = m_nUndoPos;
    bool const bOneUndo = (nCurrentRepeat + 1 == m_nUndoPos);

    SwPaM* pTmpCrsr = rUndoIter.pAktPam;
    SwUndoId nId = UNDO_EMPTY;

    // need Start/End bracketing?
    if (pTmpCrsr != pTmpCrsr->GetNext() || !bOneUndo)
    {
        if (pUndo->GetId() == UNDO_END)
        {
            SwUndoStart *const pStartUndo =
                static_cast<SwUndoStart *>((*m_pUndos)[nCurrentRepeat]);
            nId = pStartUndo->GetUserId();
        }

        StartUndo( nId, NULL );
    }
    do {    // iterate over ring
        for( USHORT nRptCnt = nRepeatCnt; nRptCnt > 0; --nRptCnt )
        {
            rUndoIter.pLastUndoObj = 0;
            for (USHORT nCnt = nCurrentRepeat; nCnt < nEndCnt; ++nCnt)
            {
                (*m_pUndos)[ nCnt ]->Repeat( rUndoIter );
            }
        }
        rUndoIter.pAktPam = static_cast<SwPaM*>(rUndoIter.pAktPam->GetNext());
    } while (pTmpCrsr != rUndoIter.pAktPam);
    if( pTmpCrsr != pTmpCrsr->GetNext() || !bOneUndo )
    {
        EndUndo( nId, NULL );
    }

    return true;
}


SwUndoId
UndoManager::GetRepeatIds(String *const o_pStr) const
{
    SwUndoId const nRepeatId = GetUndoIds(o_pStr, 0);
    if( REPEAT_START <= nRepeatId && REPEAT_END > nRepeatId )
    {
        return nRepeatId;
    }
    if (o_pStr) // not repeatable -> clear comment
    {
        *o_pStr = String();
    }
    return UNDO_EMPTY;
}


SwUndo* UndoManager::RemoveLastUndo(SwUndoId const eUndoId)
{
    SwUndo *const pUndo = (*m_pUndos)[ m_nUndoPos - 1 ];
    if ((eUndoId != pUndo->GetId()) || (m_nUndoPos != m_pUndos->Count()))
    {
        OSL_ENSURE(false, "RemoveLastUndo(): wrong Undo action");
        return 0;
    }
    if (!m_nNestingDepth)
    {
        --m_nUndoActions;
    }
    --m_nUndoPos;
    m_pUndos->Remove( m_nUndoPos, 1 );
    return pUndo;
}

} // namespace sw

// SwUndoIdAndName ///////////////////////////////////////////////////////

SwUndoIdAndName::SwUndoIdAndName( SwUndoId nId, const String* pStr )
    : eUndoId( nId ), pUndoStr( pStr ? new String( *pStr ) : 0 )
{
}

SwUndoIdAndName::~SwUndoIdAndName()
{
    delete pUndoStr;
}



