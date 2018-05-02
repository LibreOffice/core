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


#include <node.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>           // UpdateFlds der KapitelNummerierung
#include <docary.hxx>

_SV_IMPL_SORTAR_ALG( SwOutlineNodes, SwNodePtr )
sal_Bool SwOutlineNodes::Seek_Entry( const SwNodePtr rSrch, sal_uInt16* pFndPos ) const
{
    sal_uLong nIdx = rSrch->GetIndex();

    sal_uInt16 nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
//JP 17.03.98: aufgrund des Bug 48592 - wo unter anderem nach Undo/Redo
//              Nodes aus dem falschen NodesArray im OutlineArray standen,
//              jetzt mal einen Check eingebaut.
#ifdef DBG_UTIL
        {
            for( sal_uInt16 n = 1; n < nO; ++n )
                if( &(*this)[ n-1 ]->GetNodes() !=
                    &(*this)[ n ]->GetNodes() )
                {
                    ASSERT( sal_False, "Node in the wrong outline array" );
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
                return sal_True;
            }
            else if( (*this)[ nM ]->GetIndex() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return sal_False;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return sal_False;
}

void SwNodes::UpdateOutlineNode(SwNode & rNd)
{
    SwTxtNode * pTxtNd = rNd.GetTxtNode();

    if (pTxtNd && pTxtNd->IsOutlineStateChanged())
    {
        sal_Bool bFound = pOutlineNds->Seek_Entry(pTxtNd);

        if (pTxtNd->IsOutline())
        {
            if (! bFound)
            {
                // --> OD 2005-11-02 #125329#
                // assure that text is in the correct nodes array
                if ( &(pTxtNd->GetNodes()) == this )
                {
                    pOutlineNds->Insert(pTxtNd);
                }
                else
                {
                    ASSERT( false,
                            "<SwNodes::UpdateOutlineNode(..)> - given text node isn't in the correct nodes array. This is a serious defect -> inform OD" );
                }
                // <--
            }
        }
        else
        {
            if (bFound)
                pOutlineNds->Remove(pTxtNd);
        }

        pTxtNd->UpdateOutlineState();

        // die Gliederungs-Felder Updaten
        GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
    }
}

//void SwNodes::UpdateOutlineNode( const SwNode& rNd, sal_uInt8 nOldLevel, //#outline level,removed by zhaojianwei
//                                 sal_uInt8 nNewLevel )
//{
//  const SwNodePtr pSrch = (SwNodePtr)&rNd;
//  sal_uInt16 nSttPos;
//  sal_Bool bSeekIdx = pOutlineNds->Seek_Entry( pSrch, &nSttPos );
//
//  //if( NO_NUMBERING == nOldLevel )   //#outline level,zhaojianwei
//  if( 0 == nOldLevel )                //<-end, zhaojianwei
//  {
//      // nicht vorhanden, also einfuegen
//      //ASSERT( !bSeekIdx, "Der Node ist schon als OutlineNode vorhanden" );
//
//      //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
//      sal_uLong nNd = rNd.GetIndex();
//      if( nNd < GetEndOfRedlines().GetIndex() &&
//            nNd > GetEndOfRedlines().StartOfSectionNode()->GetIndex() )
//          return ;
//
//      // jetzt noch alle nachfolgende Outline-Nodes updaten
//        if (! bSeekIdx)
//        {
//            // --> OD 2005-11-03 #125329#
//            // assure that node <pSrch> is in the correct nodes array
//            if ( &(pSrch->GetNodes()) == this )
//            {
//                pOutlineNds->Insert( pSrch );
//            }
//            else
//            {
//                ASSERT( false,
//                        "<SwNodes::UpdateOutlineNode(..)> - node <pSrch> isn't in correct nodes array. This is a serious defect -> inform OD" );
//            }
//            // <--
//        }
//
//  }
//  //else if( NO_NUMBERING == nNewLevel )  //#outline level,removed by zhaojianwei
//  else if( 0 == nNewLevel )               //<-end,added by zhaojianwei    // Level entfernen
//  {
//      if( !bSeekIdx )
//          return;
//
//      // jetzt noch alle nachfolgende Outline-Nodes updaten
//      pOutlineNds->Remove( nSttPos );
//  }
//  else if( !bSeekIdx )        // Update und Index nicht gefunden ??
//      return ;
//
//    {
//        SwTxtNode & rTxtNd = (SwTxtNode &) rNd;
//        SwPaM aPam(rTxtNd); // #115901#
//
//        //if (nNewLevel != NO_NUMBERING) //#outline level,zhaojianwei // #115901#
//      if (nNewLevel != 0)              //<-end,zhaojianwei // #115901#
//        {
//            //rTxtNd.SetLevel(rTxtNd.GetTxtColl()->GetOutlineLevel());    //#outline level,zhaojianwei
//          //rTxtNd.NumRuleChgd();
//          if(rTxtNd.GetTxtColl()->IsAssignedToListLevelOfOutlineStyle())
//          {
//              rTxtNd.SetLevel(rTxtNd.GetTxtColl()->GetAssignedOutlineStyleLevel());
//              rTxtNd.NumRuleChgd();
//          }//<-end,zhaojianwei
//
//            //GetDoc()->SetNumRule(aPam, *GetDoc()->GetOutlineNumRule());
//        }
//        else
//        {
//            GetDoc()->DelNumRules(aPam);
//        }
//    }
//
//  // die Gliederungs-Felder Updaten
//  GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
//    GetDoc()->ChkCondColls();
//}//<-end,zhaojianwei

void SwNodes::UpdtOutlineIdx( const SwNode& rNd )
{
    if( !pOutlineNds->Count() )     // keine OutlineNodes vorhanden ?
        return;

    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    sal_uInt16 nPos;
    pOutlineNds->Seek_Entry( pSrch, &nPos );
    if( nPos == pOutlineNds->Count() )      // keine zum Updaten vorhanden ?
        return;

    if( nPos )
        --nPos;

    if( !GetDoc()->IsInDtor() && IsDocNodes() )
        UpdateOutlineNode( *(*pOutlineNds)[ nPos ]);
}

const SwOutlineNodes & SwNodes::GetOutLineNds() const
{
    return *pOutlineNds;
}
