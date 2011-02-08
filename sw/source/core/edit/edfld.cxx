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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <unotools/charclass.hxx>
#include <editsh.hxx>
#include <fldbas.hxx>
#include <ndtxt.hxx>        // GetCurFld
#include <doc.hxx>
#include <docary.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <edimp.hxx>
#include <dbfld.hxx>
#include <expfld.hxx>
#include <flddat.hxx>
#include <swundo.hxx>
#include <dbmgr.hxx>
#include <swddetbl.hxx>
#include <hints.hxx>


/*--------------------------------------------------------------------
    Beschreibung: Feldtypen zu einer ResId zaehlen
                  wenn 0 alle zaehlen
 --------------------------------------------------------------------*/

USHORT SwEditShell::GetFldTypeCount(USHORT nResId, BOOL bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();

    if(nResId == USHRT_MAX)
    {
        if(!bUsed)
            return nSize;
        else
        {
            USHORT nUsed = 0;
            for ( USHORT i = 0; i < nSize; i++ )
            {
                if(IsUsed(*(*pFldTypes)[i]))
                    nUsed++;
            }
            return nUsed;
        }
    }

    // Alle Typen mit gleicher ResId
    USHORT nIdx  = 0;
    for(USHORT i = 0; i < nSize; ++i)
    {   // Gleiche ResId -> Index erhoehen
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        if(rFldType.Which() == nResId)
            nIdx++;
    }
    return nIdx;
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen zu einer ResId finden
                  wenn 0 alle finden
 --------------------------------------------------------------------*/
SwFieldType* SwEditShell::GetFldType(USHORT nFld, USHORT nResId, BOOL bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();

    if(nResId == USHRT_MAX && nFld < nSize)
    {
        if(!bUsed)
            return (*pFldTypes)[nFld];
        else
        {
            USHORT i, nUsed = 0;
            for ( i = 0; i < nSize; i++ )
            {
                if(IsUsed(*(*pFldTypes)[i]))
                {
                    if(nUsed == nFld)
                        break;
                    nUsed++;
                }
            }
            return i < nSize ? (*pFldTypes)[i] : 0;
        }
    }

    USHORT nIdx = 0;
    for(USHORT i = 0; i < nSize; ++i)
    {   // Gleiche ResId -> Index erhoehen
        SwFieldType* pFldType = (*pFldTypes)[i];
        if(pFldType->Which() == nResId)
        {
            if (!bUsed || IsUsed(*pFldType))
            {
                if(nIdx == nFld)
                    return pFldType;
                nIdx++;
            }
        }
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Den ersten Typen mit ResId und Namen finden
 --------------------------------------------------------------------*/
SwFieldType* SwEditShell::GetFldType(USHORT nResId, const String& rName) const
{
    return GetDoc()->GetFldType( nResId, rName, false );
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen loeschen
 --------------------------------------------------------------------*/
void SwEditShell::RemoveFldType(USHORT nFld, USHORT nResId)
{
    if( USHRT_MAX == nResId )
    {
        GetDoc()->RemoveFldType(nFld);
        return;
    }

    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();
    USHORT nIdx = 0;
    for( USHORT i = 0; i < nSize; ++i )
        // Gleiche ResId -> Index erhoehen
        if( (*pFldTypes)[i]->Which() == nResId &&
            nIdx++ == nFld )
        {
            GetDoc()->RemoveFldType( i );
            return;
        }
}

/*--------------------------------------------------------------------
    Beschreibung: FieldType ueber Name loeschen
 --------------------------------------------------------------------*/
void SwEditShell::RemoveFldType(USHORT nResId, const String& rStr)
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();
    const CharClass& rCC = GetAppCharClass();

    String aTmp( rCC.lower( rStr ));

    for(USHORT i = 0; i < nSize; ++i)
    {
        // Gleiche ResId -> Index erhoehen
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( pFldType->Which() == nResId )
        {
            if( aTmp.Equals( rCC.lower( pFldType->GetName() ) ))
            {
                GetDoc()->RemoveFldType(i);
                return;
            }
        }
    }
}


void SwEditShell::FieldToText( SwFieldType* pType )
{
    if( !pType->GetDepends() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_DELETE );
    Push();
    SwPaM* pPaM = GetCrsr();

    BOOL bDDEFld = RES_DDEFLD == pType->Which();
    // Modify-Object gefunden, trage alle Felder ins Array ein
    SwClientIter aIter( *pType );
    SwClient * pLast = aIter.GoStart();

    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            pPaM->DeleteMark();
            const SwFmtFld* pFmtFld = bDDEFld
                        ? PTR_CAST( SwFmtFld, pLast )
                        : (SwFmtFld*)pLast;

            if( pFmtFld )
            {
                if( !pFmtFld->GetTxtFld() )
                    continue;

                // kann keine DDETabelle sein
                const SwTxtNode& rTxtNode = pFmtFld->GetTxtFld()->GetTxtNode();
                pPaM->GetPoint()->nNode = rTxtNode;
                pPaM->GetPoint()->nContent.Assign( (SwTxtNode*)&rTxtNode,
                                     *pFmtFld->GetTxtFld()->GetStart() );

                // Feldinhalt durch Text ersetzen
                String const aEntry(
                    pFmtFld->GetFld()->ExpandField(GetDoc()->IsClipBoard()) );
                pPaM->SetMark();
                pPaM->Move( fnMoveForward );
                GetDoc()->DeleteRange( *pPaM );
                GetDoc()->InsertString( *pPaM, aEntry );
            }
            else if( bDDEFld )
            {
                // DDETabelle
                SwDepend* pDep = (SwDepend*)pLast;
                SwDDETable* pDDETbl = (SwDDETable*)pDep->GetToTell();
                pDDETbl->NoDDETable();
            }

        } while( 0 != ( pLast = aIter++ ));

    Pop( FALSE );
    EndAllAction();
    EndUndo( UNDO_DELETE );
}

/*************************************************************************
|*
|*                  SwEditShell::Insert( SwField )
|*
|*    Beschreibung  an der Cursorposition ein Feld einfuegen
|*    Quelle:       vgl. SwEditShell::Insert( String )
|*
*************************************************************************/
void SwEditShell::Insert2(SwField& rFld, const bool bForceExpandHints)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwFmtFld aFld( rFld );

    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ? nsSetAttrMode::SETATTR_FORCEHINTEXPAND
        : nsSetAttrMode::SETATTR_DEFAULT;

    FOREACHPAM_START(this)                      // fuer jeden PaM
        bool bSuccess(GetDoc()->InsertPoolItem(*PCURCRSR, aFld, nInsertFlags));
        OSL_ENSURE( bSuccess, "Doc->Insert(Field) failed");
        (void) bSuccess;
    FOREACHPAM_END()                      // fuer jeden PaM

    EndAllAction();
}

/*************************************************************************
|*
|*                  SwEditShell::GetCurFld()
|*
|*    Beschreibung  Stehen die PaMs auf Feldern ?
|*    Quelle:       edtfrm.cxx:
|*
*************************************************************************/

inline SwTxtFld *GetDocTxtFld( const SwPosition* pPos )
{
    SwTxtNode * const pNode = pPos->nNode.GetNode().GetTxtNode();
    return (pNode)
        ? static_cast<SwTxtFld*>( pNode->GetTxtAttrForCharAt(
                pPos->nContent.GetIndex(), RES_TXTATR_FIELD ))
        : 0;
}

SwField* SwEditShell::GetCurFld() const
{
    // Wenn es keine Selektionen gibt, gilt der Wert der aktuellen
    // Cursor-Position.

    SwPaM* pCrsr = GetCrsr();
    SwTxtFld *pTxtFld = GetDocTxtFld( pCrsr->Start() );
    SwField *pCurFld = NULL;

    /* Field was only recognized if no selection was
        present. Now it is recognized if either the cursor is in the
        field or the selection spans exactly over the field. */
    if( pTxtFld &&
        pCrsr->GetNext() == pCrsr &&
        pCrsr->Start()->nNode == pCrsr->End()->nNode &&
        (pCrsr->End()->nContent.GetIndex() -
         pCrsr->Start()->nContent.GetIndex()) <= 1)
    {
        pCurFld = (SwField*)pTxtFld->GetFld().GetFld();
        // TabellenFormel ? wandel internen in externen Namen um
        if( RES_TABLEFLD == pCurFld->GetTyp()->Which() )
        {
            const SwTableNode* pTblNd = IsCrsrInTbl();
            ((SwTblField*)pCurFld)->PtrToBoxNm( pTblNd ? &pTblNd->GetTable() : 0 );
        }

    }

    /* removed handling of multi-selections */

    return pCurFld;
}


/*************************************************************************
|*
|*                  SwEditShell::UpdateFlds()
|*
|*    Beschreibung  Stehen die PaMs auf Feldern ?
|*
*************************************************************************/
SwTxtFld* lcl_FindInputFld( SwDoc* pDoc, SwField& rFld )
{
    // suche das Feld ueber seine Addresse. Muss fuer InputFelder in
    // geschuetzten Feldern erfolgen
    SwTxtFld* pTFld = 0;
    if( RES_INPUTFLD == rFld.Which() || ( RES_SETEXPFLD == rFld.Which() &&
        ((SwSetExpField&)rFld).GetInputFlag() ) )
    {
        const SfxPoolItem* pItem;
        USHORT n, nMaxItems =
            pDoc->GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem =
                      pDoc->GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) )
                && ((SwFmtFld*)pItem)->GetFld() == &rFld )
            {
                pTFld = ((SwFmtFld*)pItem)->GetTxtFld();
                break;
            }
    }
    return pTFld;
}

