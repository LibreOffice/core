/*************************************************************************
 *
 *  $RCSfile: layerupdatemerger.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-27 10:35:00 $
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

#include "layerupdatemerger.hxx"

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEBUILDER_HXX
#include "layerupdatebuilder.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_UPDATEDATA_HXX
#include "updatedata.hxx"
#endif
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

LayerUpdateMerger::LayerUpdateMerger( ResultHandler const & _xResultHandler)
: BasicUpdateMerger(_xResultHandler)
, m_aLayerUpdate()
, m_xCurrentNode()
{
}
// -----------------------------------------------------------------------------

LayerUpdateMerger::~LayerUpdateMerger()
{
}
// -----------------------------------------------------------------------------

LayerUpdateBuilder LayerUpdateMerger::getLayerUpdateBuilder()
{
    return LayerUpdateBuilder(m_aLayerUpdate);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::startLayer(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    BasicUpdateMerger::startLayer();

    m_xCurrentNode = m_aLayerUpdate.getContextNode();

    BasicUpdateMerger::findContext(m_aLayerUpdate.getContextPath());
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endLayer(  )
        throw (MalformedDataException, lang::IllegalAccessException, uno::RuntimeException)
{
    if (m_xCurrentNode.is())
    {
        OSL_ENSURE(false, "Path being updated not found in data");
        m_xCurrentNode.clear();
    }

    BasicUpdateMerger::endLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::overrideNode( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::overrideNode(aName, aAttributes);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    ElementUpdateRef xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::overrideNode(aName, aAttributes);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }

    if (NodeUpdate * pNodeUpdate = xUpdate->asNodeUpdate(true))
    {
        getResultWriter()->overrideNode(aName, pNodeUpdate->updateFlags(aAttributes));
        m_xCurrentNode.set(pNodeUpdate);
    }
    else
    {
        xUpdate->writeToLayer(getResultWriter().get());
        BasicUpdateMerger::startSkipping();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addOrReplaceNode(aName, aAttributes);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    ElementUpdateRef xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (!xUpdate.is())
    {
        BasicUpdateMerger::addOrReplaceNode(aName, aAttributes);
        OSL_ASSERT(BasicUpdateMerger::isHandling());
        return;
    }

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

void SAL_CALL LayerUpdateMerger::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endNode(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::endNode();
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    // write unhandled so far changes
    m_xCurrentNode->writeChildrenToLayer( getResultWriter().get() );

    NodeUpdateRef xParent( m_xCurrentNode->getParent() );

    if (xParent.is())
    {
        getResultWriter()->endNode();
        xParent->removeNodeByName(m_xCurrentNode->getName());
    }
    else
        BasicUpdateMerger::leaveContext();

    m_xCurrentNode = xParent;
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::dropNode( const OUString& aName )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::dropNode(aName);
        return;
    }

    OSL_ASSERT(m_xCurrentNode.is());

    ElementUpdateRef xUpdate = m_xCurrentNode->getNodeByName(aName);
    if (xUpdate.is())
    {
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

void SAL_CALL LayerUpdateMerger::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::UnknownPropertyException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::overrideProperty(aName, aAttributes, aType);
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::endProperty(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::endProperty();
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::setPropertyValue( const uno::Any& aValue )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::setPropertyValue(aValue);
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::setPropertyValueForLocale(aValue, aLocale);
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addProperty(aName, aAttributes, aType);
        return;
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerUpdateMerger::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (BasicUpdateMerger::isHandling())
    {
        BasicUpdateMerger::addPropertyWithValue(aName, aAttributes, aValue);
        return;
    }
}
// -----------------------------------------------------------------------------

void LayerUpdateMerger::malformedUpdate(sal_Char const * pMsg)
{
    OSL_ENSURE(false,pMsg);
}
// -----------------------------------------------------------------------------

void LayerUpdateMerger::illegalUpdate(sal_Char const * pMsg)
{
    OSL_ENSURE(false,pMsg);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

