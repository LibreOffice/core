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

#include "switerator.hxx"
#include "editsh.hxx"
#include "doc.hxx"
#include <docary.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include "edimp.hxx"
#include "expfld.hxx"
#include "pam.hxx"
#include "docfld.hxx"
#include "ndtxt.hxx"


/*--------------------------------------------------------------------
    Beschreibung: Sortieren der Input-Eintraege
 --------------------------------------------------------------------*/

SwInputFieldList::SwInputFieldList( SwEditShell* pShell, sal_Bool bBuildTmpLst )
    : pSh(pShell)
{
    // Hier die Liste aller Eingabefelder sortiert erstellen
    pSrtLst = new _SetGetExpFlds();

    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = rFldTypes.Count();

    // Alle Typen abklappern

    for(sal_uInt16 i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType || RES_DROPDOWN == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFld->GetTxtFld();

                //  nur InputFields und interaktive SetExpFlds bearbeiten
                //  and DropDown fields
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFld->GetFld())->GetInputFlag()))
                    continue;

                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    if( bBuildTmpLst )
                    {
                        VoidPtr pTmp = (VoidPtr)pTxtFld;
                        aTmpLst.Insert( pTmp, aTmpLst.Count() );
                    }
                    else
                    {
                        SwNodeIndex aIdx( rTxtNode );
                        _SetGetExpFld* pNew = new _SetGetExpFld(aIdx, pTxtFld );
                        pSrtLst->Insert( pNew );
                    }
                }
            }
        }
    }
}

SwInputFieldList::~SwInputFieldList()
{
    delete pSrtLst;
}

/*--------------------------------------------------------------------
    Beschreibung: Felder aus der Liste in sortierter Reihenfolge
 --------------------------------------------------------------------*/

sal_uInt16 SwInputFieldList::Count() const
{
    return pSrtLst->Count();
}


SwField* SwInputFieldList::GetField(sal_uInt16 nId)
{
    const SwTxtFld* pTxtFld = (*pSrtLst)[ nId ]->GetFld();
    ASSERT( pTxtFld, "kein TextFld" );
    return (SwField*)pTxtFld->GetFld().GetFld();
}

/*--------------------------------------------------------------------
    Beschreibung: Cursor sichern
 --------------------------------------------------------------------*/

void SwInputFieldList::PushCrsr()
{
    pSh->Push();
    pSh->ClearMark();
}

void SwInputFieldList::PopCrsr()
{
    pSh->Pop(sal_False);
}

/*--------------------------------------------------------------------
    Beschreibung: Position eines Feldes ansteuern
 --------------------------------------------------------------------*/

void SwInputFieldList::GotoFieldPos(sal_uInt16 nId)
{
    pSh->StartAllAction();
    (*pSrtLst)[ nId ]->GetPosOfContent( *pSh->GetCrsr()->GetPoint() );
    pSh->EndAllAction();
}

    // vergleiche TmpLst mit akt Feldern. Alle neue kommen in die SortLst
    // damit sie geupdatet werden koennen. Returnt die Anzahl.
    // (Fuer Textbausteine: nur seine Input-Felder aktualisieren)
sal_uInt16 SwInputFieldList::BuildSortLst()
{
    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    sal_uInt16 nSize = rFldTypes.Count();

    // Alle Typen abklappern

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFld->GetTxtFld();

                //  nur InputFields und interaktive SetExpFlds bearbeiten
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFld->GetFld())->GetInputFlag()))
                    continue;

                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    VoidPtr pTmp = (VoidPtr)pTxtFld;
                    // nicht in der TempListe vorhanden, also in die SortListe
                    // aufnehemen
                    sal_uInt16 nFndPos = aTmpLst.GetPos( pTmp );
                    if( USHRT_MAX == nFndPos )
                    {
                        SwNodeIndex aIdx( rTxtNode );
                        _SetGetExpFld* pNew = new _SetGetExpFld(aIdx, pTxtFld );
                        pSrtLst->Insert( pNew );
                    }
                    else
                        aTmpLst.Remove( nFndPos );
                }
            }
        }
    }

    // die Pointer werden nicht mehr gebraucht
    aTmpLst.Remove( 0, aTmpLst.Count() );
    return pSrtLst->Count();
}

/*--------------------------------------------------------------------
    Beschreibung: Alle Felder auáerhalb von Selektionen aus Liste entfernen
 --------------------------------------------------------------------*/

void SwInputFieldList::RemoveUnselectedFlds()
{
    _SetGetExpFlds* pNewLst = new _SetGetExpFlds();

    FOREACHPAM_START(pSh)
    {
        for (sal_uInt16 i = 0; i < Count();)
        {
            _SetGetExpFld* pFld = (*pSrtLst)[i];
            SwPosition aPos(*PCURCRSR->GetPoint());

            pFld->GetPos( aPos );

            if (aPos >= *PCURCRSR->Start() && aPos < *PCURCRSR->End())
            {
                // Feld innerhalb der Selektion
                pNewLst->Insert( (*pSrtLst)[i] );
                pSrtLst->Remove(i, 1);
            }
            else
                i++;
        }
    }
    FOREACHPAM_END()

    delete pSrtLst;
    pSrtLst = pNewLst;
}


