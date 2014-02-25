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

#ifndef INCLUDED_FORMS_SOURCE_INC_INTERFACECONTAINER_HXX
#define INCLUDED_FORMS_SOURCE_INC_INTERFACECONTAINER_HXX

#include <boost/unordered_map.hpp>
#include <comphelper/types.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/script/ScriptEvent.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase8.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

using namespace comphelper;


namespace frm
{




    struct ElementDescription
    {
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       xInterface;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   xPropertySet;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >     xChild;
        ::com::sun::star::uno::Any                                                  aElementTypeInterface;

    public:
        ElementDescription( );
        virtual ~ElementDescription();

    private:
        ElementDescription( const ElementDescription& );            // never implemented
        ElementDescription& operator=( const ElementDescription& ); // never implemented
    };

typedef ::std::vector<InterfaceRef> OInterfaceArray;
typedef ::boost::unordered_multimap< OUString, InterfaceRef, OUStringHash > OInterfaceMap;


// OInterfaceContainer
// implements a container for form components

typedef ::cppu::ImplHelper8 <   ::com::sun::star::container::XNameContainer
                            ,   ::com::sun::star::container::XIndexContainer
                            ,   ::com::sun::star::container::XContainer
                            ,   ::com::sun::star::container::XEnumerationAccess
                            ,   ::com::sun::star::script::XEventAttacherManager
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            ,   ::com::sun::star::io::XPersistObject
                            ,   ::com::sun::star::util::XCloneable
                            > OInterfaceContainer_BASE;

class OInterfaceContainer : public OInterfaceContainer_BASE
{
protected:
    ::osl::Mutex&                           m_rMutex;

    OInterfaceArray                         m_aItems;
    OInterfaceMap                           m_aMap;
    ::cppu::OInterfaceContainerHelper       m_aContainerListeners;

    const ::com::sun::star::uno::Type       m_aElementType;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>     m_xContext;


    // EventManager
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager>  m_xEventAttacher;

public:
    OInterfaceContainer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory,
        ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Type& _rElementType);

    OInterfaceContainer( ::osl::Mutex& _rMutex, const OInterfaceContainer& _cloneSource );

    // late constructor for cloning
    void clonedFrom( const OInterfaceContainer& _cloneSource );

protected:
    virtual ~OInterfaceContainer();

public:
// ::com::sun::star::io::XPersistObject
    virtual OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) = 0;
    virtual void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw (::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) ;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration> SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual StringSequence SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName(const OUString& Name, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName(const OUString& Name, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeByName(const OUString& Name) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XIndexReplace
    virtual void SAL_CALL replaceByIndex(sal_Int32 _nIndex, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 _nIndex, const ::com::sun::star::uno::Any& Element) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeByIndex(sal_Int32 _nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);

// ::com::sun::star::script::XEventAttacherManager
    virtual void SAL_CALL registerScriptEvent( sal_Int32 nIndex, const ::com::sun::star::script::ScriptEventDescriptor& aScriptEvent ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL registerScriptEvents( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& aScriptEvents ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL revokeScriptEvent( sal_Int32 nIndex, const OUString& aListenerType, const OUString& aEventMethod, const OUString& aRemoveListenerParam ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL revokeScriptEvents( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL insertEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor > SAL_CALL getScriptEvents( sal_Int32 Index ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL attach( sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xObject, const ::com::sun::star::uno::Any& aHelper ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ServiceNotRegisteredException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL detach( sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xObject ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addScriptListener( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xListener ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeScriptListener( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& Listener ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);

protected:
    // helper
    virtual void SAL_CALL disposing();
    virtual void removeElementsNoEvents(sal_Int32 nIndex);

    /** to be overridden if elements which are to be inserted into the container shall be checked

        <p>the ElementDescription given can be used to cache information about the object - it will be passed
        later on to implInserted/implReplaced.</p>
    */
    virtual void approveNewElement(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject,
                    ElementDescription* _pElement
                );

    virtual ElementDescription* createElementMetaData( );

    /** inserts an object into our internal structures

        @param _nIndex
            the index at which position it should be inserted
        @param _bEvents
            if <TRUE/>, event knittings will be done
        @param _pApprovalResult
            must contain the result of an approveNewElement call. Can be <NULL/>, in this case, the approval
            is done within implInsert.
        @param _bFire
            if <TRUE/>, a notification about the insertion will be fired
    */
            void implInsert(
                sal_Int32 _nIndex,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject,
                sal_Bool _bEvents /* = sal_True */,
                ElementDescription* _pApprovalResult /* = NULL */ ,
                sal_Bool _bFire /* = sal_True */
            ) throw(::com::sun::star::lang::IllegalArgumentException);

    // called after the object is inserted, but before the "real listeners" are notified
    virtual void implInserted( const ElementDescription* _pElement );
    // called after the object is removed, but before the "real listeners" are notified
    virtual void implRemoved(const InterfaceRef& _rxObject);

    /** called after an object was replaced. The default implementation notifies our listeners, after releasing
        the instance lock.
    */
    virtual void impl_replacedElement(
                    const ::com::sun::star::container::ContainerEvent& _rEvent,
                    ::osl::ClearableMutexGuard& _rInstanceLock
                );

    void SAL_CALL writeEvents(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    void SAL_CALL readEvents(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);

    /** replace an element, specified by position

        @precond <arg>_nIndex</arg> is a valid index
        @precond our mutex is locked exactly once, by the guard specified with <arg>_rClearBeforeNotify</arg>

    */
    void implReplaceByIndex(
            const sal_Int32 _nIndex,
            const ::com::sun::star::uno::Any& _rNewElement,
            ::osl::ClearableMutexGuard& _rClearBeforeNotify
        );

    /** removes an element, specified by position

        @precond <arg>_nIndex</arg> is a valid index
        @precond our mutex is locked exactly once, by the guard specified with <arg>_rClearBeforeNotify</arg>

    */
    void implRemoveByIndex(
            const sal_Int32 _nIndex,
            ::osl::ClearableMutexGuard& _rClearBeforeNotify
        );

    /** validates the given index
        @throws ::com::sun::star::lang::IndexOutOfBoundsException
            if the given index does not denote a valid position in our children array
    */
    void implCheckIndex( const sal_Int32 _nIndex ) SAL_THROW( ( ::com::sun::star::lang::IndexOutOfBoundsException ) );

private:
    // hack for Vba Events
    void impl_addVbEvents_nolck_nothrow( const sal_Int32 i_nIndex );

    // the runtime event format has changed from version SO5.2 to OOo
    enum EventFormat
    {
        efVersionSO5x,
        efVersionSO6x
    };
    void    transformEvents( const EventFormat _eTargetFormat );

    void    impl_createEventAttacher_nothrow();
};


//= OFormComponents

typedef ::cppu::ImplHelper1< ::com::sun::star::form::XFormComponent> OFormComponents_BASE;
typedef ::cppu::OComponentHelper FormComponentsBase;
    // else MSVC kills itself on some statements
class OFormComponents   :public FormComponentsBase
                        ,public OInterfaceContainer
                        ,public OFormComponents_BASE
{
protected:
    ::osl::Mutex                m_aMutex;
    ::comphelper::InterfaceRef  m_xParent;

public:
    OFormComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    OFormComponents( const OFormComponents& _cloneSource );
    virtual ~OFormComponents();

    DECLARE_UNO3_AGG_DEFAULTS(OFormComponents, FormComponentsBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::form::XFormComponent
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XEventListener
    using OInterfaceContainer::disposing;
};

}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_INTERFACECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
