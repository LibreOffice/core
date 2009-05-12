/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdatebuilder.cxx,v $
 * $Revision: 1.12 $
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

#include "layerupdatebuilder.hxx"
#include "updatedata.hxx"
#include "layerupdate.hxx"
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

LayerUpdateBuilder::LayerUpdateBuilder()
: m_aUpdate()
, m_pCurrentNode(NULL)
, m_pCurrentProp(NULL)
{
}
// -----------------------------------------------------------------------------

void LayerUpdateBuilder::clear()
{
    m_pCurrentProp = NULL;
    m_pCurrentNode = NULL;
    m_aUpdate = LayerUpdate();

    OSL_POSTCOND( this->isEmpty(), "LayerUpdateBuilder: Could not reset the stored update.");
}
// -----------------------------------------------------------------------------

LayerUpdate const & LayerUpdateBuilder::result() const
{
    OSL_ENSURE(this->isComplete(),
                "LayerUpdateBuilder: There is no result to retrieve"
                " - building the data is still in progress.");

    return m_aUpdate;
}
// -----------------------------------------------------------------------------

inline
LayerUpdate & LayerUpdateBuilder::data()
{
    return m_aUpdate;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::init()
{
    if (m_pCurrentNode) return false;

    LayerUpdate & update = data();
    m_pCurrentNode = new NodeModification(NULL, rtl::OUString(), 0, 0, false);
    update.setContextNode(m_pCurrentNode);

    OSL_ENSURE(m_pCurrentProp == NULL, "LayerUpdateBuilder: Internal error: got a current property for a new context");

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::modifyNode(rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeUpdate * pNewNode = new NodeModification(m_pCurrentNode,_aName,_nFlags,_nFlagsMask,_bReset);
    rtl::Reference<ElementUpdate> xNewNode(pNewNode);

    if (!m_pCurrentNode->addNodeUpdate(xNewNode))
        return false;

    m_pCurrentNode = pNewNode;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::replaceNode(rtl::OUString const & _aName, sal_Int16 _nFlags, backenduno::TemplateIdentifier const * _pTemplate)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeUpdate * pNewNode = _pTemplate ?
        new NodeReplace(m_pCurrentNode,_aName,_nFlags,_pTemplate->Name,_pTemplate->Component) :
        new NodeReplace(m_pCurrentNode,_aName,_nFlags);

    rtl::Reference<ElementUpdate> xNewNode(pNewNode);

    if (!m_pCurrentNode->addNodeUpdate(xNewNode))
        return false;

    m_pCurrentNode = pNewNode;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::finishNode()
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    if (m_pCurrentProp || !m_pCurrentNode) return false;

    m_pCurrentNode = m_pCurrentNode->getParent();
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::removeNode(rtl::OUString const & _aName)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeDrop * pNewNode = new NodeDrop(m_pCurrentNode,_aName);

    rtl::Reference<ElementUpdate> xNewNode(pNewNode);

    if (!m_pCurrentNode->addNodeUpdate(xNewNode))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::modifyProperty(rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyUpdate * pNewProp = new PropertyUpdate(m_pCurrentNode,_aName,_nFlags,_nFlagsMask,_aType);
    rtl::Reference<ElementUpdate> xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    m_pCurrentProp = pNewProp;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::setPropertyValue(uno::Any const & _aValue)
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");

    return m_pCurrentProp->setValue(_aValue);
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::setPropertyValueForLocale(uno::Any const & _aValue, rtl::OUString const & _aLocale)
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");

    return m_pCurrentProp->setValueFor(_aLocale, _aValue);
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::resetPropertyValue()
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");

    return m_pCurrentProp->resetValue();
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::resetPropertyValueForLocale(rtl::OUString const & _aLocale)
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");

    return m_pCurrentProp->resetValueFor(_aLocale);
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::finishProperty()
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");
    if (!m_pCurrentProp) return false;
    m_pCurrentProp->finishValue();

    OSL_ASSERT(m_pCurrentNode == m_pCurrentProp->getParent());

    m_pCurrentProp = NULL;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::addNullProperty(rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyAdd * pNewProp = new PropertyAdd(m_pCurrentNode,_aName,_nFlags,_aType);

    rtl::Reference<ElementUpdate> xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::addProperty(rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyAdd * pNewProp = new PropertyAdd(m_pCurrentNode,_aName,_nFlags,_aValue);

    rtl::Reference<ElementUpdate> xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::resetProperty(rtl::OUString const & _aName)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyReset * pNewProp = new PropertyReset(m_pCurrentNode,_aName);

    rtl::Reference<ElementUpdate> xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::finish()
{
    if (!this->finishNode()) return false;

    return m_pCurrentNode == NULL;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::isEmpty() const
{
    OSL_ENSURE( !m_pCurrentNode || !m_aUpdate.isEmpty(), "LayerUpdateBuilder: Invariant violation: got a current node without a layer");
    return m_aUpdate.isEmpty();
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::isActive() const
{
    OSL_ENSURE( !m_pCurrentNode || !m_aUpdate.isEmpty(), "LayerUpdateBuilder: Invariant violation: got a current node without a layer");
    return m_pCurrentNode != 0;
}
// -----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
bool LayerUpdateBuilder::isComplete() const
{
    OSL_ENSURE( !m_pCurrentNode || !m_aUpdate.isEmpty(), "LayerUpdateBuilder: Invariant violation: got a current node without a layer");
    return !m_aUpdate.isEmpty() && m_pCurrentNode == NULL;
}
#endif
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::isPropertyActive() const
{
    OSL_ENSURE( !m_pCurrentNode || !m_aUpdate.isEmpty(), "LayerUpdateBuilder: Invariant violation: got a current node without a layer");
    OSL_ENSURE(m_pCurrentNode || !m_pCurrentProp, "LayerUpdateBuilder: Invariant violation: got a current property without a node");
    return m_pCurrentProp != 0;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
