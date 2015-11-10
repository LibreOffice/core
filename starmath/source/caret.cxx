/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "caret.hxx"

/////////////////////////////// SmCaretPosGraph

SmCaretPosGraphEntry* SmCaretPosGraphIterator::Next(){
    if(nOffset >= pGraph->nOffset){
        if(pGraph->pNext){
            pGraph = pGraph->pNext;
            nOffset = 0;
            pEntry = Next();
        }else
            pEntry = nullptr;
    }else
        pEntry = pGraph->Graph + nOffset++;
    return pEntry;
}

SmCaretPosGraphEntry* SmCaretPosGraph::Add(SmCaretPosGraphEntry entry){
    if(nOffset >= SmCaretPosGraphSize){
        if(!pNext)
            pNext = new SmCaretPosGraph();
        return pNext->Add(entry);
    }else{
        //Set Left and Right to point to the entry itself if they are NULL
        entry.Left = entry.Left ? entry.Left : Graph + nOffset;
        entry.Right = entry.Right ? entry.Right : Graph + nOffset;
        //Save the entry
        Graph[nOffset] = entry;
        return Graph + nOffset++;
    }
}

SmCaretPosGraph::~SmCaretPosGraph(){
    delete pNext;
    pNext = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
