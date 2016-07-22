/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "cursor.hxx"
#include "visitors.hxx"
#include "document.hxx"
#include "view.hxx"
#include "accessibility.hxx"
#include <comphelper/string.hxx>
#include <cassert>

void SmCursor::Move(OutputDevice* pDev, SmMovementDirection direction, bool bMoveAnchor){
    SmCaretPosGraphEntry* NewPos = nullptr;
    switch(direction)
    {
        case MoveLeft:
            if (mpPosition)
                NewPos = mpPosition->Left;
            OSL_ENSURE(NewPos, "NewPos shouldn't be NULL here!");
            break;
        case MoveRight:
            if (mpPosition)
                NewPos = mpPosition->Right;
            OSL_ENSURE(NewPos, "NewPos shouldn't be NULL here!");
            break;
        case MoveUp:
            //Implementation is practically identical to MoveDown, except for a single if statement
            //so I've implemented them together and added a direction == MoveDown to the if statements.
        case MoveDown:
            if (mpPosition)
            {
                SmCaretLine from_line = SmCaretPos2LineVisitor(pDev, mpPosition->CaretPos).GetResult(),
                            best_line,  //Best approximated line found so far
                            curr_line;  //Current line
                long dbp_sq = 0;        //Distance squared to best line
                for(auto &pEntry : *mpGraph)
                {
                    //Reject it if it's the current position
                    if(pEntry->CaretPos == mpPosition->CaretPos) continue;
                    //Compute caret line
                    curr_line = SmCaretPos2LineVisitor(pDev, pEntry->CaretPos).GetResult();
                    //Reject anything above if we're moving down
                    if(curr_line.GetTop() <= from_line.GetTop() && direction == MoveDown) continue;
                    //Reject anything below if we're moving up
                    if(curr_line.GetTop() + curr_line.GetHeight() >= from_line.GetTop() + from_line.GetHeight()
                            && direction == MoveUp) continue;
                    //Compare if it to what we have, if we have anything yet
                    if(NewPos){
                        //Compute distance to current line squared, multiplied with a horizontal factor
                        long dp_sq = curr_line.SquaredDistanceX(from_line) * HORIZONTICAL_DISTANCE_FACTOR +
                                     curr_line.SquaredDistanceY(from_line);
                        //Discard current line if best line is closer
                        if(dbp_sq <= dp_sq) continue;
                    }
                    //Take current line as the best
                    best_line = curr_line;
                    NewPos = pEntry.get();
                    //Update distance to best line
                    dbp_sq = best_line.SquaredDistanceX(from_line) * HORIZONTICAL_DISTANCE_FACTOR +
                             best_line.SquaredDistanceY(from_line);
                }
            }
            break;
        default:
            assert(false);
    }
    if(NewPos){
        mpPosition = NewPos;
        if(bMoveAnchor)
            mpAnchor = NewPos;
        RequestRepaint();
    }
}

void SmCursor::MoveTo(OutputDevice* pDev, Point pos, bool bMoveAnchor){
    SmCaretLine best_line,  //Best line found so far, when iterating
                curr_line;  //Current line, when iterating
    SmCaretPosGraphEntry* NewPos = nullptr;
    long dp_sq = 0,     //Distance to current line squared
         dbp_sq = 1;    //Distance to best line squared
    for(auto &pEntry : *mpGraph)
    {
        OSL_ENSURE(pEntry->CaretPos.IsValid(), "The caret position graph may not have invalid positions!");
        //Compute current line
        curr_line = SmCaretPos2LineVisitor(pDev, pEntry->CaretPos).GetResult();
        //If we have a position compare to it
        if(NewPos){
            //Compute squared distance to current line
            dp_sq = curr_line.SquaredDistanceX(pos) + curr_line.SquaredDistanceY(pos);
            //If best line is closer, reject current line
            if(dbp_sq <= dp_sq) continue;
        }
        //Accept current position as the best
        best_line = curr_line;
        NewPos = pEntry.get();
        //Update distance to best line
        dbp_sq = best_line.SquaredDistanceX(pos) + best_line.SquaredDistanceY(pos);
    }
    if(NewPos){
        mpPosition = NewPos;
        if(bMoveAnchor)
            mpAnchor = NewPos;
        RequestRepaint();
    }
}

void SmCursor::BuildGraph(){
    //Save the current anchor and position
    SmCaretPos _anchor, _position;
    //Release mpGraph if allocated
    if(mpGraph){
        if(mpAnchor)
            _anchor = mpAnchor->CaretPos;
        if(mpPosition)
            _position = mpPosition->CaretPos;
        mpGraph.reset();
        //Reset anchor and position as they point into an old graph
        mpAnchor = nullptr;
        mpPosition = nullptr;
    }

    //Build the new graph
    mpGraph.reset(SmCaretPosGraphBuildingVisitor(mpTree).takeGraph());

    //Restore anchor and position pointers
    if(_anchor.IsValid() || _position.IsValid()){
        for(auto &pEntry : *mpGraph)
        {
            if(_anchor == pEntry->CaretPos)
                mpAnchor = pEntry.get();
            if(_position == pEntry->CaretPos)
                mpPosition = pEntry.get();
        }
    }
    //Set position and anchor to first caret position
    auto it = mpGraph->begin();
    assert(it != mpGraph->end());
    if(!mpPosition)
        mpPosition = it->get();
    if(!mpAnchor)
        mpAnchor = mpPosition;

    assert(mpPosition);
    assert(mpAnchor);
    OSL_ENSURE(mpPosition->CaretPos.IsValid(), "Position must be valid");
    OSL_ENSURE(mpAnchor->CaretPos.IsValid(), "Anchor must be valid");
}

bool SmCursor::SetCaretPosition(SmCaretPos pos){
    for(auto &pEntry : *mpGraph)
    {
        if(pEntry->CaretPos == pos)
        {
            mpPosition = pEntry.get();
            mpAnchor = pEntry.get();
            return true;
        }
    }
    return false;
}

void SmCursor::AnnotateSelection(){
    //TODO: Manage a state, reset it upon modification and optimize this call
    SmSetSelectionVisitor(mpAnchor->CaretPos, mpPosition->CaretPos, mpTree);
}

void SmCursor::Draw(OutputDevice& pDev, Point Offset, bool isCaretVisible){
    SmCaretDrawingVisitor(pDev, GetPosition(), Offset, isCaretVisible);
}

