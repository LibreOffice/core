/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinodeaccess.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "apinodeaccess.hxx"

#include "apitreeimplobj.hxx"
#include "apifactory.hxx"

#include "apitreeaccess.hxx"
#include "noderef.hxx"

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
