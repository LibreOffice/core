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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <stdio.h>

#include "treenodefactory.hxx"
#include "valuenode.hxx"
#include "subtree.hxx"
#include "treechangefactory.hxx"
#include "configpath.hxx"

namespace configmgr
{

//= static default ============================================================
OTreeNodeFactory& getDefaultTreeNodeFactory()
{
    static OTreeNodeFactory aDefaultFactory;
    return aDefaultFactory;
}

//= ValueNodes ============================================================

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, _aAttrs) );
}

//-----------------------------------------------

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            uno::Any const& aDefault,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue() || aDefault.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, aDefault, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ValueNode>  OTreeNodeFactory::createNullValueNode(
                            rtl::OUString const& aName,
                            uno::Type const& aType,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aType.getTypeClass() != uno::TypeClass_VOID, "OTreeNodeFactory: Creating a value node having VOID type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aType, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ISubtree> OTreeNodeFactory::createGroupNode(
                            rtl::OUString const& aName,
                            node::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ISubtree> OTreeNodeFactory::createSetNode(
                            rtl::OUString const& aName,
                            rtl::OUString const& _rTemplateName,
                            rtl::OUString const& _rTemplateModule,
                            node::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _rTemplateName, _rTemplateModule, _aAttrs) );
}

//-----------------------------------------------

} // namespace configmgr


