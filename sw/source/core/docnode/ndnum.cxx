/*************************************************************************
 *
 *  $RCSfile: ndnum.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>           // UpdateFlds der KapitelNummerierung
#endif


//-------------------------------------------------------
// Gliederung

struct _OutlinePara
{
    SwNodeNum aNum;
    const SwNodes& rNds;
    BYTE nMin, nNewLevel;
    BOOL bInitNum;

    _OutlinePara( const SwNodes& rNodes, USHORT nSttPos, BYTE nOld, BYTE nNew );
    BOOL UpdateOutline( SwTxtNode& rTxtNd );
};

_SV_IMPL_SORTAR_ALG( SwOutlineNodes, SwNodePtr )
BOOL SwOutlineNodes::Seek_Entry( const SwNodePtr rSrch, USHORT* pFndPos ) const
{
    ULONG nIdx = rSrch->GetIndex();

    register USHORT nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
//JP 17.03.98: aufgrund des Bug 48592 - wo unter anderem nach Undo/Redo
//              Nodes aus dem falschen NodesArray im OutlineArray standen,
//              jetzt mal einen Check eingebaut.
#ifndef PRODUCT
        {
            for( register USHORT n = 1; n < nO; ++n )
                if( &(*this)[ n-1 ]->GetNodes() !=
                    &(*this)[ n ]->GetNodes() )
                {
                    ASSERT( !this, "Node im falschen Outline-Array" );
                }
        }
#endif

        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( (*this)[ nM ] == rSrch )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return TRUE;
            }
            else if( (*this)[ nM ]->GetIndex() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return FALSE;
}


_OutlinePara::_OutlinePara( const SwNodes& rNodes, USHORT nSttPos,
                            BYTE nOld, BYTE nNew )
    : rNds( rNodes ),
    aNum( NO_NUM > nNew ? nNew : 0 ),
    nMin( Min( nOld, nNew )),
    bInitNum( 0 == nSttPos ),
    nNewLevel( nNew )
{
    // hole vom Vorgaenger die aktuelle Nummerierung
    SwNode* pNd;
    ULONG nEndOfExtras = rNds.GetEndOfExtras().GetIndex();
    if( nSttPos &&
        ( pNd = rNds.GetOutLineNds()[ --nSttPos ])->GetIndex() > nEndOfExtras
         && ((SwTxtNode*)pNd)->GetOutlineNum() )
    {
        const SwNodeNum* pNum = ((SwTxtNode*)pNd)->GetOutlineNum();
#ifdef TASK_59308
        if( pNum->GetLevel() & NO_NUMLEVEL )
        {
            // dann suche den mit richtigem Level:
            BYTE nSrchLvl = aNum.GetLevel();
            pNum = 0;
            while( nSttPos-- )
            {
                if( ( pNd = rNds.GetOutLineNds()[ nSttPos ])->
                    GetIndex() < nEndOfExtras )
                    break;

                if( 0 != ( pNum = ((SwTxtNode*)pNd)->GetOutlineNum() ))
                {
                    // uebergeordnete Ebene
                    if( nSrchLvl > (pNum->GetLevel() &~ NO_NUMLEVEL ))
                    {
                        pNum = 0;
                        break;
                    }
                    // gleiche Ebene und kein NO_NUMLEVEL
                    if( nSrchLvl == (pNum->GetLevel() &~ NO_NUMLEVEL)
                        && !( pNum->GetLevel() & NO_NUMLEVEL ))
                        break;

                    pNum = 0;
                }
            }
        }

#endif
        if( pNum )
        {
            aNum = *pNum;
            aNum.SetStart( FALSE );
            aNum.SetSetValue( USHRT_MAX );
        }

        if( aNum.GetLevel()+1 < MAXLEVEL )
            memset( aNum.GetLevelVal() + (aNum.GetLevel()+1), 0,
                    (MAXLEVEL - (aNum.GetLevel()+1)) * sizeof(aNum.GetLevelVal()[0]) );
    }
    else
        bInitNum = TRUE;
}



BOOL _OutlinePara::UpdateOutline( SwTxtNode& rTxtNd )
{
    // alle die ausserhalb des Fliesstextes liegen, NO_NUM zuweisen.
    if( rTxtNd.GetIndex() < rNds.GetEndOfExtras().GetIndex() )
    {
        BYTE nTmpLevel = aNum.GetLevel();
        aNum.SetLevel( NO_NUM );
        rTxtNd.UpdateOutlineNum( aNum );
        aNum.SetLevel( nTmpLevel );
        return TRUE;
    }

    BYTE nLevel = rTxtNd.GetTxtColl()->GetOutlineLevel();
    BOOL bRet = !(nMin > nLevel);
    if( bRet )
    {
        // existierte am Node schon eine Nummerierung ??
        // dann erfrage den "User definierten Wert"
        USHORT nSetValue;
        const SwNumRule* pOutlRule = rTxtNd.GetDoc()->GetOutlineNumRule();
        const SwNodeNum* pOutlNum = rTxtNd.GetOutlineNum();

#ifdef TASK_59308
        if( pOutlNum && ( pOutlNum->GetLevel() & NO_NUMLEVEL ) &&
            GetRealLevel( pOutlNum->GetLevel() ) == nLevel )
        {
            // diesen nicht mit numerieren
            BYTE nTmpLevel = aNum.GetLevel();
            aNum.SetLevel( pOutlNum->GetLevel() );
            rTxtNd.UpdateOutlineNum( aNum );
            aNum.SetLevel( nTmpLevel );
            return TRUE;
        }
#endif

        if( aNum.GetLevel() < nLevel && NO_NUM > nNewLevel )
        {
            if( aNum.GetLevel()+1 < MAXLEVEL )
                memset( aNum.GetLevelVal() + (aNum.GetLevel()+1), 0,
                        (MAXLEVEL - ( aNum.GetLevel()+1 )) *
                            sizeof( aNum.GetLevelVal()[0]));
            nSetValue = pOutlRule->Get( nLevel ).GetStartValue();
        }
        else if( bInitNum )
        {
            nSetValue= pOutlRule->Get( nLevel ).GetStartValue();
            bInitNum = FALSE;
        }
        else
            nSetValue = aNum.GetLevelVal()[ nLevel ] + 1;

         // alle unter dem neuen Level liegenden auf 0 setzen
        if( aNum.GetLevel() > nLevel && nLevel+1 < MAXLEVEL
            /* ??? && NO_NUM > nNewLevel */ )
            memset( aNum.GetLevelVal() + (nLevel+1), 0,
                    (MAXLEVEL - ( nLevel+1 )) * sizeof(aNum.GetLevelVal()[0]) );

        if( pOutlNum && USHRT_MAX != pOutlNum->GetSetValue() )
            aNum.SetSetValue( nSetValue = pOutlNum->GetSetValue() );

        aNum.GetLevelVal()[ nLevel ] = nSetValue;
        aNum.SetLevel( nLevel );
        rTxtNd.UpdateOutlineNum( aNum );
        aNum.SetSetValue( USHRT_MAX );
    }
    return bRet;
}



