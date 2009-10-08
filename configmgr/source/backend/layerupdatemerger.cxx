/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdatemerger.cxx,v $
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

#include "layerupdatemerger.hxx"
#include "layerupdatebuilder.hxx"
#include "updatedata.hxx"
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

LayerUpdateMerger::LayerUpdateMerger(uno::Reference< backenduno::XLayer > const & _xSourceLayer, LayerUpdate const & _aLayerUpdate)
: BasicUpdateMerger(_xSourceLayer)
, m_aLayerUpdate(_aLayerUpdate)
, m_xCurrentNode()
{
}
// -----------------------------------------------------------------------------

LayerUpdateMerger::~LayerUpdateMerger()
{
}
// -----------------------------------------------------------------------------

void LayerUpdateMerger::flushUpdate()
{
    OSL_ENSURE(!BasicUpdateMerger::isHandling(), "LayerUpdateMerger: Unexpected: flushing data, while base implementation is active");
    OSL_ENSURE(m_xCurrentNode.is(),"LayerUpdateMerger: No data for flushing.");

    if (m_xCurrentNode.is())
    {
        m_xCurrentNode->writeChildrenToLayer(getResultWriter().get());
        m_xCurrentNode.clear();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::startLayer(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    BasicUpdateMerger::startLayer();

    m_xCurrentNode = m_aLayerUpdate.getContextNode();

    BasicUpdateMerger::findContext(m_aLayerUpdate.getContextPath());
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endLayer(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    BasicUpdateMerger::endLayer();

    OSL_ENSURE(!m_xCurrentNode.is(), "Path being updated not found in data - update not written");
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::overrideNode(aName, aAttributes, bClear);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::overrideNode(aName, aAttributes, bClear);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }
    m_xCurrentNode->removeNodeByName(aName);

    if (NodeUpdate * pNodeUpdate = xUpdate->asNodeUpdate(true))
    {
        getResultWriter()->overrideNode(aName, pNodeUpdate->updateFlags(aAttributes), bClear);
        m_xCurrentNode.set(pNodeUpdate);
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
        BasicUpdateMerger::startSkipping();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addOrReplaceNode(aName, aAttributes);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::addOrReplaceNode(aName, aAttributes);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }
    m_xCurrentNode->removeNodeByName(aName);

    if (NodeUpdate * pNodeUpdate = xUpdate->asNodeUpdate(true))
    {
        getResultWriter()->addOrReplaceNode(aName, pNodeUpdate->updateFlags(aAttributes));
        m_xCurrentNode.set(pNodeUpdate);
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
        BasicUpdateMerger::startSkipping();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }
    m_xCurrentNode->removeNodeByName(aName);

    if (NodeUpdate * pNodeUpdate = xUpdate->asNodeUpdate(true))
    {
        getResultWriter()->addOrReplaceNodeFromTemplate(aName, aTemplate, pNodeUpdate->updateFlags(aAttributes));
        m_xCurrentNode.set(pNodeUpdate);
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
        BasicUpdateMerger::startSkipping();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endNode(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::endNode();
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    // write unhandled so far changes
    m_xCurrentNode->writeChildrenToLayer( getResultWriter().get() );

    rtl::Reference<NodeUpdate> xParent( m_xCurrentNode->getParent() );

    if (xParent.is())
        getResultWriter()->endNode();

    else
        BasicUpdateMerger::leaveContext();

    m_xCurrentNode = xParent;
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::dropNode( const rtl::OUString& aName )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::dropNode(aName);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (xUpdate.is())
    {
        m_xCurrentNode->removeNodeByName(aName);

        if (NodeUpdate * pNodeUpdate = xUpdate->asNodeUpdate())
        {
            if (pNodeUpdate-> getOperation() == NodeUpdate::replace)
            {
                xUpdate->writeToLayer( getResultWriter().get() );
                return;
            }
            else
            {
                malformedUpdate("LayerUpdateMerger: Applying modification to dropped node");
            }
        }
    }

    getResultWriter()->dropNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::overrideProperty(aName, aAttributes, aType, bClear);
        return;
    }

    OSL_ASSERT( m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getPropertyByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::overrideProperty(aName, aAttributes, aType, bClear);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }

    m_xCurrentNode->removePropertyByName(aName);

    if (PropertyUpdate * pPropUpdate = xUpdate->asPropertyUpdate())
    {
        OSL_ENSURE( aType == pPropUpdate->getValueType() ||
                    aType == uno::Type()                 ||
                    pPropUpdate->getValueType() == uno::Type(),
                    "Error in update merger: type mismatch overriding property ...");

        getResultWriter()->overrideProperty(aName, pPropUpdate->updateFlags(aAttributes), aType, bClear);
        m_xCurrentProp.set(pPropUpdate);
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
        BasicUpdateMerger::startSkipping();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endProperty(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::endProperty();
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    if (!m_xCurrentProp.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("LayerUpdateMerger: Invalid data: Ending property that wasn't started.") );
        throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
    }

    // write unhandled so far values
    m_xCurrentProp->writeValuesToLayer( getResultWriter().get() );

    getResultWriter()->endProperty();

    m_xCurrentProp.clear();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::setPropertyValue( const uno::Any& aValue )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::setPropertyValue(aValue);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    if (!m_xCurrentProp.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("LayerUpdateMerger: Invalid data: setting value, but no property is started.") );
        throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
    }

    if (!m_xCurrentProp->hasChange())
    {
        BasicUpdateMerger::setPropertyValue(aValue);
        return;
    }

#ifndef CFG_UPDATEMERGER_BATCHWRITE_PROPERTIES
    if (m_xCurrentProp->hasValue())
    {
        getResultWriter()->setPropertyValue(m_xCurrentProp->getValue());
    }
    else
    {
        OSL_ENSURE(m_xCurrentProp->hasReset(),"LayerUpdateMerger: ERROR: Unknown change type in PropertyUpdate");
        // write nothing to result
    }

    // mark handled
    m_xCurrentProp->removeValue();
#endif
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString & aLocale )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::setPropertyValueForLocale(aValue, aLocale);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    if (!m_xCurrentProp.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("LayerUpdateMerger: Invalid data: setting value, but no property is started.") );
        throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
    }

    if (!m_xCurrentProp->hasChangeFor(aLocale))
    {
        BasicUpdateMerger::setPropertyValueForLocale(aValue, aLocale);
        return;
    }

#ifndef CFG_UPDATEMERGER_BATCHWRITE_PROPERTIES
    if (m_xCurrentProp->hasValueFor(aLocale))
    {
        getResultWriter()->setPropertyValueForLocale(m_xCurrentProp->getValueFor(aLocale),aLocale);
    }
    else
    {
        OSL_ENSURE(m_xCurrentProp->hasResetFor(aLocale),"LayerUpdateMerger: ERROR: Unknown change type in PropertyUpdate");
        // write nothing to result
    }

    // mark handled
    m_xCurrentProp->removeValueFor(aLocale);
#endif
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addProperty(aName, aAttributes, aType);
        return;
    }
    OSL_ASSERT( m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getPropertyByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::addProperty(aName, aAttributes, aType);
        return;
    }

    m_xCurrentNode->removePropertyByName(aName);

    if (PropertyUpdate * pPropUpdate = xUpdate->asPropertyUpdate())
    {
        if (pPropUpdate->hasValue() && pPropUpdate->getValue().hasValue())
        {
            // TODO: validate value-type
            uno::Any aNewValue = pPropUpdate->getValue();
            OSL_ASSERT( aNewValue.hasValue() );

            if (aNewValue.getValueType() != aType)
                malformedUpdate("LayerUpdateMerger: cannot do type conversion while writing updates");

            getResultWriter()->addPropertyWithValue(aName, pPropUpdate->updateFlags(aAttributes), aNewValue);
        }
        else
        {
            // TODO: validate type
            if (pPropUpdate->getValueType() != aType && pPropUpdate->getValueType() != uno::Type())
                malformedUpdate("LayerUpdateMerger: types for property update do not match");

            OSL_ENSURE(!pPropUpdate->hasReset(),"Warning: resetting the value of an added property is undefined - reverting to NULL");
            getResultWriter()->addProperty(aName, pPropUpdate->updateFlags(aAttributes), aType);
        }
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addPropertyWithValue(aName, aAttributes, aValue);
        return;
    }
    OSL_ASSERT( m_xCurrentNode.is());

    rtl::Reference<ElementUpdate> xUpdate = m_xCurrentNode->getPropertyByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::addPropertyWithValue(aName, aAttributes, aValue);
        return;
    }

    m_xCurrentNode->removePropertyByName(aName);

    if (PropertyUpdate * pPropUpdate = xUpdate->asPropertyUpdate())
    {
        if (!pPropUpdate->hasChange()) // attribute change only
        {
            getResultWriter()->addPropertyWithValue(aName, pPropUpdate->updateFlags(aAttributes), aValue);
        }
        else if (pPropUpdate->hasReset())
        {
            // write nothing
        }
        else if (pPropUpdate->getValue().hasValue()) // setting to non-NULL value
        {
            OSL_ASSERT(pPropUpdate->hasValue());

            // TODO: validate value-type
            uno::Any aNewValue = pPropUpdate->getValue();

            if (aNewValue.getValueType() != aValue.getValueType())
                malformedUpdate("LayerUpdateMerger: cannot do type conversion while writing updates");

            getResultWriter()->addPropertyWithValue(aName, pPropUpdate->updateFlags(aAttributes), aNewValue);
        }
        else // setting to null value
        {
            OSL_ASSERT(pPropUpdate->hasValue());

            // TODO: validate type
            if (pPropUpdate->getValueType() != aValue.getValueType() && pPropUpdate->getValueType() != uno::Type())
                malformedUpdate("LayerUpdateMerger: types for property update do not match");

            getResultWriter()->addProperty(aName, pPropUpdate->updateFlags(aAttributes), aValue.getValueType());
        }
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
    }
}
// -----------------------------------------------------------------------------

void LayerUpdateMerger::malformedUpdate(sal_Char const * pMsg)
{
    { (void)pMsg; }
    OSL_ENSURE(false,pMsg);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

