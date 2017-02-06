/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "definitioncontainer.hxx"
#include "dbastrings.hrc"
#include "apitools.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <comphelper/types.hxx>
#include <ucbhelper/contentidentifier.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::ucb;

namespace dbaccess
{

// ODefinitionContainer_Impl
void ODefinitionContainer_Impl::erase( const TContentPtr& _pDefinition )
{
    NamedDefinitions::const_iterator aPos = find( _pDefinition );
    if ( aPos != end() )
        m_aDefinitions.erase( aPos );
}

ODefinitionContainer_Impl::const_iterator ODefinitionContainer_Impl::find( const TContentPtr& _pDefinition ) const
{
    return ::std::find_if(
        m_aDefinitions.begin(),
        m_aDefinitions.end(),
        [&_pDefinition] (const NamedDefinitions::value_type& namedDef) {
            return namedDef.second == _pDefinition;
        });
}

ODefinitionContainer_Impl::iterator ODefinitionContainer_Impl::find( const TContentPtr& _pDefinition )
{
    return ::std::find_if(
        m_aDefinitions.begin(),
        m_aDefinitions.end(),
        [&_pDefinition] (const NamedDefinitions::value_type& namedDef) {
            return namedDef.second == _pDefinition;
        });
}

// ODefinitionContainer

ODefinitionContainer::ODefinitionContainer(   const Reference< XComponentContext >& _xORB
                                            , const Reference< XInterface >&    _xParentContainer
                                            , const TContentPtr& _pImpl
                                            , bool _bCheckSlash
                                            )
    :OContentHelper(_xORB,_xParentContainer,_pImpl)
    ,m_aApproveListeners(m_aMutex)
    ,m_aContainerListeners(m_aMutex)
    ,m_bInPropertyChange(false)
    ,m_bCheckSlash(_bCheckSlash)
{
    m_pImpl->m_aProps.bIsDocument = false;
    m_pImpl->m_aProps.bIsFolder = true;

    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    ODefinitionContainer_Impl::const_iterator aEnd = rDefinitions.end();
    for (   ODefinitionContainer_Impl::const_iterator aDefinition = rDefinitions.begin();
            aDefinition != aEnd;
            ++aDefinition
        )
        m_aDocuments.push_back(
            m_aDocumentMap.insert(
                Documents::value_type( aDefinition->first, Documents::mapped_type() ) ).first );

}

void SAL_CALL ODefinitionContainer::disposing()
{
    OContentHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // say goodbye to our listeners
    EventObject aEvt(*this);
    m_aApproveListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);

    // dispose our elements
    Documents::const_iterator aIter = m_aDocumentMap.begin();
    Documents::const_iterator aEnd = m_aDocumentMap.end();

    for (; aIter != aEnd; ++aIter)
    {
        Reference<XContent> xProp = aIter->second;
        if ( xProp.is() )
        {
            removeObjectListener(xProp);
            ::comphelper::disposeComponent(xProp);
        }
    }

    // remove our elements
    m_aDocuments.clear();
    //  !!! do this before clearing the map which the vector elements refer to !!!
    m_aDocumentMap.clear();
}

ODefinitionContainer::~ODefinitionContainer()
{
}

IMPLEMENT_FORWARD_XINTERFACE2( ODefinitionContainer,OContentHelper,ODefinitionContainer_Base)
IMPLEMENT_GETTYPES2(ODefinitionContainer,OContentHelper,ODefinitionContainer_Base);

css::uno::Sequence<sal_Int8> ODefinitionContainer::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
OUString SAL_CALL ODefinitionContainer::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.ODefinitionContainer");
}

Sequence< OUString > SAL_CALL ODefinitionContainer::getSupportedServiceNames(  )
{
    Sequence< OUString > aReturn(2);
    aReturn.getArray()[0] = "com.sun.star.sdb.DefinitionContainer";
    aReturn.getArray()[1] = "com.sun.star.ucb.Content";
    return aReturn;
}