void SmCursor::DeletePrev(OutputDevice* pDev){
    //Delete only a selection if there's a selection
    if(HasSelection()){
        Delete();
        return;
    }

    SmNode* pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);
    SmStructureNode* pLineParent = pLine->GetParent();
    int nLineOffset = pLineParent->IndexOfSubNode(pLine);
    assert(nLineOffset >= 0);

    //If we're in front of a node who's parent is a TABLE
    if(pLineParent->GetType() == NTABLE && mpPosition->CaretPos.Index == 0 && nLineOffset > 0){
        //Now we can merge with nLineOffset - 1
        BeginEdit();
        //Line to merge things into, so we can delete pLine
        SmNode* pMergeLine = pLineParent->GetSubNode(nLineOffset-1);
        OSL_ENSURE(pMergeLine, "pMergeLine cannot be NULL!");
        SmCaretPos PosAfterDelete;
        //Convert first line to list
        SmNodeList *pLineList = NodeToList(pMergeLine);
        if(!pLineList->empty()){
            //Find iterator to patch
            SmNodeList::iterator patchPoint = pLineList->end();
            --patchPoint;
            //Convert second line to list
            NodeToList(pLine, pLineList);
            //Patch the line list
            ++patchPoint;
            PosAfterDelete = PatchLineList(pLineList, patchPoint);
            //Parse the line
            pLine = SmNodeListParser().Parse(pLineList);
        }
        delete pLineList;
        pLineParent->SetSubNode(nLineOffset-1, pLine);
        //Delete the removed line slot
        SmNodeArray lines(pLineParent->GetNumSubNodes()-1);
        for(int i = 0; i < pLineParent->GetNumSubNodes(); i++){
            if(i < nLineOffset)
                lines[i] = pLineParent->GetSubNode(i);
            else if(i > nLineOffset)
                lines[i-1] = pLineParent->GetSubNode(i);
        }
        pLineParent->SetSubNodes(lines);
        //Rebuild graph
        mpAnchor = nullptr;
        mpPosition = nullptr;
        BuildGraph();
        AnnotateSelection();
        //Set caret position
        if(!SetCaretPosition(PosAfterDelete))
            SetCaretPosition(SmCaretPos(pLine, 0));
        //Finish editing
        EndEdit();

    //TODO: If we're in an empty (sub/super/*) script
    /*}else if(pLineParent->GetType() == NSUBSUP &&
             nLineOffset != 0 &&
             pLine->GetType() == NEXPRESSION &&
             pLine->GetNumSubNodes() == 0){
        //There's a (sub/super) script we can delete
    //Consider selecting the entire script if GetNumSubNodes() != 0 or pLine->GetType() != NEXPRESSION
    //TODO: Handle case where we delete a limit
    */

    //Else move select, and delete if not complex
    }else{
        this->Move(pDev, MoveLeft, false);
        if(!this->HasComplexSelection())
            Delete();
    }
}

void SmCursor::Delete(){
    //Return if we don't have a selection to delete
    if(!HasSelection())
        return;

    //Enter edit section
    BeginEdit();

    //Set selected on nodes
    AnnotateSelection();

    //Find an arbitrary selected node
    SmNode* pSNode = FindSelectedNode(mpTree);
    OSL_ENSURE(pSNode != nullptr, "There must be a selection when HasSelection is true!");

    //Find the topmost node of the line that holds the selection
    SmNode* pLine = FindTopMostNodeInLine(pSNode, true);
    OSL_ENSURE(pLine != mpTree, "Shouldn't be able to select the entire tree");

    //Get the parent of the line
    SmStructureNode* pLineParent = pLine->GetParent();
    //Find line offset in parent
    int nLineOffset = pLineParent->IndexOfSubNode(pLine);
    assert(nLineOffset >= 0);

    //Position after delete
    SmCaretPos PosAfterDelete;

    SmNodeList* pLineList = NodeToList(pLine);

    //Take the selected nodes and delete them...
    SmNodeList::iterator patchIt = TakeSelectedNodesFromList(pLineList);

    //Get the position to set after delete
    PosAfterDelete = PatchLineList(pLineList, patchIt);

    //Finish editing
    FinishEdit(pLineList, pLineParent, nLineOffset, PosAfterDelete);
}

void SmCursor::InsertNodes(SmNodeList* pNewNodes){
    if(pNewNodes->empty()){
        delete pNewNodes;
        return;
    }

    //Begin edit section
    BeginEdit();

    //Position after insert should be after pNewNode
    SmCaretPos PosAfterInsert = SmCaretPos(pNewNodes->back(), 1);

    //Get the current position
    const SmCaretPos pos = mpPosition->CaretPos;

    //Find top most of line that holds position
    SmNode* pLine = FindTopMostNodeInLine(pos.pSelectedNode);

    //Find line parent and line index in parent
    SmStructureNode* pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //Convert line to list
    SmNodeList* pLineList = NodeToList(pLine);

    //Find iterator for place to insert nodes
    SmNodeList::iterator it = FindPositionInLineList(pLineList, pos);

    //Insert all new nodes
    SmNodeList::iterator newIt,
                         patchIt = it, // (pointless default value, fixes compiler warnings)
                         insIt;
    for(newIt = pNewNodes->begin(); newIt != pNewNodes->end(); ++newIt){
        insIt = pLineList->insert(it, *newIt);
        if(newIt == pNewNodes->begin())
            patchIt = insIt;
        if((*newIt)->GetType() == NTEXT)
            PosAfterInsert = SmCaretPos(*newIt, static_cast<SmTextNode*>(*newIt)->GetText().getLength());
        else
            PosAfterInsert = SmCaretPos(*newIt, 1);
    }
    //Patch the places we've changed stuff
                        PatchLineList(pLineList, patchIt);
    PosAfterInsert =    PatchLineList(pLineList, it);
    //Release list, we've taken the nodes
    delete pNewNodes;
    pNewNodes = nullptr;

    //Finish editing
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
}

SmNodeList::iterator SmCursor::FindPositionInLineList(SmNodeList* pLineList,
                                                      const SmCaretPos& rCaretPos)
{
    //Find iterator for position
    SmNodeList::iterator it;
    for(it = pLineList->begin(); it != pLineList->end(); ++it){
        if(*it == rCaretPos.pSelectedNode)
        {
            if((*it)->GetType() == NTEXT)
            {
                //Split textnode if needed
                if(rCaretPos.Index > 0)
                {
                    SmTextNode* pText = static_cast<SmTextNode*>(rCaretPos.pSelectedNode);
                    if (rCaretPos.Index == pText->GetText().getLength())
                        return ++it;
                    OUString str1 = pText->GetText().copy(0, rCaretPos.Index);
                    OUString str2 = pText->GetText().copy(rCaretPos.Index);
                    pText->ChangeText(str1);
                    ++it;
                    //Insert str2 as new text node
                    assert(!str2.isEmpty());
                    SmTextNode* pNewText = new SmTextNode(pText->GetToken(), pText->GetFontDesc());
                    pNewText->ChangeText(str2);
                    it = pLineList->insert(it, pNewText);
                }
            }else
                ++it;
            //it now pointer to the node following pos, so pLineList->insert(it, ...) will insert correctly
            return it;

        }
    }
    //If we didn't find pSelectedNode, it must be because the caret is in front of the line
    return pLineList->begin();
}