BOOL lcl_UpdateOutline( const SwNodePtr& rpNd, void* pPara )
{
    _OutlinePara* pOutlPara = (_OutlinePara*)pPara;
    SwTxtNode* pTxtNd = rpNd->GetTxtNode();
    ASSERT( pTxtNd, "kein TextNode als OutlineNode !" );

    return pOutlPara->UpdateOutline( *pTxtNd );
}




void SwNodes::UpdateOutlineNode( const SwNode& rNd, BYTE nOldLevel,
                                BYTE nNewLevel )
{
    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    USHORT nSttPos;
    BOOL bSeekIdx = pOutlineNds->Seek_Entry( pSrch, &nSttPos );

    if( NO_NUMBERING == nOldLevel )         // neuen Level einfuegen
    {
        // nicht vorhanden, also einfuegen
        ASSERT( !bSeekIdx, "Der Node ist schon als OutlineNode vorhanden" );

        //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
        ULONG nNd = rNd.GetIndex();
        if( nNd < GetEndOfRedlines().GetIndex() &&
            nNd > GetEndOfRedlines().FindStartNode()->GetIndex() )
            return ;

        // jetzt noch alle nachfolgende Outline-Nodes updaten
        pOutlineNds->Insert( pSrch );
        if( NO_NUM <= nNewLevel )
            return;     // keine Nummerierung dann kein Update
    }
    else if( NO_NUMBERING == nNewLevel )    // Level entfernen
    {
        if( !bSeekIdx )
            return;

        // jetzt noch alle nachfolgende Outline-Nodes updaten
        pOutlineNds->Remove( nSttPos );
        if( NO_NUM <= nOldLevel )
            return;     // keine Nummerierung dann kein Update
    }
    else if( !bSeekIdx )        // Update und Index nicht gefunden ??
        return ;

    _OutlinePara aPara( *this, nSttPos, nOldLevel, nNewLevel );
    pOutlineNds->ForEach( nSttPos, pOutlineNds->Count(),
                        lcl_UpdateOutline, &aPara );

//FEATURE::CONDCOLL
    {
        SwCntntNode* pCNd;
        ULONG nSttNd = rNd.GetIndex();
        if( NO_NUMBERING != nNewLevel )
            ++nSttPos;

        ULONG nChkCount = ( nSttPos < pOutlineNds->Count()
                                ? (*pOutlineNds)[ nSttPos ]->GetIndex()
                                : GetEndOfContent().GetIndex()  )
                            - nSttNd;
        for( ; nChkCount--; ++nSttNd )
            if( 0 != (pCNd = (*this)[ nSttNd ]->GetCntntNode() ) &&
                RES_CONDTXTFMTCOLL == pCNd->GetFmtColl()->Which() )
                pCNd->ChkCondColl();
    }
//FEATURE::CONDCOLL

    // die Gliederungs-Felder Updaten
    GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
}



void SwNodes::UpdtOutlineIdx( const SwNode& rNd )
{
    if( !pOutlineNds->Count() )     // keine OutlineNodes vorhanden ?
        return;

    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    USHORT nPos;
    pOutlineNds->Seek_Entry( pSrch, &nPos );
    if( nPos == pOutlineNds->Count() )      // keine zum Updaten vorhanden ?
        return;

    if( nPos )
        --nPos;

    if( !GetDoc()->IsInDtor() && IsDocNodes() )
        UpdateOutlineNode( *(*pOutlineNds)[ nPos ], 0, 0 );
}



void SwNodes::UpdateOutlineNodes()
{
    if( pOutlineNds->Count() )      // OutlineNodes vorhanden ?
        UpdateOutlineNode( *(*pOutlineNds)[ 0 ], 0, 0 );
}




