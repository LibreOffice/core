/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */
#include "cursor.hxx"
#include "parse.hxx"
#include "visitors.hxx"
#include "document.hxx"
#include "view.hxx"
#include "accessibility.hxx"
#include <comphelper/string.hxx>

void SmCursor::Move(OutputDevice* pDev, SmMovementDirection direction, bool bMoveAnchor){
    SmCaretPosGraphEntry* NewPos = NULL;
    switch(direction){
        case MoveLeft:
        {
            NewPos = position->Left;
            OSL_ENSURE(NewPos, "NewPos shouldn't be NULL here!");
        }break;
        case MoveRight:
        {
            NewPos = position->Right;
            OSL_ENSURE(NewPos, "NewPos shouldn't be NULL here!");
        }break;
        case MoveUp:
            
            
        case MoveDown:
        {
            SmCaretLine from_line = SmCaretPos2LineVisitor(pDev, position->CaretPos).GetResult(),
                        best_line,  
                        curr_line;  
            long dbp_sq = 0;        
            SmCaretPosGraphIterator it = pGraph->GetIterator();
            while(it.Next()){
                
                if(it->CaretPos == position->CaretPos) continue;
                
                curr_line = SmCaretPos2LineVisitor(pDev, it->CaretPos).GetResult();
                
                if(curr_line.GetTop() <= from_line.GetTop() && direction == MoveDown) continue;
                
                if(curr_line.GetTop() + curr_line.GetHeight() >= from_line.GetTop() + from_line.GetHeight()
                        && direction == MoveUp) continue;
                
                if(NewPos){
                    
                    long dp_sq = curr_line.SquaredDistanceX(from_line) * HORIZONTICAL_DISTANCE_FACTOR +
                                 curr_line.SquaredDistanceY(from_line);
                    
                    if(dbp_sq <= dp_sq) continue;
                }
                
                best_line = curr_line;
                NewPos = it.Current();
                
                dbp_sq = best_line.SquaredDistanceX(from_line) * HORIZONTICAL_DISTANCE_FACTOR +
                         best_line.SquaredDistanceY(from_line);
            }
        }break;
        default:
            SAL_WARN("starmath", "Movement direction not supported!");
    }
    if(NewPos){
        position = NewPos;
        if(bMoveAnchor)
            anchor = NewPos;
        RequestRepaint();
    }
}

void SmCursor::MoveTo(OutputDevice* pDev, Point pos, bool bMoveAnchor){
    SmCaretLine best_line,  
                curr_line;  
    SmCaretPosGraphEntry* NewPos = NULL;
    long dp_sq = 0,     
         dbp_sq = 1;    
    SmCaretPosGraphIterator it = pGraph->GetIterator();
    while(it.Next()){
        OSL_ENSURE(it->CaretPos.IsValid(), "The caret position graph may not have invalid positions!");
        
        curr_line = SmCaretPos2LineVisitor(pDev, it->CaretPos).GetResult();
        
        if(NewPos){
            
            dp_sq = curr_line.SquaredDistanceX(pos) + curr_line.SquaredDistanceY(pos);
            
            if(dbp_sq <= dp_sq) continue;
        }
        
        best_line = curr_line;
        NewPos = it.Current();
        
        dbp_sq = best_line.SquaredDistanceX(pos) + best_line.SquaredDistanceY(pos);
    }
    if(NewPos){
        position = NewPos;
        if(bMoveAnchor)
            anchor = NewPos;
        RequestRepaint();
    }
}

void SmCursor::BuildGraph(){
    
    SmCaretPos _anchor, _position;
    
    if(pGraph){
        if(anchor)
            _anchor = anchor->CaretPos;
        if(position)
            _position = position->CaretPos;
        delete pGraph;
        
        anchor = NULL;
        position = NULL;
    }

    
    pGraph = SmCaretPosGraphBuildingVisitor(pTree).takeGraph();

    
    if(_anchor.IsValid() || _position.IsValid()){
        SmCaretPosGraphIterator it = pGraph->GetIterator();
        while(it.Next()){
            if(_anchor == it->CaretPos)
                anchor = it.Current();
            if(_position == it->CaretPos)
                position = it.Current();
        }
    }
    
    SmCaretPosGraphIterator it = pGraph->GetIterator();
    if(!position)
        position = it.Next();
    if(!anchor)
        anchor = position;

    OSL_ENSURE(position->CaretPos.IsValid(), "Position must be valid");
    OSL_ENSURE(anchor->CaretPos.IsValid(), "Anchor must be valid");
}

bool SmCursor::SetCaretPosition(SmCaretPos pos, bool moveAnchor){
    SmCaretPosGraphIterator it = pGraph->GetIterator();
    while(it.Next()){
        if(it->CaretPos == pos){
            position = it.Current();
            if(moveAnchor)
                anchor = it.Current();
            return true;
        }
    }
    return false;
}

void SmCursor::AnnotateSelection(){
    
    SmSetSelectionVisitor(anchor->CaretPos, position->CaretPos, pTree);
}

void SmCursor::Draw(OutputDevice& pDev, Point Offset, bool isCaretVisible){
    SmCaretDrawingVisitor(pDev, GetPosition(), Offset, isCaretVisible);
}

