/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatedata.cxx,v $
 * $Revision: 1.14 $
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

#include "updatedata.hxx"

#include <com/sun/star/configuration/backend/NodeAttribute.hpp>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>

#include <iterator>
#include <algorithm>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

ElementUpdate::ElementUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask)
: NamedUpdate(_aName)
, m_pParent(_pParent)
, m_nFlags(_nFlags)
, m_nFlagsMask(_nFlagsMask)
{
}
// -----------------------------------------------------------------------------

sal_Int16 ElementUpdate::updateFlags(sal_Int16 _nFlags) const
{
    return (_nFlags & ~m_nFlagsMask) | (m_nFlags & m_nFlagsMask);
}
// -----------------------------------------------------------------------------

NodeUpdate * ElementUpdate::asNodeUpdate(bool )
{
    return NULL;
}
// -----------------------------------------------------------------------------

PropertyUpdate  * ElementUpdate::asPropertyUpdate()
{
    return NULL;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

NodeUpdate::NodeUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, Op _op)
: ElementUpdate(_pParent,_aName,_nFlags, _nFlagsMask)
, m_aNodes()
, m_aProperties()
, m_aRemovedElements()
, m_op(_op)
{
}
// -----------------------------------------------------------------------------

NodeUpdate * NodeUpdate::asNodeUpdate(bool _bMerged)
{
    return (!_bMerged || m_op == modify
            || (updateFlags() & backenduno::NodeAttribute::FUSE) != 0)
        ? this : NULL;
}
// -----------------------------------------------------------------------------

bool NodeUpdate::addNodeUpdate(rtl::Reference<ElementUpdate> const & _aNode)
{
    OSL_PRECOND( _aNode.is(), "ERROR: NodeUpdate: Trying to add NULL node.");
    OSL_PRECOND( _aNode->getParent() == this, "ERROR: NodeUpdate: Node being added has wrong parent.");
    OSL_ENSURE(m_aNodes.find(_aNode->getName()) == m_aNodes.end(),
                "NodeUpdate: Child node being added already exists in this node.");

    return m_aNodes.insert( ElementList::value_type(_aNode->getName(),_aNode) ).second;
}
// -----------------------------------------------------------------------------

bool NodeUpdate::addPropertyUpdate(rtl::Reference<ElementUpdate> const & _aProp)
{
    OSL_PRECOND( _aProp.is(), "ERROR: NodeUpdate: Trying to add NULL property.");
    OSL_PRECOND( _aProp->getParent() == this, "ERROR: NodeUpdate: Property being added has wrong parent.");
    OSL_ENSURE(m_aProperties.find(_aProp->getName()) == m_aProperties.end(),
                "NodeUpdate: Property being added already exists in this node.");

    return m_aProperties.insert( ElementList::value_type(_aProp->getName(),_aProp) ).second;
}
// -----------------------------------------------------------------------------

void NodeUpdate::removeNodeByName(rtl::OUString const & _aName)
{
    ElementList::iterator it = m_aNodes.find(_aName);
    OSL_ENSURE(it != m_aNodes.end(),
                "NodeUpdate: Child node being removed is not in this node.");

    if (it != m_aNodes.end())
    {
        m_aRemovedElements.insert(*it);
        m_aNodes.erase(it);
    }
}
// -----------------------------------------------------------------------------

void NodeUpdate::removePropertyByName  (rtl::OUString const & _aName)
{
    ElementList::iterator it = m_aProperties.find(_aName);
    OSL_ENSURE(it != m_aProperties.end(),
                "NodeUpdate: Property being removed is not in this node.");

    if (it != m_aProperties.end())
    {
        m_aRemovedElements.insert(*it);
        m_aProperties.erase(it);
    }
}
// -----------------------------------------------------------------------------

rtl::Reference<ElementUpdate> NodeUpdate::getNodeByName(rtl::OUString const & _aName) const
{
    ElementList::const_iterator it = m_aNodes.find(_aName);

    return it != m_aNodes.end() ? it->second : rtl::Reference<ElementUpdate>();
}
// -----------------------------------------------------------------------------

rtl::Reference<ElementUpdate> NodeUpdate::getPropertyByName  (rtl::OUString const & _aName) const
{
    ElementList::const_iterator it = m_aProperties.find(_aName);

    return it != m_aProperties.end() ? it->second : rtl::Reference<ElementUpdate>();
}
// -----------------------------------------------------------------------------

void NodeUpdate::writeChildrenToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);
    for (ElementList::const_iterator itP = beginProperties(); itP != endProperties(); ++itP)
        itP->second->writeToLayer(_pLayer);

    for (ElementList::const_iterator itN = beginNodes(); itN != endNodes(); ++itN)
        itN->second->writeToLayer(_pLayer);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

