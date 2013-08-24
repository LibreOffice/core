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


#include "unmovss.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

SdMoveStyleSheetsUndoAction::SdMoveStyleSheetsUndoAction( SdDrawDocument* pTheDoc, SdStyleSheetVector& rTheStyles, bool bInserted)
: SdUndoAction(pTheDoc)
, mbMySheets( !bInserted )
{
    maStyles.swap( rTheStyles );

    maListOfChildLists.resize( maStyles.size() );
    // create list with lists of style sheet children
    std::size_t i = 0;
    for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
    {
        maListOfChildLists[i++] = SdStyleSheetPool::CreateChildList( (*iter).get() );
    }
}

void SdMoveStyleSheetsUndoAction::Undo()
{
    SfxStyleSheetBasePool* pPool  = mpDoc->GetStyleSheetPool();

    if (mbMySheets)
    {
        // the styles have to be inserted in the pool

        // first insert all styles to the pool
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
        {
            pPool->Insert((*iter).get());
        }

        // now assign the children again
        std::vector< SdStyleSheetVector >::iterator childlistiter( maListOfChildLists.begin() );
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter, ++childlistiter )
        {
            OUString aParent((*iter)->GetName());
            for( SdStyleSheetVector::iterator childiter = (*childlistiter).begin(); childiter != (*childlistiter).end(); ++childiter )
            {
                (*childiter)->SetParent(aParent);
            }
        }
    }
    else
    {
        // remove the styles again from the pool
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
        {
            pPool->Remove((*iter).get());
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
