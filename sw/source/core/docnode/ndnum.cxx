/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <node.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>           // UpdateFlds of the chapter numbering
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

        // update the structure fields
        GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
    }
}

void SwNodes::UpdtOutlineIdx( const SwNode& rNd )
{
    if( pOutlineNds->empty() )     // no OutlineNodes present ?
        return;

    const SwNodePtr pSrch = (SwNodePtr)&rNd;
    sal_uInt16 nPos;
    pOutlineNds->Seek_Entry( pSrch, &nPos );
    if( nPos == pOutlineNds->size() )      // none present for updating ?
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
