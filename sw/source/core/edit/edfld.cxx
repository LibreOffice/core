/*************************************************************************
 *
 *  $RCSfile: edfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // GetCurFld
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwRefMarkFldUpdate
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif


// wenn Selektion groesser Max Nodes oder mehr als Max Selektionen
// => keine Attribute
static const USHORT nMaxLookup = 40;

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
    return GetDoc()->GetFldType( nResId, rName );
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
                String aEntry( pFmtFld->GetFld()->Expand() );
                pPaM->SetMark();
                pPaM->Move( fnMoveForward );
                GetDoc()->Delete( *pPaM );
                GetDoc()->Insert( *pPaM, aEntry );
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
void SwEditShell::Insert(SwField& rFld)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwFmtFld aFld( rFld );

    FOREACHPAM_START(this)                      // fuer jeden PaM
        if( !GetDoc()->Insert( *PCURCRSR, aFld ) )
            ASSERT( FALSE, "Doc->Insert(Field) failed");
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
    SwTxtNode *pNode = pPos->nNode.GetNode().GetTxtNode();
    if( pNode )
        return pNode->GetTxtFld( pPos->nContent );
    else
        return 0;
}

SwField* SwEditShell::GetCurFld() const
{
    // Wenn es keine Selektionen gibt, gilt der Wert der aktuellen
    // Cursor-Position.
    SwPaM* pCrsr = GetCrsr();
    SwTxtFld *pTxtFld = GetDocTxtFld( pCrsr->Start() );
    SwField *pCurFld;
    if( pTxtFld && pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() )
    {
        pCurFld = (SwField*)pTxtFld->GetFld().GetFld();
        // TabellenFormel ? wandel internen in externen Namen um
        if( RES_TABLEFLD == pCurFld->GetTyp()->Which() )
        {
            const SwTableNode* pTblNd = IsCrsrInTbl();
            ((SwTblField*)pCurFld)->PtrToBoxNm( pTblNd ? &pTblNd->GetTable() : 0 );
        }
        return pCurFld;
    }
    pCurFld = 0;

    USHORT nCrsrCnt = 0;
    SwMsgPoolItem aHint( RES_TXTATR_FIELD );  // Such-Hint
    FOREACHPAM_START(this)                      // fuer jeden PaM
        if( nMaxLookup < ++nCrsrCnt  )
            break;

        if( PCURCRSR->HasMark() )               // ... mit Selektion
        {
            // Kopie des PaM
            SwPaM aCurPam( *PCURCRSR->GetMark(), *PCURCRSR->GetPoint() );
            SwPaM aPam( *PCURCRSR->GetPoint() );

            SwPosition *pCurStt = aCurPam.Start(), *pCurEnd = aCurPam.End();
            /*
             * Fuer den Fall, dass zwei aneinanderliegende Felder in einem
             * PaM liegen, hangelt sich aPam portionsweise bis zum Ende.
             * aCurPam wird dabei nach jeder Schleifenrunde verkuerzt.
             * Wenn aCurPam vollstaendig durchsucht wurde, ist Start = End
             * und die Schleife terminiert.
             */

            // Suche nach SwTxtFld ...
            while( pCurStt->nContent != pCurEnd->nContent
                   && aPam.Find( aHint, FALSE, fnMoveForward, &aCurPam,
                                 IsReadOnlyAvailable() ) )
            {
                // Wenn die Start's der beiden Bereiche nicht uebereinstimmen,
                // so liegt in dem selektierten Bereich etwas anderes als
                // ein/mehrere Felder: RETURN.

                const SwPosition* pStt;
                if( ( pStt = aPam.Start())->nContent != pCurStt->nContent )
                    return( 0 );

                SwTxtFld *pTxtFld = GetDocTxtFld( pStt );
                if( pTxtFld )
                {
                    // RETURN bei gemischten Feldtypen
                    if( pCurFld && pCurFld->GetTyp()->Which() !=
                        pTxtFld->GetFld().GetFld()->GetTyp()->Which() )
                        return( 0 );
                    pCurFld = (SwField*)pTxtFld->GetFld().GetFld();
                    // TabellenFormel ? wandel internen in externen Namen um
                    if( RES_TABLEFLD == pCurFld->GetTyp()->Which() )
                    {
                        const SwTableNode* pTblNd = GetDoc()->IsIdxInTbl( aPam.GetPoint()->nNode );
                        ((SwTblField*)pCurFld)->PtrToBoxNm( pTblNd
                                        ? &pTblNd->GetTable() : 0 );
                    }
                }

                // Der Suchbereich wird um den gefundenen Bereich verkuerzt.
                pCurStt->nContent++;
            }
        }
    FOREACHPAM_END()                      // fuer jeden PaM
    return pCurFld;
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
    // suche das Feld ueber seine Addresse. Muss fuer InputFelder in
    // geschuetzten Feldern erfolgen
    SwTxtFld* pTFld = 0;
    if( RES_INPUTFLD == rFld.Which() || ( RES_SETEXPFLD == rFld.Which() &&
        ((SwSetExpField&)rFld).GetInputFlag() ) )
    {
        const SfxPoolItem* pItem;
        USHORT n, nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) )
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
        if( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
            ( 0 != ( pTxtFld = GetDocTxtFld( pCrsr->Start() ) ) ||
              0 != ( pTxtFld = lcl_FindInputFld( GetDoc(), rFld ) ) ) &&
            ( pFmtFld = (SwFmtFld*)&pTxtFld->GetFld())->GetFld()
                ->GetTyp()->Which() == rFld.GetTyp()->Which() )
        {
            // Das gefundene Feld wird angepasst ...
            pFmtFld->GetFld()->ChangeFormat( rFld.GetFormat() );
            pFmtFld->GetFld()->SetLanguage( rFld.GetLanguage() );
            switch( nFldWhich )
            {
            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                GetDoc()->UpdateExpFlds( pTxtFld );
                break;

            case RES_TABLEFLD:
                {
                    const SwTableNode* pTblNd = GetDoc()->IsIdxInTbl(
                                            pCrsr->GetPoint()->nNode );
                    if( pTblNd )
                    {
                        SwTableFmlUpdate aTblUpdate( &pTblNd->GetTable() );
                        GetDoc()->UpdateTblFlds( &aTblUpdate );
                    }
                }
                break;

            case RES_MACROFLD:
                if( pTxtFld->GetpTxtNode() )
                    ((SwTxtNode*)pTxtFld->GetpTxtNode())->Modify( 0, pFmtFld );
                break;

            case RES_DBFLD:
                {
                    // JP 10.02.96: ChgValue aufrufen, damit die Format-
                    //              aenderung den ContentString richtig setzt
                    SwDBField* pDBFld = (SwDBField*)pFmtFld->GetFld();
                    if (pDBFld->IsInitialized())
                        pDBFld->ChgValue( pDBFld->GetValue(), TRUE );
                }
                // kein break;

            default:
                pFmtFld->Modify( 0, pMsgHnt );
            }

            // Die Felder die wir berechnen koennen werden hier expli.
            // zum Update angestossen.
            if( nFldWhich == RES_USERFLD )
                GetDoc()->UpdateUsrFlds();
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

                SwPosition *pCurStt = aCurPam.Start(), *pCurEnd = aCurPam.End();
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
                        if( pCurFld->GetTyp()->Which() != rFld.GetTyp()->Which() )
                            bOkay = FALSE;

                        // Das gefundene selektierte Feld wird angepasst ...
                        pCurFld->ChangeFormat( rFld.GetFormat() );
                        if( RES_SETEXPFLD == nFldWhich ||
                            RES_GETEXPFLD == nFldWhich ||
                            RES_HIDDENTXTFLD == nFldWhich )
                            GetDoc()->UpdateExpFlds( pTxtFld );
                        else if( RES_TABLEFLD == nFldWhich )
                        {
                            SwPaM* pPam = IsTableMode() ? GetTblCrs() : &aCurPam;
                            const SwTableNode* pTblNd = GetDoc()->IsIdxInTbl(
                                                    pPam->GetPoint()->nNode );
                            if( pTblNd )
                            {
                                SwTableFmlUpdate aTblUpdate( &pTblNd->GetTable() );
                                pCurFld->GetTyp()->Modify( 0, &aTblUpdate );
                            }
                            // bei Tabellen-SSelection ist hier Ende !!
                            if( IsTableMode() )
                            {
                                bTblSelBreak = TRUE;
                                break;
                            }
                        }
                        else
                            pFmtFld->Modify( 0, pMsgHnt );

                        // Die Felder die wir berechnen koennen werden hier
                        //  expli. zum Update angestossen.
                        if( nFldWhich == RES_USERFLD )
                            GetDoc()->UpdateUsrFlds();
                    }
                    // Der Suchbereich wird um den gefundenen Bereich verkuerzt.
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

