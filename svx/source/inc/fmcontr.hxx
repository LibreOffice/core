/*************************************************************************
 *
 *  $RCSfile: fmcontr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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
#ifndef _SVX_FMCONTR_HXX
#define _SVX_FMCONTR_HXX

#include <unotools/stl_types.hxx>

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

typedef ::std::hash_multimap< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > , ::utl::UStringHash, ::utl::UStringEqual> OInterfaceMap;

DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > ,InterfaceArray);

class ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > ;
class ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > ;

//==================================================================
// FmForms
// Implementiert den UNO-::com::sun::star::sdbcx::Container fuer Formulare
// enthaelt alle zugeordneten ::com::sun::star::form::Forms
// dieses ::com::sun::star::sdbcx::Container kann selbst den Context fuer Formulare darstellen
// oder außen einen Context uebergeben bekommen
//==================================================================
class FmXInterfaceContainer : public ::com::sun::star::container::XNameContainer,
                              public ::com::sun::star::container::XIndexContainer,
                              public ::com::sun::star::container::XContainer,
                              public ::com::sun::star::container::XEnumerationAccess,
                              public ::com::sun::star::script::XEventAttacherManager,
                              public ::com::sun::star::beans::XPropertyChangeListener
{
protected:
    OInterfaceMap               m_aMap;
    OInterfaceArray             m_aItems;
    ::usr::OInterfaceContainerHelper
                                m_aContainerListeners;

    InterfaceReflection*        m_pElementReflection;
    ::VOS::IMutex&      m_rMutex;
    ::com::sun::star::uno::Uik                          m_aElementUik;


    // EventManager
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >     m_xEventAttacher;

public:
    FmXInterfaceContainer(::VOS::IMutex& _rMutex, InterfaceReflection* pElementReflection, const ::com::sun::star::uno::Uik& rUik );

public:
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  virtual sal_Bool queryInterface( ::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  & rOut );


// ::com::sun::star::io::XPersistObject
    void write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream);
    void read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > & InStream);

// ::com::sun::star::lang::XEventListener
    virtual void disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getElementType(void) const throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool hasElements(void) const throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any getByName(const ::rtl::OUString& Name) const throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > getElementNames(void) const throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool hasByName(const ::rtl::OUString& Name) const throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XNameReplace
    virtual void replaceByName(const ::rtl::OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XNameContainer
    virtual void insertByName(const ::rtl::OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void removeByName(const ::rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 getCount(void) const throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any getByIndex(sal_Int32 ::com::sun::star::sdbcx::Index) const throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XIndexReplace
    virtual void replaceByIndex(sal_Int32 ::com::sun::star::sdbcx::Index, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XIndexContainer
    virtual void insertByIndex(sal_Int32 ::com::sun::star::sdbcx::Index, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void removeByIndex(sal_Int32 ::com::sun::star::sdbcx::Index) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XContainer
    virtual void addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > & l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > & l) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::script::XEventAttacherManager
    virtual void registerScriptEvent(sal_Int32 nIndex, const ::com::sun::star::script::ScriptEventDescriptor& ::com::sun::star::script::ScriptEvent);
    virtual void registerScriptEvents(sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& ScriptEvents);
    virtual void revokeScriptEvent(sal_Int32 nIndex, const ::rtl::OUString& ListenerType, const ::rtl::OUString& EventMethod, const ::rtl::OUString& rRemoveListenerParam);
    virtual void revokeScriptEvents(sal_Int32 nIndex);
    virtual void insertEntry(sal_Int32 nIndex);
    virtual void removeEntry(sal_Int32 nIndex);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor > getScriptEvents(sal_Int32 nIndex);
    virtual void attach(sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Object, const ::com::sun::star::uno::Any& Helper);
    virtual void detach(sal_Int32 nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xObject);
    virtual void addScriptListener(const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > & aListener);
    virtual void removeScriptListener(const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > & aListener);

    static ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getStaticIdlClass();

protected:
    // helper
    virtual void disposing();
    virtual void insert(sal_Int32 ::com::sun::star::sdbcx::Index, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _Object, sal_Bool bEvents = sal_True)
                    throw( ::com::sun::star::lang::IllegalArgumentException );
    virtual void removeElementsNoEvents(sal_Int32 nIndex);

    // called after the object is inserted, but before the "real listeners" are notified
    virtual void implInserted(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _rxObject) { }
    // called after the object is removed, but before the "real listeners" are notified
    virtual void implRemoved(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _rxObject) { }

    void writeEvents(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream);
    void readEvents(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > & InStream, sal_Int32 nCount);
};

//==================================================================
// FmForms
// Implementiert den UNO-::com::sun::star::sdbcx::Container fuer Formulare
// enthaelt alle zugeordneten ::com::sun::star::form::Forms
// dieses ::com::sun::star::sdbcx::Container kann selbst den Context fuer Formulare darstellen
// oder außen einen Context uebergeben bekommen
//==================================================================
class FmXForms : public ::usr::OComponentHelper,
                 public FmXInterfaceContainer,
                 public ::com::sun::star::container::XChild

{
    friend Reflection* ::getCppuType((const FmXForms*)0);
    ::osl::Mutex        m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xParent;          // Parent

public:
    FmXForms();
    virtual ~FmXForms();

public:
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  virtual sal_Bool queryInterface( ::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  & rOut )

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//          { return OComponentHelper::queryInterface( aUik, rOut ); }

    virtual sal_Bool queryAggregation(::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rOut);
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > >  getIdlClasses(void);

// automatisch auskommentiert - [getImplementation] - Wird von OWeakObject nicht weiter unterstützt!
//  virtual void* getImplementation(Reflection * pRef);

    virtual void acquire() { OComponentHelper::acquire(); }
    virtual void release() { OComponentHelper::release(); }

// OComponentHelper
    virtual void disposing();

// ::com::sun::star::container::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  getParent(void) const throw( ::com::sun::star::uno::RuntimeException );
    virtual void setParent(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Parent) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException );
};

//==================================================================
// FmForm
// Implementiert den UNO-::com::sun::star::sdbcx::Container fuer FormularControls
//==================================================================
class FmXFormComponents : public ::usr::OComponentHelper,
                          public FmXInterfaceContainer,
                          public ::com::sun::star::form::XFormComponent
{
protected:
    ::osl::Mutex        m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xParent;          // Parent

public:
    FmXFormComponents();
    virtual ~FmXFormComponents();

    SMART_UNO_DECLARATION(FmXFormComponents, OComponentHelper);

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  virtual sal_Bool queryInterface( ::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  & rOut )

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//          { return OComponentHelper::queryInterface( aUik, rOut ); }

    virtual sal_Bool queryAggregation(::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rOut);
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > >  getIdlClasses(void);


// OComponentHelper
    virtual void disposing();

// ::com::sun::star::form::XFormComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  getParent(void) const throw( ::com::sun::star::uno::RuntimeException );
    virtual void setParent(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Parent) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getStaticIdlClass();
};


#endif          // _SVX_FMCONTR_HXX