void SmCursor::DeletePrev(OutputDevice* pDev){
    
    if(HasSelection()){
        Delete();
        return;
    }

    SmNode* pLine = FindTopMostNodeInLine(position->CaretPos.pSelectedNode);
    SmStructureNode* pLineParent = pLine->GetParent();
    int nLineOffset = pLineParent->IndexOfSubNode(pLine);

    
    if(pLineParent->GetType() == NTABLE && position->CaretPos.Index == 0 && nLineOffset > 0){
        
        BeginEdit();
        
        SmNode* pMergeLine = pLineParent->GetSubNode(nLineOffset-1);
        OSL_ENSURE(pMergeLine, "pMergeLine cannot be NULL!");
        SmCaretPos PosAfterDelete;
        
        SmNodeList *pLineList = NodeToList(pMergeLine);
        if(!pLineList->empty()){
            
            SmNodeList::iterator patchPoint = pLineList->end();
            --patchPoint;
            
            NodeToList(pLine, pLineList);
            
            ++patchPoint;
            PosAfterDelete = PatchLineList(pLineList, patchPoint);
            
            pLine = SmNodeListParser().Parse(pLineList);
        }
        delete pLineList;
        pLineParent->SetSubNode(nLineOffset-1, pLine);
        
        SmNodeArray lines(pLineParent->GetNumSubNodes()-1);
        for(int i = 0; i < pLineParent->GetNumSubNodes(); i++){
            if(i < nLineOffset)
                lines[i] = pLineParent->GetSubNode(i);
            else if(i > nLineOffset)
                lines[i-1] = pLineParent->GetSubNode(i);
        }
        pLineParent->SetSubNodes(lines);
        
        anchor = NULL;
        position = NULL;
        BuildGraph();
        AnnotateSelection();
        
        if(!SetCaretPosition(PosAfterDelete, true))
            SetCaretPosition(SmCaretPos(pLine, 0), true);
        
        EndEdit();

    
    /*}else if(pLineParent->GetType() == NSUBSUP &&
             nLineOffset != 0 &&
             pLine->GetType() == NEXPRESSION &&
             pLine->GetNumSubNodes() == 0){
        
    
    
    */

    
    }else{
        this->Move(pDev, MoveLeft, false);
        if(!this->HasComplexSelection())
            Delete();
    }
}

void SmCursor::Delete(){
    
    if(!HasSelection())
        return;

    
    BeginEdit();

    
    AnnotateSelection();

    
    SmNode* pSNode = FindSelectedNode(pTree);
    OSL_ENSURE(pSNode != NULL, "There must be a selection when HasSelection is true!");

    
    SmNode* pLine = FindTopMostNodeInLine(pSNode, true);
    OSL_ENSURE(pLine != pTree, "Shouldn't be able to select the entire tree");

    
    SmStructureNode* pLineParent = pLine->GetParent();
    
    int nLineOffset = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE(nLineOffset != -1, "pLine must be a child of it's parent!");

    
    SmCaretPos PosAfterDelete;

    SmNodeList* pLineList = NodeToList(pLine);

    
    SmNodeList::iterator patchIt = TakeSelectedNodesFromList(pLineList);

    
    PosAfterDelete = PatchLineList(pLineList, patchIt);

    
    FinishEdit(pLineList, pLineParent, nLineOffset, PosAfterDelete);
}

void SmCursor::InsertNodes(SmNodeList* pNewNodes){
    if(pNewNodes->empty()){
        delete pNewNodes;
        return;
    }

    
    BeginEdit();

    
    SmCaretPos PosAfterInsert = SmCaretPos(pNewNodes->back(), 1);

    
    const SmCaretPos pos = position->CaretPos;

    
    SmNode* pLine = FindTopMostNodeInLine(pos.pSelectedNode, false);

    
    SmStructureNode* pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE(nParentIndex != -1, "pLine must be a subnode of pLineParent!");

    
    SmNodeList* pLineList = NodeToList(pLine);

    
    SmNodeList::iterator it = FindPositionInLineList(pLineList, pos);

    
    SmNodeList::iterator newIt,
                         patchIt = it, 
                         insIt;
    for(newIt = pNewNodes->begin(); newIt != pNewNodes->end(); ++newIt){
        insIt = pLineList->insert(it, *newIt);
        if(newIt == pNewNodes->begin())
            patchIt = insIt;
        if((*newIt)->GetType() == NTEXT)
            PosAfterInsert = SmCaretPos(*newIt, ((SmTextNode*)*newIt)->GetText().getLength());
        else
            PosAfterInsert = SmCaretPos(*newIt, 1);
    }
    
                        PatchLineList(pLineList, patchIt);
    PosAfterInsert =    PatchLineList(pLineList, it);
    
    delete pNewNodes;
    pNewNodes = NULL;

    
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
}

SmNodeList::iterator SmCursor::FindPositionInLineList(SmNodeList* pLineList, SmCaretPos aCaretPos) {
    
    SmNodeList::iterator it;
    for(it = pLineList->begin(); it != pLineList->end(); ++it){
        if(*it == aCaretPos.pSelectedNode){
            if((*it)->GetType() == NTEXT){
                
                if(aCaretPos.Index > 0){
                    SmTextNode* pText = (SmTextNode*)aCaretPos.pSelectedNode;
                    OUString str1 = pText->GetText().copy(0, aCaretPos.Index);
                    OUString str2 = pText->GetText().copy(aCaretPos.Index);
                    pText->ChangeText(str1);
                    ++it;
                    
                    if(!str2.isEmpty()){
                        SmTextNode* pNewText = new SmTextNode(pText->GetToken(), pText->GetFontDesc());
                        pNewText->ChangeText(str2);
                        it = pLineList->insert(it, pNewText);
                    }
                }
            }else
                ++it;
            
            return it;

        }
    }
    
    return pLineList->begin();
}