SmCaretPos SmCursor::PatchLineList(SmNodeList* pLineList, SmNodeList::iterator aIter) {
    //The nodes we should consider merging
    SmNode *prev = nullptr,
           *next = nullptr;
    if(aIter != pLineList->end())
        next = *aIter;
    if(aIter != pLineList->begin()) {
        --aIter;
        prev = *aIter;
        ++aIter;
    }

    //Check if there's textnodes to merge
    if( prev &&
        next &&
        prev->GetType() == NTEXT &&
        next->GetType() == NTEXT &&
        ( prev->GetToken().eType != TNUMBER ||
          next->GetToken().eType == TNUMBER) ){
        SmTextNode *pText = static_cast<SmTextNode*>(prev),
                   *pOldN = static_cast<SmTextNode*>(next);
        SmCaretPos retval(pText, pText->GetText().getLength());
        OUString newText;
        newText += pText->GetText();
        newText += pOldN->GetText();
        pText->ChangeText(newText);
        delete pOldN;
        pLineList->erase(aIter);
        return retval;
    }

    //Check if there's a SmPlaceNode to remove:
    if(prev && next && prev->GetType() == NPLACE && !SmNodeListParser::IsOperator(next->GetToken())){
        --aIter;
        aIter = pLineList->erase(aIter);
        delete prev;
        //Return caret pos in front of aIter
        if(aIter != pLineList->begin())
            --aIter; //Thus find node before aIter
        if(aIter == pLineList->begin())
            return SmCaretPos();
        if((*aIter)->GetType() == NTEXT)
            return SmCaretPos(*aIter, static_cast<SmTextNode*>(*aIter)->GetText().getLength());
        return SmCaretPos(*aIter, 1);
    }
    if(prev && next && next->GetType() == NPLACE && !SmNodeListParser::IsOperator(prev->GetToken())){
        aIter = pLineList->erase(aIter);
        delete next;
        if(prev->GetType() == NTEXT)
            return SmCaretPos(prev, static_cast<SmTextNode*>(prev)->GetText().getLength());
        return SmCaretPos(prev, 1);
    }

    //If we didn't do anything return
    if(!prev) //return an invalid to indicate we're in front of line
        return SmCaretPos();
    if(prev->GetType() == NTEXT)
        return SmCaretPos(prev, static_cast<SmTextNode*>(prev)->GetText().getLength());
    return SmCaretPos(prev, 1);
}

SmNodeList::iterator SmCursor::TakeSelectedNodesFromList(SmNodeList *pLineList,
                                                         SmNodeList *pSelectedNodes) {
    SmNodeList::iterator retval;
    SmNodeList::iterator it = pLineList->begin();
    while(it != pLineList->end()){
        if((*it)->IsSelected()){
            //Split text nodes
            if((*it)->GetType() == NTEXT) {
                SmTextNode* pText = static_cast<SmTextNode*>(*it);
                OUString aText = pText->GetText();
                //Start and lengths of the segments, 2 is the selected segment
                int start2 = pText->GetSelectionStart(),
                    start3 = pText->GetSelectionEnd(),
                    len1 = start2 - 0,
                    len2 = start3 - start2,
                    len3 = aText.getLength() - start3;
                SmToken aToken = pText->GetToken();
                sal_uInt16 eFontDesc = pText->GetFontDesc();
                //If we need make segment 1
                if(len1 > 0) {
                    int start1 = 0;
                    OUString str = aText.copy(start1, len1);
                    pText->ChangeText(str);
                    ++it;
                } else {//Remove it if not needed
                    it = pLineList->erase(it);
                    delete pText;
                }
                //Set retval to be right after the selection
                retval = it;
                //if we need make segment 3
                if(len3 > 0) {
                    OUString str = aText.copy(start3, len3);
                    SmTextNode* pSeg3 = new SmTextNode(aToken, eFontDesc);
                    pSeg3->ChangeText(str);
                    retval = pLineList->insert(it, pSeg3);
                }
                //If we need to save the selected text
                if(pSelectedNodes && len2 > 0) {
                    OUString str = aText.copy(start2, len2);
                    SmTextNode* pSeg2 = new SmTextNode(aToken, eFontDesc);
                    pSeg2->ChangeText(str);
                    pSelectedNodes->push_back(pSeg2);
                }
            } else { //if it's not textnode
                SmNode* pNode = *it;
                retval = it = pLineList->erase(it);
                if(pSelectedNodes)
                    pSelectedNodes->push_back(pNode);
                else
                    delete pNode;
            }
        } else
            ++it;
    }
    return retval;
}

void SmCursor::InsertSubSup(SmSubSup eSubSup) {
    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        OSL_ENSURE(pSNode != nullptr, "There must be a selected node when HasSelection is true!");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);

    //Find Parent and offset in parent
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //TODO: Consider handling special cases where parent is an SmOperNode,
    //      Maybe this method should be able to add limits to an SmOperNode...

    //We begin modifying the tree here
    BeginEdit();

    //Convert line to list
    SmNodeList* pLineList = NodeToList(pLine);

    //Take the selection, and/or find iterator for current position
    SmNodeList* pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, mpPosition->CaretPos);

    //Find node that this should be applied to
    SmNode* pSubject;
    bool bPatchLine = pSelectedNodesList->size() > 0; //If the line should be patched later
    if(it != pLineList->begin()) {
        --it;
        pSubject = *it;
        ++it;
    } else {
        //Create a new place node
        pSubject = new SmPlaceNode();
        pSubject->Prepare(mpDocShell->GetFormat(), *mpDocShell);
        it = pLineList->insert(it, pSubject);
        ++it;
        bPatchLine = true;  //We've modified the line it should be patched later.
    }

    //Wrap the subject in a SmSubSupNode
    SmSubSupNode* pSubSup;
    if(pSubject->GetType() != NSUBSUP){
        SmToken token;
        token.nGroup = TG::Power;
        pSubSup = new SmSubSupNode(token);
        pSubSup->SetBody(pSubject);
        *(--it) = pSubSup;
        ++it;
    }else
        pSubSup = static_cast<SmSubSupNode*>(pSubject);
    //pSubject shouldn't be referenced anymore, pSubSup is the SmSubSupNode in pLineList we wish to edit.
    //and it pointer to the element following pSubSup in pLineList.
    pSubject = nullptr;

    //Patch the line if we noted that was needed previously
    if(bPatchLine)
        PatchLineList(pLineList, it);

    //Convert existing, if any, sub-/superscript line to list
    SmNode *pScriptLine = pSubSup->GetSubSup(eSubSup);
    SmNodeList* pScriptLineList = NodeToList(pScriptLine);

    //Add selection to pScriptLineList
    unsigned int nOldSize = pScriptLineList->size();
    pScriptLineList->insert(pScriptLineList->end(), pSelectedNodesList->begin(), pSelectedNodesList->end());
    delete pSelectedNodesList;
    pSelectedNodesList = nullptr;

    //Patch pScriptLineList if needed
    if(0 < nOldSize && nOldSize < pScriptLineList->size()) {
        SmNodeList::iterator iPatchPoint = pScriptLineList->begin();
        std::advance(iPatchPoint, nOldSize);
        PatchLineList(pScriptLineList, iPatchPoint);
    }

    //Find caret pos, that should be used after sub-/superscription.
    SmCaretPos PosAfterScript; //Leave invalid for first position
    if(pScriptLineList->size() > 0)
        PosAfterScript = SmCaretPos::GetPosAfter(pScriptLineList->back());

    //Parse pScriptLineList
    pScriptLine = SmNodeListParser().Parse(pScriptLineList);
    delete pScriptLineList;
    pScriptLineList = nullptr;

    //Insert pScriptLine back into the tree
    pSubSup->SetSubSup(eSubSup, pScriptLine);

    //Finish editing
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterScript, pScriptLine);
}