void SwEditShell::UpdateFlds( SwField &rFld )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    {
        SwField *pCurFld = 0;

        // Wenn es keine Selektionen gibt, gilt der Wert der aktuellen
        // Cursor-Position.
        SwMsgPoolItem* pMsgHnt = 0;
        SwRefMarkFldUpdate aRefMkHt( GetOut() );
        USHORT nFldWhich = rFld.GetTyp()->Which();
        if( RES_GETREFFLD == nFldWhich )
            pMsgHnt = &aRefMkHt;

        SwPaM* pCrsr = GetCrsr();
        SwTxtFld *pTxtFld;
        SwFmtFld *pFmtFld;

        if ( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark())
        {
            pTxtFld = GetDocTxtFld(pCrsr->Start());

            if (!pTxtFld) // #i30221#
                pTxtFld = lcl_FindInputFld( GetDoc(), rFld);

            if (pTxtFld != 0)
                GetDoc()->UpdateFld(pTxtFld, rFld, pMsgHnt, TRUE);
        }

        // bOkay (statt return wg. EndAllAction) wird FALSE,
        // 1) wenn nur ein Pam mehr als ein Feld enthaelt oder
        // 2) bei gemischten Feldtypen
        BOOL bOkay = TRUE;
        BOOL bTblSelBreak = FALSE;

        SwMsgPoolItem aHint( RES_TXTATR_FIELD );  // Such-Hint
        FOREACHPAM_START(this)                      // fuer jeden PaM
            if( PCURCRSR->HasMark() && bOkay )      // ... mit Selektion
            {
                // Kopie des PaM
                SwPaM aCurPam( *PCURCRSR->GetMark(), *PCURCRSR->GetPoint() );
                SwPaM aPam( *PCURCRSR->GetPoint() );

                SwPosition *pCurStt = aCurPam.Start(), *pCurEnd =
                    aCurPam.End();
                /*
                 * Fuer den Fall, dass zwei aneinanderliegende Felder in einem
                 * PaM liegen, hangelt sich aPam portionsweise bis zum Ende.
                 * aCurPam wird dabei nach jeder Schleifenrunde verkuerzt.
                 * Wenn aCurPam vollstaendig durchsucht wurde, ist Start = End
                 * und die Schleife terminiert.
                 */

                // Suche nach SwTxtFld ...
                while(  bOkay
                     && pCurStt->nContent != pCurEnd->nContent
                     && aPam.Find( aHint, FALSE, fnMoveForward, &aCurPam ) )
                {
                    //  wenn nur ein Pam mehr als ein Feld enthaelt ...
                    if( aPam.Start()->nContent != pCurStt->nContent )
                        bOkay = FALSE;

                    if( 0 != (pTxtFld = GetDocTxtFld( pCurStt )) )
                    {
                        pFmtFld = (SwFmtFld*)&pTxtFld->GetFld();
                        pCurFld = pFmtFld->GetFld();

                        // bei gemischten Feldtypen
                        if( pCurFld->GetTyp()->Which() !=
                            rFld.GetTyp()->Which() )
                            bOkay = FALSE;

                        bTblSelBreak = GetDoc()->UpdateFld(pTxtFld, rFld,
                                                           pMsgHnt, FALSE);
                    }
                    // Der Suchbereich wird um den gefundenen Bereich
                    // verkuerzt.
                    pCurStt->nContent++;
                }
            }

            if( bTblSelBreak )      // wenn Tabellen Selektion und Tabellen-
                break;              // Formel aktualisiert wurde -> beenden

        FOREACHPAM_END()                      // fuer jeden PaM
    }
    GetDoc()->SetModified();
    EndAllAction();
}