// XNameContainer
void SAL_CALL ODefinitionContainer::insertByName( const OUString& _rName, const Any& aElement )
{
    ResettableMutexGuard aGuard(m_aMutex);

    // approve the new object
    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    approveNewObject( _rName, xNewElement );  // will throw if necessary

    notifyByName( aGuard, _rName, xNewElement, nullptr, E_INSERTED, ApproveListeners );
    implAppend( _rName, xNewElement );
    notifyByName( aGuard, _rName, xNewElement, nullptr, E_INSERTED, ContainerListemers );
}

void SAL_CALL ODefinitionContainer::removeByName( const OUString& _rName )
{
    ResettableMutexGuard aGuard(m_aMutex);

    // check the arguments
    if (_rName.isEmpty())
        throw IllegalArgumentException();

    if (!checkExistence(_rName))
        throw NoSuchElementException(_rName,*this);

    // the old element (for the notifications)
    Reference< XContent > xOldElement = implGetByName( _rName, impl_haveAnyListeners_nothrow() );

    // do the removal
    notifyByName( aGuard, _rName, nullptr, xOldElement, E_REMOVED, ApproveListeners );
    implRemove( _rName );
    notifyByName( aGuard, _rName, nullptr, xOldElement, E_REMOVED, ContainerListemers );

    removeObjectListener( xOldElement );
    disposeComponent(xOldElement);
}

// XNameReplace
void SAL_CALL ODefinitionContainer::replaceByName( const OUString& _rName, const Any& aElement )
{
    ResettableMutexGuard aGuard(m_aMutex);

    try
    {
        // let derived classes approve the new object
        Reference< XContent > xNewElement(aElement,UNO_QUERY);
        approveNewObject( _rName, xNewElement );    // will throw if necessary

        // the old element (for the notifications)
        Reference< XContent > xOldElement = implGetByName( _rName, impl_haveAnyListeners_nothrow() );

        notifyByName( aGuard, _rName, xNewElement, xOldElement, E_REPLACED, ApproveListeners );
        implReplace( _rName, xNewElement );
        notifyByName( aGuard, _rName, xNewElement, xOldElement, E_REPLACED, ContainerListemers );

        // and dispose it
        disposeComponent(xOldElement);
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const NoSuchElementException&)
    {
        throw;
    }
    catch (const WrappedTargetException&)
    {
        throw;
    }
    catch (const Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetException(
          "wrapped Exception " + e.Message,
          css::uno::Reference<css::uno::XInterface>(), a);
    }
}

namespace
{
    typedef Reference< XVeto > ( SAL_CALL XContainerApproveListener::*ContainerApprovalMethod )( const ContainerEvent& );

    struct RaiseExceptionFromVeto
    {
    private:
        ContainerApprovalMethod m_pMethod;
        const ContainerEvent&   m_rEvent;

    public:
        explicit RaiseExceptionFromVeto( ContainerApprovalMethod _pMethod, const ContainerEvent& _rEvent )
            :m_pMethod( _pMethod )
            ,m_rEvent( _rEvent )
        {
        }

        void operator()( const Reference< XContainerApproveListener >& Listener ) const
        {
            Reference< XVeto > xVeto = (Listener.get()->*m_pMethod)( m_rEvent );
            if ( !xVeto.is() )
                return;

            Any eVetoDetails = xVeto->getDetails();

            IllegalArgumentException aIllegalArgumentError;
            if ( eVetoDetails >>= aIllegalArgumentError )
                throw aIllegalArgumentError;

            WrappedTargetException aWrappedError;
            if ( eVetoDetails >>= aWrappedError )
                throw aWrappedError;

            throw WrappedTargetException( xVeto->getReason(), Listener.get(), eVetoDetails );
        }
    };
}