bool SmCursor::InsertLimit(SmSubSup eSubSup) {
    //Find a subject to set limits on
    SmOperNode *pSubject = nullptr;
    //Check if pSelectedNode might be a subject
    if(mpPosition->CaretPos.pSelectedNode->GetType() == NOPER)
        pSubject = static_cast<SmOperNode*>(mpPosition->CaretPos.pSelectedNode);
    else {
        //If not, check if parent of the current line is a SmOperNode
        SmNode *pLineNode = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);
        if(pLineNode->GetParent() && pLineNode->GetParent()->GetType() == NOPER)
            pSubject = static_cast<SmOperNode*>(pLineNode->GetParent());
    }

    //Abort operation if we're not in the appropriate context
    if(!pSubject)
        return false;

    BeginEdit();

    //Find the sub sup node
    SmSubSupNode *pSubSup = nullptr;
    //Check if there's already one there...
    if(pSubject->GetSubNode(0)->GetType() == NSUBSUP)
        pSubSup = static_cast<SmSubSupNode*>(pSubject->GetSubNode(0));
    else { //if not create a new SmSubSupNode
        SmToken token;
        token.nGroup = TG::Limit;
        pSubSup = new SmSubSupNode(token);
        //Set it's body
        pSubSup->SetBody(pSubject->GetSubNode(0));
        //Replace the operation of the SmOperNode
        pSubject->SetSubNode(0, pSubSup);
    }

    //Create the limit, if needed
    SmCaretPos PosAfterLimit;
    SmNode *pLine = nullptr;
    if(!pSubSup->GetSubSup(eSubSup)){
        pLine = new SmPlaceNode();
        pSubSup->SetSubSup(eSubSup, pLine);
        PosAfterLimit = SmCaretPos(pLine, 1);
    //If it's already there... let's move the caret
    } else {
        pLine = pSubSup->GetSubSup(eSubSup);
        SmNodeList* pLineList = NodeToList(pLine);
        if(pLineList->size() > 0)
            PosAfterLimit = SmCaretPos::GetPosAfter(pLineList->back());
        pLine = SmNodeListParser().Parse(pLineList);
        delete pLineList;
        pSubSup->SetSubSup(eSubSup, pLine);
    }

    //Rebuild graph of caret positions
    BuildGraph();
    AnnotateSelection();

    //Set caret position
    if(!SetCaretPosition(PosAfterLimit))
        SetCaretPosition(SmCaretPos(pLine, 0));

    EndEdit();

    return true;
}

void SmCursor::InsertBrackets(SmBracketType eBracketType) {
    BeginEdit();

    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        OSL_ENSURE(pSNode != nullptr, "There must be a selected node if HasSelection()");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);

    //Find parent and offset in parent
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //Convert line to list
    SmNodeList *pLineList = NodeToList(pLine);

    //Take the selection, and/or find iterator for current position
    SmNodeList *pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, mpPosition->CaretPos);

    //If there's no selected nodes, create a place node
    SmNode *pBodyNode;
    SmCaretPos PosAfterInsert;
    if(pSelectedNodesList->empty()) {
        pBodyNode = new SmPlaceNode();
        PosAfterInsert = SmCaretPos(pBodyNode, 1);
    } else
        pBodyNode = SmNodeListParser().Parse(pSelectedNodesList);

    delete pSelectedNodesList;

    //Create SmBraceNode
    SmToken aTok(TLEFT, '\0', "left", TG::NONE, 5);
    SmBraceNode *pBrace = new SmBraceNode(aTok);
    pBrace->SetScaleMode(SCALE_HEIGHT);
    SmNode *pLeft = CreateBracket(eBracketType, true),
           *pRight = CreateBracket(eBracketType, false);
    SmBracebodyNode *pBody = new SmBracebodyNode(SmToken());
    pBody->SetSubNodes(pBodyNode, nullptr);
    pBrace->SetSubNodes(pLeft, pBody, pRight);
    pBrace->Prepare(mpDocShell->GetFormat(), *mpDocShell);

    //Insert into line
    pLineList->insert(it, pBrace);
    //Patch line (I think this is good enough)
    SmCaretPos aAfter = PatchLineList(pLineList, it);
    if( !PosAfterInsert.IsValid() )
        PosAfterInsert = aAfter;

    //Finish editing
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
}

SmNode *SmCursor::CreateBracket(SmBracketType eBracketType, bool bIsLeft) {
    SmToken aTok;
    if(bIsLeft){
        switch(eBracketType){
            case NoneBrackets:
                aTok = SmToken(TNONE, '\0', "none", TG::LBrace | TG::RBrace, 0);
                break;
            case RoundBrackets:
                aTok = SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
                break;
            case SquareBrackets:
                aTok = SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
                break;
            case DoubleSquareBrackets:
                aTok = SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TG::LBrace, 5);
                break;
            case LineBrackets:
                aTok = SmToken(TLLINE, MS_VERTLINE, "lline", TG::LBrace, 5);
                break;
            case DoubleLineBrackets:
                aTok = SmToken(TLDLINE, MS_DVERTLINE, "ldline", TG::LBrace, 5);
                break;
            case CurlyBrackets:
                aTok = SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
                break;
            case AngleBrackets:
                aTok = SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
                break;
            case CeilBrackets:
                aTok = SmToken(TLCEIL, MS_LCEIL, "lceil", TG::LBrace, 5);
                break;
            case FloorBrackets:
                aTok = SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TG::LBrace, 5);
                break;
        }
    } else {
        switch(eBracketType) {
            case NoneBrackets:
                aTok = SmToken(TNONE, '\0', "none", TG::LBrace | TG::RBrace, 0);
                break;
            case RoundBrackets:
                aTok = SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
                break;
            case SquareBrackets:
                aTok = SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
                break;
            case DoubleSquareBrackets:
                aTok = SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TG::RBrace, 5);
                break;
            case LineBrackets:
                aTok = SmToken(TRLINE, MS_VERTLINE, "rline", TG::RBrace, 5);
                break;
            case DoubleLineBrackets:
                aTok = SmToken(TRDLINE, MS_DVERTLINE, "rdline", TG::RBrace, 5);
                break;
            case CurlyBrackets:
                aTok = SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
                break;
            case AngleBrackets:
                aTok = SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
                break;
            case CeilBrackets:
                aTok = SmToken(TRCEIL, MS_RCEIL, "rceil", TG::RBrace, 5);
                break;
            case FloorBrackets:
                aTok = SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TG::RBrace, 5);
                break;
        }
    }
    SmNode* pRetVal = new SmMathSymbolNode(aTok);
    pRetVal->SetScaleMode(SCALE_HEIGHT);
    return pRetVal;
}

