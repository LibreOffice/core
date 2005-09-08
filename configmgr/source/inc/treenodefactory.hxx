/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treenodefactory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:59:56 $
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

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#define CONFIGMGR_TREE_NODEFACTORY_HXX

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif


namespace configmgr
{

    namespace uno = ::com::sun::star::uno;

    //==========================================================================
    namespace configuration { class Name;}

    //==========================================================================

    class OTreeNodeFactory
    {
    public:
        typedef configuration::Name Name;
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
        //-----------------------------------------------

    //= special case: Dummy ISubtree ============================================================
        static std::auto_ptr<ISubtree> createDummyTree(
                                    Name const& _aName,
                                    Name const& _aElementTypeName);

        //-----------------------------------------------
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
