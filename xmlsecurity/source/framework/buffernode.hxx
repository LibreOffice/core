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

#ifndef _BUFFERNODE_HXX
#define _BUFFERNODE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>

#include <vector>

class ElementMark;
class ElementCollector;

class BufferNode
/****** buffernode.hxx/CLASS BufferNode ***************************************
 *
 *   NAME
 *  BufferNode -- Class to maintain the tree of bufferred elements
 *
 *   FUNCTION
 *  One BufferNode object represents a bufferred element in the document
 *  wrapper component.
 *  All BufferNode objects construct a tree which has the same structure
 *  of all bufferred elements. That is to say, if one bufferred element is
 *  an ancestor of another bufferred element, then the corresponding
 *  BufferNode objects are also in ancestor/descendant relationship.
 *  This class is used to manipulate the tree of bufferred elements.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /* the parent BufferNode */
    BufferNode* m_pParent;

    /* all child BufferNodes */
    std::vector< const BufferNode* > m_vChildren;

    /* all ElementCollector holding this BufferNode */
    std::vector< const ElementCollector* > m_vElementCollectors;

    /*
     * the blocker holding this BufferNode, one BufferNode can have one
     * blocker at most
     */
    ElementMark* m_pBlocker;

    /*
     * whether the element has completely bufferred by the document wrapper
     * component
     */
    bool m_bAllReceived;

    /* the XMLElementWrapper of the bufferred element */
    com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > m_xXMLElement;

private:
    bool isECInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const;
    bool isECOfBeforeModifyInAncestorIncluded(sal_Int32 nIgnoredSecurityId) const;
    bool isBlockerInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const;
    const BufferNode* getNextChild(const BufferNode* pChild) const;

public:
    explicit BufferNode(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& xXMLElement);
    virtual ~BufferNode() {};

    bool isECOfBeforeModifyIncluded(sal_Int32 nIgnoredSecurityId) const;
        void setReceivedAll();
        bool isAllReceived() const;
    void addElementCollector(const ElementCollector* pElementCollector);
    void removeElementCollector(const ElementCollector* pElementCollector);
    ElementMark* getBlocker() const;
    void setBlocker(const ElementMark* pBlocker);
    OUString printChildren() const;
    bool hasAnything() const;
    bool hasChildren() const;
    std::vector< const BufferNode* >* getChildren() const;
    const BufferNode* getFirstChild() const;
    void addChild(const BufferNode* pChild, sal_Int32 nPosition);
    void addChild(const BufferNode* pChild);
    void removeChild(const BufferNode* pChild);
    sal_Int32 indexOfChild(const BufferNode* pChild) const;
    const BufferNode* getParent() const;
    void setParent(const BufferNode* pParent);
    const BufferNode* getNextSibling() const;
    const BufferNode* isAncestor(const BufferNode* pDescendant) const;
    bool isPrevious(const BufferNode* pFollowing) const;
    const BufferNode* getNextNodeByTreeOrder() const;
    com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > getXMLElement() const;
    void setXMLElement(const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& xXMLElement);
    void notifyBranch();
    void elementCollectorNotify();
    void freeAllChildren();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