bool SmCursor::InsertRow() {
    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        OSL_ENSURE(pSNode != nullptr, "There must be a selected node if HasSelection()");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);

    //Find parent and offset in parent
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //Discover the context of this command
    SmTableNode  *pTable  = nullptr;
    SmMatrixNode *pMatrix = nullptr;
    int nTableIndex = nParentIndex;
    if(pLineParent->GetType() == NTABLE)
        pTable = static_cast<SmTableNode*>(pLineParent);
    //If it's wrapped in a SmLineNode, we can still insert a newline
    else if(pLineParent->GetType() == NLINE &&
            pLineParent->GetParent() &&
            pLineParent->GetParent()->GetType() == NTABLE) {
        //NOTE: This hack might give problems if we stop ignoring SmAlignNode
        pTable = static_cast<SmTableNode*>(pLineParent->GetParent());
        nTableIndex = pTable->IndexOfSubNode(pLineParent);
        assert(nTableIndex >= 0);
    }
    if(pLineParent->GetType() == NMATRIX)
        pMatrix = static_cast<SmMatrixNode*>(pLineParent);

    //If we're not in a context that supports InsertRow, return sal_False
    if(!pTable && !pMatrix)
        return false;

    //Now we start editing
    BeginEdit();

    //Convert line to list
    SmNodeList *pLineList = NodeToList(pLine);

    //Find position in line
    SmNodeList::iterator it;
    if(HasSelection()) {
        //Take the selected nodes and delete them...
        it = TakeSelectedNodesFromList(pLineList);
    } else
        it = FindPositionInLineList(pLineList, mpPosition->CaretPos);

    //New caret position after inserting the newline/row in whatever context
    SmCaretPos PosAfterInsert;

    //If we're in the context of a table
    if(pTable) {
        SmNodeList *pNewLineList = new SmNodeList();
        //Move elements from pLineList to pNewLineList
        pNewLineList->splice(pNewLineList->begin(), *pLineList, it, pLineList->end());
        //Make sure it is valid again
        it = pLineList->end();
        if(it != pLineList->begin())
            --it;
        if(pNewLineList->empty())
            pNewLineList->push_front(new SmPlaceNode());
        //Parse new line
        SmNode *pNewLine = SmNodeListParser().Parse(pNewLineList);
        delete pNewLineList;
        //Wrap pNewLine in SmLineNode if needed
        if(pLineParent->GetType() == NLINE) {
            SmLineNode *pNewLineNode = new SmLineNode(SmToken(TNEWLINE, '\0', "newline"));
            pNewLineNode->SetSubNodes(pNewLine, nullptr);
            pNewLine = pNewLineNode;
        }
        //Get position
        PosAfterInsert = SmCaretPos(pNewLine, 0);
        //Move other nodes if needed
        for( int i = pTable->GetNumSubNodes(); i > nTableIndex + 1; i--)
            pTable->SetSubNode(i, pTable->GetSubNode(i-1));

        //Insert new line
        pTable->SetSubNode(nTableIndex + 1, pNewLine);

        //Check if we need to change token type:
        if(pTable->GetNumSubNodes() > 2 && pTable->GetToken().eType == TBINOM) {
            SmToken tok = pTable->GetToken();
            tok.eType = TSTACK;
            pTable->SetToken(tok);
        }
    }
    //If we're in the context of a matrix
    else {
        //Find position after insert and patch the list
        PosAfterInsert = PatchLineList(pLineList, it);
        //Move other children
        sal_uInt16 rows = pMatrix->GetNumRows();
        sal_uInt16 cols = pMatrix->GetNumCols();
        int nRowStart = (nParentIndex - nParentIndex % cols) + cols;
        for( int i = pMatrix->GetNumSubNodes() + cols - 1; i >= nRowStart + cols; i--)
            pMatrix->SetSubNode(i, pMatrix->GetSubNode(i - cols));
        for( int i = nRowStart; i < nRowStart + cols; i++) {
            SmPlaceNode *pNewLine = new SmPlaceNode();
            if(i == nParentIndex + cols)
                PosAfterInsert = SmCaretPos(pNewLine, 0);
            pMatrix->SetSubNode(i, pNewLine);
        }
        pMatrix->SetRowCol(rows + 1, cols);
    }

    //Finish editing
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
    //FinishEdit is actually used to handle siturations where parent is an instance of
    //SmSubSupNode. In this case parent should always be a table or matrix, however, for
    //code reuse we just use FinishEdit() here too.
    return true;
}

void SmCursor::InsertFraction() {
    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        OSL_ENSURE(pSNode != nullptr, "There must be a selected node when HasSelection is true!");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);

    //Find Parent and offset in parent
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //We begin modifying the tree here
    BeginEdit();

    //Convert line to list
    SmNodeList* pLineList = NodeToList(pLine);

    //Take the selection, and/or find iterator for current position
    SmNodeList* pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, mpPosition->CaretPos);

    //Create pNum, and pDenom
    bool bEmptyFraction = pSelectedNodesList->empty();
    SmNode *pNum = bEmptyFraction
        ? new SmPlaceNode()
        : SmNodeListParser().Parse(pSelectedNodesList);
    SmNode *pDenom = new SmPlaceNode();
    delete pSelectedNodesList;
    pSelectedNodesList = nullptr;

    //Create new fraction
    SmBinVerNode *pFrac = new SmBinVerNode(SmToken(TOVER, '\0', "over", TG::Product, 0));
    SmNode *pRect = new SmRectangleNode(SmToken());
    pFrac->SetSubNodes(pNum, pRect, pDenom);

    //Insert in pLineList
    SmNodeList::iterator patchIt = pLineList->insert(it, pFrac);
    PatchLineList(pLineList, patchIt);
    PatchLineList(pLineList, it);

    //Finish editing
    SmNode *pSelectedNode = bEmptyFraction ? pNum : pDenom;
    FinishEdit(pLineList, pLineParent, nParentIndex, SmCaretPos(pSelectedNode, 1));
}

void SmCursor::InsertText(const OUString& aString)
{
    BeginEdit();

    Delete();

    SmToken token;
    token.eType = TIDENT;
    token.cMathChar = '\0';
    token.nGroup = TG::NONE;
    token.nLevel = 5;
    token.aText = aString;

    SmTextNode* pText = new SmTextNode(token, FNT_VARIABLE);
    pText->SetText(aString);
    pText->AdjustFontDesc();
    pText->Prepare(mpDocShell->GetFormat(), *mpDocShell);

    SmNodeList* pList = new SmNodeList();
    pList->push_front(pText);
    InsertNodes(pList);

    EndEdit();
}

