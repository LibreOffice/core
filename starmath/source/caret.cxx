/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Jonas Finnemann Jensen <jopsen@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jonas Finnemann Jensen <jopsen@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include "caret.hxx"

/////////////////////////////// SmCaretPosGraph ////////////////////////////////

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
        //Set Left and Right to point to the entry itself if they are NULL
        entry.Left = entry.Left ? entry.Left : Graph + nOffset;
        entry.Right = entry.Right ? entry.Right : Graph + nOffset;
        //Save the entry
        Graph[nOffset] = entry;
        return Graph + nOffset++;
    }
}

SmCaretPosGraph::~SmCaretPosGraph(){
    if(pNext)
        delete pNext;
    pNext = NULL;
}