String SwEditShell::GetDBName() const
{
    return GetDoc()->GetDBName();
}

const String& SwEditShell::GetDBDesc() const
{
    return GetDoc()->GetDBDesc();
}

void SwEditShell::ChgDBName(const String& rNewName)
{
    GetDoc()->ChgDBName(rNewName);
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

BOOL SwEditShell::RenameUserFields(const String& rOldName, const String& rNewName)
{
    return GetDoc()->RenameUserFields(rOldName, rNewName);
}


/*--------------------------------------------------------------------
    Beschreibung:  Alle Expression-Felder erneuern
 --------------------------------------------------------------------*/
void SwEditShell::UpdateExpFlds(BOOL bCloseDB)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UpdateExpFlds();
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

BOOL SwEditShell::IsExpFldsLocked() const
{
    return GetDoc()->IsExpFldsLocked();
}

void SwEditShell::SetFldUpdateFlags( USHORT eFlags )
{
    GetDoc()->SetFldUpdateFlags( eFlags );
}

USHORT SwEditShell::GetFldUpdateFlags(BOOL bDocSettings) const
{
    return bDocSettings ? GetDoc()->_GetFldUpdateFlags() : GetDoc()->GetFldUpdateFlags();
}

void SwEditShell::SetFixFields( BOOL bOnlyTimeDate,
                                const DateTime* pNewDateTime )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetFixFields( bOnlyTimeDate, pNewDateTime );
    EndAllAction();
}

void SwEditShell::SetLabelDoc( BOOL bFlag )
{
    GetDoc()->SetLabelDoc( bFlag );
}

BOOL SwEditShell::IsLabelDoc() const
{
    return GetDoc()->IsLabelDoc();
}
/* -----------------------------21.12.99 12:53--------------------------------

 ---------------------------------------------------------------------------*/
void SwEditShell::ChangeAuthorityData(const SwAuthEntry* pNewData)
{
    GetDoc()->ChangeAuthorityData(pNewData);
}

