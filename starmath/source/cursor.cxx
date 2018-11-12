/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <memory>
#include <cursor.hxx>
#include <visitors.hxx>
#include <document.hxx>
#include <view.hxx>
#include <comphelper/string.hxx>
#include <editeng/editeng.hxx>
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

void SmCursor::MoveTo(OutputDevice* pDev, const Point& pos, bool bMoveAnchor)
{
    SmCaretPosGraphEntry* NewPos = nullptr;
    long dp_sq = 0,     //Distance to current line squared
         dbp_sq = 1;    //Distance to best line squared
    for(auto &pEntry : *mpGraph)
    {
        OSL_ENSURE(pEntry->CaretPos.IsValid(), "The caret position graph may not have invalid positions!");
        //Compute current line
        SmCaretLine curr_line = SmCaretPos2LineVisitor(pDev, pEntry->CaretPos).GetResult();
        //Compute squared distance to current line
        dp_sq = curr_line.SquaredDistanceX(pos) + curr_line.SquaredDistanceY(pos);
        //If we have a position compare to it
        if(NewPos){
            //If best line is closer, reject current line
            if(dbp_sq <= dp_sq) continue;
        }
        //Accept current position as the best
        NewPos = pEntry.get();
        //Update distance to best line
        dbp_sq = dp_sq;
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
    int nLineOffsetIdx = pLineParent->IndexOfSubNode(pLine);
    assert(nLineOffsetIdx >= 0);

    //If we're in front of a node who's parent is a TABLE
    if (pLineParent->GetType() == SmNodeType::Table && mpPosition->CaretPos.nIndex == 0 && nLineOffsetIdx > 0)
    {
        size_t nLineOffset = nLineOffsetIdx;
        //Now we can merge with nLineOffset - 1
        BeginEdit();
        //Line to merge things into, so we can delete pLine
        SmNode* pMergeLine = pLineParent->GetSubNode(nLineOffset-1);
        OSL_ENSURE(pMergeLine, "pMergeLine cannot be NULL!");
        SmCaretPos PosAfterDelete;
        //Convert first line to list
        std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
        NodeToList(pMergeLine, *pLineList);
        if(!pLineList->empty()){
            //Find iterator to patch
            SmNodeList::iterator patchPoint = pLineList->end();
            --patchPoint;
            //Convert second line to list
            NodeToList(pLine, *pLineList);
            //Patch the line list
            ++patchPoint;
            PosAfterDelete = PatchLineList(pLineList.get(), patchPoint);
            //Parse the line
            pLine = SmNodeListParser().Parse(pLineList.get());
        }
        pLineList.reset();
        pLineParent->SetSubNode(nLineOffset-1, pLine);
        //Delete the removed line slot
        SmNodeArray lines(pLineParent->GetNumSubNodes()-1);
        for (size_t i = 0; i < pLineParent->GetNumSubNodes(); ++i)
        {
            if(i < nLineOffset)
                lines[i] = pLineParent->GetSubNode(i);
            else if(i > nLineOffset)
                lines[i-1] = pLineParent->GetSubNode(i);
        }
        pLineParent->SetSubNodes(std::move(lines));
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
    /*}else if(pLineParent->GetType() == SmNodeType::SubSup &&
             nLineOffset != 0 &&
             pLine->GetType() == SmNodeType::Expression &&
             pLine->GetNumSubNodes() == 0){
        //There's a (sub/super) script we can delete
    //Consider selecting the entire script if GetNumSubNodes() != 0 or pLine->GetType() != SmNodeType::Expression
    //TODO: Handle case where we delete a limit
    */

    //Else move select, and delete if not complex
    }else{
        Move(pDev, MoveLeft, false);
        if(!HasComplexSelection())
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
    assert(pSNode);

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

    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Take the selected nodes and delete them...
    SmNodeList::iterator patchIt = TakeSelectedNodesFromList(pLineList.get());

    //Get the position to set after delete
    PosAfterDelete = PatchLineList(pLineList.get(), patchIt);

    //Finish editing
    FinishEdit(std::move(pLineList), pLineParent, nLineOffset, PosAfterDelete);
}

void SmCursor::InsertNodes(std::unique_ptr<SmNodeList> pNewNodes){
    if(pNewNodes->empty()){
        return;
    }

    //Begin edit section
    BeginEdit();

    //Get the current position
    const SmCaretPos pos = mpPosition->CaretPos;

    //Find top most of line that holds position
    SmNode* pLine = FindTopMostNodeInLine(pos.pSelectedNode);

    //Find line parent and line index in parent
    SmStructureNode* pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //Convert line to list
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Find iterator for place to insert nodes
    SmNodeList::iterator it = FindPositionInLineList(pLineList.get(), pos);

    //Insert all new nodes
    SmNodeList::iterator newIt,
                         patchIt = it, // (pointless default value, fixes compiler warnings)
                         insIt;
    for(newIt = pNewNodes->begin(); newIt != pNewNodes->end(); ++newIt){
        insIt = pLineList->insert(it, *newIt);
        if(newIt == pNewNodes->begin())
            patchIt = insIt;
    }
    //Patch the places we've changed stuff
    PatchLineList(pLineList.get(), patchIt);
    SmCaretPos PosAfterInsert = PatchLineList(pLineList.get(), it);
    //Release list, we've taken the nodes
    pNewNodes.reset();

    //Finish editing
    FinishEdit(std::move(pLineList), pLineParent, nParentIndex, PosAfterInsert);
}

SmNodeList::iterator SmCursor::FindPositionInLineList(SmNodeList* pLineList,
                                                      const SmCaretPos& rCaretPos)
{
    //Find iterator for position
    SmNodeList::iterator it = std::find(pLineList->begin(), pLineList->end(), rCaretPos.pSelectedNode);
    if (it != pLineList->end())
    {
        if((*it)->GetType() == SmNodeType::Text)
        {
            //Split textnode if needed
            if(rCaretPos.nIndex > 0)
            {
                SmTextNode* pText = static_cast<SmTextNode*>(rCaretPos.pSelectedNode);
                if (rCaretPos.nIndex == pText->GetText().getLength())
                    return ++it;
                OUString str1 = pText->GetText().copy(0, rCaretPos.nIndex);
                OUString str2 = pText->GetText().copy(rCaretPos.nIndex);
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
        prev->GetType() == SmNodeType::Text &&
        next->GetType() == SmNodeType::Text &&
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
    if(prev && next && prev->GetType() == SmNodeType::Place && !SmNodeListParser::IsOperator(next->GetToken())){
        --aIter;
        aIter = pLineList->erase(aIter);
        delete prev;
        //Return caret pos in front of aIter
        if(aIter != pLineList->begin())
            --aIter; //Thus find node before aIter
        if(aIter == pLineList->begin())
            return SmCaretPos();
        return SmCaretPos::GetPosAfter(*aIter);
    }
    if(prev && next && next->GetType() == SmNodeType::Place && !SmNodeListParser::IsOperator(prev->GetToken())){
        aIter = pLineList->erase(aIter);
        delete next;
        return SmCaretPos::GetPosAfter(prev);
    }

    //If we didn't do anything return
    if(!prev) //return an invalid to indicate we're in front of line
        return SmCaretPos();
    return SmCaretPos::GetPosAfter(prev);
}

SmNodeList::iterator SmCursor::TakeSelectedNodesFromList(SmNodeList *pLineList,
                                                         SmNodeList *pSelectedNodes) {
    SmNodeList::iterator retval;
    SmNodeList::iterator it = pLineList->begin();
    while(it != pLineList->end()){
        if((*it)->IsSelected()){
            //Split text nodes
            if((*it)->GetType() == SmNodeType::Text) {
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
                    OUString str = aText.copy(0, len1);
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
        assert(pSNode);
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
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Take the selection, and/or find iterator for current position
    std::unique_ptr<SmNodeList> pSelectedNodesList(new SmNodeList);
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList.get(), pSelectedNodesList.get());
    else
        it = FindPositionInLineList(pLineList.get(), mpPosition->CaretPos);

    //Find node that this should be applied to
    SmNode* pSubject;
    bool bPatchLine = !pSelectedNodesList->empty(); //If the line should be patched later
    if(it != pLineList->begin()) {
        --it;
        pSubject = *it;
        ++it;
    } else {
        //Create a new place node
        pSubject = new SmPlaceNode();
        pSubject->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);
        it = pLineList->insert(it, pSubject);
        ++it;
        bPatchLine = true;  //We've modified the line it should be patched later.
    }

    //Wrap the subject in a SmSubSupNode
    SmSubSupNode* pSubSup;
    if(pSubject->GetType() != SmNodeType::SubSup){
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
        PatchLineList(pLineList.get(), it);

    //Convert existing, if any, sub-/superscript line to list
    SmNode *pScriptLine = pSubSup->GetSubSup(eSubSup);
    std::unique_ptr<SmNodeList> pScriptLineList(new SmNodeList);
    NodeToList(pScriptLine, *pScriptLineList);

    //Add selection to pScriptLineList
    unsigned int nOldSize = pScriptLineList->size();
    pScriptLineList->insert(pScriptLineList->end(), pSelectedNodesList->begin(), pSelectedNodesList->end());
    pSelectedNodesList.reset();

    //Patch pScriptLineList if needed
    if(0 < nOldSize && nOldSize < pScriptLineList->size()) {
        SmNodeList::iterator iPatchPoint = pScriptLineList->begin();
        std::advance(iPatchPoint, nOldSize);
        PatchLineList(pScriptLineList.get(), iPatchPoint);
    }

    //Find caret pos, that should be used after sub-/superscription.
    SmCaretPos PosAfterScript; //Leave invalid for first position
    if (!pScriptLineList->empty())
        PosAfterScript = SmCaretPos::GetPosAfter(pScriptLineList->back());

    //Parse pScriptLineList
    pScriptLine = SmNodeListParser().Parse(pScriptLineList.get());
    pScriptLineList.reset();

    //Insert pScriptLine back into the tree
    pSubSup->SetSubSup(eSubSup, pScriptLine);

    //Finish editing
    FinishEdit(std::move(pLineList), pLineParent, nParentIndex, PosAfterScript, pScriptLine);
}

void SmCursor::InsertBrackets(SmBracketType eBracketType) {
    BeginEdit();

    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        assert(pSNode);
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(mpPosition->CaretPos.pSelectedNode);

    //Find parent and offset in parent
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    assert(nParentIndex >= 0);

    //Convert line to list
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Take the selection, and/or find iterator for current position
    std::unique_ptr<SmNodeList> pSelectedNodesList(new SmNodeList);
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList.get(), pSelectedNodesList.get());
    else
        it = FindPositionInLineList(pLineList.get(), mpPosition->CaretPos);

    //If there's no selected nodes, create a place node
    std::unique_ptr<SmNode> pBodyNode;
    SmCaretPos PosAfterInsert;
    if(pSelectedNodesList->empty()) {
        pBodyNode.reset(new SmPlaceNode());
        PosAfterInsert = SmCaretPos(pBodyNode.get(), 1);
    } else
        pBodyNode.reset(SmNodeListParser().Parse(pSelectedNodesList.get()));

    pSelectedNodesList.reset();

    //Create SmBraceNode
    SmToken aTok(TLEFT, '\0', "left", TG::NONE, 5);
    SmBraceNode *pBrace = new SmBraceNode(aTok);
    pBrace->SetScaleMode(SmScaleMode::Height);
    std::unique_ptr<SmNode> pLeft( CreateBracket(eBracketType, true) ),
                            pRight( CreateBracket(eBracketType, false) );
    std::unique_ptr<SmBracebodyNode> pBody(new SmBracebodyNode(SmToken()));
    pBody->SetSubNodes(std::move(pBodyNode), nullptr);
    pBrace->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
    pBrace->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);

    //Insert into line
    pLineList->insert(it, pBrace);
    //Patch line (I think this is good enough)
    SmCaretPos aAfter = PatchLineList(pLineList.get(), it);
    if( !PosAfterInsert.IsValid() )
        PosAfterInsert = aAfter;

    //Finish editing
    FinishEdit(std::move(pLineList), pLineParent, nParentIndex, PosAfterInsert);
}

SmNode *SmCursor::CreateBracket(SmBracketType eBracketType, bool bIsLeft) {
    SmToken aTok;
    if(bIsLeft){
        switch(eBracketType){
            case SmBracketType::Round:
                aTok = SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
                break;
            case SmBracketType::Square:
                aTok = SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
                break;
            case SmBracketType::Curly:
                aTok = SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
                break;
        }
    } else {
        switch(eBracketType) {
            case SmBracketType::Round:
                aTok = SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
                break;
            case SmBracketType::Square:
                aTok = SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
                break;
            case SmBracketType::Curly:
                aTok = SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
                break;
        }
    }
    SmNode* pRetVal = new SmMathSymbolNode(aTok);
    pRetVal->SetScaleMode(SmScaleMode::Height);
    return pRetVal;
}

bool SmCursor::InsertRow() {
    AnnotateSelection();

    //Find line
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(mpTree);
        assert(pSNode);
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
    if(pLineParent->GetType() == SmNodeType::Table)
        pTable = static_cast<SmTableNode*>(pLineParent);
    //If it's wrapped in a SmLineNode, we can still insert a newline
    else if(pLineParent->GetType() == SmNodeType::Line &&
            pLineParent->GetParent() &&
            pLineParent->GetParent()->GetType() == SmNodeType::Table) {
        //NOTE: This hack might give problems if we stop ignoring SmAlignNode
        pTable = static_cast<SmTableNode*>(pLineParent->GetParent());
        nTableIndex = pTable->IndexOfSubNode(pLineParent);
        assert(nTableIndex >= 0);
    }
    if(pLineParent->GetType() == SmNodeType::Matrix)
        pMatrix = static_cast<SmMatrixNode*>(pLineParent);

    //If we're not in a context that supports InsertRow, return sal_False
    if(!pTable && !pMatrix)
        return false;

    //Now we start editing
    BeginEdit();

    //Convert line to list
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Find position in line
    SmNodeList::iterator it;
    if(HasSelection()) {
        //Take the selected nodes and delete them...
        it = TakeSelectedNodesFromList(pLineList.get());
    } else
        it = FindPositionInLineList(pLineList.get(), mpPosition->CaretPos);

    //New caret position after inserting the newline/row in whatever context
    SmCaretPos PosAfterInsert;

    //If we're in the context of a table
    if(pTable) {
        std::unique_ptr<SmNodeList> pNewLineList(new SmNodeList);
        //Move elements from pLineList to pNewLineList
        pNewLineList->splice(pNewLineList->begin(), *pLineList, it, pLineList->end());
        //Make sure it is valid again
        it = pLineList->end();
        if(it != pLineList->begin())
            --it;
        if(pNewLineList->empty())
            pNewLineList->push_front(new SmPlaceNode());
        //Parse new line
        std::unique_ptr<SmNode> pNewLine(SmNodeListParser().Parse(pNewLineList.get()));
        pNewLineList.reset();
        //Wrap pNewLine in SmLineNode if needed
        if(pLineParent->GetType() == SmNodeType::Line) {
            std::unique_ptr<SmLineNode> pNewLineNode(new SmLineNode(SmToken(TNEWLINE, '\0', "newline")));
            pNewLineNode->SetSubNodes(std::move(pNewLine), nullptr);
            pNewLine = std::move(pNewLineNode);
        }
        //Get position
        PosAfterInsert = SmCaretPos(pNewLine.get(), 0);
        //Move other nodes if needed
        for( int i = pTable->GetNumSubNodes(); i > nTableIndex + 1; i--)
            pTable->SetSubNode(i, pTable->GetSubNode(i-1));

        //Insert new line
        pTable->SetSubNode(nTableIndex + 1, pNewLine.get());

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
        PosAfterInsert = PatchLineList(pLineList.get(), it);
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
    FinishEdit(std::move(pLineList), pLineParent, nParentIndex, PosAfterInsert);
    //FinishEdit is actually used to handle situations where parent is an instance of
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
        assert(pSNode);
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
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pLine, *pLineList);

    //Take the selection, and/or find iterator for current position
    std::unique_ptr<SmNodeList> pSelectedNodesList(new SmNodeList);
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList.get(), pSelectedNodesList.get());
    else
        it = FindPositionInLineList(pLineList.get(), mpPosition->CaretPos);

    //Create pNum, and pDenom
    bool bEmptyFraction = pSelectedNodesList->empty();
    std::unique_ptr<SmNode> pNum( bEmptyFraction
        ? new SmPlaceNode()
        : SmNodeListParser().Parse(pSelectedNodesList.get()) );
    std::unique_ptr<SmNode> pDenom(new SmPlaceNode());
    pSelectedNodesList.reset();

    //Create new fraction
    SmBinVerNode *pFrac = new SmBinVerNode(SmToken(TOVER, '\0', "over", TG::Product, 0));
    std::unique_ptr<SmNode> pRect(new SmRectangleNode(SmToken()));
    pFrac->SetSubNodes(std::move(pNum), std::move(pRect), std::move(pDenom));

    //Insert in pLineList
    SmNodeList::iterator patchIt = pLineList->insert(it, pFrac);
    PatchLineList(pLineList.get(), patchIt);
    PatchLineList(pLineList.get(), it);

    //Finish editing
    SmNode *pSelectedNode = bEmptyFraction ? pFrac->GetSubNode(0) : pFrac->GetSubNode(2);
    FinishEdit(std::move(pLineList), pLineParent, nParentIndex, SmCaretPos(pSelectedNode, 1));
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
    pText->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);

    std::unique_ptr<SmNodeList> pList(new SmNodeList);
    pList->push_front(pText);
    InsertNodes(std::move(pList));

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
            token.eType = TBLANK;
            token.nGroup = TG::Blank;
            token.aText = "~";
            SmBlankNode* pBlankNode = new SmBlankNode(token);
            pBlankNode->IncreaseBy(token);
            pNewNode = pBlankNode;
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
    pNewNode->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);

    //Insert new node
    std::unique_ptr<SmNodeList> pList(new SmNodeList);
    pList->push_front(pNewNode);
    InsertNodes(std::move(pList));

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
    pSpecial->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);

    //Insert the node
    std::unique_ptr<SmNodeList> pList(new SmNodeList);
    pList->push_front(pSpecial);
    InsertNodes(std::move(pList));

    EndEdit();
}

void SmCursor::InsertCommandText(const OUString& aCommandText) {
    //Parse the sub expression
    auto xSubExpr = SmParser().ParseExpression(aCommandText);

    //Prepare the subtree
    xSubExpr->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);

    //Convert subtree to list
    SmNode* pSubExpr = xSubExpr.release();
    std::unique_ptr<SmNodeList> pLineList(new SmNodeList);
    NodeToList(pSubExpr, *pLineList);

    BeginEdit();

    //Delete any selection
    Delete();

    //Insert it
    InsertNodes(std::move(pLineList));

    EndEdit();
}

