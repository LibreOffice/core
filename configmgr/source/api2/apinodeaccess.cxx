/*************************************************************************
 *
 *  $RCSfile: apinodeaccess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-13 12:22:35 $
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
#include <stdio.h>
#include "apinodeaccess.hxx"

#include "apitreeimplobj.hxx"
#include "apifactory.hxx"

#include "apitreeaccess.hxx"
#include "noderef.hxx"
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

configuration::NodeRef NodeAccess::getNode() const
{
    return doGetNode();
}
//-----------------------------------------------------------------------------

configuration::Tree NodeAccess::getTree() const
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
    getApiTree().disposeNode(getNode(), getUnoInstance());
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

ISynchronizedData const* NodeAccess::getDataLock() const
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

ISynchronizedData const* NodeAccess::getProviderLock() const
{
    return getApiTree().getProviderLock();
}
//-----------------------------------------------------------------------------
osl::Mutex& NodeAccess::getApiLock()
{
    return getApiTree().getApiLock();
}
//-----------------------------------------------------------------------------

UnoAny  makeElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::NodeRef const& aNode)
{
    UnoAny aRet;
    if (!aTree.isEmpty() && aNode.isValid())
    {
        if (configuration::isSimpleValue(aTree,aNode))
        {
            aRet = configuration::getSimpleValue(aTree,aNode);
        }
        else if (!aTree.isEmpty() && aNode.isValid())
        {
            aRet <<= rFactory.makeUnoElement(aTree,aNode);
        }
    }
    return aRet;
}
//-----------------------------------------------------------------------------

UnoAny  makeElement(configapi::Factory& rFactory, configuration::ElementTree const& aTree)
{
    UnoAny aRet;
    if (aTree.isValid())
    {
        aRet <<= rFactory.makeUnoSetElement(aTree);
    }
    return aRet;
}
//-----------------------------------------------------------------------------

configuration::ElementTree extractElementTree(configapi::Factory& rFactory, UnoAny const& aElement, configuration::SetElementInfo const& aElementInfo )
{
    using configuration::ElementTree;
    configuration::TemplateHolder const aRequestedTemplate = aElementInfo.getTemplate();
    OSL_ENSURE(aRequestedTemplate.isValid(), "ERROR: Need a template to extract a matching set element");
    if (!aRequestedTemplate.isValid())
        return ElementTree(0);

    ElementTree aRet(0);

    if (SetElement* pSetElement = rFactory.extractSetElement(aElement))
    {
        configuration::TemplateHolder aFoundTemplate = pSetElement->getTemplateInfo().getTemplate();
        if (!aFoundTemplate.isValid())
            return ElementTree(0);

        if (aFoundTemplate != aRequestedTemplate)
            throw configuration::TypeMismatch(aFoundTemplate->getPath().toString(), aRequestedTemplate->getPath().toString());

        aRet = pSetElement->getElementTree();
    }
    return aRet;
}
//-----------------------------------------------------------------------------

SetElement* findSetElement(Factory& rFactory, configuration::ElementTree const& aElementTree)
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
    configuration::ElementTree aElement = configuration::ElementTree::extract(getTree());
    OSL_ENSURE(aElement.isValid(), "INTERNAL ERROR: This must be an element tree ");

    return configuration::SetElementInfo(aElement.getTemplate());
}
//-----------------------------------------------------------------------------

NodeReadGuardImpl::NodeReadGuardImpl(NodeAccess& rNode) throw()
: m_aLock(rNode.getDataLock())
, m_rNode(rNode)
{
    rNode.checkAlive();
}
//-----------------------------------------------------------------------------

NodeReadGuardImpl::~NodeReadGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------
    }
}
