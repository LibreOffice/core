/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeiterators.cxx,v $
 * $Revision: 1.5 $
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

#include "treeiterators.hxx"

#include "apitypes.hxx"
#include "configpath.hxx"
#include "attributes.hxx"
#include "valueref.hxx"
#include "propertyinfohelper.hxx"

// .......................................................................
namespace configmgr
{
// .......................................................................
    namespace configapi
    {
        // ===================================================================
        // = CollectNodeNames
        // ===================================================================
        // -------------------------------------------------------------------
        CollectPropertyInfo::Result CollectNodeNames::handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
        {
            m_aList.push_back(aTree->getSimpleNodeName(aNode.getOffset()));
            return CONTINUE;
        }

        // -------------------------------------------------------------------
        CollectPropertyInfo::Result CollectNodeNames::handle(rtl::Reference< configuration::Tree > const&, configuration::ValueRef const& aNode)
        {
            m_aList.push_back(aNode.m_sNodeName);
            return CONTINUE;
        }

        // ===================================================================
        // = CollectPropertyInfo
        // ===================================================================
        // -------------------------------------------------------------------
        CollectNodeNames::Result CollectPropertyInfo::handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
        {
            rtl::OUString aName       = aTree->getSimpleNodeName(aNode.getOffset());
            node::Attributes    aAttributes = aTree->getAttributes(aNode);
            uno::Type   aApiType    = getUnoInterfaceType();

            m_aList.push_back( helperMakeProperty(aName,aAttributes,aApiType,aTree->hasNodeDefault(aNode)) );
            return CONTINUE;
        }

        // -------------------------------------------------------------------
        CollectNodeNames::Result CollectPropertyInfo::handle(rtl::Reference< configuration::Tree > const& aTree, configuration::ValueRef const& aNode)
        {
            rtl::OUString aName       = aNode.m_sNodeName;
            node::Attributes    aAttributes = aTree->getAttributes(aNode);
            uno::Type   aApiType    = aTree->getUnoType(aNode);

            m_aList.push_back( helperMakeProperty(aName,aAttributes,aApiType,aTree->hasNodeDefault(aNode)) );
            return CONTINUE;
        }
// .......................................................................
    }   // namespace configapi

// .......................................................................
}   // namespace configmgr
// .......................................................................

