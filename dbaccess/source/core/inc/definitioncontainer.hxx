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

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#define _DBA_CORE_DEFINITIONCONTAINER_HXX_

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase7.hxx>
#include <comphelper/stl_types.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/container/XContainerApproveBroadcaster.hpp>
#include "ContentHelper.hxx"
#include "containerapprove.hxx"
#include <comphelper/uno3.hxx>
#include <rtl/ref.hxx>
#include "apitools.hxx"

namespace dbaccess
{

class ODefinitionContainer_Impl : public OContentHelper_Impl
{
public:
    typedef ::std::map< OUString, TContentPtr >  NamedDefinitions;
    typedef NamedDefinitions::iterator                  iterator;
    typedef NamedDefinitions::const_iterator            const_iterator;

private:
    NamedDefinitions    m_aDefinitions;

public:
    inline size_t size() const { return m_aDefinitions.size(); }

    inline const_iterator begin() const   { return m_aDefinitions.begin(); }
    inline const_iterator end() const     { return m_aDefinitions.end(); }

    inline const_iterator find( const OUString& _rName ) const { return m_aDefinitions.find( _rName ); }
           const_iterator find( TContentPtr _pDefinition ) const;

    inline void erase( const OUString& _rName ) { m_aDefinitions.erase( _rName ); }
    void erase( TContentPtr _pDefinition );

    inline void insert( const OUString& _rName, TContentPtr _pDefinition )
    {
        m_aDefinitions.insert( NamedDefinitions::value_type( _rName, _pDefinition ) );
    }

private:
    iterator find( TContentPtr _pDefinition );
        // (for the moment, this is private. Make it public if needed. If really needed.)
};

// ODefinitionContainer -  base class of collections of database definition
//                         documents
typedef ::cppu::ImplHelper7 <   ::com::sun::star::container::XIndexAccess
                            ,   ::com::sun::star::container::XNameContainer
                            ,   ::com::sun::star::container::XEnumerationAccess
                            ,   ::com::sun::star::container::XContainer
                            ,   ::com::sun::star::container::XContainerApproveBroadcaster
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            ,   ::com::sun::star::beans::XVetoableChangeListener
                            >   ODefinitionContainer_Base;

class ODefinitionContainer
            :public OContentHelper
            ,public ODefinitionContainer_Base
{
protected:
    DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::WeakReference< ::com::sun::star::ucb::XContent >, Documents);
    DECLARE_STL_VECTOR(Documents::iterator, DocumentsIndexAccess);

    enum ContainerOperation
    {
        E_REPLACED,
        E_REMOVED,
        E_INSERTED
    };

    enum ListenerType
    {
        ApproveListeners,
        ContainerListemers
    };

private:
    PContainerApprove   m_pElementApproval;

protected:
    // we can't just hold a vector of XContentRefs, as after initialization they're all empty
    // cause we load them only on access
    DocumentsIndexAccess    m_aDocuments;               // for a efficient index access
    Documents               m_aDocumentMap;             // for a efficient name access

    ::cppu::OInterfaceContainerHelper
                            m_aApproveListeners;
    ::cppu::OInterfaceContainerHelper
                            m_aContainerListeners;

    sal_Bool                m_bInPropertyChange;
    bool                    m_bCheckSlash;

protected:
    /** Additionally to our own approvals which new elements must pass, derived classes
        can specifiy an additional approval instance here.

        Every time a new element is inserted into the container (or an element is replaced
        with a new one), this new element must pass our own internal approval, plus the approval
        given here.
    */
    void                setElementApproval( PContainerApprove _pElementApproval ) { m_pElementApproval = _pElementApproval; }
    PContainerApprove   getElementApproval() const { return m_pElementApproval; }

protected:
    virtual ~ODefinitionContainer();

    inline const ODefinitionContainer_Impl& getDefinitions() const
    {
        return dynamic_cast< const ODefinitionContainer_Impl& >( *m_pImpl.get() );
    }

    inline       ODefinitionContainer_Impl&  getDefinitions()
    {
        return dynamic_cast<       ODefinitionContainer_Impl& >( *m_pImpl.get() );
    }
public:
    /** constructs the container.
    */
    ODefinitionContainer(
          const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  _xParentContainer
        , const TContentPtr& _pImpl
        , bool _bCheckSlash = true
        );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )
// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 _nIndex ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName( const OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& _rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // XContainerApproveBroadcaster
    virtual void SAL_CALL addContainerApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerApproveListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerApproveListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);
    // XVetoableChangeListener
    virtual void SAL_CALL vetoableChange( const ::com::sun::star::beans::PropertyChangeEvent& aEvent ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

protected:
    // helper
    virtual void SAL_CALL disposing();

    /** create a object from it's persistent data within the configuration. To be overwritten by derived classes.
        @param      _rName          the name the object has within the container
        @return                     the newly created object or an empty reference if something went wrong
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(
        const OUString& _rName) = 0;

    /** get the object specified by the given name. If desired, the object will be read if not already done so.<BR>
        @param      _rName              the object name
        @param      _bReadIfNeccessary  if sal_True, the object will be created if necessary
        @return                         the property set interface of the object. Usually the return value is not NULL, but
                                        if so, then the object could not be read from the configuration
        @throws                         NoSuchElementException if there is no object with the given name.
        @see    createObject
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >
                implGetByName(const OUString& _rName, sal_Bool _bCreateIfNecessary) throw (::com::sun::star::container::NoSuchElementException);

    /** quickly checks if there already is an element with a given name. No access to the configuration occures, i.e.
        if there is such an object which is not already loaded, it won't be loaded now.
        @param      _rName      the object name to check
        @return                 sal_True if there already exists such an object
    */
    virtual sal_Bool checkExistence(const OUString& _rName);

    /** append a new object to the container. No plausibility checks are done, e.g. if the object is non-NULL or
        if the name is already used by another object or anything like this. This method is for derived classes
        which may support different methods to create and/or append objects, and don't want to deal with the
        internal structures of this class.<BR>
        The old component will not be disposed, this is the callers responsibility, too.
        @param      _rName          the name of the new object
        @param      _rxNewObject    the new object (not surprising, is it ?)
        @see        createConfigKey
        @see        implReplace
        @see        implRemove
    */
    void    implAppend(
        const OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxNewObject
        );

    /** remove all references to an object from the container. No plausibility checks are done, e.g. whether
        or not there exists an object with the given name. This is the responsibility of the caller.<BR>
        Additionally the node for the given object will be removed from the registry (including all sub nodes).<BR>
        The old component will not be disposed, this is the callers responsibility, too.
        @param          _rName      the objects name
        @see            implReplace
        @see            implAppend
    */
    void implRemove(const OUString& _rName);

    /** remove a object in the container. No plausibility checks are done, e.g. whether
        or not there exists an object with the given name or the object is non-NULL. This is the responsibility of the caller.<BR>
        Additionally all object-related information within the registry will be deleted. The new object config node,
        where the caller may want to store the new objects information, is returned.<BR>
        The old component will not be disposed, this is the callers responsibility, too.
        @param          _rName              the objects name
        @param          _rxNewObject        the new object
        @param          _rNewObjectNode     the configuration node where the new object may be stored
        @see            implAppend
        @see            implRemove
    */
    void implReplace(
        const OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxNewObject
        );

    /** notifies our container/approve listeners
    */
    void notifyByName(
            ::osl::ResettableMutexGuard& _rGuard,
            const OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewElement,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& xOldElement,
            ContainerOperation _eOperation,
            ListenerType _eType
        );

    inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () const
    {
        return const_cast< XContainer* >( static_cast< const XContainer* >( this ) );
    }

private:
    void    addObjectListener(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewObject);
    void    removeObjectListener(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewObject);

    /** approve that the object given may be inserted into the container. Should be overloaded by derived classes,
        the default implementation just checks the object to be non-void.

        @throws IllegalArgumentException
            if the name or the object are invalid
        @throws ElementExistException
            if the object already exists in the container, or another object with the same name
            already exists
        @throws WrappedTargetException
            if another error occures which prevents insertion of the object into the container
    */
    void approveNewObject(
            const OUString& _sName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxObject
        ) const;

    inline bool impl_haveAnyListeners_nothrow() const
    {
        return ( m_aContainerListeners.getLength() > 0 ) || ( m_aApproveListeners.getLength() > 0 );
    }
};

}   // namespace dbaccess

#endif // _DBA_CORE_DEFINITIONCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
