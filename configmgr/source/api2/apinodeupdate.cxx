/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinodeupdate.cxx,v $
 * $Revision: 1.13 $
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
#include "apinodeupdate.hxx"
#include "apitreeimplobj.hxx"
#include "apitreeaccess.hxx"
#include "noderef.hxx"
#include "configset.hxx"
#include "configgroup.hxx"
#include "configpath.hxx"
#include "apifactory.hxx"

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------

NodeGroupAccess& withDefaultData(NodeGroupAccess& _aGroup)
{
    configuration::GroupDefaulter::ensureDataAvailable(_aGroup.getTreeRef(),_aGroup.getNodeRef(),
                                                        _aGroup.getApiTree().getDefaultProvider());
    return _aGroup;
}
//-----------------------------------------------------------------------------

configuration::GroupUpdater NodeGroupAccess::getNodeUpdater()
{
    return configuration::GroupUpdater(getTree(),getNodeRef(), getApiTree().getProvider().getTypeConverter());
}
//-----------------------------------------------------------------------------

configuration::GroupDefaulter NodeGroupAccess::getNodeDefaulter()
{
    return configuration::GroupDefaulter(getTree(),getNodeRef(), getApiTree().getDefaultProvider());
}
//-----------------------------------------------------------------------------

configuration::SetElementFactory NodeTreeSetAccess::getElementFactory()
{
    using namespace configuration;
    TemplateProvider aProvider = SetElementFactory::findTemplateProvider(getTree(),getNodeRef());
    return SetElementFactory(aProvider);
}
//-----------------------------------------------------------------------------

configuration::SetDefaulter NodeSetAccess::getNodeDefaulter()
{
    return configuration::SetDefaulter(getTree(),getNodeRef(), getApiTree().getDefaultProvider());
}
//-----------------------------------------------------------------------------

configuration::TreeSetUpdater NodeTreeSetAccess::getNodeUpdater()
{
    return configuration::TreeSetUpdater(getTree(),getNodeRef(),getElementInfo());
}
//-----------------------------------------------------------------------------


configuration::ValueSetUpdater NodeValueSetAccess::getNodeUpdater()
{
    return configuration::ValueSetUpdater(getTree(),getNodeRef(),getElementInfo(), getApiTree().getProvider().getTypeConverter());
}
//-----------------------------------------------------------------------------

void attachSetElement(NodeTreeSetAccess& aSet, SetElement& aElement)
{
    using configuration::NodeID;
    OSL_ENSURE( NodeID(aSet.getTreeRef(),aSet.getNodeRef()) ==
                NodeID(aElement.getTreeRef().getContextTree(),aElement.getTreeRef().getContextNode()),
                "ERROR: Attaching an unrelated SetElement to a SetInfoAccess");

    aElement.haveNewParent(&aSet);
}
//-----------------------------------------------------------------------------

bool attachSetElement(NodeTreeSetAccess& aSet, configuration::ElementTree const& aElementTree)
{
    using configuration::NodeID;
    OSL_ENSURE( NodeID(aSet.getTreeRef(),aSet.getNodeRef()) ==
                NodeID(aElementTree.getTree().getContextTree(),aElementTree.getTree().getContextNode()),
                "ERROR: Attaching an unrelated ElementTree to a SetInfoAccess");

    Factory& rFactory = aSet.getFactory();

    configuration::ElementRef aElementRef( aElementTree.getImpl() ); // no other conversion available
    if (SetElement* pSetElement = rFactory.findSetElement(aElementRef))
    {
        // the factory always does an extra acquire
        UnoInterfaceRef xReleaseSetElement(pSetElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);

        attachSetElement(aSet, *pSetElement);
        return true;
    }
    else
        return false;

}
//-----------------------------------------------------------------------------

void detachSetElement(SetElement& aElement)
{
    OSL_ENSURE( aElement.getTreeRef().getContextTree().isEmpty(),
                "ERROR: Detaching a SetElement that has a parent");

    aElement.haveNewParent(0);
}
//-----------------------------------------------------------------------------

bool detachSetElement(Factory& rFactory, configuration::ElementRef const& aElementTree)
{
    OSL_ENSURE( aElementTree.getTreeRef().getContextTree().isEmpty(),
                "ERROR: Detaching an ElementTree that has a parent");

    if (SetElement* pSetElement = rFactory.findSetElement(aElementTree))
    {
        // the factory always does an extra acquire
        UnoInterfaceRef xReleaseSetElement(pSetElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);

        detachSetElement(*pSetElement);
        return true;
    }
    else
        return false;

}
//-----------------------------------------------------------------------------

UpdateGuardImpl::UpdateGuardImpl(NodeGroupAccess& rNode)
: m_rNode(rNode)
{
    rNode.checkAlive();
}
//-----------------------------------------------------------------------------

UpdateGuardImpl::UpdateGuardImpl(NodeSetAccess& rNode)
: m_rNode(rNode)
{
}
//-----------------------------------------------------------------------------

UpdateGuardImpl::~UpdateGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------
    }
}