void SmCursor::InsertElement(SmFormulaElement element){
    BeginEdit();

    Delete();

    //Create new node
    SmNode* pNewNode = nullptr;
    switch(element){
        case BlankElement:
        {
            SmToken token;
            token.nGroup = TG::Blank;
            token.aText = "~";
            pNewNode = new SmBlankNode(token);
        }break;
        case FactorialElement:
        {
            SmToken token(TFACT, MS_FACT, "fact", TG::UnOper, 5);
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case PlusElement:
        {
            SmToken token;
            token.eType = TPLUS;
            token.cMathChar = MS_PLUS;
            token.nGroup = TG::UnOper | TG::Sum;
            token.nLevel = 5;
            token.aText = "+";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case MinusElement:
        {
            SmToken token;
            token.eType = TMINUS;
            token.cMathChar = MS_MINUS;
            token.nGroup = TG::UnOper | TG::Sum;
            token.nLevel = 5;
            token.aText = "-";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case CDotElement:
        {
            SmToken token;
            token.eType = TCDOT;
            token.cMathChar = MS_CDOT;
            token.nGroup = TG::Product;
            token.aText = "cdot";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case EqualElement:
        {
            SmToken token;
            token.eType = TASSIGN;
            token.cMathChar = MS_ASSIGN;
            token.nGroup = TG::Relation;
            token.aText = "=";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case LessThanElement:
        {
            SmToken token;
            token.eType = TLT;
            token.cMathChar = MS_LT;
            token.nGroup = TG::Relation;
            token.aText = "<";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case GreaterThanElement:
        {
            SmToken token;
            token.eType = TGT;
            token.cMathChar = MS_GT;
            token.nGroup = TG::Relation;
            token.aText = ">";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case PercentElement:
        {
            SmToken token;
            token.eType = TTEXT;
            token.cMathChar = MS_PERCENT;
            token.nGroup = TG::NONE;
            token.aText = "\"%\"";
            pNewNode = new SmMathSymbolNode(token);
        }break;
    }
    assert(pNewNode);

    //Prepare the new node
    pNewNode->Prepare(mpDocShell->GetFormat(), *mpDocShell);

    //Insert new node
    SmNodeList* pList = new SmNodeList();
    pList->push_front(pNewNode);
    InsertNodes(pList);

    EndEdit();
}

void SmCursor::InsertSpecial(const OUString& _aString)
{
    BeginEdit();
    Delete();

    OUString aString = comphelper::string::strip(_aString, ' ');

    //Create instance of special node
    SmToken token;
    token.eType = TSPECIAL;
    token.cMathChar = '\0';
    token.nGroup = TG::NONE;
    token.nLevel = 5;
    token.aText = aString;
    SmSpecialNode* pSpecial = new SmSpecialNode(token);

    //Prepare the special node
    pSpecial->Prepare(mpDocShell->GetFormat(), *mpDocShell);

    //Insert the node
    SmNodeList* pList = new SmNodeList();
    pList->push_front(pSpecial);
    InsertNodes(pList);

    EndEdit();
}

void SmCursor::InsertCommand(sal_uInt16 nCommand) {
    switch(nCommand){
        case RID_NEWLINE:
            InsertRow();
            break;
        case RID_FROMX:
            InsertLimit(CSUB);
            break;
        case RID_TOX:
            InsertLimit(CSUP);
            break;
        case RID_FROMXTOY:
            if(InsertLimit(CSUB))
                InsertLimit(CSUP);
            break;
        default:
            InsertCommandText(SM_RESSTR(nCommand));
            break;
    }
}

void SmCursor::InsertCommandText(const OUString& aCommandText) {
    //Parse the sub expression
    SmNode* pSubExpr = SmParser().ParseExpression(aCommandText);

    //Prepare the subtree
    pSubExpr->Prepare(mpDocShell->GetFormat(), *mpDocShell);

    //Convert subtree to list
    SmNodeList* pLineList = NodeToList(pSubExpr);

    BeginEdit();

    //Delete any selection
    Delete();

    //Insert it
    InsertNodes(pLineList);

    EndEdit();
}

void SmCursor::Copy(){
    if(!HasSelection())
        return;

    AnnotateSelection();
    //Find selected node
    SmNode* pSNode = FindSelectedNode(mpTree);
    //Find visual line
    SmNode* pLine = FindTopMostNodeInLine(pSNode, true);
    assert(pLine);

    //Clone selected nodes
    SmClipboard aClipboard;
    if(IsLineCompositionNode(pLine))
        CloneLineToClipboard(static_cast<SmStructureNode*>(pLine), &aClipboard);
    else{
        //Special care to only clone selected text
        if(pLine->GetType() == NTEXT) {
            SmTextNode *pText = static_cast<SmTextNode*>(pLine);
            std::unique_ptr<SmTextNode> pClone(new SmTextNode( pText->GetToken(), pText->GetFontDesc() ));
            int start  = pText->GetSelectionStart(),
                length = pText->GetSelectionEnd() - pText->GetSelectionStart();
            pClone->ChangeText(pText->GetText().copy(start, length));
            pClone->SetScaleMode(pText->GetScaleMode());
            aClipboard.push_front(std::move(pClone));
        } else {
            SmCloningVisitor aCloneFactory;
            aClipboard.push_front(std::unique_ptr<SmNode>(aCloneFactory.Clone(pLine)));
        }
    }

    //Set clipboard
    if (aClipboard.size() > 0)
        maClipboard = std::move(aClipboard);
}

void SmCursor::Paste() {
    BeginEdit();
    Delete();

    if(maClipboard.size() > 0)
        InsertNodes(CloneList(maClipboard));

    EndEdit();
}

SmNodeList* SmCursor::CloneList(SmClipboard &rClipboard){
    SmCloningVisitor aCloneFactory;
    SmNodeList* pClones = new SmNodeList();

    for(auto &xNode : rClipboard){
        SmNode *pClone = aCloneFactory.Clone(xNode.get());
        pClones->push_back(pClone);
    }

    return pClones;
}

SmNode* SmCursor::FindTopMostNodeInLine(SmNode* pSNode, bool MoveUpIfSelected){
    //If we haven't got a subnode
    if(!pSNode)
        return nullptr;

    //Move up parent until we find a node who's
    //parent is NULL or isn't selected and not a type of:
    //      SmExpressionNode
    //      SmLineNode
    //      SmBinHorNode
    //      SmUnHorNode
    //      SmAlignNode
    //      SmFontNode
    while(pSNode->GetParent() &&
          ((MoveUpIfSelected &&
            pSNode->GetParent()->IsSelected()) ||
           IsLineCompositionNode(pSNode->GetParent())))
        pSNode = pSNode->GetParent();
    //Now we have the selection line node
    return pSNode;
}

SmNode* SmCursor::FindSelectedNode(SmNode* pNode){
    SmNodeIterator it(pNode);
    while(it.Next()){
        if(it->IsSelected())
            return it.Current();
        SmNode* pRetVal = FindSelectedNode(it.Current());
        if(pRetVal)
            return pRetVal;
    }
    return nullptr;
}

SmNodeList* SmCursor::LineToList(SmStructureNode* pLine, SmNodeList* list){
    SmNodeIterator it(pLine);
    while(it.Next()){
        switch(it->GetType()){
            case NLINE:
            case NUNHOR:
            case NEXPRESSION:
            case NBINHOR:
            case NALIGN:
            case NFONT:
                LineToList(static_cast<SmStructureNode*>(it.Current()), list);
                break;
            case NERROR:
                delete it.Current();
                break;
            default:
                list->push_back(it.Current());
        }
    }
    SmNodeArray emptyArray(0);
    pLine->SetSubNodes(emptyArray);
    delete pLine;
    return list;
}

void SmCursor::CloneLineToClipboard(SmStructureNode* pLine, SmClipboard* pClipboard){
    SmCloningVisitor aCloneFactory;
    SmNodeIterator it(pLine);
    while(it.Next()){
        if( IsLineCompositionNode( it.Current() ) )
            CloneLineToClipboard( static_cast<SmStructureNode*>(it.Current()), pClipboard );
        else if( it->IsSelected() && it->GetType() != NERROR ) {
            //Only clone selected text from SmTextNode
            if(it->GetType() == NTEXT) {
                SmTextNode *pText = static_cast<SmTextNode*>(it.Current());
                std::unique_ptr<SmTextNode> pClone(new SmTextNode( it->GetToken(), pText->GetFontDesc() ));
                int start = pText->GetSelectionStart(),
                    length = pText->GetSelectionEnd() - pText->GetSelectionStart();
                pClone->ChangeText(pText->GetText().copy(start, length));
                pClone->SetScaleMode(pText->GetScaleMode());
                pClipboard->push_back(std::move(pClone));
            } else
                pClipboard->push_back(std::unique_ptr<SmNode>(aCloneFactory.Clone(it.Current())));
        }
    }
}

bool SmCursor::IsLineCompositionNode(SmNode* pNode){
    switch(pNode->GetType()){
        case NLINE:
        case NUNHOR:
        case NEXPRESSION:
        case NBINHOR:
        case NALIGN:
        case NFONT:
            return true;
        default:
            return false;
    }
}

int SmCursor::CountSelectedNodes(SmNode* pNode){
    int nCount = 0;
    SmNodeIterator it(pNode);
    while(it.Next()){
        if(it->IsSelected() && !IsLineCompositionNode(it.Current()))
            nCount++;
        nCount += CountSelectedNodes(it.Current());
    }
    return nCount;
}

bool SmCursor::HasComplexSelection(){
    if(!HasSelection())
        return false;
    AnnotateSelection();

    return CountSelectedNodes(mpTree) > 1;
}

void SmCursor::FinishEdit(SmNodeList* pLineList,
                          SmStructureNode* pParent,
                          int nParentIndex,
                          SmCaretPos PosAfterEdit,
                          SmNode* pStartLine) {
    //Store number of nodes in line for later
    int entries = pLineList->size();

    //Parse list of nodes to a tree
    SmNodeListParser parser;
    SmNode* pLine = parser.Parse(pLineList);
    delete pLineList;

    //Check if we're making the body of a subsup node bigger than one
    if(pParent->GetType() == NSUBSUP &&
       nParentIndex == 0 &&
       entries > 1) {
        //Wrap pLine in scalable round brackets
        SmToken aTok(TLEFT, '\0', "left", TG::NONE, 5);
        SmBraceNode *pBrace = new SmBraceNode(aTok);
        pBrace->SetScaleMode(SCALE_HEIGHT);
        SmNode *pLeft  = CreateBracket(RoundBrackets, true),
               *pRight = CreateBracket(RoundBrackets, false);
        SmBracebodyNode *pBody = new SmBracebodyNode(SmToken());
        pBody->SetSubNodes(pLine, nullptr);
        pBrace->SetSubNodes(pLeft, pBody, pRight);
        pBrace->Prepare(mpDocShell->GetFormat(), *mpDocShell);
        pLine = pBrace;
        //TODO: Consider the following alternative behavior:
        //Consider the line: A + {B + C}^D lsub E
        //Here pLineList is B, + and C and pParent is a subsup node with
        //both RSUP and LSUB set. Imagine the user just inserted "B +" in
        //the body of the subsup node...
        //The most natural thing to do would be to make the line like this:
        //A + B lsub E + C ^ D
        //E.g. apply LSUB and LSUP to the first element in pLineList and RSUP
        //and RSUB to the last element in pLineList. But how should this act
        //for CSUP and CSUB ???
        //For this reason and because brackets was faster to implement, this solution
        //have been chosen. It might be worth working on the other solution later...
    }

    //Set pStartLine if NULL
    if(!pStartLine)
        pStartLine = pLine;

    //Insert it back into the parent
    pParent->SetSubNode(nParentIndex, pLine);

    //Rebuild graph of caret position
    mpAnchor = nullptr;
    mpPosition = nullptr;
    BuildGraph();
    AnnotateSelection(); //Update selection annotation!

    //Set caret position
    if(!SetCaretPosition(PosAfterEdit))
        SetCaretPosition(SmCaretPos(pStartLine, 0));

    //End edit section
    EndEdit();
}

void SmCursor::BeginEdit(){
    if(mnEditSections++ > 0) return;

    mbIsEnabledSetModifiedSmDocShell = mpDocShell->IsEnableSetModified();
    if( mbIsEnabledSetModifiedSmDocShell )
        mpDocShell->EnableSetModified( false );
}

void SmCursor::EndEdit(){
    if(--mnEditSections > 0) return;

    mpDocShell->SetFormulaArranged(false);
    //Okay, I don't know what this does... :)
    //It's used in SmDocShell::SetText and with places where everything is modified.
    //I think it does some magic, with sfx, but everything is totally undocumented so
    //it's kinda hard to tell...
    if ( mbIsEnabledSetModifiedSmDocShell )
        mpDocShell->EnableSetModified( mbIsEnabledSetModifiedSmDocShell );
    //I think this notifies people around us that we've modified this document...
    mpDocShell->SetModified();
    //I think SmDocShell uses this value when it sends an update graphics event
    //Anyway comments elsewhere suggests it need to be updated...
    mpDocShell->mnModifyCount++;

    //TODO: Consider copying the update accessibility code from SmDocShell::SetText in here...
    //This somehow updates the size of SmGraphicView if it is running in embedded mode
    if( mpDocShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
        mpDocShell->OnDocumentPrinterChanged(nullptr);

    //Request a repaint...
    RequestRepaint();

    //Update the edit engine and text of the document
    OUString formula;
    SmNodeToTextVisitor(mpTree, formula);
    //mpTree->CreateTextFromNode(formula);
    mpDocShell->maText = formula;
    mpDocShell->GetEditEngine().QuickInsertText( formula, ESelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL ) );
    mpDocShell->GetEditEngine().QuickFormatDoc();
}

void SmCursor::RequestRepaint(){
    SmViewShell *pViewSh = SmGetActiveView();
    if( pViewSh ) {
        if ( SfxObjectCreateMode::EMBEDDED == mpDocShell->GetCreateMode() )
            mpDocShell->Repaint();
        else
            pViewSh->GetGraphicWindow().Invalidate();
    }
}

bool SmCursor::IsAtTailOfBracket(SmBracketType eBracketType, SmBraceNode** ppBraceNode) const {
    const SmCaretPos pos = GetPosition();
    if (!pos.IsValid()) {
        return false;
    }

    SmNode* pNode = pos.pSelectedNode;

    if (pNode->GetType() == NTEXT) {
        SmTextNode* pTextNode = static_cast<SmTextNode*>(pNode);
        if (pos.Index < pTextNode->GetText().getLength()) {
            // The cursor is on a text node and at the middle of it.
            return false;
        }
    } else {
        if (pos.Index < 1) {
            return false;
        }
    }

    while (true) {
        SmStructureNode* pParentNode = pNode->GetParent();
        if (!pParentNode) {
            // There's no brace body node in the ancestors.
            return false;
        }

        int index = pParentNode->IndexOfSubNode(pNode);
        assert(index >= 0);
        if (index + 1 != pParentNode->GetNumSubNodes()) {
            // The cursor is not at the tail at one of ancestor nodes.
            return false;
        }

        pNode = pParentNode;
        if (pNode->GetType() == NBRACEBODY) {
            // Found the brace body node.
            break;
        }
    }

    SmStructureNode* pBraceNodeTmp = pNode->GetParent();
    if (!pBraceNodeTmp || pBraceNodeTmp->GetType() != NBRACE) {
        // Brace node is invalid.
        return false;
    }

    SmBraceNode* pBraceNode = static_cast<SmBraceNode*>(pBraceNodeTmp);
    SmMathSymbolNode* pClosingNode = pBraceNode->ClosingBrace();
    if (!pClosingNode) {
        // Couldn't get closing symbol node.
        return false;
    }

    // Check if the closing brace matches eBracketType.
    SmTokenType eClosingTokenType = pClosingNode->GetToken().eType;
    switch (eBracketType) {
    case NoneBrackets:         if (eClosingTokenType != TNONE)      { return false; } break;
    case RoundBrackets:        if (eClosingTokenType != TRPARENT)   { return false; } break;
    case SquareBrackets:       if (eClosingTokenType != TRBRACKET)  { return false; } break;
    case DoubleSquareBrackets: if (eClosingTokenType != TRDBRACKET) { return false; } break;
    case LineBrackets:         if (eClosingTokenType != TRLINE)     { return false; } break;
    case DoubleLineBrackets:   if (eClosingTokenType != TRDLINE)    { return false; } break;
    case CurlyBrackets:        if (eClosingTokenType != TRBRACE)    { return false; } break;
    case AngleBrackets:        if (eClosingTokenType != TRANGLE)    { return false; } break;
    case CeilBrackets:         if (eClosingTokenType != TRCEIL)     { return false; } break;
    case FloorBrackets:        if (eClosingTokenType != TRFLOOR)    { return false; } break;
    default:
        return false;
    }

    if (ppBraceNode) {
        *ppBraceNode = pBraceNode;
    }

    return true;
}

void SmCursor::MoveAfterBracket(SmBraceNode* pBraceNode)
{
    mpPosition->CaretPos.pSelectedNode = pBraceNode;
    mpPosition->CaretPos.Index = 1;
    mpAnchor->CaretPos.pSelectedNode = pBraceNode;
    mpAnchor->CaretPos.Index = 1;
    RequestRepaint();
}


/////////////////////////////////////// SmNodeListParser

SmNode* SmNodeListParser::Parse(SmNodeList* list){
    pList = list;
    //Delete error nodes
    SmNodeList::iterator it = pList->begin();
    while(it != pList->end()) {
        if((*it)->GetType() == NERROR){
            //Delete and erase
            delete *it;
            it = pList->erase(it);
        }else
            ++it;
    }
    SmNode* retval = Expression();
    pList = nullptr;
    return retval;
}

SmNode* SmNodeListParser::Expression(){
    SmNodeArray NodeArray;
    //Accept as many relations as there is
    while(Terminal())
        NodeArray.push_back(Relation());

    //Create SmExpressionNode, I hope SmToken() will do :)
    SmStructureNode* pExpr = new SmExpressionNode(SmToken());
    pExpr->SetSubNodes(NodeArray);
    return pExpr;
}

SmNode* SmNodeListParser::Relation(){
    //Read a sum
    SmNode* pLeft = Sum();
    //While we have tokens and the next is a relation
    while(Terminal() && IsRelationOperator(Terminal()->GetToken())){
        //Take the operator
        SmNode* pOper = Take();
        //Find the right side of the relation
        SmNode* pRight = Sum();
        //Create new SmBinHorNode
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Sum(){
    //Read a product
    SmNode* pLeft = Product();
    //While we have tokens and the next is a sum
    while(Terminal() && IsSumOperator(Terminal()->GetToken())){
        //Take the operator
        SmNode* pOper = Take();
        //Find the right side of the sum
        SmNode* pRight = Product();
        //Create new SmBinHorNode
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Product(){
    //Read a Factor
    SmNode* pLeft = Factor();
    //While we have tokens and the next is a product
    while(Terminal() && IsProductOperator(Terminal()->GetToken())){
        //Take the operator
        SmNode* pOper = Take();
        //Find the right side of the operation
        SmNode* pRight = Factor();
        //Create new SmBinHorNode
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Factor(){
    //Read unary operations
    if(!Terminal())
        return Error();
    //Take care of unary operators
    else if(IsUnaryOperator(Terminal()->GetToken()))
    {
        SmStructureNode *pUnary = new SmUnHorNode(SmToken());
        SmNode *pOper = Terminal(),
               *pArg;

        if(Next())
            pArg = Factor();
        else
            pArg = Error();

        pUnary->SetSubNodes(pOper, pArg);
        return pUnary;
    }
    return Postfix();
}

SmNode* SmNodeListParser::Postfix(){
    if(!Terminal())
        return Error();
    SmNode *pArg = nullptr;
    if(IsPostfixOperator(Terminal()->GetToken()))
        pArg = Error();
    else if(IsOperator(Terminal()->GetToken()))
        return Error();
    else
        pArg = Take();
    while(Terminal() && IsPostfixOperator(Terminal()->GetToken())) {
        SmStructureNode *pUnary = new SmUnHorNode(SmToken());
        SmNode *pOper = Take();
        pUnary->SetSubNodes(pArg, pOper);
        pArg = pUnary;
    }
    return pArg;
}

SmNode* SmNodeListParser::Error(){
    return new SmErrorNode(PE_UNEXPECTED_TOKEN, SmToken());
}

bool SmNodeListParser::IsOperator(const SmToken &token) {
    return  IsRelationOperator(token) ||
            IsSumOperator(token) ||
            IsProductOperator(token) ||
            IsUnaryOperator(token) ||
            IsPostfixOperator(token);
}

bool SmNodeListParser::IsRelationOperator(const SmToken &token) {
    return bool(token.nGroup & TG::Relation);
}

bool SmNodeListParser::IsSumOperator(const SmToken &token) {
    return bool(token.nGroup & TG::Sum);
}

bool SmNodeListParser::IsProductOperator(const SmToken &token) {
    return token.nGroup & TG::Product &&
           token.eType != TWIDESLASH &&
           token.eType != TWIDEBACKSLASH &&
           token.eType != TUNDERBRACE &&
           token.eType != TOVERBRACE &&
           token.eType != TOVER;
}

bool SmNodeListParser::IsUnaryOperator(const SmToken &token) {
    return  token.nGroup & TG::UnOper &&
            (token.eType == TPLUS ||
             token.eType == TMINUS ||
             token.eType == TPLUSMINUS ||
             token.eType == TMINUSPLUS ||
             token.eType == TNEG ||
             token.eType == TUOPER);
}

bool SmNodeListParser::IsPostfixOperator(const SmToken &token) {
    return token.eType == TFACT;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