SmCaretPos SmCursor::PatchLineList(SmNodeList* pLineList, SmNodeList::iterator aIter) {
    
    SmNode *prev = NULL,
           *next = NULL;
    if(aIter != pLineList->end())
        next = *aIter;
    if(aIter != pLineList->begin()) {
        --aIter;
        prev = *aIter;
        ++aIter;
    }

    
    if( prev &&
        next &&
        prev->GetType() == NTEXT &&
        next->GetType() == NTEXT &&
        ( prev->GetToken().eType != TNUMBER ||
          next->GetToken().eType == TNUMBER) ){
        SmTextNode *pText = (SmTextNode*)prev,
                   *pOldN = (SmTextNode*)next;
        SmCaretPos retval(pText, pText->GetText().getLength());
        OUString newText;
        newText += pText->GetText();
        newText += pOldN->GetText();
        pText->ChangeText(newText);
        delete pOldN;
        pLineList->erase(aIter);
        return retval;
    }

    
    if(prev && next && prev->GetType() == NPLACE && !SmNodeListParser::IsOperator(next->GetToken())){
        --aIter;
        aIter = pLineList->erase(aIter);
        delete prev;
        
        if(aIter != pLineList->begin())
            --aIter; 
        if(aIter == pLineList->begin())
            return SmCaretPos();
        if((*aIter)->GetType() == NTEXT)
            return SmCaretPos(*aIter, ((SmTextNode*)*aIter)->GetText().getLength());
        return SmCaretPos(*aIter, 1);
    }
    if(prev && next && next->GetType() == NPLACE && !SmNodeListParser::IsOperator(prev->GetToken())){
        aIter = pLineList->erase(aIter);
        delete next;
        if(prev->GetType() == NTEXT)
            return SmCaretPos(prev, ((SmTextNode*)prev)->GetText().getLength());
        return SmCaretPos(prev, 1);
    }

    
    if(!prev) 
        return SmCaretPos();
    if(prev->GetType() == NTEXT)
        return SmCaretPos(prev, ((SmTextNode*)prev)->GetText().getLength());
    return SmCaretPos(prev, 1);
}

