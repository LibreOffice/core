/*************************************************************************
 *
 *  $RCSfile: fldlst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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

#include "editsh.hxx"
#include "doc.hxx"

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#include "edimp.hxx"
#include "expfld.hxx"
#include "pam.hxx"
#include "docfld.hxx"
#include "ndtxt.hxx"


/*--------------------------------------------------------------------
    Beschreibung: Sortieren der Input-Eintraege
 --------------------------------------------------------------------*/

SwInputFieldList::SwInputFieldList( SwEditShell* pShell, FASTBOOL bBuildTmpLst )
    : pSh(pShell)
{
    // Hier die Liste aller Eingabefelder sortiert erstellen
    pSrtLst = new _SetGetExpFlds();

    SwNodes* pNds = &pSh->GetDoc()->GetNodes();
    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const USHORT nSize = rFldTypes.Count();

    // Alle Typen abklappern

    for(USHORT i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        USHORT nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwClientIter aIter( *pFldType );
            for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                    pFld; pFld = (SwFmtFld*)aIter.Next() )

            {
                const SwTxtFld* pTxtFld = pFld->GetTxtFld();

                //  nur InputFields und interaktive SetExpFlds bearbeiten
                //
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

USHORT SwInputFieldList::Count() const
{
    return pSrtLst->Count();
}


SwField* SwInputFieldList::GetField(USHORT nId)
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
    pSh->Pop(FALSE);
}

/*--------------------------------------------------------------------
    Beschreibung: Position eines Feldes ansteuern
 --------------------------------------------------------------------*/

void SwInputFieldList::GotoFieldPos(USHORT nId)
{
    pSh->StartAllAction();
    (*pSrtLst)[ nId ]->GetPosOfContent( *pSh->GetCrsr()->GetPoint() );
    pSh->EndAllAction();
}

    // vergleiche TmpLst mit akt Feldern. Alle neue kommen in die SortLst
    // damit sie geupdatet werden koennen. Returnt die Anzahl.
    // (Fuer Textbausteine: nur seine Input-Felder aktualisieren)
USHORT SwInputFieldList::BuildSortLst()
{
    SwNodes* pNds = &pSh->GetDoc()->GetNodes();
    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    USHORT nSize = rFldTypes.Count();

    // Alle Typen abklappern

    for( USHORT i = 0; i < nSize; ++i )
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        USHORT nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwClientIter aIter( *pFldType );
            for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                    pFld; pFld = (SwFmtFld*)aIter.Next() )
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
                    USHORT nFndPos = aTmpLst.GetPos( pTmp );
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
        for (USHORT i = 0; i < Count();)
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


