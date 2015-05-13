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

#ifndef INCLUDED_SW_INC_SWNODENUM_HXX
#define INCLUDED_SW_INC_SWNODENUM_HXX

#include <SwNumberTree.hxx>

class SwTextNode;
struct SwPosition;
class SwNumRule;
class SwNumFormat;

class SW_DLLPUBLIC SwNodeNum : public SwNumberTreeNode
{
public:

    explicit SwNodeNum( SwTextNode* pTextNode );
    explicit SwNodeNum( SwNumRule* pNumRule );
    virtual ~SwNodeNum();

    SwNumRule* GetNumRule() const { return mpNumRule;}
    void ChangeNumRule( SwNumRule& rNumRule );
    SwTextNode* GetTextNode() const { return mpTextNode;}

    virtual bool IsNotificationEnabled() const SAL_OVERRIDE;

    virtual bool IsContinuous() const SAL_OVERRIDE;

    virtual bool IsCounted() const SAL_OVERRIDE;

    virtual bool LessThan(const SwNumberTreeNode & rNode) const SAL_OVERRIDE;

    virtual bool IsRestart() const SAL_OVERRIDE;

    virtual SwNumberTree::tSwNumTreeNumber GetStartValue() const SAL_OVERRIDE;

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
    const SwNodeNum* GetPrecedingNodeNumOf( const SwTextNode& rTextNode ) const;

protected:
    virtual SwNumberTreeNode * Create() const SAL_OVERRIDE;

    // --> #i64010#
    virtual bool HasCountedChildren() const SAL_OVERRIDE;
    virtual bool IsCountedForNumbering() const SAL_OVERRIDE;

    // method called before this tree node has been added to the list tree
    virtual void PreAdd() SAL_OVERRIDE;
    // method called at a child after this child has been removed from the list tree
    virtual void PostRemove() SAL_OVERRIDE;
private:
    SwTextNode * mpTextNode;
    SwNumRule * mpNumRule;

    static void _UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum );

    SwNodeNum( const SwNodeNum& ) SAL_DELETED_FUNCTION;
    SwNodeNum& operator=( const SwNodeNum& ) SAL_DELETED_FUNCTION;

    virtual bool IsCountPhantoms() const SAL_OVERRIDE;

    virtual bool IsNotifiable() const SAL_OVERRIDE;

    virtual void NotifyNode() SAL_OVERRIDE;
};

#endif // INCLUDED_SW_INC_SWNODENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
