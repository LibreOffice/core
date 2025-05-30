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

#include <unmovss.hxx>
#include <drawdoc.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>

SdMoveStyleSheetsUndoAction::SdMoveStyleSheetsUndoAction( SdDrawDocument& rTheDoc, StyleSheetCopyResultVector& rTheStyles, bool bInserted)
: SdUndoAction(rTheDoc)
, mbMySheets( !bInserted )
{
    maStyles.swap( rTheStyles );

    maListOfChildLists.resize( maStyles.size() );
    // create list with lists of style sheet children
    std::size_t i = 0;
    for (const auto& a : maStyles)
    {
        maListOfChildLists[i++] = SdStyleSheetPool::CreateChildList(a.m_xStyleSheet.get());
    }
}

void SdMoveStyleSheetsUndoAction::Undo()
{
    SfxStyleSheetBasePool* pPool  = mrDoc.GetStyleSheetPool();

    if (mbMySheets)
    {
        // the styles have to be inserted in the pool

        // first insert all styles to the pool
        for (auto& a : maStyles)
        {
            if (!a.m_bCreatedByCopy) // tdf#119259, existed before this action, so leave it alone
                continue;
            pPool->Insert(a.m_xStyleSheet.get());
        }

        // now assign the children again
        std::vector< SdStyleSheetVector >::iterator childlistiter( maListOfChildLists.begin() );
        for (const auto& a : maStyles)
        {
            OUString aParent(a.m_xStyleSheet->GetName());
            for( auto& rxChild : *childlistiter )
            {
                rxChild->SetParent(aParent);
            }
            ++childlistiter;
        }
    }
    else
    {
        // remove the styles again from the pool
        for (auto& a : maStyles)
        {
            if (!a.m_bCreatedByCopy) // tdf#119259, existed before this action, so leave it alone
                continue;
            pPool->Remove(a.m_xStyleSheet.get());
        }
    }
    mbMySheets = !mbMySheets;
}

void SdMoveStyleSheetsUndoAction::Redo()
{
    Undo();
}

SdMoveStyleSheetsUndoAction::~SdMoveStyleSheetsUndoAction()
{
}

OUString SdMoveStyleSheetsUndoAction::GetComment() const
{
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
