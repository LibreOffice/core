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

#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <fchrfmt.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>

sal_uInt16 SwEditShell::GetCharFormatCount() const
{
    return GetDoc()->GetCharFormats()->size();
}

SwCharFormat& SwEditShell::GetCharFormat(sal_uInt16 nFormat) const
{
    return *((*(GetDoc()->GetCharFormats()))[nFormat]);
}

SwCharFormat* SwEditShell::GetCurCharFormat() const
{
    SwCharFormat *pFormat = nullptr;
    SfxItemSetFixed<RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT> aSet( GetDoc()->GetAttrPool() );
    const SwFormatCharFormat* pItem;
    if( GetCurAttr( aSet ) &&
        (pItem = aSet.GetItemIfSet( RES_TXTATR_CHARFMT, false ) ) )
        pFormat = pItem->GetCharFormat();

    return pFormat;
}

void SwEditShell::FillByEx(SwCharFormat* pCharFormat)
{
    SwPaM* pPam = GetCursor();
    const SwContentNode* pCNd = pPam->GetPointContentNode();
    if( pCNd->IsTextNode() )
    {
        SwTextNode const*const pTextNode(pCNd->GetTextNode());
        sal_Int32 nStt;
        sal_Int32 nEnd;
        if( pPam->HasMark() )
        {
            const SwPosition* pPtPos = pPam->GetPoint();
            const SwPosition* pMkPos = pPam->GetMark();
            if( pPtPos->GetNode() == pMkPos->GetNode() )        // in the same node?
            {
                nStt = pPtPos->GetContentIndex();
                if( nStt < pMkPos->GetContentIndex() )
                    nEnd = pMkPos->GetContentIndex();
                else
                {
                    nEnd = nStt;
                    nStt = pMkPos->GetContentIndex();
                }
            }
            else
            {
                nStt = pMkPos->GetContentIndex();
                if( pPtPos->GetNode() < pMkPos->GetNode() )
                {
                    nEnd = nStt;
                    nStt = 0;
                }
                else
                    nEnd = pTextNode->GetText().getLength();
            }
        }
        else
            nStt = nEnd = pPam->GetPoint()->GetContentIndex();

        SfxItemSet aSet( mxDoc->GetAttrPool(),
                            pCharFormat->GetAttrSet().GetRanges() );
        pTextNode->GetParaAttr(aSet, nStt, nEnd, false, true, false, GetLayout());
        pCharFormat->SetFormatAttr( aSet );
    }
    else if( pCNd->HasSwAttrSet() )
        pCharFormat->SetFormatAttr( *pCNd->GetpSwAttrSet() );
}

size_t SwEditShell::GetTableFrameFormatCount(bool bUsed) const
{
    return GetDoc()->GetTableFrameFormatCount(bUsed);
}

SwFrameFormat& SwEditShell::GetTableFrameFormat(size_t nFormat, bool bUsed ) const
{
    return GetDoc()->GetTableFrameFormat(nFormat, bUsed );
}

UIName SwEditShell::GetUniqueTableName() const
{
    return GetDoc()->GetUniqueTableName();
}

SwCharFormat* SwEditShell::MakeCharFormat( const UIName& rName )
{
    SwCharFormat* pDerivedFrom = GetDoc()->GetDfltCharFormat();

    return GetDoc()->MakeCharFormat( rName, pDerivedFrom );
}

SwTextFormatColl* SwEditShell::GetTextCollFromPool( sal_uInt16 nId )
{
    return GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool( nId );
}

/// return the requested automatic format - base-class !
SwFormat* SwEditShell::GetFormatFromPool( sal_uInt16 nId )
{
    return GetDoc()->getIDocumentStylePoolAccess().GetFormatFromPool( nId );
}

SwPageDesc* SwEditShell::GetPageDescFromPool( sal_uInt16 nId )
{
    return GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool( nId );
}

bool SwEditShell::IsUsed( const sw::BroadcastingModify& rModify ) const
{
    return mxDoc->IsUsed( rModify );
}

const SwFlyFrameFormat* SwEditShell::FindFlyByName( const UIName& rName ) const
{
    return mxDoc->FindFlyByName(rName);
}

SwCharFormat* SwEditShell::FindCharFormatByName( const UIName& rName ) const
{
    return mxDoc->FindCharFormatByName( rName );
}

SwTextFormatColl* SwEditShell::FindTextFormatCollByName( const UIName& rName ) const
{
    return mxDoc->FindTextFormatCollByName( rName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