SmNodeList::iterator SmCursor::TakeSelectedNodesFromList(SmNodeList *pLineList,
                                                         SmNodeList *pSelectedNodes) {
    SmNodeList::iterator retval;
    SmNodeList::iterator it = pLineList->begin();
    while(it != pLineList->end()){
        if((*it)->IsSelected()){
            
            if((*it)->GetType() == NTEXT) {
                SmTextNode* pText = (SmTextNode*)*it;
                OUString aText = pText->GetText();
                
                int start2 = pText->GetSelectionStart(),
                    start3 = pText->GetSelectionEnd(),
                    len1 = start2 - 0,
                    len2 = start3 - start2,
                    len3 = aText.getLength() - start3;
                SmToken aToken = pText->GetToken();
                sal_uInt16 eFontDesc = pText->GetFontDesc();
                
                if(len1 > 0) {
                    int start1 = 0;
                    OUString str = aText.copy(start1, len1);
                    pText->ChangeText(str);
                    ++it;
                } else {
                    it = pLineList->erase(it);
                    delete pText;
                }
                
                retval = it;
                
                if(len3 > 0) {
                    OUString str = aText.copy(start3, len3);
                    SmTextNode* pSeg3 = new SmTextNode(aToken, eFontDesc);
                    pSeg3->ChangeText(str);
                    retval = pLineList->insert(it, pSeg3);
                }
                
                if(pSelectedNodes && len2 > 0) {
                    OUString str = aText.copy(start2, len2);
                    SmTextNode* pSeg2 = new SmTextNode(aToken, eFontDesc);
                    pSeg2->ChangeText(str);
                    pSelectedNodes->push_back(pSeg2);
                }
            } else { 
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

    
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(pTree);
        OSL_ENSURE(pSNode != NULL, "There must be a selected node when HasSelection is true!");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(position->CaretPos.pSelectedNode, false);

    
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE(nParentIndex != -1, "pLine must be a subnode of pLineParent!");

    
    

    
    BeginEdit();

    
    SmNodeList* pLineList = NodeToList(pLine);

    
    SmNodeList* pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, position->CaretPos);

    
    SmNode* pSubject;
    bool bPatchLine = pSelectedNodesList->size() > 0; 
    if(it != pLineList->begin()) {
        --it;
        pSubject = *it;
        ++it;
    } else {
        
        pSubject = new SmPlaceNode();
        pSubject->Prepare(pDocShell->GetFormat(), *pDocShell);
        it = pLineList->insert(it, pSubject);
        ++it;
        bPatchLine = true;  
    }

    
    SmSubSupNode* pSubSup;
    if(pSubject->GetType() != NSUBSUP){
        SmToken token;
        token.nGroup = TGPOWER;
        pSubSup = new SmSubSupNode(token);
        pSubSup->SetBody(pSubject);
        *(--it) = pSubSup;
        ++it;
    }else
        pSubSup = (SmSubSupNode*)pSubject;
    
    
    pSubject = NULL;

    
    if(bPatchLine)
        PatchLineList(pLineList, it);

    
    SmNode *pScriptLine = pSubSup->GetSubSup(eSubSup);
    SmNodeList* pScriptLineList = NodeToList(pScriptLine);

    
    unsigned int nOldSize = pScriptLineList->size();
    pScriptLineList->insert(pScriptLineList->end(), pSelectedNodesList->begin(), pSelectedNodesList->end());
    delete pSelectedNodesList;
    pSelectedNodesList = NULL;

    
    if(0 < nOldSize && nOldSize < pScriptLineList->size()) {
        SmNodeList::iterator iPatchPoint = pScriptLineList->begin();
        std::advance(iPatchPoint, nOldSize);
        PatchLineList(pScriptLineList, iPatchPoint);
    }

    
    SmCaretPos PosAfterScript; 
    if(pScriptLineList->size() > 0)
        PosAfterScript = SmCaretPos::GetPosAfter(pScriptLineList->back());

    
    pScriptLine = SmNodeListParser().Parse(pScriptLineList);
    delete pScriptLineList;
    pScriptLineList = NULL;

    
    pSubSup->SetSubSup(eSubSup, pScriptLine);

    
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterScript, pScriptLine);
}

bool SmCursor::InsertLimit(SmSubSup eSubSup, bool bMoveCaret) {
    
    SmOperNode *pSubject = NULL;
    
    if(position->CaretPos.pSelectedNode->GetType() == NOPER)
        pSubject = (SmOperNode*)position->CaretPos.pSelectedNode;
    else {
        
        SmNode *pLineNode = FindTopMostNodeInLine(position->CaretPos.pSelectedNode, false);
        if(pLineNode->GetParent() && pLineNode->GetParent()->GetType() == NOPER)
            pSubject = (SmOperNode*)pLineNode->GetParent();
    }

    
    if(!pSubject)
        return false;

    BeginEdit();

    
    SmSubSupNode *pSubSup = NULL;
    
    if(pSubject->GetSubNode(0)->GetType() == NSUBSUP)
        pSubSup = (SmSubSupNode*)pSubject->GetSubNode(0);
    else { 
        SmToken token;
        token.nGroup = TGLIMIT;
        pSubSup = new SmSubSupNode(token);
        
        pSubSup->SetBody(pSubject->GetSubNode(0));
        
        pSubject->SetSubNode(0, pSubSup);
    }

    
    SmCaretPos PosAfterLimit;
    SmNode *pLine = NULL;
    if(!pSubSup->GetSubSup(eSubSup)){
        pLine = new SmPlaceNode();
        pSubSup->SetSubSup(eSubSup, pLine);
        PosAfterLimit = SmCaretPos(pLine, 1);
    
    } else if(bMoveCaret){
        pLine = pSubSup->GetSubSup(eSubSup);
        SmNodeList* pLineList = NodeToList(pLine);
        if(pLineList->size() > 0)
            PosAfterLimit = SmCaretPos::GetPosAfter(pLineList->back());
        pLine = SmNodeListParser().Parse(pLineList);
        delete pLineList;
        pSubSup->SetSubSup(eSubSup, pLine);
    }

    
    BuildGraph();
    AnnotateSelection();

    
    if(bMoveCaret)
        if(!SetCaretPosition(PosAfterLimit, true))
            SetCaretPosition(SmCaretPos(pLine, 0), true);

    EndEdit();

    return true;
}

void SmCursor::InsertBrackets(SmBracketType eBracketType) {
    BeginEdit();

    AnnotateSelection();

    
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(pTree);
        OSL_ENSURE(pSNode != NULL, "There must be a selected node if HasSelection()");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(position->CaretPos.pSelectedNode, false);

    
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE( nParentIndex != -1, "pLine must be a subnode of pLineParent!");

    
    SmNodeList *pLineList = NodeToList(pLine);

    
    SmNodeList *pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, position->CaretPos);

    
    SmNode *pBodyNode;
    SmCaretPos PosAfterInsert;
    if(pSelectedNodesList->empty()) {
        pBodyNode = new SmPlaceNode();
        PosAfterInsert = SmCaretPos(pBodyNode, 1);
    } else
        pBodyNode = SmNodeListParser().Parse(pSelectedNodesList);

    delete pSelectedNodesList;

    
    SmToken aTok(TLEFT, '\0', "left", 0, 5);
    SmBraceNode *pBrace = new SmBraceNode(aTok);
    pBrace->SetScaleMode(SCALE_HEIGHT);
    SmNode *pLeft = CreateBracket(eBracketType, true),
           *pRight = CreateBracket(eBracketType, false);
    SmBracebodyNode *pBody = new SmBracebodyNode(SmToken());
    pBody->SetSubNodes(pBodyNode, NULL);
    pBrace->SetSubNodes(pLeft, pBody, pRight);
    pBrace->Prepare(pDocShell->GetFormat(), *pDocShell);

    
    pLineList->insert(it, pBrace);
    
    SmCaretPos pAfter = PatchLineList(pLineList, it);
    if( !PosAfterInsert.IsValid() )
        PosAfterInsert = pAfter;

    
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
}

