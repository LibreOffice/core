/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdatehandler.cxx,v $
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

#include "layerupdatehandler.hxx"
#include "layerupdatemerger.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>

// -----------------------------------------------------------------------------
#define OUSTR( str ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateUpdateMerger
( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new LayerUpdateHandler( xContext );
}

// -----------------------------------------------------------------------------

LayerUpdateHandler::LayerUpdateHandler(uno::Reference< uno::XComponentContext > const & _xContext)
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
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this,uno::Any());
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raiseNodeChangedBeforeException(sal_Char const * pMsg)
{
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this,uno::Any());
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raisePropChangedBeforeException(sal_Char const * pMsg)
{
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);
    throw backenduno::MalformedDataException(sMsg,*this,uno::Any());
}
// -----------------------------------------------------------------------------

void LayerUpdateHandler::raisePropExistsException(sal_Char const * pMsg)
{
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);
    com::sun::star::beans::PropertyExistException e(sMsg,*this);

    throw backenduno::MalformedDataException(sMsg,*this, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

// XUpdateHandler
void SAL_CALL
    LayerUpdateHandler::startUpdate(  )
        throw ( backenduno::MalformedDataException, lang::IllegalAccessException,
                lang::WrappedTargetException, uno::RuntimeException)
{
    this->checkSourceLayer();
    if (!m_aBuilder.init())
        raiseMalformedDataException("LayerUpdateHandler: Cannot start update - update is already in progress");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endUpdate(  )
        throw ( backenduno::MalformedDataException, lang::IllegalAccessException,
                lang::WrappedTargetException, uno::RuntimeException)
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
    LayerUpdateHandler::modifyNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.modifyNode(aName,aAttributes,aAttributeMask,bReset))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start node modification - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.replaceNode(aName,aAttributes,NULL))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start added/replaced node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aTemplate )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.replaceNode(aName,aAttributes,&aTemplate))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot start added/replaced node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endNode(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
    LayerUpdateHandler::removeNode( const rtl::OUString& aName )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder();

    if (!m_aBuilder.removeNode(aName))
        raiseNodeChangedBeforeException("LayerUpdateHandler: Cannot remove node - node has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: modifyProperty( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, const uno::Type & aType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(false);

    if (!m_aBuilder.modifyProperty(aName,aAttributes,aAttributeMask, aType))
        raisePropChangedBeforeException("LayerUpdateHandler: Cannot start property modification - property has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: setPropertyValue( const uno::Any& aValue )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.setPropertyValue(aValue) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler:: setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.setPropertyValueForLocale(aValue,aLocale) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetPropertyValue( )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.resetPropertyValue() );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetPropertyValueForLocale( const rtl::OUString& aLocale )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY( m_aBuilder.resetPropertyValueForLocale(aLocale) );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::endProperty(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    checkBuilder(true); // already checks for open property

    OSL_VERIFY ( m_aBuilder.finishProperty() );
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::resetProperty( const rtl::OUString& aName )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!m_aBuilder.resetProperty(aName))
        raisePropChangedBeforeException("LayerUpdateHandler: Cannot reset property - property has already been changed.");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplaceProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!m_aBuilder.addNullProperty(aName,aAttributes,aType))
        raisePropExistsException("LayerUpdateHandler: Cannot add property - property exists (and has already been changed).");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::addOrReplacePropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!m_aBuilder.addProperty(aName,aAttributes,aValue))
        raisePropExistsException("LayerUpdateHandler: Cannot add property - property exists (and has already been changed).");
}
// -----------------------------------------------------------------------------

void SAL_CALL
    LayerUpdateHandler::removeProperty( const rtl::OUString& aName )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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