NodeModification::NodeModification(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset)
: NodeUpdate(_pParent,_aName,_nFlags, _nFlagsMask, _bReset ? reset : modify)
{
}
// -----------------------------------------------------------------------------

void NodeModification::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);

    if ( this->getOperation() == reset &&   // if we have an empty
        !this->changedFlags() &&            // 'reset' node, that means
        !this->hasChildren() )              // we need to write
        return;                             // nothing

    _pLayer->overrideNode( this->getName(), this->updateFlags(), false );
    this->writeChildrenToLayer(_pLayer);
    _pLayer->endNode();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

NodeReplace::NodeReplace(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags)
: NodeUpdate(_pParent,_aName,_nFlags, _nFlags, replace)
, m_aTemplateName()
, m_aTemplateComponent()
{
}
// -----------------------------------------------------------------------------

NodeReplace::NodeReplace(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateComponent)
: NodeUpdate(_pParent,_aName,_nFlags, _nFlags, replace)
, m_aTemplateName(_aTemplateName)
, m_aTemplateComponent(_aTemplateComponent)
{
}
// -----------------------------------------------------------------------------

bool NodeReplace::hasTemplate() const
{
    return m_aTemplateName.getLength() != 0;
}
// -----------------------------------------------------------------------------

void NodeReplace::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);

    if (this->hasTemplate())
    {
        backenduno::TemplateIdentifier aTemplate( m_aTemplateName, m_aTemplateComponent );
        _pLayer->addOrReplaceNodeFromTemplate( this->getName(), aTemplate, this->updateFlags() );
    }
    else
        _pLayer->addOrReplaceNode( this->getName(), this->updateFlags() );

    this->writeChildrenToLayer(_pLayer);
    _pLayer->endNode();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

NodeDrop::NodeDrop(NodeUpdate * _pParent, rtl::OUString const & _aName)
: ElementUpdate(_pParent,_aName,0,0)
{
}
// -----------------------------------------------------------------------------

void NodeDrop::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);
    _pLayer->dropNode(this->getName());
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

PropertyUpdate::PropertyUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType)
: ElementUpdate(_pParent,_aName,_nFlags,_nFlagsMask)
, m_aValues()
, m_aType(_aType)
{}
// -----------------------------------------------------------------------------

PropertyUpdate  * PropertyUpdate::asPropertyUpdate()
{
    return this;
}
// -----------------------------------------------------------------------------

static uno::Any makeResetMarker()
{
    uno::Reference< backenduno::XLayerHandler > xNull;
    return uno::makeAny(xNull);
}
// -----------------------------------------------------------------------------

inline bool PropertyUpdate::isResetMarker(uno::Any const & _aValue)
{
    OSL_ENSURE( _aValue.getValueTypeClass() != uno::TypeClass_INTERFACE ||
                _aValue == makeResetMarker() && _aValue.getValueType() == makeResetMarker().getValueType(),
                "Unexpected any: Interface reference will be taken as reset marker");

    return _aValue.getValueTypeClass() == uno::TypeClass_INTERFACE;
}
// -----------------------------------------------------------------------------

uno::Any const & PropertyUpdate::getResetMarker()
{
    static uno::Any const aMarker = makeResetMarker();

    OSL_ASSERT( isResetMarker(aMarker) );

    return aMarker;
}
// -----------------------------------------------------------------------------

bool PropertyUpdate::setValueFor(rtl::OUString const & _aLocale, uno::Any const & _aValueUpdate)
{
    OSL_PRECOND( !isResetMarker(_aValueUpdate), "PropertyUpdate: ERROR: Trying to set a reset marker as regular value" );

    OSL_ENSURE(m_aValues.find(_aLocale) == m_aValues.end(),
                "PropertyUpdate: Locale being added already has a value in this property.");

    if (_aValueUpdate.hasValue())
    {
        if (m_aType.getTypeClass() == uno::TypeClass_ANY)
            m_aType = _aValueUpdate.getValueType();

        else
            OSL_ENSURE( m_aType == _aValueUpdate.getValueType() ||
                        m_aType == uno::Type(),
                        "ValueType mismatch in PropertyUpdate");
    }
    return m_aValues.insert( ValueList::value_type(_aLocale,_aValueUpdate) ).second;
}
// -----------------------------------------------------------------------------

