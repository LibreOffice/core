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
        Graph[nOffset] = entry;
        return Graph + nOffset++;
    }
}

SmCaretPosGraph::~SmCaretPosGraph(){
    if(pNext)
        delete pNext;
    pNext = NULL;
}
