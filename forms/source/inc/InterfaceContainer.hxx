/*************************************************************************
 *
 *  $RCSfile: InterfaceContainer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:06 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#define _FRM_INTERFACE_CONTAINER_HXX_

#ifndef _UTL_STLTYPES_HXX_
#include <unotools/stl_types.hxx>
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENT_HPP_
#include <com/sun/star/script/ScriptEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

using namespace utl;

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starcontainer = ::com::sun::star::container;
    namespace starscript    = ::com::sun::star::script;
    namespace starbeans     = ::com::sun::star::beans;
    namespace stario        = ::com::sun::star::io;
    namespace starlang      = ::com::sun::star::lang;
    namespace starform  = ::com::sun::star::form;


typedef ::std::vector<InterfaceRef> OInterfaceArray;
typedef ::std::hash_multimap< ::rtl::OUString, InterfaceRef, ::utl::UStringHash, ::utl::UStringEqual> OInterfaceMap;

//==================================================================
// FmForms
// Implementiert den UNO-Container fuer Formulare
// enthaelt alle zugeordneten Forms
// dieses Container kann selbst den Context fuer Formulare darstellen
// oder auﬂen einen Context uebergeben bekommen
//==================================================================
class OInterfaceContainer
                        :public starcontainer::XNameContainer
                        ,public starcontainer::XIndexContainer
                        ,public starcontainer::XContainer
                        ,public starcontainer::XEnumerationAccess
                        ,public starscript::XEventAttacherManager
                        ,public starbeans::XPropertyChangeListener
                        ,public stario::XPersistObject
{
protected:
    OInterfaceArray                         m_aItems;
    OInterfaceMap                           m_aMap;
    ::cppu::OInterfaceContainerHelper       m_aContainerListeners;

    ::osl::Mutex&           m_rMutex;
    staruno::Type           m_aElementType;

    staruno::Reference<starlang::XMultiServiceFactory>  m_xServiceFactory;


    // EventManager
    staruno::Reference<starscript::XEventAttacherManager>   m_xEventAttacher;

public:
    OInterfaceContainer(
        const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory,
        ::osl::Mutex& _rMutex,
        const staruno::Type& _rElementType);

public:
    virtual staruno::Any SAL_CALL queryInterface(const staruno::Type& _rType) throw (staruno::RuntimeException);

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw(staruno::RuntimeException) = 0;
    virtual void SAL_CALL write( const staruno::Reference< stario::XObjectOutputStream >& OutStream ) throw(stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL read( const staruno::Reference< stario::XObjectInputStream >& InStream ) throw(stario::IOException, staruno::RuntimeException);

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

// starbeans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const starbeans::PropertyChangeEvent& evt);

// starcontainer::XElementAccess
    virtual staruno::Type SAL_CALL getElementType() throw(staruno::RuntimeException) ;
    virtual sal_Bool SAL_CALL hasElements() throw(staruno::RuntimeException);

// starcontainer::XEnumerationAccess
    virtual staruno::Reference<starcontainer::XEnumeration> SAL_CALL createEnumeration() throw(staruno::RuntimeException);

// starcontainer::XNameAccess
    virtual staruno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual StringSequence SAL_CALL getElementNames(  ) throw(staruno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(staruno::RuntimeException);

// starcontainer::XNameReplace
    virtual void SAL_CALL replaceByName(const ::rtl::OUString& Name, const staruno::Any& _rElement) throw(starlang::IllegalArgumentException, starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

// starcontainer::XNameContainer
    virtual void SAL_CALL insertByName(const ::rtl::OUString& Name, const staruno::Any& _rElement) throw(starlang::IllegalArgumentException, starcontainer::ElementExistException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual void SAL_CALL removeByName(const ::rtl::OUString& Name) throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

// starcontainer::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL getByIndex(sal_Int32 _nIndex) throw(starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException);

// starcontainer::XIndexReplace
    virtual void SAL_CALL replaceByIndex(sal_Int32 _nIndex, const staruno::Any& _rElement) throw(starlang::IllegalArgumentException, starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException);

// starcontainer::XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 _nIndex, const staruno::Any& Element) throw(starlang::IllegalArgumentException, starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual void SAL_CALL removeByIndex(sal_Int32 _nIndex) throw(starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException);

// starcontainer::XContainer
    virtual void SAL_CALL addContainerListener(const staruno::Reference<starcontainer::XContainerListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeContainerListener(const staruno::Reference<starcontainer::XContainerListener>& _rxListener) throw(staruno::RuntimeException);

// starscript::XEventAttacherManager
    virtual void SAL_CALL registerScriptEvent( sal_Int32 nIndex, const starscript::ScriptEventDescriptor& aScriptEvent ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL registerScriptEvents( sal_Int32 nIndex, const staruno::Sequence< starscript::ScriptEventDescriptor >& aScriptEvents ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL revokeScriptEvent( sal_Int32 nIndex, const ::rtl::OUString& aListenerType, const ::rtl::OUString& aEventMethod, const ::rtl::OUString& aRemoveListenerParam ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL revokeScriptEvents( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL insertEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL removeEntry( sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual staruno::Sequence< starscript::ScriptEventDescriptor > SAL_CALL getScriptEvents( sal_Int32 Index ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL attach( sal_Int32 nIndex, const staruno::Reference< staruno::XInterface >& xObject, const staruno::Any& aHelper ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ServiceNotRegisteredException, staruno::RuntimeException);
    virtual void SAL_CALL detach( sal_Int32 nIndex, const staruno::Reference< staruno::XInterface >& xObject ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL addScriptListener( const staruno::Reference< starscript::XScriptListener >& xListener ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);
    virtual void SAL_CALL removeScriptListener( const staruno::Reference< starscript::XScriptListener >& Listener ) throw(::com::sun::star::lang::IllegalArgumentException, staruno::RuntimeException);

    virtual staruno::Sequence< staruno::Type > SAL_CALL getTypes(  ) throw(staruno::RuntimeException);

protected:
    // helper
    virtual void SAL_CALL disposing();
    virtual void insert(sal_Int32 _nIndex, const InterfaceRef& _Object, sal_Bool bEvents = sal_True)
                    throw(starlang::IllegalArgumentException);
    virtual void removeElementsNoEvents(sal_Int32 nIndex);

    // called after the object is inserted, but before the "real listeners" are notified
    virtual void implInserted(const InterfaceRef& _rxObject) { }
    // called after the object is removed, but before the "real listeners" are notified
    virtual void implRemoved(const InterfaceRef& _rxObject) { }

    void SAL_CALL writeEvents(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    void SAL_CALL readEvents(const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int32 nCount);
};

//==================================================================
//= OFormComponents
//==================================================================
typedef ::cppu::OComponentHelper FormComponentsBase;
    // else MSVC kills itself on some statements
class OFormComponents   : public FormComponentsBase,
                          public OInterfaceContainer,
                          public starform::XFormComponent
{
protected:
    ::osl::Mutex            m_aMutex;
    ::utl::InterfaceRef     m_xParent;

public:
    OFormComponents(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormComponents();

    DECLARE_UNO3_AGG_DEFAULTS(OFormComponents, FormComponentsBase);

    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);
    virtual staruno::Sequence< staruno::Type > SAL_CALL getTypes(  ) throw(staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// starform::XFormComponent
    virtual ::utl::InterfaceRef SAL_CALL getParent() throw(staruno::RuntimeException);
    virtual void SAL_CALL setParent(const ::utl::InterfaceRef& Parent) throw(starlang::NoSupportException, staruno::RuntimeException);
};
//.........................................................................
}   // namespace frm
//.........................................................................

#endif          // _FRM_INTERFACE_CONTAINER_HXX_