SmNode *SmCursor::CreateBracket(SmBracketType eBracketType, bool bIsLeft) {
    SmToken aTok;
    if(bIsLeft){
        switch(eBracketType){
            case NoneBrackets:
                aTok = SmToken(TNONE, '\0', "none", TGLBRACES | TGRBRACES, 0);
                break;
            case RoundBrackets:
                aTok = SmToken(TLPARENT, MS_LPARENT, "(", TGLBRACES, 5);
                break;
            case SquareBrackets:
                aTok = SmToken(TLBRACKET, MS_LBRACKET, "[", TGLBRACES, 5);
                break;
            case DoubleSquareBrackets:
                aTok = SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TGLBRACES, 5);
                break;
            case LineBrackets:
                aTok = SmToken(TLLINE, MS_VERTLINE, "lline", TGLBRACES, 5);
                break;
            case DoubleLineBrackets:
                aTok = SmToken(TLDLINE, MS_DVERTLINE, "ldline", TGLBRACES, 5);
                break;
            case CurlyBrackets:
                aTok = SmToken(TLBRACE, MS_LBRACE, "lbrace", TGLBRACES, 5);
                break;
            case AngleBrackets:
                aTok = SmToken(TLANGLE, MS_LMATHANGLE, "langle", TGLBRACES, 5);
                break;
            case CeilBrackets:
                aTok = SmToken(TLCEIL, MS_LCEIL, "lceil", TGLBRACES, 5);
                break;
            case FloorBrackets:
                aTok = SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TGLBRACES, 5);
                break;
        }
    } else {
        switch(eBracketType) {
            case NoneBrackets:
                aTok = SmToken(TNONE, '\0', "none", TGLBRACES | TGRBRACES, 0);
                break;
            case RoundBrackets:
                aTok = SmToken(TRPARENT, MS_RPARENT, ")", TGRBRACES, 5);
                break;
            case SquareBrackets:
                aTok = SmToken(TRBRACKET, MS_RBRACKET, "]", TGRBRACES, 5);
                break;
            case DoubleSquareBrackets:
                aTok = SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TGRBRACES, 5);
                break;
            case LineBrackets:
                aTok = SmToken(TRLINE, MS_VERTLINE, "rline", TGRBRACES, 5);
                break;
            case DoubleLineBrackets:
                aTok = SmToken(TRDLINE, MS_DVERTLINE, "rdline", TGRBRACES, 5);
                break;
            case CurlyBrackets:
                aTok = SmToken(TRBRACE, MS_RBRACE, "rbrace", TGRBRACES, 5);
                break;
            case AngleBrackets:
                aTok = SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TGRBRACES, 5);
                break;
            case CeilBrackets:
                aTok = SmToken(TRCEIL, MS_RCEIL, "rceil", TGRBRACES, 5);
                break;
            case FloorBrackets:
                aTok = SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TGRBRACES, 5);
                break;
        }
    }
    SmNode* pRetVal = new SmMathSymbolNode(aTok);
    pRetVal->SetScaleMode(SCALE_HEIGHT);
    return pRetVal;
}

bool SmCursor::InsertRow() {
    AnnotateSelection();

    
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(pTree);
        OSL_ENSURE(pSNode != NULL, "There must be a selected node if HasSelection()");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(position->CaretPos.pSelectedNode, false);

    
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE( nParentIndex != -1, "pLine must be a subnode of pLineParent!");

    
    SmTableNode  *pTable  = NULL;
    SmMatrixNode *pMatrix = NULL;
    int nTableIndex = nParentIndex;
    if(pLineParent->GetType() == NTABLE)
        pTable = (SmTableNode*)pLineParent;
    
    else if(pLineParent->GetType() == NLINE &&
            pLineParent->GetParent() &&
            pLineParent->GetParent()->GetType() == NTABLE) {
        
        pTable = (SmTableNode*)pLineParent->GetParent();
        nTableIndex = pTable->IndexOfSubNode(pLineParent);
        OSL_ENSURE(nTableIndex != -1, "pLineParent must be a child of its parent!");
    }
    if(pLineParent->GetType() == NMATRIX)
        pMatrix = (SmMatrixNode*)pLineParent;

    
    if(!pTable && !pMatrix)
        return false;

    
    BeginEdit();

    
    SmNodeList *pLineList = NodeToList(pLine);

    
    SmNodeList::iterator it;
    if(HasSelection()) {
        
        it = TakeSelectedNodesFromList(pLineList);
    } else
        it = FindPositionInLineList(pLineList, position->CaretPos);

    
    SmCaretPos PosAfterInsert;

    
    if(pTable) {
        SmNodeList *pNewLineList = new SmNodeList();
        
        pNewLineList->splice(pNewLineList->begin(), *pLineList, it, pLineList->end());
        
        it = pLineList->end();
        if(it != pLineList->begin())
            --it;
        if(pNewLineList->empty())
            pNewLineList->push_front(new SmPlaceNode());
        
        SmNode *pNewLine = SmNodeListParser().Parse(pNewLineList);
        delete pNewLineList;
        
        if(pLineParent->GetType() == NLINE) {
            SmLineNode *pNewLineNode = new SmLineNode(SmToken(TNEWLINE, '\0', "newline"));
            pNewLineNode->SetSubNodes(pNewLine, NULL);
            pNewLine = pNewLineNode;
        }
        
        PosAfterInsert = SmCaretPos(pNewLine, 0);
        
        for( int i = pTable->GetNumSubNodes(); i > nTableIndex + 1; i--)
            pTable->SetSubNode(i, pTable->GetSubNode(i-1));

        
        pTable->SetSubNode(nTableIndex + 1, pNewLine);

        
        if(pTable->GetNumSubNodes() > 2 && pTable->GetToken().eType == TBINOM) {
            SmToken tok = pTable->GetToken();
            tok.eType = TSTACK;
            pTable->SetToken(tok);
        }
    }
    
    else if(pMatrix) {
        
        PosAfterInsert = PatchLineList(pLineList, it);
        
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
    } else
        SAL_WARN("starmath", "We must be either the context of a table or matrix!");

    
    FinishEdit(pLineList, pLineParent, nParentIndex, PosAfterInsert);
    
    
    
    return true;
}