void ODefinitionContainer::notifyByName( ResettableMutexGuard& _rGuard, const OUString& _rName,
        const Reference< XContent >& _xNewElement, const Reference< XContent >& _xOldElement,
        ContainerOperation _eOperation, ListenerType _eType )
{
    bool bApprove = ( _eType == ApproveListeners );

    ::comphelper::OInterfaceContainerHelper2& rContainer( bApprove ? m_aApproveListeners : m_aContainerListeners );
    if ( !rContainer.getLength() )
        return;

    ContainerEvent aEvent( *this, makeAny( _rName ), makeAny( _xNewElement ), makeAny( _xOldElement ) );

    _rGuard.clear();
    switch ( _eOperation )
    {
        case E_INSERTED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveInsertElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementInserted, aEvent );
            break;
        case E_REPLACED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveReplaceElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementReplaced, aEvent );
            break;
        case E_REMOVED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveRemoveElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementRemoved, aEvent );
            break;
    }

    if ( bApprove )
        _rGuard.reset();
}

void SAL_CALL ODefinitionContainer::addContainerListener( const Reference< XContainerListener >& _rxListener )
{
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

void SAL_CALL ODefinitionContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener )
{
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

void SAL_CALL ODefinitionContainer::addContainerApproveListener( const Reference< XContainerApproveListener >& Listener )
{
    if ( Listener.is() )
        m_aApproveListeners.addInterface( Listener );
}

void SAL_CALL ODefinitionContainer::removeContainerApproveListener( const Reference< XContainerApproveListener >& Listener )
{
    if ( Listener.is() )
        m_aApproveListeners.removeInterface( Listener );
}

// XElementAccess
Type SAL_CALL ODefinitionContainer::getElementType( )
{
    return cppu::UnoType<XContent>::get();
}

sal_Bool SAL_CALL ODefinitionContainer::hasElements( )
{
    MutexGuard aGuard(m_aMutex);
    return !m_aDocuments.empty();
}

// XEnumerationAccess
Reference< XEnumeration > SAL_CALL ODefinitionContainer::createEnumeration(  )
{
    MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

// XIndexAccess
sal_Int32 SAL_CALL ODefinitionContainer::getCount(  )
{
    MutexGuard aGuard(m_aMutex);
    return m_aDocuments.size();
}

Any SAL_CALL ODefinitionContainer::getByIndex( sal_Int32 _nIndex )
{
    MutexGuard aGuard(m_aMutex);

    if ((_nIndex < 0) || (_nIndex >= (sal_Int32)m_aDocuments.size()))
        throw IndexOutOfBoundsException();

    Documents::iterator aPos = m_aDocuments[_nIndex];
    Reference<XContent> xProp = aPos->second;
    if (!xProp.is())
    {   // that's the first access to the object
        // -> create it
        xProp = createObject(aPos->first);
        aPos->second = Documents::mapped_type();
        // and update the name-access map
    }

    return makeAny(xProp);
}

Any SAL_CALL ODefinitionContainer::getByName( const OUString& _rName )
{
    MutexGuard aGuard(m_aMutex);

    return makeAny( implGetByName( _rName, true ) );
}

Reference< XContent > ODefinitionContainer::implGetByName(const OUString& _rName, bool _bReadIfNecessary)
{
    Documents::iterator aMapPos = m_aDocumentMap.find(_rName);
    if (aMapPos == m_aDocumentMap.end())
        throw NoSuchElementException(_rName,*this);

    Reference< XContent > xProp = aMapPos->second;

    if (_bReadIfNecessary && !xProp.is())
    {   // the object has never been accessed before, so we have to read it now
        // (that's the expensive part)

        // create the object and insert it into the map
        xProp = createObject(_rName);
        aMapPos->second = xProp;
        addObjectListener(xProp);
    }

    return xProp;
}

Sequence< OUString > SAL_CALL ODefinitionContainer::getElementNames(  )
{
    MutexGuard aGuard(m_aMutex);

    Sequence< OUString > aNames(m_aDocumentMap.size());
    OUString* pNames = aNames.getArray();
    Documents::const_iterator aEnd = m_aDocumentMap.end();
    for (   Documents::const_iterator aNameIter = m_aDocumentMap.begin();
            aNameIter != aEnd;
            ++pNames, ++aNameIter
        )
    {
        *pNames = aNameIter->first;
    }

    return aNames;
}

sal_Bool SAL_CALL ODefinitionContainer::hasByName( const OUString& _rName )
{
    MutexGuard aGuard(m_aMutex);

    return checkExistence(_rName);
}

void SAL_CALL ODefinitionContainer::disposing( const EventObject& _rSource )
{
    MutexGuard aGuard(m_aMutex);
    Reference< XContent > xSource(_rSource.Source, UNO_QUERY);
    // it's one of our documents ....
    Documents::iterator aIter = m_aDocumentMap.begin();
    Documents::const_iterator aEnd = m_aDocumentMap.end();
    for (;aIter != aEnd;++aIter )
    {
        if ( xSource == aIter->second.get() )
        {
            removeObjectListener(xSource);
            // and clear our document map/vector, so the object will be recreated on next access
            aIter->second = Documents::mapped_type();
        }
    }
}

void ODefinitionContainer::implRemove(const OUString& _rName)
{
    // from the object maps
    Documents::const_iterator aFind = m_aDocumentMap.find(_rName);
    if ( aFind != m_aDocumentMap.end() )
    {
        m_aDocuments.erase( ::std::find(m_aDocuments.begin(),m_aDocuments.end(),aFind));
        m_aDocumentMap.erase(aFind);

        getDefinitions().erase( _rName );

        notifyDataSourceModified();
    }
}

namespace
{
    bool    lcl_ensureName( const Reference< XContent >& _rxContent, const OUString& _rName )
    {
        if ( !_rxContent.is() )
            return true;

        // obtain the current name. If it's the same as the new one,
        // don't do anything
        try
        {
            Reference< XPropertySet > xProps( _rxContent, UNO_QUERY );
            if ( xProps.is() )
            {
                OUString sCurrentName;
                OSL_VERIFY( xProps->getPropertyValue( PROPERTY_NAME ) >>= sCurrentName );
                if ( sCurrentName.equals( _rName ) )
                    return true;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_ensureName: caught an exception while obtaining the current name!" );
        }

        // set the new name
        Reference< XRename > xRename( _rxContent, UNO_QUERY );
        OSL_ENSURE( xRename.is(), "lcl_ensureName: invalid content (not renameable)!" );
        if ( !xRename.is() )
            return false;
        try
        {
            xRename->rename( _rName );
            return true;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_ensureName: caught an exception!" );
        }
        return false;
    }
}

void ODefinitionContainer::implAppend(const OUString& _rName, const Reference< XContent >& _rxNewObject)
{
    MutexGuard aGuard(m_aMutex);
    try
    {
        Reference<XChild> xChild(_rxNewObject,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(static_cast<OWeakObject*>(this));

        ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( _rName );
        if ( aFind == rDefinitions.end() )
        {
            // ensure that the new object has the proper name.
            // Somebody could create an object with name "foo", and insert it as "bar"
            // into a container. In this case, we need to ensure that the object name
            // is also "bar"
            // #i44786#
            lcl_ensureName( _rxNewObject, _rName );

            ::rtl::Reference< OContentHelper > pContent = OContentHelper::getImplementation( _rxNewObject );
            if ( pContent.is() )
            {
                TContentPtr pImpl = pContent->getImpl();
                rDefinitions.erase( pImpl );
                pImpl->m_aProps.aTitle = _rName;
                rDefinitions.insert( _rName, pImpl );
            }
        }

        m_aDocuments.push_back(m_aDocumentMap.insert(Documents::value_type(_rName,_rxNewObject)).first);
        notifyDataSourceModified();
        // now update our structures
        if ( _rxNewObject.is() )
            addObjectListener(_rxNewObject);
    }
    catch(Exception&)
    {
        OSL_FAIL("ODefinitionContainer::implAppend: caught something !");
    }
}

void ODefinitionContainer::implReplace(const OUString& _rName, const Reference< XContent >& _rxNewObject)
{
    OSL_ENSURE(checkExistence(_rName), "ODefinitionContainer::implReplace : invalid name !");

    Documents::iterator aFind = m_aDocumentMap.find(_rName);
    removeObjectListener(aFind->second);
    aFind->second = _rxNewObject;
    addObjectListener(aFind->second);
}

void ODefinitionContainer::approveNewObject(const OUString& _sName,const Reference< XContent >& _rxObject) const
{
    // check the arguments
    if ( _sName.isEmpty() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_NAME_MUST_NOT_BE_EMPTY ),
            *this,
            0 );

    if ( m_bCheckSlash && _sName.indexOf( '/' ) != -1 )
        throw IllegalArgumentException(
            m_aErrorHelper.getErrorMessage( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES ),
            *this,
            0 );

    if ( !_rxObject.is() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_NO_NULL_OBJECTS_IN_CONTAINER ),
            *this,
            0 );

    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    if ( rDefinitions.find( _sName ) != rDefinitions.end() )
        throw ElementExistException(
            DBA_RES( RID_STR_NAME_ALREADY_USED ),
            *this );

    ::rtl::Reference< OContentHelper > pContent( OContentHelper::getImplementation( _rxObject ) );
    if ( !pContent.is() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_OBJECT_CONTAINER_MISMATCH ),
            *this,
            1 );

    if ( rDefinitions.find( pContent->getImpl() ) != rDefinitions.end() )
        throw ElementExistException(
            DBA_RES( RID_STR_OBJECT_ALREADY_CONTAINED ),
            *this );
}

// XPropertyChangeListener
void SAL_CALL ODefinitionContainer::propertyChange( const PropertyChangeEvent& evt )
{
    ClearableMutexGuard aGuard(m_aMutex);
    if( evt.PropertyName == PROPERTY_NAME || evt.PropertyName ==  "Title" )
    {
        m_bInPropertyChange = true;
        try
        {
            OUString sNewName,sOldName;
            evt.OldValue >>= sOldName;
            evt.NewValue >>= sNewName;
            Reference<XContent> xContent( evt.Source, UNO_QUERY );
            removeObjectListener( xContent );
            implRemove( sOldName );
            implAppend( sNewName, xContent );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
            throw RuntimeException();
        }
        m_bInPropertyChange = false;
    }
}

// XVetoableChangeListener
void SAL_CALL ODefinitionContainer::vetoableChange( const PropertyChangeEvent& aEvent )
{
    MutexGuard aGuard(m_aMutex);

    if( aEvent.PropertyName == PROPERTY_NAME || aEvent.PropertyName == "Title" )
    {
        OUString sNewName;
        aEvent.NewValue >>= sNewName;
        if(hasByName(sNewName))
            throw PropertyVetoException();
    }
}

void ODefinitionContainer::addObjectListener(const Reference< XContent >& _xNewObject)
{
    OSL_ENSURE(_xNewObject.is(),"ODefinitionContainer::addObjectListener: Object is null!");
    Reference<XPropertySet> xProp(_xNewObject,UNO_QUERY);
    if ( xProp.is() )
    {
        xProp->addPropertyChangeListener(PROPERTY_NAME, this);
        xProp->addVetoableChangeListener(PROPERTY_NAME, this);
    }
}

void ODefinitionContainer::removeObjectListener(const Reference< XContent >& _xNewObject)
{
    Reference<XPropertySet> xProp(_xNewObject,UNO_QUERY);
    if ( xProp.is() )
    {
        xProp->removePropertyChangeListener(PROPERTY_NAME, this);
        xProp->removeVetoableChangeListener(PROPERTY_NAME, this);
    }
}

bool ODefinitionContainer::checkExistence(const OUString& _rName)
{
    return m_aDocumentMap.find(_rName) != m_aDocumentMap.end();
}

}

// namespace dbaccess
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
