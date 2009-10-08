/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinodeaccess.cxx,v $
 * $Revision: 1.16 $
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

rtl::Reference< configuration::Tree > NodeAccess::getTreeRef() const
{
    return getApiTree().getTree();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::Tree > NodeAccess::getTree() const
{
    return getApiTree().getTree();
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

uno::Any    makeElement(configapi::Factory& rFactory, rtl::Reference< configuration::Tree > const& aTree, configuration::AnyNodeRef const& aNode)
{
    if (!configuration::isEmpty(aTree.get()) && aNode.isValid())
    {
        if (aNode.isNode())
        {
            configuration::NodeRef aInnerNode = aNode.toNode();

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

    return uno::Any();
}
//-----------------------------------------------------------------------------

uno::Any    makeInnerElement(configapi::Factory& rFactory, rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    if (!configuration::isEmpty(aTree.get()) && aNode.isValid())
    {
        OSL_ENSURE(configuration::isStructuralNode(aTree,aNode), "Trying to makeInnerElement for a simple value");

        return uno::makeAny( rFactory.makeUnoElement(aTree,aNode) );
    }

    return uno::Any();
}
//-----------------------------------------------------------------------------

uno::Any    makeElement(configapi::Factory& rFactory, rtl::Reference< configuration::ElementTree > const& aTree)
{
    if (aTree.is())
    {
        return uno::makeAny( rFactory.makeUnoSetElement(aTree) );
    }

    return uno::Any();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::ElementTree > extractElementTree(configapi::Factory& rFactory, uno::Any const& aElement, rtl::Reference< configuration::Template > const& aTemplate )
{
    OSL_ENSURE(aTemplate.is(), "ERROR: Need a template to extract a matching set element");

    if (SetElement* pSetElement = rFactory.extractSetElement(aElement))
    {
        rtl::Reference<configuration::Template> aFoundTemplate = pSetElement->getTemplateInfo();
        if (aFoundTemplate.is())
        {
            if (aFoundTemplate != aTemplate)
                throw configuration::TypeMismatch(aFoundTemplate->getPathString(), aTemplate->getPathString());

            return pSetElement->getElementRef( );
        }
    }
    return rtl::Reference< configuration::ElementTree >();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::Template > NodeSetInfoAccess::getElementInfo() const
{
    rtl::Reference<configuration::Template> aTemplate = getTree()->extractElementInfo(getNodeRef());

    OSL_ENSURE(aTemplate.is(), "ERROR: Set must have an element template");

    return aTemplate;
}

}
}
