/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configgroup.cxx,v $
 * $Revision: 1.14.14.1 $
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

#include "configgroup.hxx"
#include "configset.hxx"
#include "valueref.hxx"
#include "anynoderef.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "tree.hxx"
#include "groupnodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "typeconverter.hxx"
#include "tracer.hxx"
#include <com/sun/star/script/XTypeConverter.hpp>

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
// class GroupUpdateHelper
//-----------------------------------------------------------------------------

GroupUpdateHelper::GroupUpdateHelper(rtl::Reference< Tree > const& aParentTree, NodeRef const& aGroupNode)
: m_aTree(aParentTree)
, m_aNode(aGroupNode)
{
    implValidateTree(m_aTree);
    implValidateNode(m_aTree,m_aNode);

    if (! view::ViewTreeAccess(m_aTree.get()).isGroupNode(m_aNode) )
        throw Exception("INTERNAL ERROR: Group Member Update: node is not a group");
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateTree(rtl::Reference< Tree > const& aTree) const
{
    if (isEmpty(aTree.get()))
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL tree");

    // check for proper nesting
    for(rtl::Reference<Tree> aTestTree = aTree;
        aTestTree != m_aTree;           // search this as ancestor tree
        aTestTree = aTestTree->getContextTree() )
    {
        if (!aTestTree.is()) // no more trees to look for
            throw Exception("INTERNAL ERROR: Group Member Update: improper tree relationship");
    }
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode) const
{
    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree->isValidNode(aNode.getOffset()))
        throw Exception("INTERNAL ERROR: Group Member Update: node does not match tree");
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateNode(rtl::Reference< Tree > const& aTree, ValueRef const& aNode) const
{
    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree->isValidValueNode(aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: changed node does not match tree");

    if (aTree->getAttributes(aNode).isReadonly())
        throw ConstraintViolation( "Group Member Update: Node is read-only !" );

}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::validateNode(ValueRef const& aNode) const
{
    implValidateNode(m_aTree,aNode);
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::validateNode(NodeRef const& aNode) const
{
    implValidateNode(m_aTree,aNode);
}
//-----------------------------------------------------------------------------

/** a helper that gets the UNO <type scope='com::sun::star::uno'>Type</type>
    for a UNO <type scope='com::sun::star::uno'>Any</type>.
*/
static inline com::sun::star::uno::Type getUnoAnyType()
{
    com::sun::star::uno::Any const * const selectAny = 0;
    return ::getCppuType(selectAny);
}
//-----------------------------------------------------------------------------

bool isPossibleValueType(com::sun::star::uno::Type const& aValueType)
{
    switch(aValueType.getTypeClass())
    {
    case uno::TypeClass_BOOLEAN:
    case uno::TypeClass_SHORT:
    case uno::TypeClass_LONG:
    case uno::TypeClass_HYPER:
    case uno::TypeClass_DOUBLE:
    case uno::TypeClass_STRING:
        return true;

    case uno::TypeClass_SEQUENCE:
        switch(getSequenceElementType(aValueType).getTypeClass())
        {
        case uno::TypeClass_BYTE:  // scalar binary

        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_HYPER:
        case uno::TypeClass_DOUBLE:
        case uno::TypeClass_STRING:
            return true;

        case uno::TypeClass_SEQUENCE:
            {
                uno::Sequence< uno::Sequence< sal_Int8 > > const * const forBinaryList = 0;
                return !!(aValueType == ::getCppuType(forBinaryList));
            }

        default:
            return false;
        }

    default:
        return false;
    }
}

//-----------------------------------------------------------------------------
bool convertCompatibleValue(com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xTypeConverter, uno::Any& rConverted, com::sun::star::uno::Any const& rNewValue, com::sun::star::uno::Type const& rTargetType)
{
    OSL_ASSERT( isPossibleValueType(rTargetType) );

    if (rTargetType == rNewValue.getValueType())
    {
        rConverted = rNewValue;
        return true;
    }

    if (xTypeConverter.is())
    try
    {
        rConverted = xTypeConverter->convertTo(rNewValue,rTargetType);

        OSL_ASSERT( rConverted.getValueType() == rTargetType );
    }
    catch(uno::RuntimeException&) { throw; }
    catch(css::lang::IllegalArgumentException&)
    {
        // try to do more conversion here ?!
        return false;
    }
    catch(css::script::CannotConvertException&)
    {
        // try to do more conversion here ?!
        return false;
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(sal_False, "ValueUpdater::convertValue : generic exception ... thought we caught all allowed exceptions !");
        // try to do more conversion here ?!
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupUpdater
//-----------------------------------------------------------------------------

GroupUpdater::GroupUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aGroupNode, com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xConverter)
: m_aHelper(aParentTree,aGroupNode)
, m_xTypeConverter(xConverter)
{
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any GroupUpdater::implValidateValue(rtl::Reference< Tree > const& aTree, ValueRef const& aNode, com::sun::star::uno::Any const& aValue) const
{
    com::sun::star::uno::Type aValueType    = aValue.getValueType();
    com::sun::star::uno::Type aTargetType = aTree->getUnoType(aNode);

    OSL_ENSURE( aTargetType.getTypeClass() == uno::TypeClass_ANY || isPossibleValueType(aTargetType),
                "Invalid value type found on existing property" );

    OSL_ASSERT( aValueType.getTypeClass() != uno::TypeClass_ANY);

    com::sun::star::uno::Any aRet;

    if (!aValue.hasValue())
    {
        if (!aTree->getAttributes(aNode).isNullable())
        {
            rtl::OString sError("Group Member Update: Node (");
            sError += OUSTRING2ASCII(aNode.m_sNodeName);
            sError += ") is not nullable !";
            throw ConstraintViolation( sError );
        }
        OSL_ASSERT( !aRet.hasValue() );
    }

    else if (aValueType == aTargetType)
    {
        aRet = aValue;
    }

    else if (aTargetType == getUnoAnyType())
    {
        if ( ! isPossibleValueType(aValueType) )
            throw TypeMismatch(aValueType.getTypeName(), aTargetType.getTypeName(), " - new property value has no legal configuration data type");

        // OK - any type
        aRet = aValue;
    }

    else
    {
        if (!convertCompatibleValue(m_xTypeConverter, aRet, aValue,aTargetType))
            throw TypeMismatch(aValueType.getTypeName(), aTargetType.getTypeName(), " cannot set incompatible value");
    }


    OSL_ASSERT( !aRet.hasValue() || isPossibleValueType(aRet.getValueType()) );

    return aRet;
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetValue(ValueRef const& aValueNode, com::sun::star::uno::Any const& newValue )
{
    m_aHelper.validateNode(aValueNode);

    com::sun::star::uno::Any aNewValue = implValidateValue(m_aHelper.tree(), aValueNode, newValue);

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueReplaceImpl(aNewValue) );

    NodeRef aParent = m_aHelper.tree()->getParent(aValueNode);
    pChange->setTarget(
               view::ViewTreeAccess(m_aHelper.tree().get()).toGroupNode(aParent),
                aValueNode.m_sNodeName
            );

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// helper class NodeDefaulter
//-----------------------------------------------------------------------------

namespace
{
    struct NodeDefaulter : NodeVisitor
    {
        GroupDefaulter& updater;
        NodeChanges     result;

        explicit
        NodeDefaulter(GroupDefaulter& _rUpdater) : updater(_rUpdater), result() {}

        /// do the operation on <var>aNode</var>. needs to be implemented by concrete visitor classes
        Result handle(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

        /// do the operation on <var>aValue</var>. needs to be implemented by concrete visitor classes
        Result handle(rtl::Reference< Tree > const& aTree, ValueRef const& aValue);

        inline void addResult(NodeChange const& aChange)
        {
            if (aChange.maybeChange())
                this->result.add(aChange);
        }
    };

    NodeVisitor::Result NodeDefaulter::handle(rtl::Reference< Tree > const& , NodeRef const& aNode)
    {
        addResult( updater.validateSetToDefaultState(aNode) );
        return CONTINUE;
    }

    NodeVisitor::Result NodeDefaulter::handle(rtl::Reference< Tree > const& , ValueRef const& aValue)
    {
        addResult( updater.validateSetToDefaultValue(aValue) );
        return CONTINUE;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupDefaulter
//-----------------------------------------------------------------------------

GroupDefaulter::GroupDefaulter(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aProvider)
: m_aHelper(_aParentTree,_aGroupNode)
, m_aDefaultProvider(_aProvider)
, m_bHasDoneSet(false)
{
}
//-----------------------------------------------------------------------------
bool GroupDefaulter::isDataAvailable(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode)
{
    return _aParentTree->areValueDefaultsAvailable(_aGroupNode);
}
//-----------------------------------------------------------------------------
bool GroupDefaulter::ensureDataAvailable(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aDataSource)
{
    return  isDataAvailable(_aParentTree, _aGroupNode) ||
            _aDataSource.fetchDefaultData( _aParentTree );
}
//-----------------------------------------------------------------------------

bool GroupDefaulter::isDataAvailable()
{
    return m_aHelper.tree()->areValueDefaultsAvailable(m_aHelper.node());
}
//-----------------------------------------------------------------------------

NodeChange GroupDefaulter::validateSetToDefaultValue(ValueRef const& aValueNode)
{
    m_aHelper.validateNode(aValueNode);

    if (!m_aHelper.tree()->hasNodeDefault(aValueNode))
        throw Exception("INTERNAL ERROR: Group Member Update: Node has no default value" );

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueResetImpl() );

    NodeRef aParent = m_aHelper.tree()->getParent(aValueNode);
    pChange->setTarget(
               view::ViewTreeAccess(m_aHelper.tree().get()).toGroupNode(aParent),
                aValueNode.m_sNodeName
            );

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange GroupDefaulter::validateSetToDefaultState(NodeRef const& aNode)
{
    m_aHelper.validateNode(aNode);

    NodeChange aResult;

    // only works for set nodes - groups are left alone
    if ( view::ViewTreeAccess(m_aHelper.tree().get()).isSetNode(aNode) )
    {
       aResult = SetDefaulter( m_aHelper.tree(), aNode, m_aDefaultProvider ).validateSetToDefaultState();
    }

    m_bHasDoneSet = aResult.maybeChange();

    return aResult;
}
//-----------------------------------------------------------------------------

NodeChanges GroupDefaulter::validateSetAllToDefault()
{
    NodeDefaulter aDefaulter(*this);

    m_aHelper.tree()->dispatchToChildren(m_aHelper.node(),aDefaulter);

    return aDefaulter.result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

