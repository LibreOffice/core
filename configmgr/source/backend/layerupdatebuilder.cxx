/*************************************************************************
 *
 *  $RCSfile: layerupdatebuilder.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:16:39 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "layerupdatebuilder.hxx"

#ifndef CONFIGMGR_BACKEND_UPDATEDATA_HXX
#include "updatedata.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_LAYERUPDATE_HXX
#include "layerupdate.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_TEMPLATEIDENTIFIER_HPP_
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>
#endif

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
    m_pCurrentNode = new NodeModification(NULL, OUString(), 0, 0, false);
    update.setContextNode(m_pCurrentNode);

    OSL_ENSURE(m_pCurrentProp == NULL, "LayerUpdateBuilder: Internal error: got a current property for a new context");

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::modifyNode(OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeUpdate * pNewNode = new NodeModification(m_pCurrentNode,_aName,_nFlags,_nFlagsMask,_bReset);
    ElementUpdateRef xNewNode(pNewNode);

    if (!m_pCurrentNode->addNodeUpdate(xNewNode))
        return false;

    m_pCurrentNode = pNewNode;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::replaceNode(OUString const & _aName, sal_Int16 _nFlags, backenduno::TemplateIdentifier const * _pTemplate)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeUpdate * pNewNode = _pTemplate ?
        new NodeReplace(m_pCurrentNode,_aName,_nFlags,_pTemplate->Name,_pTemplate->Component) :
        new NodeReplace(m_pCurrentNode,_aName,_nFlags);

    ElementUpdateRef xNewNode(pNewNode);

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

bool LayerUpdateBuilder::removeNode(OUString const & _aName)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    NodeDrop * pNewNode = new NodeDrop(m_pCurrentNode,_aName);

    ElementUpdateRef xNewNode(pNewNode);

    if (!m_pCurrentNode->addNodeUpdate(xNewNode))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::modifyProperty(OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyUpdate * pNewProp = new PropertyUpdate(m_pCurrentNode,_aName,_nFlags,_nFlagsMask,_aType);
    ElementUpdateRef xNewProp(pNewProp);

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

bool LayerUpdateBuilder::setPropertyValueForLocale(uno::Any const & _aValue, OUString const & _aLocale)
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

bool LayerUpdateBuilder::resetPropertyValueForLocale(OUString const & _aLocale)
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");

    return m_pCurrentProp->resetValueFor(_aLocale);
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::finishProperty()
{
    OSL_PRECOND(m_pCurrentProp, "LayerUpdateBuilder: Illegal state for property operation");
    if (!m_pCurrentProp) return false;

    OSL_ASSERT(m_pCurrentNode == m_pCurrentProp->getParent());

    m_pCurrentProp = NULL;
    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::addNullProperty(OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyAdd * pNewProp = new PropertyAdd(m_pCurrentNode,_aName,_nFlags,_aType);

    ElementUpdateRef xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::addProperty(OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyAdd * pNewProp = new PropertyAdd(m_pCurrentNode,_aName,_nFlags,_aValue);

    ElementUpdateRef xNewProp(pNewProp);

    if (!m_pCurrentNode->addPropertyUpdate(xNewProp))
        return false;

    return true;
}
// -----------------------------------------------------------------------------

bool LayerUpdateBuilder::resetProperty(OUString const & _aName)
{
    OSL_PRECOND(m_pCurrentNode && !m_pCurrentProp, "LayerUpdateBuilder: Illegal state for this operation");

    PropertyReset * pNewProp = new PropertyReset(m_pCurrentNode,_aName);

    ElementUpdateRef xNewProp(pNewProp);

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

bool LayerUpdateBuilder::isComplete() const
{
    OSL_ENSURE( !m_pCurrentNode || !m_aUpdate.isEmpty(), "LayerUpdateBuilder: Invariant violation: got a current node without a layer");
    return !m_aUpdate.isEmpty() && m_pCurrentNode == NULL;
}
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
