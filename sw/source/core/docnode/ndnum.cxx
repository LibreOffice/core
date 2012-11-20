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

#include <node.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>           // UpdateFlds der KapitelNummerierung
#include <docary.hxx>

bool CompareSwOutlineNodes::operator()( SwNode* const& lhs, SwNode* const& rhs) const
{
    return lhs->GetIndex() < rhs->GetIndex();
}

bool SwOutlineNodes::Seek_Entry(SwNode* const &rP, sal_uInt16* pnPos) const
{
    const_iterator it = lower_bound(rP);
    *pnPos = it - begin();
    return it != end() && rP->GetIndex() == (*it)->GetIndex();
}

void SwNodes::UpdateOutlineNode(SwNode & rNd)
{
    SwTxtNode * pTxtNd = rNd.GetTxtNode();

    if (pTxtNd && pTxtNd->IsOutlineStateChanged())
    {
        bool bFound = pOutlineNds->find(pTxtNd) != pOutlineNds->end();

        if (pTxtNd->IsOutline())
        {
            if (! bFound)
            {
                // assure that text is in the correct nodes array
                if ( &(pTxtNd->GetNodes()) == this )
                {
                    pOutlineNds->insert(pTxtNd);
                }
                else
                {
                    OSL_FAIL( "<SwNodes::UpdateOutlineNode(..)> - given text node isn't in the correct nodes array. This is a serious defect -> inform OD" );
                }
            }
        }
        else
        {
            if (bFound)
                pOutlineNds->erase(pTxtNd);
        }

        pTxtNd->UpdateOutlineState();

        // die Gliederungs-Felder Updaten
        GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
    }
}

void SwNodes::UpdtOutlineIdx( const SwNode& rNd )
{
    if( pOutlineNds->empty() )     // keine OutlineNodes vorhanden ?
        return;

    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    sal_uInt16 nPos;
    pOutlineNds->Seek_Entry( pSrch, &nPos );
    if( nPos == pOutlineNds->size() )      // keine zum Updaten vorhanden ?
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
