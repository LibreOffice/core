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
#include <IDocumentFieldsAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docary.hxx>

bool CompareSwOutlineNodes::operator()( SwNode* const& lhs, SwNode* const& rhs) const
{
    return lhs->GetIndex() < rhs->GetIndex();
}

bool SwOutlineNodes::Seek_Entry(SwNode* rP, sal_uInt16* pnPos) const
{
    const_iterator it = lower_bound(rP);
    *pnPos = it - begin();
    return it != end() && rP->GetIndex() == (*it)->GetIndex();
}

void SwNodes::UpdateOutlineNode(SwNode & rNd)
{
    SwTextNode * pTextNd = rNd.GetTextNode();

    if (pTextNd && pTextNd->IsOutlineStateChanged())
    {
        bool bFound = pOutlineNds->find(pTextNd) != pOutlineNds->end();

        if (pTextNd->IsOutline())
        {
            if (! bFound)
            {
                // assure that text is in the correct nodes array
                if ( &(pTextNd->GetNodes()) == this )
                {
                    pOutlineNds->insert(pTextNd);
                }
                else
                {
                    OSL_FAIL( "<SwNodes::UpdateOutlineNode(..)> - given text node isn't in the correct nodes array. This is a serious defect" );
                }
            }
        }
        else
        {
            if (bFound)
                pOutlineNds->erase(pTextNd);
        }

        pTextNd->UpdateOutlineState();

        // update the structure fields
        GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( RES_CHAPTERFLD )->UpdateFields();
    }
}

void SwNodes::UpdateOutlineIdx( const SwNode& rNd )
{
    if( pOutlineNds->empty() )     // no OutlineNodes present ?
        return;

    const SwNodePtr pSrch = const_cast<SwNodePtr>(&rNd);

    sal_uInt16 nPos;
    if (!pOutlineNds->Seek_Entry(pSrch, &nPos))
        return;
    if( nPos == pOutlineNds->size() )      // none present for updating ?
        return;

    if( nPos )
        --nPos;

    if( !GetDoc()->IsInDtor() && IsDocNodes() )
        UpdateOutlineNode( *(*pOutlineNds)[ nPos ]);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
