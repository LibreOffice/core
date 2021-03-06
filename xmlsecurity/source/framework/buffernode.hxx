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

#pragma once

#include <com/sun/star/uno/Reference.hxx>

#include <memory>
#include <vector>

namespace com::sun::star::xml::wrapper
{
class XXMLElementWrapper;
}

class ElementMark;
class ElementCollector;

class BufferNode final
/****** buffernode.hxx/CLASS BufferNode ***************************************
 *
 *   NAME
 *  BufferNode -- Class to maintain the tree of buffered elements
 *
 *   FUNCTION
 *  One BufferNode object represents a buffered element in the document
 *  wrapper component.
 *  All BufferNode objects construct a tree which has the same structure
 *  of all buffered elements. That is to say, if one buffered element is
 *  an ancestor of another buffered element, then the corresponding
 *  BufferNode objects are also in ancestor/descendant relationship.
 *  This class is used to manipulate the tree of buffered elements.
 ******************************************************************************/
{
private:
    /* the parent BufferNode */
    BufferNode* m_pParent;

    /* all child BufferNodes */
    std::vector<std::unique_ptr<BufferNode>> m_vChildren;

    /* all ElementCollector holding this BufferNode */
    std::vector<const ElementCollector*> m_vElementCollectors;

    /*
     * the blocker holding this BufferNode, one BufferNode can have one
     * blocker at most
     */
    ElementMark* m_pBlocker;

    /*
     * whether the element has completely buffered by the document wrapper
     * component
     */
    bool m_bAllReceived;

    /* the XMLElementWrapper of the buffered element */
    css::uno::Reference<css::xml::wrapper::XXMLElementWrapper> m_xXMLElement;

private:
    bool isECInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const;
    bool isECOfBeforeModifyInAncestorIncluded(sal_Int32 nIgnoredSecurityId) const;
    bool isBlockerInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const;
    const BufferNode* getNextChild(const BufferNode* pChild) const;

public:
    explicit BufferNode(
        const css::uno::Reference<css::xml::wrapper::XXMLElementWrapper>& xXMLElement);

    bool isECOfBeforeModifyIncluded(sal_Int32 nIgnoredSecurityId) const;
    void setReceivedAll();
    bool isAllReceived() const { return m_bAllReceived; }
    void addElementCollector(const ElementCollector* pElementCollector);
    void removeElementCollector(const ElementCollector* pElementCollector);
    ElementMark* getBlocker() const { return m_pBlocker; }
    void setBlocker(const ElementMark* pBlocker);
    OUString printChildren() const;
    bool hasAnything() const;
    bool hasChildren() const;
    std::vector<std::unique_ptr<BufferNode>> const& getChildren() const;
    std::vector<std::unique_ptr<BufferNode>> releaseChildren();
    const BufferNode* getFirstChild() const;
    void addChild(std::unique_ptr<BufferNode> pChild, sal_Int32 nPosition);
    void addChild(std::unique_ptr<BufferNode> pChild);
    void removeChild(const BufferNode* pChild);
    sal_Int32 indexOfChild(const BufferNode* pChild) const;
    const BufferNode* getParent() const { return m_pParent; }
    void setParent(const BufferNode* pParent);
    const BufferNode* getNextSibling() const;
    const BufferNode* isAncestor(const BufferNode* pDescendant) const;
    bool isPrevious(const BufferNode* pFollowing) const;
    const BufferNode* getNextNodeByTreeOrder() const;
    const css::uno::Reference<css::xml::wrapper::XXMLElementWrapper>& getXMLElement() const
    {
        return m_xXMLElement;
    }
    void
    setXMLElement(const css::uno::Reference<css::xml::wrapper::XXMLElementWrapper>& xXMLElement);
    void notifyBranch();
    void elementCollectorNotify();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