void SmCursor::InsertFraction() {
    AnnotateSelection();

    
    SmNode *pLine;
    if(HasSelection()) {
        SmNode *pSNode = FindSelectedNode(pTree);
        OSL_ENSURE(pSNode != NULL, "There must be a selected node when HasSelection is true!");
        pLine = FindTopMostNodeInLine(pSNode, true);
    } else
        pLine = FindTopMostNodeInLine(position->CaretPos.pSelectedNode, false);

    
    SmStructureNode *pLineParent = pLine->GetParent();
    int nParentIndex = pLineParent->IndexOfSubNode(pLine);
    OSL_ENSURE(nParentIndex != -1, "pLine must be a subnode of pLineParent!");

    
    BeginEdit();

    
    SmNodeList* pLineList = NodeToList(pLine);

    
    SmNodeList* pSelectedNodesList = new SmNodeList();
    SmNodeList::iterator it;
    if(HasSelection())
        it = TakeSelectedNodesFromList(pLineList, pSelectedNodesList);
    else
        it = FindPositionInLineList(pLineList, position->CaretPos);

    
    bool bEmptyFraction = pSelectedNodesList->empty();
    SmNode *pNum = bEmptyFraction
        ? new SmPlaceNode()
        : SmNodeListParser().Parse(pSelectedNodesList);
    SmNode *pDenom = new SmPlaceNode();
    delete pSelectedNodesList;
    pSelectedNodesList = NULL;

    
    SmBinVerNode *pFrac = new SmBinVerNode(SmToken(TOVER, '\0', "over", TGPRODUCT, 0));
    SmNode *pRect = new SmRectangleNode(SmToken());
    pFrac->SetSubNodes(pNum, pRect, pDenom);

    
    SmNodeList::iterator patchIt = pLineList->insert(it, pFrac);
    PatchLineList(pLineList, patchIt);
    PatchLineList(pLineList, it);

    
    SmNode *pSelectedNode = bEmptyFraction ? pNum : pDenom;
    FinishEdit(pLineList, pLineParent, nParentIndex, SmCaretPos(pSelectedNode, 1));
}

void SmCursor::InsertText(OUString aString)
{
    BeginEdit();

    Delete();

    SmToken token;
    token.eType = TIDENT;
    token.cMathChar = '\0';
    token.nGroup = 0;
    token.nLevel = 5;
    token.aText = aString;

    SmTextNode* pText = new SmTextNode(token, FNT_VARIABLE);

    
    pText->Prepare(pDocShell->GetFormat(), *pDocShell);
    pText->AdjustFontDesc();

    SmNodeList* pList = new SmNodeList();
    pList->push_front(pText);
    InsertNodes(pList);

    EndEdit();
}

