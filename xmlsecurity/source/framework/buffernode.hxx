/*************************************************************************
 *
 *  $RCSfile: buffernode.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BUFFERNODE_HXX
#define _BUFFERNODE_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_WRAPPER_XXMLELEMENTWRAPPER_HPP_
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#endif

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
 *   HISTORY
 *  05.01.2004 -    implemented
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