bool PropertyUpdate::resetValueFor(rtl::OUString const & _aLocale)
{
    OSL_ENSURE(m_aValues.find(_aLocale) == m_aValues.end(),
                "PropertyUpdate: Locale being reset already has a value in this property.");

    return m_aValues.insert( ValueList::value_type(_aLocale,getResetMarker()) ).second;
}
// -----------------------------------------------------------------------------

void PropertyUpdate::removeValueFor(rtl::OUString const & _aLocale)
{
    OSL_ENSURE(m_aValues.find(_aLocale) != m_aValues.end(),
                "PropertyUpdate: Locale being removed is not in this node.");

    m_aValues.erase(_aLocale);
}
// -----------------------------------------------------------------------------

void PropertyUpdate::finishValue()
{
    if (m_aType.getTypeClass() == uno::TypeClass_ANY)
        m_aType = uno::Type();
}
// -----------------------------------------------------------------------------

bool PropertyUpdate::hasValueFor(rtl::OUString const & _aLocale) const
{
    ValueList::const_iterator it = m_aValues.find(_aLocale);

    return it != m_aValues.end() && ! isResetMarker(it->second);
}
// -----------------------------------------------------------------------------

bool PropertyUpdate::hasResetFor(rtl::OUString const & _aLocale) const
{
    ValueList::const_iterator it = m_aValues.find(_aLocale);

    return it != m_aValues.end() && isResetMarker(it->second);
}
// -----------------------------------------------------------------------------

bool PropertyUpdate::hasChangeFor(rtl::OUString const & _aLocale) const
{
    ValueList::const_iterator it = m_aValues.find(_aLocale);

    return it != m_aValues.end();
}
// -----------------------------------------------------------------------------

uno::Any PropertyUpdate::getValueFor(rtl::OUString const & _aLocale) const
{
    ValueList::const_iterator it = m_aValues.find(_aLocale);

    OSL_ENSURE(it != m_aValues.end() && !isResetMarker(it->second),
                "PropertyUpdate: Should not call getValue() unless hasValue() returns true" );

    return it != m_aValues.end() && !isResetMarker(it->second) ? it->second : uno::Any();
}
// -----------------------------------------------------------------------------

void PropertyUpdate::writeValueToLayer(backenduno::XLayerHandler * _pLayer, uno::Any const & _aValue)
{
    OSL_ASSERT(_pLayer);
    if ( !isResetMarker(_aValue) )
        _pLayer->setPropertyValue(_aValue);

    // else - to reset - do nothing
}
// -----------------------------------------------------------------------------

void PropertyUpdate::writeValueToLayerFor(backenduno::XLayerHandler * _pLayer, uno::Any const & _aValue, rtl::OUString const & _aLocale)
{
    OSL_ASSERT(_pLayer);
    if (_aLocale == this->primarySlot())
        this->writeValueToLayer(_pLayer,_aValue);

    else if ( !isResetMarker(_aValue) )
        _pLayer->setPropertyValueForLocale(_aValue,_aLocale);

    // else - to reset - do nothing
}
// -----------------------------------------------------------------------------

void PropertyUpdate::writeValuesToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);
    for (ValueList::const_iterator itV = beginValues(); itV != endValues(); ++itV)
        this->writeValueToLayerFor(_pLayer, itV->second, itV->first);
}
// -----------------------------------------------------------------------------

void PropertyUpdate::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);

    _pLayer->overrideProperty( this->getName(), this->updateFlags(), this->m_aType, false );
    this->writeValuesToLayer(_pLayer);
    _pLayer->endProperty();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


PropertyAdd::PropertyAdd(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType)
: ElementUpdate(_pParent,_aName,_nFlags,_nFlags)
, m_aValueType(_aType)
, m_aValue()
{

}
// -----------------------------------------------------------------------------

PropertyAdd::PropertyAdd(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue)
: ElementUpdate(_pParent,_aName,_nFlags,_nFlags)
, m_aValueType(_aValue.getValueType())
, m_aValue(_aValue)
{
}
// -----------------------------------------------------------------------------

void PropertyAdd::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    OSL_ASSERT(_pLayer);
    if (this->hasValue())
        _pLayer->addPropertyWithValue(this->getName(),this->updateFlags(),this->getValue());
    else
        _pLayer->addProperty(this->getName(),this->updateFlags(),this->getValueType());
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

PropertyReset::PropertyReset(NodeUpdate * _pParent, rtl::OUString const & _aName)
: ElementUpdate(_pParent,_aName,0,0)
{
}
// -----------------------------------------------------------------------------

void PropertyReset::writeToLayer(backenduno::XLayerHandler * _pLayer)
{
    (void) _pLayer; // avoid warning about unused parameter
    OSL_ASSERT(_pLayer);
    // skip - nothing to write
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
