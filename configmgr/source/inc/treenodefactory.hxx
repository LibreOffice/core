/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treenodefactory.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#define CONFIGMGR_TREE_NODEFACTORY_HXX

#include "valuenode.hxx"

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable : 4350)
#endif
namespace configmgr
{

    namespace uno = ::com::sun::star::uno;

    //==========================================================================

    class OTreeNodeFactory
    {
    public:
    //= ValueNodes ============================================================
        std::auto_ptr<ValueNode> createValueNode(
                                    rtl::OUString const& aName,
                                    uno::Any const& aValue,
                                    node::Attributes _aAttrs);

        //-----------------------------------------------
        std::auto_ptr<ValueNode> createValueNode(
                                    rtl::OUString const& aName,
                                    uno::Any const& aValue,
                                    uno::Any const& aDefault,
                                    node::Attributes _aAttrs);

        //-----------------------------------------------
        std::auto_ptr<ValueNode>  createNullValueNode(
                                    rtl::OUString const& aName,
                                    uno::Type const& aType,
                                    node::Attributes _aAttrs);

    //= ISubtree ============================================================
        std::auto_ptr<ISubtree> createGroupNode(
                                    rtl::OUString const& aName,
                                    node::Attributes _aAttrs);

        //-----------------------------------------------
        std::auto_ptr<ISubtree> createSetNode(
                                    rtl::OUString const& aName,
                                    rtl::OUString const& _rTemplateName,
                                    rtl::OUString const& _rTemplateModule,
                                    node::Attributes _aAttrs);
    };

    OTreeNodeFactory& getDefaultTreeNodeFactory();
    //==========================================================================
    inline
    std::auto_ptr<INode> base_ptr(std::auto_ptr<ISubtree> pNode)
    { return std::auto_ptr<INode>(pNode.release()); }
    //==========================================================================
    inline
    std::auto_ptr<INode> base_ptr(std::auto_ptr<ValueNode> pNode)
    { return std::auto_ptr<INode>(pNode.release()); }
    //==========================================================================


} // namespace configmgr

#endif
