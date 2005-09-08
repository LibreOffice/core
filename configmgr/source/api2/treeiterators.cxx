/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeiterators.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:22:03 $
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
        using configuration::Name;
        using node::Attributes;

        // ===================================================================
        // = CollectNodeNames
        // ===================================================================
        // -------------------------------------------------------------------
        CollectPropertyInfo::Result CollectNodeNames::handle(configuration::Tree const& aTree, configuration::NodeRef const& aNode)
        {
            m_aList.push_back(aTree.getName(aNode).toString());
            return CONTINUE;
        }

        // -------------------------------------------------------------------
        CollectPropertyInfo::Result CollectNodeNames::handle(configuration::Tree const& aTree, configuration::ValueRef const& aNode)
        {
            m_aList.push_back(aTree.getName(aNode).toString());
            return CONTINUE;
        }

        // ===================================================================
        // = CollectPropertyInfo
        // ===================================================================
        // -------------------------------------------------------------------
        CollectNodeNames::Result CollectPropertyInfo::handle(configuration::Tree const& aTree, configuration::NodeRef const& aNode)
        {
            Name        aName       = aTree.getName(aNode);
            Attributes  aAttributes = aTree.getAttributes(aNode);
            uno::Type   aApiType    = getUnoInterfaceType();

            m_aList.push_back( helperMakeProperty(aName,aAttributes,aApiType,aTree.hasNodeDefault(aNode)) );
            return CONTINUE;
        }

        // -------------------------------------------------------------------
        CollectNodeNames::Result CollectPropertyInfo::handle(configuration::Tree const& aTree, configuration::ValueRef const& aNode)
        {
            Name        aName       = aTree.getName(aNode);
            Attributes  aAttributes = aTree.getAttributes(aNode);
            uno::Type   aApiType    = aTree.getUnoType(aNode);

            m_aList.push_back( helperMakeProperty(aName,aAttributes,aApiType,aTree.hasNodeDefault(aNode)) );
            return CONTINUE;
        }
// .......................................................................
    }   // namespace configapi

// .......................................................................
}   // namespace configmgr
// .......................................................................

