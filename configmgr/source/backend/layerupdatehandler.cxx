/*************************************************************************
 *
 *  $RCSfile: layerupdatehandler.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:48 $
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

#include "layerupdatehandler.hxx"

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEMERGER_HXX
#include "layerupdatemerger.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#include <drafts/com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <drafts/com/sun/star/configuration/backend/XLayer.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace container = ::com::sun::star::container;
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateUpdateMerger
( CreationContext const& xContext )
{
    return * new LayerUpdateHandler( xContext );
}

// -----------------------------------------------------------------------------

LayerUpdateHandler::LayerUpdateHandler(CreationArg _xContext)
: UpdateService(_xContext)
, m_aBuilder()
{
}
// -----------------------------------------------------------------------------

LayerUpdateHandler::~LayerUpdateHandler()
{
}
// -----------------------------------------------------------------------------
inline
void LayerUpdateHandler::checkBuilder(bool _bForProperty)
{
    if ( m_aBuilder.isEmpty() )
        raiseMalformedDataException("LayerUpdateHandler: Illegal operation - no update is in progress");

    if ( !m_aBuilder.isActive() )
        raiseMalformedDataException("LayerUpdateHandler: Illegal operation - no context for update available");

    if ( m_aBuilder.isPropertyActive() != _bForProperty )
        raiseMalformedDataException("LayerUpdateHandler: Illegal operation - a property is in progress");
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raiseMalformedDataException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this);
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raiseNodeChangedBeforeException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this);
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raisePropChangedBeforeException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this);
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raisePropExistsException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);
    throw beans::PropertyExistException(sMsg,*this);
}
// -----------------------------------------------------------------------------

// XUpdateHandler
void SAL_CALL
    LayerUpdateHandler::startUpdate( const OUString& aContext )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    this->checkSourceLayer();
    if (!m_aBuilder.setContext(aContext))
        raiseMalformedDataException("LayerUpdateHandler: Cannot start update - update is already in progress");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endUpdate(  )
        throw (MalformedDataException, lang::IllegalAccessException, uno::RuntimeException)

{
    checkBuilder();

    if (!m_aBuilder.finish())
        raiseMalformedDataException("LayerUpdateHandler: Cannot finish update - a node is still open.");

    uno::Reference< backenduno::XLayer > xMergedLayer( LayerUpdateMerger::getMergedLayer(this->getSourceLayer(), m_aBuilder.result()) );

    m_aBuilder.clear();

    this->writeUpdatedLayer(xMergedLayer);
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::modifyNode( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.modifyNode(aName,aAttributes,aAttributeMask,bReset))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start node modification - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.replaceNode(aName,aAttributes,NULL))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start added/replaced node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.replaceNode(aName,aAttributes,&aTemplate))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start added/replaced node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endNode(  )
        throw (MalformedDataException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.finishNode())
    {
        OSL_ENSURE(m_aBuilder.isPropertyActive() || !m_aBuilder.isActive(), "LayerUpdateHandler: Unexpected failure mode for finishNode");
        if (m_aBuilder.isPropertyActive())
            raiseMalformedDataException("LayerUpdateHandler: Cannot finish node update - open property has not been ended.");
        else
            raiseMalformedDataException("LayerUpdateHandler: Cannot finish node update - no node has been started.");
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::removeNode( const OUString& aName )
        throw (MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.removeNode(aName))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot remove node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: modifyProperty( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask )
        throw (MalformedDataException, beans::UnknownPropertyException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder(false);

    if (!m_aBuilder.modifyProperty(aName,aAttributes,aAttributeMask,uno::Type()))
        raisePropChangedBeforeException("LayerUpdateHandler: Cannot start property modification - property has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: setPropertyValue( const uno::Any& aValue )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.setPropertyValue(aValue) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.setPropertyValueForLocale(aValue,aLocale) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetPropertyValue( )
        throw (MalformedDataException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.resetPropertyValue() );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetPropertyValueForLocale( const OUString& aLocale )
        throw (MalformedDataException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.resetPropertyValueForLocale(aLocale) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endProperty(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY ( m_aBuilder.finishProperty() );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetProperty( const OUString& aName )
        throw (MalformedDataException, beans::UnknownPropertyException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!m_aBuilder.resetProperty(aName))
        raisePropChangedBeforeException("LayerUpdateHandler: Cannot reset property - property has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!m_aBuilder.addNullProperty(aName,aAttributes,aType))
        raisePropExistsException("LayerUpdateHandler: Cannot add property - property exists (and has already been changed).");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplacePropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!m_aBuilder.addProperty(aName,aAttributes,aValue))
        raisePropExistsException("LayerUpdateHandler: Cannot add property - property exists (and has already been changed).");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::removeProperty( const OUString& aName )
        throw (MalformedDataException, beans::UnknownPropertyException, beans::PropertyExistException, lang::IllegalArgumentException, uno::RuntimeException)
{
    // treat 'remove' as 'reset'. (Note: does not verify that this actually amounts to dropping the property)
    if (!m_aBuilder.resetProperty(aName))
        raisePropChangedBeforeException("LayerUpdateHandler: Cannot remove property - property has already been changed.");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

