/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    explicit SwNodeNum( SwTxtNode* pTxtNode );
    explicit SwNodeNum( SwNumRule* pNumRule );
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

    SwPosition GetPosition() const;

    // The number tree root node is deleted, when the corresponding numbering
    // rule is deleted. In this situation the number tree should be empty -
    // still registered text nodes aren't allowed. But it is possible, that
    // text nodes of the undo nodes array are still registered. These will be
    // unregistered.
    // Text nodes of the document nodes array aren't allowed to be registered
    // in this situation - this will be asserted.
    static void HandleNumberTreeRootNodeDelete( SwNodeNum& rNodeNum );

    /** determines the <SwNodeNum> instance, which is preceding the given text node

        #i81002#

        @author OD
    */
    const SwNodeNum* GetPrecedingNodeNumOf( const SwTxtNode& rTxtNode ) const;

protected:
    virtual SwNumberTreeNode * Create() const;

    // --> #i64010#
    virtual bool HasCountedChildren() const;
    virtual bool IsCountedForNumbering() const;
    // <--

    // method called before this tree node has been added to the list tree
    virtual void PreAdd();
    // method called at a child after this child has been removed from the list tree
    virtual void PostRemove();
private:
    SwTxtNode * mpTxtNode;
    SwNumRule * mpNumRule;

    static void _UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum );

    SwNodeNum( const SwNodeNum& );              // no copy constructor
    SwNodeNum& operator=( const SwNodeNum& );   // no assignment operator

    virtual bool IsCountPhantoms() const;

    virtual bool IsNotifiable() const;

    virtual void NotifyNode();
};

#endif // _SW_NODE_NUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
