/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apinodeaccess.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:02:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "apinodeaccess.hxx"

#include "apitreeimplobj.hxx"
#include "apifactory.hxx"

#include "apitreeaccess.hxx"

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

#include "anynoderef.hxx"
#include "valueref.hxx"

#include "configset.hxx"
#include "configpath.hxx"
#include "confignotifier.hxx"

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------

NodeAccess::~NodeAccess()
{
}
//-----------------------------------------------------------------------------

configuration::NodeRef NodeAccess::getNodeRef() const
{
    return doGetNode();
}
//-----------------------------------------------------------------------------

configuration::TreeRef  NodeAccess::getTreeRef() const
{
    return getApiTree().getTree();
}
//-----------------------------------------------------------------------------

configuration::Tree NodeAccess::getTree() const
{
    return configuration::Tree(getApiTree().getTree());
}
//-----------------------------------------------------------------------------

void NodeAccess::checkAlive() const
{
    getApiTree().checkAlive();
}
//-----------------------------------------------------------------------------

void NodeAccess::disposeNode()
{
    getApiTree().disposeNode(getNodeRef(), getUnoInstance());
}
//-----------------------------------------------------------------------------


Factory& NodeAccess::getFactory() const
{
    return getApiTree().getFactory();
}
//-----------------------------------------------------------------------------

Notifier NodeAccess::getNotifier() const
{
    return getApiTree().getNotifier();
}

//-----------------------------------------------------------------------------

UnoAny  makeElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::AnyNodeRef const& aNode)
{
    if (!aTree.isEmpty() && aNode.isValid())
    {
        if (aNode.isNode())
        {
            NodeRef aInnerNode = aNode.toNode();

            if (configuration::isStructuralNode(aTree,aInnerNode))
                return uno::makeAny( rFactory.makeUnoElement(aTree,aInnerNode) );

            else
                return configuration::getSimpleElementValue(aTree,aInnerNode);
        }
        else
        {
            return configuration::getSimpleValue(aTree,aNode.toValue());
        }
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

UnoAny  makeInnerElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::NodeRef const& aNode)
{
    if (!aTree.isEmpty() && aNode.isValid())
    {
        OSL_ENSURE(configuration::isStructuralNode(aTree,aNode), "Trying to makeInnerElement for a simple value");

        return uno::makeAny( rFactory.makeUnoElement(aTree,aNode) );
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

UnoAny  makeElement(configapi::Factory& rFactory, configuration::ElementTree const& aTree)
{
    if (aTree.isValid())
    {
        return uno::makeAny( rFactory.makeUnoSetElement(aTree) );
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

configuration::ElementRef extractElementRef(configapi::Factory& rFactory, UnoAny const& aElement, configuration::TemplateInfo const& aTemplateInfo )
{
    using configuration::ElementRef;
    configuration::TemplateHolder const aRequestedTemplate = aTemplateInfo.getTemplate();
    OSL_ENSURE(aRequestedTemplate.is(), "ERROR: Need a template to extract a matching set element");
    if (!aRequestedTemplate.is())
        return ElementRef(NULL);

    if (SetElement* pSetElement = rFactory.extractSetElement(aElement))
    {
        configuration::TemplateHolder aFoundTemplate = pSetElement->getTemplateInfo().getTemplate();
        if (aFoundTemplate.is())
        {
            if (aFoundTemplate != aRequestedTemplate)
                throw configuration::TypeMismatch(aFoundTemplate->getPathString(), aRequestedTemplate->getPathString());

            return pSetElement->getElementRef( );
        }
    }
    return ElementRef(NULL);
}
//-----------------------------------------------------------------------------

configuration::ElementTree extractElementTree(configapi::Factory& rFactory, UnoAny const& aElement, configuration::SetElementInfo const& aElementInfo )
{
    using namespace configuration;
    ElementRef aExtractedRef = extractElementRef(rFactory,aElement,aElementInfo.getTemplateInfo());
    return aExtractedRef.getElementTree();
}
//-----------------------------------------------------------------------------

SetElement* findSetElement(Factory& rFactory, configuration::ElementRef const& aElementTree)
{
    SetElement* pSetElement = rFactory.findSetElement(aElementTree);
    if (pSetElement)
    {
        // the factory always does an extra acquire
        pSetElement->getUnoInstance()->release();
    }
    return pSetElement;
}
//-----------------------------------------------------------------------------

configuration::SetElementInfo NodeSetInfoAccess::getElementInfo() const
{
    using configuration::SetElementInfo;
    using configuration::TemplateHolder;

    TemplateHolder aTemplate = SetElementInfo::extractElementInfo(getTree(),getNodeRef());

    OSL_ENSURE(aTemplate.is(), "ERROR: Set must have an element template");

    return SetElementInfo(aTemplate);
}

}
}
