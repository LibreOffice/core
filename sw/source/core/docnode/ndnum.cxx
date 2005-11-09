/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ndnum.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-09 10:07:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


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
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

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

void SwNodes::UpdateOutlineNode(SwNode & rNd)
{
    SwTxtNode * pTxtNd = rNd.GetTxtNode();

    if (pTxtNd && pTxtNd->IsOutlineStateChanged())
    {
        BOOL bFound = pOutlineNds->Seek_Entry(pTxtNd);

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

void SwNodes::UpdateOutlineNode( const SwNode& rNd, BYTE nOldLevel,
                                 BYTE nNewLevel )
{
    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    USHORT nSttPos;
    BOOL bSeekIdx = pOutlineNds->Seek_Entry( pSrch, &nSttPos );

    if( NO_NUMBERING == nOldLevel )         // neuen Level einfuegen
    {
        // nicht vorhanden, also einfuegen
        //ASSERT( !bSeekIdx, "Der Node ist schon als OutlineNode vorhanden" );

        //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
        ULONG nNd = rNd.GetIndex();
        if( nNd < GetEndOfRedlines().GetIndex() &&
            nNd > GetEndOfRedlines().FindStartNode()->GetIndex() )
            return ;

        // jetzt noch alle nachfolgende Outline-Nodes updaten
        if (! bSeekIdx)
        {
            // --> OD 2005-11-03 #125329#
            // assure that node <pSrch> is in the correct nodes array
            if ( &(pSrch->GetNodes()) == this )
            {
                pOutlineNds->Insert( pSrch );
            }
            else
            {
                ASSERT( false,
                        "<SwNodes::UpdateOutlineNode(..)> - node <pSrch> isn't in correct nodes array. This is a serious defect -> inform OD" );
            }
            // <--
        }

    }
    else if( NO_NUMBERING == nNewLevel )    // Level entfernen
    {
        if( !bSeekIdx )
            return;

        // jetzt noch alle nachfolgende Outline-Nodes updaten
        pOutlineNds->Remove( nSttPos );
    }
    else if( !bSeekIdx )        // Update und Index nicht gefunden ??
        return ;

    {
        SwTxtNode & rTxtNd = (SwTxtNode &) rNd;
        SwPaM aPam(rTxtNd); // #115901#

        if (nNewLevel != NO_NUMBERING) // #115901#
        {
            rTxtNd.SetLevel(rTxtNd.GetTxtColl()->GetOutlineLevel());

            rTxtNd.NumRuleChgd();
            //GetDoc()->SetNumRule(aPam, *GetDoc()->GetOutlineNumRule());
        }
        else
        {
            GetDoc()->DelNumRules(aPam);
        }
    }

    // die Gliederungs-Felder Updaten
    GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
    GetDoc()->ChkCondColls();
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
        UpdateOutlineNode( *(*pOutlineNds)[ nPos ]);
}

const SwOutlineNodes & SwNodes::GetOutLineNds() const
{
    return *pOutlineNds;
}
