/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <unotools/charclass.hxx>
#include <editsh.hxx>
#include <fldbas.hxx>
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
#include <switerator.hxx>
#include <fieldhint.hxx>

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen zu einer ResId zaehlen
                  wenn 0 alle zaehlen
 --------------------------------------------------------------------*/

sal_uInt16 SwEditShell::GetFldTypeCount(sal_uInt16 nResId, sal_Bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();

    if(nResId == USHRT_MAX)
    {
        if(!bUsed)
            return nSize;
        else
        {
            sal_uInt16 nUsed = 0;
            for ( sal_uInt16 i = 0; i < nSize; i++ )
            {
                if(IsUsed(*(*pFldTypes)[i]))
                    nUsed++;
            }
            return nUsed;
        }
    }

    // Alle Typen mit gleicher ResId
    sal_uInt16 nIdx  = 0;
    for(sal_uInt16 i = 0; i < nSize; ++i)
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
SwFieldType* SwEditShell::GetFldType(sal_uInt16 nFld, sal_uInt16 nResId, sal_Bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();

    if(nResId == USHRT_MAX && nFld < nSize)
    {
        if(!bUsed)
            return (*pFldTypes)[nFld];
        else
        {
            sal_uInt16 i, nUsed = 0;
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

    sal_uInt16 nIdx = 0;
    for(sal_uInt16 i = 0; i < nSize; ++i)
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
SwFieldType* SwEditShell::GetFldType(sal_uInt16 nResId, const String& rName) const
{
    return GetDoc()->GetFldType( nResId, rName, false );
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen loeschen
 --------------------------------------------------------------------*/
void SwEditShell::RemoveFldType(sal_uInt16 nFld, sal_uInt16 nResId)
{
    if( USHRT_MAX == nResId )
    {
        GetDoc()->RemoveFldType(nFld);
        return;
    }

    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();
    sal_uInt16 nIdx = 0;
    for( sal_uInt16 i = 0; i < nSize; ++i )
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
void SwEditShell::RemoveFldType(sal_uInt16 nResId, const String& rStr)
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();
    const CharClass& rCC = GetAppCharClass();

    String aTmp( rCC.lower( rStr ));

    for(sal_uInt16 i = 0; i < nSize; ++i)
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

    SwFieldHint aHint( pPaM );
    SwClientIter aIter( *pType );
    for ( SwClient* pClient = aIter.SwClientIter_First(); pClient; pClient = aIter.SwClientIter_Next() )
    {
        pPaM->DeleteMark();
        pClient->SwClientNotifyCall( *pType, aHint );
     }

    Pop( sal_False );
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

    FOREACHPAM_START(this)
        const bool bSuccess(GetDoc()->InsertPoolItem(*PCURCRSR, aFld, nInsertFlags));
        ASSERT( bSuccess, "Doc->Insert(Field) failed");
        (void) bSuccess;
    FOREACHPAM_END()

    EndAllAction();
}



/*************************************************************************
|*
|*                  SwEditShell::UpdateFlds()
|*
|*    Beschreibung  Stehen die PaMs auf Feldern ?
|*                  BP 12.05.92
|*
*************************************************************************/
SwTxtFld* lcl_FindInputFld( SwDoc* pDoc, SwField& rFld )
{
    SwTxtFld* pTFld = 0;
    if( RES_INPUTFLD == rFld.Which() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INPUTFIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_INPUTFIELD, n ) )
                && ((SwFmtFld*)pItem)->GetField() == &rFld )
            {
                pTFld = ((SwFmtFld*)pItem)->GetTxtFld();
                break;
            }
    }
    else if( RES_SETEXPFLD == rFld.Which()
        && ((SwSetExpField&)rFld).GetInputFlag() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) )
                && ((SwFmtFld*)pItem)->GetField() == &rFld )
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
        sal_uInt16 nFldWhich = rFld.GetTyp()->Which();
        if( RES_GETREFFLD == nFldWhich )
            pMsgHnt = &aRefMkHt;

        SwPaM* pCrsr = GetCrsr();
        SwTxtFld *pTxtFld;
        SwFmtFld *pFmtFld;

        if ( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark())
        {
            pTxtFld = GetTxtFldAtPos( pCrsr->Start(), true );

            if (!pTxtFld) // #i30221#
                pTxtFld = lcl_FindInputFld( GetDoc(), rFld);

            if (pTxtFld != 0)
                GetDoc()->UpdateFld(pTxtFld, rFld, pMsgHnt, sal_True); // #111840#
        }

        // bOkay (statt return wg. EndAllAction) wird sal_False,
        // 1) wenn nur ein Pam mehr als ein Feld enthaelt oder
        // 2) bei gemischten Feldtypen
        sal_Bool bOkay = sal_True;
        sal_Bool bTblSelBreak = sal_False;

        SwMsgPoolItem aFldHint( RES_TXTATR_FIELD );
        SwMsgPoolItem aInputFldHint( RES_TXTATR_INPUTFIELD );
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
                     && ( aPam.Find( aFldHint, sal_False, fnMoveForward, &aCurPam )
                          || aPam.Find( aInputFldHint, sal_False, fnMoveForward, &aCurPam ) ) )
                {
                    //  wenn nur ein Pam mehr als ein Feld enthaelt ...
                    if( aPam.Start()->nContent != pCurStt->nContent )
                        bOkay = sal_False;

                    if( 0 != (pTxtFld = GetTxtFldAtPos( pCurStt, true )) )
                    {
                        pFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();
                        pCurFld = pFmtFld->GetField();

                        // bei gemischten Feldtypen
                        if( pCurFld->GetTyp()->Which() !=
                            rFld.GetTyp()->Which() )
                            bOkay = sal_False;

                        bTblSelBreak = GetDoc()->UpdateFld(pTxtFld, rFld,
                                                           pMsgHnt, sal_False); // #111840#
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

/*-----------------13.05.92 10:54-------------------
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
void SwEditShell::UpdateExpFlds(sal_Bool bCloseDB)
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

SwFldUpdateFlags SwEditShell::GetFldUpdateFlags(sal_Bool bDocSettings) const
{
    return getIDocumentSettingAccess()->getFieldUpdateFlags( !bDocSettings );
}

void SwEditShell::SetFixFields( sal_Bool bOnlyTimeDate,
                                const DateTime* pNewDateTime )
{
    SET_CURR_SHELL( this );
    sal_Bool bUnLockView = !IsViewLocked();
    LockView( sal_True );
    StartAllAction();
    GetDoc()->SetFixFields( bOnlyTimeDate, pNewDateTime );
    EndAllAction();
    if( bUnLockView )
        LockView( sal_False );
}

void SwEditShell::SetLabelDoc( sal_Bool bFlag )
{
    GetDoc()->set(IDocumentSettingAccess::LABEL_DOCUMENT, bFlag );
}

sal_Bool SwEditShell::IsLabelDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::LABEL_DOCUMENT);
}
/* -----------------------------21.12.99 12:53--------------------------------

 ---------------------------------------------------------------------------*/
void SwEditShell::ChangeAuthorityData(const SwAuthEntry* pNewData)
{
    GetDoc()->ChangeAuthorityData(pNewData);
}
/* -----------------------------03.08.2001 12:04------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwEditShell::IsAnyDatabaseFieldInDoc()const
{
    const SwFldTypes * pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->Count();
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        sal_uInt16 nWhich = rFldType.Which();
        if(IsUsed(rFldType))
        {
            switch(nWhich)
            {
                case RES_DBFLD:
                case RES_DBNEXTSETFLD:
                case RES_DBNUMSETFLD:
                case RES_DBSETNUMBERFLD:
                {
                    SwIterator<SwFmtFld,SwFieldType> aIter( rFldType );
                    SwFmtFld* pFld = aIter.First();
                    while(pFld)
                    {
                        if(pFld->IsFldInDoc())
                            return sal_True;
                        pFld = aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return sal_False;
}