void SmCursor::InsertElement(SmFormulaElement element){
    BeginEdit();

    Delete();

    
    SmNode* pNewNode = NULL;
    switch(element){
        case BlankElement:
        {
            SmToken token;
            token.nGroup = TGBLANK;
            token.aText = "~";
            pNewNode = new SmBlankNode(token);
        }break;
        case FactorialElement:
        {
            SmToken token(TFACT, MS_FACT, "fact", TGUNOPER, 5);
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case PlusElement:
        {
            SmToken token;
            token.eType = TPLUS;
            token.cMathChar = MS_PLUS;
            token.nGroup = TGUNOPER | TGSUM;
            token.nLevel = 5;
            token.aText = "+";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case MinusElement:
        {
            SmToken token;
            token.eType = TMINUS;
            token.cMathChar = MS_MINUS;
            token.nGroup = TGUNOPER | TGSUM;
            token.nLevel = 5;
            token.aText = "-";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case CDotElement:
        {
            SmToken token;
            token.eType = TCDOT;
            token.cMathChar = MS_CDOT;
            token.nGroup = TGPRODUCT;
            token.aText = "cdot";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case EqualElement:
        {
            SmToken token;
            token.eType = TASSIGN;
            token.cMathChar = MS_ASSIGN;
            token.nGroup = TGRELATION;
            token.aText = "=";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case LessThanElement:
        {
            SmToken token;
            token.eType = TLT;
            token.cMathChar = MS_LT;
            token.nGroup = TGRELATION;
            token.aText = "<";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case GreaterThanElement:
        {
            SmToken token;
            token.eType = TGT;
            token.cMathChar = MS_GT;
            token.nGroup = TGRELATION;
            token.aText = ">";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        case PercentElement:
        {
            SmToken token;
            token.eType = TTEXT;
            token.cMathChar = MS_PERCENT;
            token.nGroup = 0;
            token.aText = "\"%\"";
            pNewNode = new SmMathSymbolNode(token);
        }break;
        default:
            SAL_WARN("starmath", "Element unknown!");
    }
    OSL_ENSURE(pNewNode != NULL, "No new node was created!");
    if(!pNewNode)
        return;

    
    pNewNode->Prepare(pDocShell->GetFormat(), *pDocShell);

    
    SmNodeList* pList = new SmNodeList();
    pList->push_front(pNewNode);
    InsertNodes(pList);

    EndEdit();
}

void SmCursor::InsertSpecial(OUString aString)
{
    BeginEdit();
    Delete();

    aString = comphelper::string::strip(aString, ' ');

    
    SmToken token;
    token.eType = TSPECIAL;
    token.cMathChar = '\0';
    token.nGroup = 0;
    token.nLevel = 5;
    token.aText = aString;
    SmSpecialNode* pSpecial = new SmSpecialNode(token);

    
    pSpecial->Prepare(pDocShell->GetFormat(), *pDocShell);

    
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
            InsertLimit(CSUB, true);
            break;
        case RID_TOX:
            InsertLimit(CSUP, true);
            break;
        case RID_FROMXTOY:
            if(InsertLimit(CSUB, true))
                InsertLimit(CSUP, true);
            break;
        default:
            InsertCommandText(SM_RESSTR(nCommand));
            break;
    }
}

void SmCursor::InsertCommandText(OUString aCommandText) {
    
    SmNode* pSubExpr = SmParser().ParseExpression(aCommandText);

    
    pSubExpr->Prepare(pDocShell->GetFormat(), *pDocShell);

    
    SmNodeList* pLineList = NodeToList(pSubExpr);

    BeginEdit();

    
    Delete();

    
    InsertNodes(pLineList);

    EndEdit();
}

void SmCursor::Copy(){
    if(!HasSelection())
        return;

    
    SmNode* pSNode = FindSelectedNode(pTree);
    
    SmNode* pLine = FindTopMostNodeInLine(pSNode, true);

    
    SmNodeList* pList;
    if(IsLineCompositionNode(pLine))
        pList = CloneLineToList((SmStructureNode*)pLine, true);
    else{
        pList = new SmNodeList();
        
        if(pLine->GetType() == NTEXT) {
            SmTextNode *pText = (SmTextNode*)pLine;
            SmTextNode *pClone = new SmTextNode( pText->GetToken(), pText->GetFontDesc() );
            int start  = pText->GetSelectionStart(),
                length = pText->GetSelectionEnd() - pText->GetSelectionStart();
            pClone->ChangeText(pText->GetText().copy(start, length));
            pClone->SetScaleMode(pText->GetScaleMode());
            pList->push_front(pClone);
        } else {
            SmCloningVisitor aCloneFactory;
            pList->push_front(aCloneFactory.Clone(pLine));
        }
    }

    
    if (pList->size() > 0)
        SetClipboard(pList);
    else
        delete pList;
}

void SmCursor::Paste() {
    BeginEdit();
    Delete();

    if(pClipboard && pClipboard->size() > 0)
        InsertNodes(CloneList(pClipboard));

    EndEdit();
}

SmNodeList* SmCursor::CloneList(SmNodeList* pList){
    SmCloningVisitor aCloneFactory;
    SmNodeList* pClones = new SmNodeList();

    SmNodeList::iterator it;
    for(it = pList->begin(); it != pList->end(); ++it){
        SmNode *pClone = aCloneFactory.Clone(*it);
        pClones->push_back(pClone);
    }

    return pClones;
}

void SmCursor::SetClipboard(SmNodeList* pList){
    if(pClipboard){
        
        SmNodeList::iterator it;
        for(it = pClipboard->begin(); it != pClipboard->end(); ++it)
            delete (*it);
        delete pClipboard;
    }
    pClipboard = pList;
}

SmNode* SmCursor::FindTopMostNodeInLine(SmNode* pSNode, bool MoveUpIfSelected){
    
    if(!pSNode)
        return NULL;

    
    
    
    
    
    
    
    
    while(pSNode->GetParent() &&
          ((MoveUpIfSelected &&
            pSNode->GetParent()->IsSelected()) ||
           IsLineCompositionNode(pSNode->GetParent())))
        pSNode = pSNode->GetParent();
    
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
    return NULL;
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
                LineToList((SmStructureNode*)it.Current(), list);
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

SmNodeList* SmCursor::CloneLineToList(SmStructureNode* pLine, bool bOnlyIfSelected, SmNodeList* pList){
    SmCloningVisitor aCloneFactory;
    SmNodeIterator it(pLine);
    while(it.Next()){
        if( IsLineCompositionNode( it.Current() ) )
            CloneLineToList( (SmStructureNode*)it.Current(), bOnlyIfSelected, pList );
        else if( (!bOnlyIfSelected || it->IsSelected()) && it->GetType() != NERROR ) {
            
            if(it->GetType() == NTEXT) {
                SmTextNode *pText = (SmTextNode*)it.Current();
                SmTextNode *pClone = new SmTextNode( it->GetToken(), pText->GetFontDesc() );
                int start = pText->GetSelectionStart(),
                    length = pText->GetSelectionEnd() - pText->GetSelectionStart();
                pClone->ChangeText(pText->GetText().copy(start, length));
                pClone->SetScaleMode(pText->GetScaleMode());
                pList->push_back(pClone);
            } else
                pList->push_back(aCloneFactory.Clone(it.Current()));
        }
    }
    return pList;
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

    return CountSelectedNodes(pTree) > 1;
}

void SmCursor::FinishEdit(SmNodeList* pLineList,
                          SmStructureNode* pParent,
                          int nParentIndex,
                          SmCaretPos PosAfterEdit,
                          SmNode* pStartLine) {
    
    int entries = pLineList->size();

    
    SmNodeListParser parser;
    SmNode* pLine = parser.Parse(pLineList);
    delete pLineList;

    
    if(pParent->GetType() == NSUBSUP &&
       nParentIndex == 0 &&
       entries > 1) {
        
        SmToken aTok(TLEFT, '\0', "left", 0, 5);
        SmBraceNode *pBrace = new SmBraceNode(aTok);
        pBrace->SetScaleMode(SCALE_HEIGHT);
        SmNode *pLeft  = CreateBracket(RoundBrackets, true),
               *pRight = CreateBracket(RoundBrackets, false);
        SmBracebodyNode *pBody = new SmBracebodyNode(SmToken());
        pBody->SetSubNodes(pLine, NULL);
        pBrace->SetSubNodes(pLeft, pBody, pRight);
        pBrace->Prepare(pDocShell->GetFormat(), *pDocShell);
        pLine = pBrace;
        
        
        
        
        
        
        
        
        
        
        
        
    }

    
    if(!pStartLine)
        pStartLine = pLine;

    
    pParent->SetSubNode(nParentIndex, pLine);

    
    anchor = NULL;
    position = NULL;
    BuildGraph();
    AnnotateSelection(); 

    
    if(!SetCaretPosition(PosAfterEdit, true))
        SetCaretPosition(SmCaretPos(pStartLine, 0), true);

    
    EndEdit();
}

void SmCursor::BeginEdit(){
    if(nEditSections++ > 0) return;

    bIsEnabledSetModifiedSmDocShell = pDocShell->IsEnableSetModified();
    if( bIsEnabledSetModifiedSmDocShell )
        pDocShell->EnableSetModified( sal_False );
}

void SmCursor::EndEdit(){
    if(--nEditSections > 0) return;

    pDocShell->SetFormulaArranged(false);
    
    
    
    
    if ( bIsEnabledSetModifiedSmDocShell )
        pDocShell->EnableSetModified( bIsEnabledSetModifiedSmDocShell );
    
    pDocShell->SetModified(sal_True);
    
    
    pDocShell->nModifyCount++;

    
    
    if( pDocShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        pDocShell->OnDocumentPrinterChanged(0);

    
    RequestRepaint();

    
    OUString formula;
    SmNodeToTextVisitor(pTree, formula);
    
    pDocShell->aText = formula;
    pDocShell->GetEditEngine().QuickInsertText( formula, ESelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL ) );
    pDocShell->GetEditEngine().QuickFormatDoc();
}

void SmCursor::RequestRepaint(){
    SmViewShell *pViewSh = SmGetActiveView();
    if( pViewSh ) {
        if ( SFX_CREATE_MODE_EMBEDDED == pDocShell->GetCreateMode() )
            pDocShell->Repaint();
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
            
            return false;
        }

        sal_uInt16 index = pNode->FindIndex();
        if (index + 1 != pParentNode->GetNumSubNodes()) {
            
            return false;
        }

        pNode = pParentNode;
        if (pNode->GetType() == NBRACEBODY) {
            
            break;
        }
    }

    SmStructureNode* pBraceNodeTmp = pNode->GetParent();
    if (!pBraceNodeTmp || pBraceNodeTmp->GetType() != NBRACE) {
        
        return false;
    }

    SmBraceNode* pBraceNode = static_cast<SmBraceNode*>(pBraceNodeTmp);
    SmMathSymbolNode* pClosingNode = pBraceNode->ClosingBrace();
    if (!pClosingNode) {
        
        return false;
    }

    
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
        *ppBraceNode = static_cast<SmBraceNode*>(pBraceNode);
    }

    return true;
}

void SmCursor::MoveAfterBracket(SmBraceNode* pBraceNode, bool bMoveAnchor)
{
    position->CaretPos.pSelectedNode = pBraceNode;
    position->CaretPos.Index = 1;
    if (bMoveAnchor) {
        anchor->CaretPos.pSelectedNode = pBraceNode;
        anchor->CaretPos.Index = 1;
    }
    RequestRepaint();
}




SmNode* SmNodeListParser::Parse(SmNodeList* list, bool bDeleteErrorNodes){
    pList = list;
    if(bDeleteErrorNodes){
        
        SmNodeList::iterator it = pList->begin();
        while(it != pList->end()) {
            if((*it)->GetType() == NERROR){
                
                delete *it;
                it = pList->erase(it);
            }else
                ++it;
        }
    }
    SmNode* retval = Expression();
    pList = NULL;
    return retval;
}

SmNode* SmNodeListParser::Expression(){
    SmNodeArray NodeArray;
    
    while(Terminal())
        NodeArray.push_back(Relation());

    
    SmStructureNode* pExpr = new SmExpressionNode(SmToken());
    pExpr->SetSubNodes(NodeArray);
    return pExpr;
}

SmNode* SmNodeListParser::Relation(){
    
    SmNode* pLeft = Sum();
    
    while(Terminal() && IsRelationOperator(Terminal()->GetToken())){
        
        SmNode* pOper = Take();
        
        SmNode* pRight = Sum();
        
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Sum(){
    
    SmNode* pLeft = Product();
    
    while(Terminal() && IsSumOperator(Terminal()->GetToken())){
        
        SmNode* pOper = Take();
        
        SmNode* pRight = Product();
        
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Product(){
    
    SmNode* pLeft = Factor();
    
    while(Terminal() && IsProductOperator(Terminal()->GetToken())){
        
        SmNode* pOper = Take();
        
        SmNode* pRight = Factor();
        
        SmStructureNode* pNewNode = new SmBinHorNode(SmToken());
        pNewNode->SetSubNodes(pLeft, pOper, pRight);
        pLeft = pNewNode;
    }
    return pLeft;
}

SmNode* SmNodeListParser::Factor(){
    
    if(!Terminal())
        return Error();
    
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
    SmNode *pArg = NULL;
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
    return token.nGroup & TGRELATION;
}

bool SmNodeListParser::IsSumOperator(const SmToken &token) {
    return token.nGroup & TGSUM;
}

bool SmNodeListParser::IsProductOperator(const SmToken &token) {
    return token.nGroup & TGPRODUCT &&
           token.eType != TWIDESLASH &&
           token.eType != TWIDEBACKSLASH &&
           token.eType != TUNDERBRACE &&
           token.eType != TOVERBRACE &&
           token.eType != TOVER;
}

bool SmNodeListParser::IsUnaryOperator(const SmToken &token) {
    return  token.nGroup & TGUNOPER &&
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