/*--------------------------------------------------
 Liefert den logischen fuer die Datenbank zurueck
 --------------------------------------------------*/

SwDBData SwEditShell::GetDBData() const
{
    return GetDoc()->GetDBData();
}

const SwDBData& SwEditShell::GetDBDesc() const
{
    return GetDoc()->GetDBDesc();
}

void SwEditShell::ChgDBData(const SwDBData& rNewData)
{
    GetDoc()->ChgDBData(rNewData);
}

void SwEditShell::GetAllUsedDB( SvStringsDtor& rDBNameList,
                                SvStringsDtor* pAllDBNames )
{
    GetDoc()->GetAllUsedDB( rDBNameList, pAllDBNames );
}

void SwEditShell::ChangeDBFields( const SvStringsDtor& rOldNames,
                                    const String& rNewName )
{
    GetDoc()->ChangeDBFields( rOldNames, rNewName );
}

/*--------------------------------------------------------------------
    Beschreibung:  Alle Expression-Felder erneuern
 --------------------------------------------------------------------*/
void SwEditShell::UpdateExpFlds(BOOL bCloseDB)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UpdateExpFlds(NULL, true);
    if (bCloseDB)
        GetDoc()->GetNewDBMgr()->CloseAll();    // Alle Datenbankverbindungen dichtmachen
    EndAllAction();
}

