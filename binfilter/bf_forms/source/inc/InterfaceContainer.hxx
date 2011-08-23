/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#define _FRM_INTERFACE_CONTAINER_HXX_

#include <hash_map>

#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XFormComponent.hpp>


#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase7.hxx>
namespace binfilter {

    using namespace ::comphelper;

//.........................................................................
namespace frm
{
//.........................................................................


//==================================================================
    struct ElementDescription
    {
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >		xInterface;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >	xPropertySet;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >		xChild;
        ::com::sun::star::uno::Any													aElementTypeInterface;

    public:
        ElementDescription( );
        virtual ~ElementDescription();

    private:
        ElementDescription( const ElementDescription& );			// never implemented
        ElementDescription& operator=( const ElementDescription& );	// never implemented
    };

typedef ::std::vector<InterfaceRef> OInterfaceArray;
typedef ::std::hash_multimap< ::rtl::OUString, InterfaceRef, ::comphelper::UStringHash, ::comphelper::UStringEqual> OInterfaceMap;

//==================================================================
// OInterfaceContainer
// implements a container for form components
//==================================================================
typedef ::cppu::ImplHelper7<	::com::sun::star::container::XNameContainer,
                                ::com::sun::star::container::XIndexContainer,    
                                ::com::sun::star::container::XContainer,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::script::XEventAttacherManager, 
                                ::com::sun::star::beans::XPropertyChangeListener,
                                ::com::sun::star::io::XPersistObject > OInterfaceContainer_BASE;

class OInterfaceContainer :	public OInterfaceContainer_BASE
{
protected:
    OInterfaceArray							m_aItems;
    OInterfaceMap							m_aMap;
    ::cppu::OInterfaceContainerHelper		m_aContainerListeners;

    ::osl::Mutex&							m_rMutex;
    ::com::sun::star::uno::Type				m_aElementType;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>		m_xServiceFactory;


    // EventManager
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager> 	m_xEventAttacher;

public:
    OInterfaceContainer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
        ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Type& _rElementType);

public:
// ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException) = 0;
    virtual void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration> SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName(const ::rtl::OUString& Name, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName(const ::rtl::OUString& Name, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName(const ::rtl::OUString& Name) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexReplace
    virtual void SAL_CALL replaceByIndex(sal_Int32 _nIndex, const ::com::sun::star::uno::Any& _rElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 _nIndex, const ::com::sun::star::uno::Any& Element) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex(sal_Int32 _nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::script::XEventAttacherManager
    virtual void SAL_CALL registerScriptEvent( sal_Int32 nIndex, const ::com::sun::star::script::ScriptEventDescriptor& aScriptEvent ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerScriptEvents( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& aScriptEvents ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeScriptEvent( sal_Int32 nIndex, const ::rtl::OUString& aListenerType, const ::rtl::OUString& aEventMethod, const ::rtl::OUString& aRemoveListenerParam ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeScriptEvents( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor > SAL_CALL getScriptEvents( sal_Int32 Index ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attach( sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xObject, const ::com::sun::star::uno::Any& aHelper ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ServiceNotRegisteredException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL detach( sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xObject ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addScriptListener( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xListener ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeScriptListener( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& Listener ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

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
    virtual void implInserted( const ElementDescription* _pElement ) { }
    // called after the object is removed, but before the "real listeners" are notified
    virtual void implRemoved(const InterfaceRef& _rxObject) { }
    // called after an object was replaced, but before the "real listeners" are notified
    virtual void implReplaced( const InterfaceRef& _rxReplacedObject, const ElementDescription* _pElement ) { }

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
            if the given index does not denote a valid position in our childs array
    */
    void implCheckIndex( const sal_Int32 _nIndex ) SAL_THROW( ( ::com::sun::star::lang::IndexOutOfBoundsException ) );

private:
    // the runtime event format has changed from version SO5.2 to OOo
    enum EventFormat
    {
        efVersionSO5x,
        efVersionSO6x
    };
    void	transformEvents( const EventFormat _eTargetFormat );
};

//==================================================================
//= OFormComponents
//==================================================================
typedef ::cppu::ImplHelper1< ::com::sun::star::form::XFormComponent> OFormComponents_BASE;
typedef ::cppu::OComponentHelper FormComponentsBase;
    // else MSVC kills itself on some statements
class OFormComponents	: public FormComponentsBase,
                          public OInterfaceContainer,
                          public OFormComponents_BASE
{
protected:
    ::osl::Mutex				m_aMutex;
    ::comphelper::InterfaceRef 	m_xParent;

public:
    OFormComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormComponents();

    DECLARE_UNO3_AGG_DEFAULTS(OFormComponents, FormComponentsBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::form::XFormComponent
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
};
//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif          // _FRM_INTERFACE_CONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
