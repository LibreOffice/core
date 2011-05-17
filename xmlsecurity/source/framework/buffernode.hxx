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

#ifndef _BUFFERNODE_HXX
#define _BUFFERNODE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

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
    rtl::OUString printChildren() const;
    bool hasAnything() const;
    bool hasChildren() const;
    std::vector< const BufferNode* >* getChildren() const;
    const BufferNode* getFirstChild() const;
    void addChild(const BufferNode* pChild, sal_Int32 nPosition);
    void addChild(const BufferNode* pChild);
    void removeChild(const BufferNode* pChild);
    sal_Int32 indexOfChild(const BufferNode* pChild) const;
    const BufferNode* childAt(sal_Int32 nIndex) const;
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
    void notifyAncestor();
    void elementCollectorNotify();
    void freeAllChildren();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