SwNewDBMgr* SwEditShell::GetNewDBMgr() const
{
    return GetDoc()->GetNewDBMgr();
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen einfuegen
 --------------------------------------------------------------------*/
SwFieldType* SwEditShell::InsertFldType(const SwFieldType& rFldType)
{
    return GetDoc()->InsertFldType(rFldType);
}

void SwEditShell::LockExpFlds()
{
    GetDoc()->LockExpFlds();
}

void SwEditShell::UnlockExpFlds()
{
    GetDoc()->UnlockExpFlds();
}


void SwEditShell::SetFldUpdateFlags( SwFldUpdateFlags eFlags )
{
    getIDocumentSettingAccess()->setFieldUpdateFlags( eFlags );
}

SwFldUpdateFlags SwEditShell::GetFldUpdateFlags(BOOL bDocSettings) const
{
    return getIDocumentSettingAccess()->getFieldUpdateFlags( !bDocSettings );
}

void SwEditShell::SetFixFields( BOOL bOnlyTimeDate,
                                const DateTime* pNewDateTime )
{
    SET_CURR_SHELL( this );
    BOOL bUnLockView = !IsViewLocked();
    LockView( TRUE );
    StartAllAction();
    GetDoc()->SetFixFields( bOnlyTimeDate, pNewDateTime );
    EndAllAction();
    if( bUnLockView )
        LockView( FALSE );
}

void SwEditShell::SetLabelDoc( BOOL bFlag )
{
    GetDoc()->set(IDocumentSettingAccess::LABEL_DOCUMENT, bFlag );
}

BOOL SwEditShell::IsLabelDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::LABEL_DOCUMENT);
}

void SwEditShell::ChangeAuthorityData(const SwAuthEntry* pNewData)
{
    GetDoc()->ChangeAuthorityData(pNewData);
}

BOOL SwEditShell::IsAnyDatabaseFieldInDoc()const
{
    const SwFldTypes * pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();
    for(USHORT i = 0; i < nSize; ++i)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        USHORT nWhich = rFldType.Which();
        if(IsUsed(rFldType))
        {
            switch(nWhich)
            {
                case RES_DBFLD:
                case RES_DBNEXTSETFLD:
                case RES_DBNUMSETFLD:
                case RES_DBSETNUMBERFLD:
                {
                    SwClientIter aIter( rFldType );
                    SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
                    while(pFld)
                    {
                        if(pFld->IsFldInDoc())
                            return TRUE;
                        pFld = (SwFmtFld*)aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
