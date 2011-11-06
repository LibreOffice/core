/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SW_NODE_NUM_HXX
#define _SW_NODE_NUM_HXX

#include <SwNumberTree.hxx>

class SwTxtNode;
struct SwPosition;
class SwNumRule;
class SwNumFmt;

class SW_DLLPUBLIC SwNodeNum : public SwNumberTreeNode
{
public:
    // --> OD 2008-02-19 #refactorlists#
    explicit SwNodeNum( SwTxtNode* pTxtNode );
    explicit SwNodeNum( SwNumRule* pNumRule );
    // <--
    virtual ~SwNodeNum();

    SwNumRule* GetNumRule() const;
    void ChangeNumRule( SwNumRule& rNumRule );
    SwTxtNode* GetTxtNode() const;

    virtual bool IsNotificationEnabled() const;

    virtual bool IsContinuous() const;

    virtual bool IsCounted() const;

    virtual bool LessThan(const SwNumberTreeNode & rNode) const;

    virtual bool IsRestart() const;

    virtual SwNumberTree::tSwNumTreeNumber GetStartValue() const;

//    String ToString() const;

    SwPosition GetPosition() const;

    // --> OD 2006-03-07 #131436#
    // The number tree root node is deleted, when the corresponding numbering
    // rule is deleted. In this situation the number tree should be empty -
    // still registered text nodes aren't allowed. But it is possible, that
    // text nodes of the undo nodes array are still registered. These will be
    // unregistered.
    // Text nodes of the document nodes array aren't allowed to be registered
    // in this situation - this will be asserted.
    static void HandleNumberTreeRootNodeDelete( SwNodeNum& rNodeNum );
    // <--

    /** determines the <SwNodeNum> instance, which is preceding the given text node

        OD 2007-09-06 #i81002#

        @author OD
    */
    const SwNodeNum* GetPrecedingNodeNumOf( const SwTxtNode& rTxtNode ) const;

protected:
    virtual SwNumberTreeNode * Create() const;

    // --> OD 2006-04-26 #i64010#
    virtual bool HasCountedChildren() const;
    virtual bool IsCountedForNumbering() const;
    // <--

    // --> OD 2008-02-19 #refactorlists#
    // method called before this tree node has been added to the list tree
    virtual void PreAdd();
    // method called at a child after this child has been removed from the list tree
    virtual void PostRemove();
    // <--
private:
    SwTxtNode * mpTxtNode;
    SwNumRule * mpNumRule;

    // --> OD 2006-03-07 #131436#
    static void _UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum );
    // <--

    SwNodeNum( const SwNodeNum& );              // no copy constructor
    SwNodeNum& operator=( const SwNodeNum& );   // no assignment operator

    virtual bool IsCountPhantoms() const;

    virtual bool IsNotifiable() const;

    virtual void NotifyNode();
};

#endif // _SW_NODE_NUM_HXX
