/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */
#include "caret.hxx"



SmCaretPosGraphEntry* SmCaretPosGraphIterator::Next(){
    if(nOffset >= pGraph->nOffset){
        if(pGraph->pNext){
            pGraph = pGraph->pNext;
            nOffset = 0;
            pEntry = Next();
        }else
            pEntry = NULL;
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
        
        entry.Left = entry.Left ? entry.Left : Graph + nOffset;
        entry.Right = entry.Right ? entry.Right : Graph + nOffset;
        
        Graph[nOffset] = entry;
        return Graph + nOffset++;
    }
}

SmCaretPosGraph::~SmCaretPosGraph(){
    delete pNext;
    pNext = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