void SmCursor::Copy(){
    if(!HasSelection())
        return;

    AnnotateSelection();
    //Find selected node
    SmNode* pSNode = FindSelectedNode(mpTree);
    assert(pSNode);
    //Find visual line
    SmNode* pLine = FindTopMostNodeInLine(pSNode, true);
    assert(pLine);

    //Clone selected nodes
    SmClipboard aClipboard;
    if(IsLineCompositionNode(pLine))
        CloneLineToClipboard(static_cast<SmStructureNode*>(pLine), &aClipboard);
    else{
        //Special care to only clone selected text
        if(pLine->GetType() == SmNodeType::Text) {
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
    if (!aClipboard.empty())
        maClipboard = std::move(aClipboard);
}

void SmCursor::Paste() {
    BeginEdit();
    Delete();

    if (!maClipboard.empty())
        InsertNodes(CloneList(maClipboard));

    EndEdit();
}

std::unique_ptr<SmNodeList> SmCursor::CloneList(SmClipboard &rClipboard){
    SmCloningVisitor aCloneFactory;
    std::unique_ptr<SmNodeList> pClones(new SmNodeList);

    for(auto &xNode : rClipboard){
        SmNode *pClone = aCloneFactory.Clone(xNode.get());
        pClones->push_back(pClone);
    }

    return pClones;
}

SmNode* SmCursor::FindTopMostNodeInLine(SmNode* pSNode, bool MoveUpIfSelected){
    assert(pSNode);
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
    if(pNode->GetNumSubNodes() == 0)
        return nullptr;
    for(auto pChild : *static_cast<SmStructureNode*>(pNode))
    {
        if(!pChild)
            continue;
        if(pChild->IsSelected())
            return pChild;
        SmNode* pRetVal = FindSelectedNode(pChild);
        if(pRetVal)
            return pRetVal;
    }
    return nullptr;
}

void SmCursor::LineToList(SmStructureNode* pLine, SmNodeList& list){
    for(auto pChild : *pLine)
    {
        if (!pChild)
            continue;
        switch(pChild->GetType()){
            case SmNodeType::Line:
            case SmNodeType::UnHor:
            case SmNodeType::Expression:
            case SmNodeType::BinHor:
            case SmNodeType::Align:
            case SmNodeType::Font:
                LineToList(static_cast<SmStructureNode*>(pChild), list);
                break;
            case SmNodeType::Error:
                delete pChild;
                break;
            default:
                list.push_back(pChild);
        }
    }
    pLine->ClearSubNodes();
    delete pLine;
}

void SmCursor::CloneLineToClipboard(SmStructureNode* pLine, SmClipboard* pClipboard){
    SmCloningVisitor aCloneFactory;
    for(auto pChild : *pLine)
    {
        if (!pChild)
            continue;
        if( IsLineCompositionNode( pChild ) )
            CloneLineToClipboard( static_cast<SmStructureNode*>(pChild), pClipboard );
        else if( pChild->IsSelected() && pChild->GetType() != SmNodeType::Error ) {
            //Only clone selected text from SmTextNode
            if(pChild->GetType() == SmNodeType::Text) {
                SmTextNode *pText = static_cast<SmTextNode*>(pChild);
                std::unique_ptr<SmTextNode> pClone(new SmTextNode( pChild->GetToken(), pText->GetFontDesc() ));
                int start = pText->GetSelectionStart(),
                    length = pText->GetSelectionEnd() - pText->GetSelectionStart();
                pClone->ChangeText(pText->GetText().copy(start, length));
                pClone->SetScaleMode(pText->GetScaleMode());
                pClipboard->push_back(std::move(pClone));
            } else
                pClipboard->push_back(std::unique_ptr<SmNode>(aCloneFactory.Clone(pChild)));
        }
    }
}

bool SmCursor::IsLineCompositionNode(SmNode const * pNode){
    switch(pNode->GetType()){
        case SmNodeType::Line:
        case SmNodeType::UnHor:
        case SmNodeType::Expression:
        case SmNodeType::BinHor:
        case SmNodeType::Align:
        case SmNodeType::Font:
            return true;
        default:
            return false;
    }
}

int SmCursor::CountSelectedNodes(SmNode* pNode){
    if(pNode->GetNumSubNodes() == 0)
        return 0;
    int nCount = 0;
    for(auto pChild : *static_cast<SmStructureNode*>(pNode))
    {
        if (!pChild)
            continue;
        if(pChild->IsSelected() && !IsLineCompositionNode(pChild))
            nCount++;
        nCount += CountSelectedNodes(pChild);
    }
    return nCount;
}

bool SmCursor::HasComplexSelection(){
    if(!HasSelection())
        return false;
    AnnotateSelection();

    return CountSelectedNodes(mpTree) > 1;
}

void SmCursor::FinishEdit(std::unique_ptr<SmNodeList> pLineList,
                          SmStructureNode* pParent,
                          int nParentIndex,
                          SmCaretPos PosAfterEdit,
                          SmNode* pStartLine) {
    //Store number of nodes in line for later
    int entries = pLineList->size();

    //Parse list of nodes to a tree
    SmNodeListParser parser;
    std::unique_ptr<SmNode> pLine(parser.Parse(pLineList.get()));
    pLineList.reset();

    //Check if we're making the body of a subsup node bigger than one
    if(pParent->GetType() == SmNodeType::SubSup &&
       nParentIndex == 0 &&
       entries > 1) {
        //Wrap pLine in scalable round brackets
        SmToken aTok(TLEFT, '\0', "left", TG::NONE, 5);
        std::unique_ptr<SmBraceNode> pBrace(new SmBraceNode(aTok));
        pBrace->SetScaleMode(SmScaleMode::Height);
        std::unique_ptr<SmNode> pLeft(  CreateBracket(SmBracketType::Round, true) ),
                                pRight( CreateBracket(SmBracketType::Round, false) );
        std::unique_ptr<SmBracebodyNode> pBody(new SmBracebodyNode(SmToken()));
        pBody->SetSubNodes(std::move(pLine), nullptr);
        pBrace->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
        pBrace->Prepare(mpDocShell->GetFormat(), *mpDocShell, 0);
        pLine = std::move(pBrace);
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
        pStartLine = pLine.get();

    //Insert it back into the parent
    pParent->SetSubNode(nParentIndex, pLine.release());

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

    if (pNode->GetType() == SmNodeType::Text) {
        SmTextNode* pTextNode = static_cast<SmTextNode*>(pNode);
        if (pos.nIndex < pTextNode->GetText().getLength()) {
            // The cursor is on a text node and at the middle of it.
            return false;
        }
    } else {
        if (pos.nIndex < 1) {
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
        if (static_cast<size_t>(index + 1) != pParentNode->GetNumSubNodes()) {
            // The cursor is not at the tail at one of ancestor nodes.
            return false;
        }

        pNode = pParentNode;
        if (pNode->GetType() == SmNodeType::Bracebody) {
            // Found the brace body node.
            break;
        }
    }

    SmStructureNode* pBraceNodeTmp = pNode->GetParent();
    if (!pBraceNodeTmp || pBraceNodeTmp->GetType() != SmNodeType::Brace) {
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
    case SmBracketType::Round:        if (eClosingTokenType != TRPARENT)   { return false; } break;
    case SmBracketType::Square:       if (eClosingTokenType != TRBRACKET)  { return false; } break;
    case SmBracketType::Curly:        if (eClosingTokenType != TRBRACE)    { return false; } break;
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
    mpPosition->CaretPos.nIndex = 1;
    mpAnchor->CaretPos.pSelectedNode = pBraceNode;
    mpAnchor->CaretPos.nIndex = 1;
    RequestRepaint();
}


/////////////////////////////////////// SmNodeListParser

SmNode* SmNodeListParser::Parse(SmNodeList* list){
    pList = list;
    //Delete error nodes
    SmNodeList::iterator it = pList->begin();
    while(it != pList->end()) {
        if((*it)->GetType() == SmNodeType::Error){
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
    pExpr->SetSubNodes(std::move(NodeArray));
    return pExpr;
}

SmNode* SmNodeListParser::Relation(){
    //Read a sum
    std::unique_ptr<SmNode> pLeft(Sum());
    //While we have tokens and the next is a relation
    while(Terminal() && IsRelationOperator(Terminal()->GetToken())){
        //Take the operator
        std::unique_ptr<SmNode> pOper(Take());
        //Find the right side of the relation
        std::unique_ptr<SmNode> pRight(Sum());
        //Create new SmBinHorNode
        std::unique_ptr<SmStructureNode> pNewNode(new SmBinHorNode(SmToken()));
        pNewNode->SetSubNodes(std::move(pLeft), std::move(pOper), std::move(pRight));
        pLeft = std::move(pNewNode);
    }
    return pLeft.release();
}

SmNode* SmNodeListParser::Sum(){
    //Read a product
    std::unique_ptr<SmNode> pLeft(Product());
    //While we have tokens and the next is a sum
    while(Terminal() && IsSumOperator(Terminal()->GetToken())){
        //Take the operator
        std::unique_ptr<SmNode> pOper(Take());
        //Find the right side of the sum
        std::unique_ptr<SmNode> pRight(Product());
        //Create new SmBinHorNode
        std::unique_ptr<SmStructureNode> pNewNode(new SmBinHorNode(SmToken()));
        pNewNode->SetSubNodes(std::move(pLeft), std::move(pOper), std::move(pRight));
        pLeft = std::move(pNewNode);
    }
    return pLeft.release();
}

SmNode* SmNodeListParser::Product(){
    //Read a Factor
    std::unique_ptr<SmNode> pLeft(Factor());
    //While we have tokens and the next is a product
    while(Terminal() && IsProductOperator(Terminal()->GetToken())){
        //Take the operator
        std::unique_ptr<SmNode> pOper(Take());
        //Find the right side of the operation
        std::unique_ptr<SmNode> pRight(Factor());
        //Create new SmBinHorNode
        std::unique_ptr<SmStructureNode> pNewNode(new SmBinHorNode(SmToken()));
        pNewNode->SetSubNodes(std::move(pLeft), std::move(pOper), std::move(pRight));
        pLeft = std::move(pNewNode);
    }
    return pLeft.release();
}

SmNode* SmNodeListParser::Factor(){
    //Read unary operations
    if(!Terminal())
        return Error();
    //Take care of unary operators
    else if(IsUnaryOperator(Terminal()->GetToken()))
    {
        SmStructureNode *pUnary = new SmUnHorNode(SmToken());
        std::unique_ptr<SmNode> pOper(Terminal()),
                                pArg;

        if(Next())
            pArg.reset(Factor());
        else
            pArg.reset(Error());

        pUnary->SetSubNodes(std::move(pOper), std::move(pArg));
        return pUnary;
    }
    return Postfix();
}

SmNode* SmNodeListParser::Postfix(){
    if(!Terminal())
        return Error();
    std::unique_ptr<SmNode> pArg;
    if(IsPostfixOperator(Terminal()->GetToken()))
        pArg.reset(Error());
    else if(IsOperator(Terminal()->GetToken()))
        return Error();
    else
        pArg.reset(Take());
    while(Terminal() && IsPostfixOperator(Terminal()->GetToken())) {
        std::unique_ptr<SmStructureNode> pUnary(new SmUnHorNode(SmToken()) );
        std::unique_ptr<SmNode> pOper(Take());
        pUnary->SetSubNodes(std::move(pArg), std::move(pOper));
        pArg = std::move(pUnary);
    }
    return pArg.release();
}

SmNode* SmNodeListParser::Error(){
    return new SmErrorNode(SmToken());
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
