/*************************************************************************
 *
 *  $RCSfile: definitioncontainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:14:23 $
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

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#define _DBA_CORE_DEFINITIONCONTAINER_HXX_

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

class ODefinitionContainer_Impl : public OContentHelper_Impl
{
public:
    DECLARE_STL_USTRINGACCESS_MAP(TContentPtr, Documents);
    //  DECLARE_STL_VECTOR(Documents::iterator, DocumentsIndexAccess);

    //  DocumentsIndexAccess    m_aDocuments;               // for a efficient index access
    Documents               m_aDocumentMap;             // for a efficient name access
};

//==========================================================================
//= ODefinitionContainer -  base class of collections of database definition
//=                         documents
//==========================================================================
typedef ::cppu::ImplHelper6<        ::com::sun::star::container::XIndexAccess
                                ,   ::com::sun::star::container::XNameContainer
                                ,   ::com::sun::star::container::XEnumerationAccess
                                ,   ::com::sun::star::container::XContainer
                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                ,   ::com::sun::star::beans::XVetoableChangeListener
                                >   ODefinitionContainer_Base;

class ODefinitionContainer
            :public OContentHelper
            ,public ODefinitionContainer_Base
{
public:
    DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::WeakReference< ::com::sun::star::ucb::XContent >, Documents);
    DECLARE_STL_VECTOR(Documents::iterator, DocumentsIndexAccess);

    enum ENotifyKind
    {
        E_REPLACED,
        E_REMOVED,
        E_INSERTED
    };
protected:
    // we can't just hold a vector of XContentRefs, as after initialization they're all empty
    // cause we load them only on access
    DocumentsIndexAccess    m_aDocuments;               // for a efficient index access
    Documents               m_aDocumentMap;             // for a efficient name access

    ::cppu::OInterfaceContainerHelper
                            m_aContainerListeners;
    sal_Bool                m_bInPropertyChange;

    /** looks like the dtor ...
    */
    virtual ~ODefinitionContainer();
public:
    /** constructs the container.<BR>
        after the construction of the object the creator has to call <code>initialize</code>.
    */
    ODefinitionContainer(
          const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  _xParentContainer
        ,const TContentPtr& _pImpl
        );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )
// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 _nIndex ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& _rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);
    // XVetoableChangeListener
    virtual void SAL_CALL vetoableChange( const ::com::sun::star::beans::PropertyChangeEvent& aEvent ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);
protected:
    // helper
    virtual void SAL_CALL disposing();

    /** approve that the object given may be inserted into the container. Should be overloaded by derived classes,
        the default implementation just checks the object to be non-void.
    */
    virtual sal_Bool approveNewObject(const ::rtl::OUString& _sName,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxObject) const;

    /** create a object from it's persistent data within the configuration. To be overwritten by derived classes.
        @param      _rName          the name the object has within the container
        @return                     the newly created object or an empty reference if somthing went wrong
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(
        const ::rtl::OUString& _rName) = 0;

    /** checks whether or not the configuration data the object is based on is readonly
    */
    sal_Bool    isReadOnly() const;

    /** get the object specified by the given name. If desired, the object will be read if not already done so.<BR>
        @param      _rName              the object name
        @param      _bReadIfNeccessary  if sal_True, the object will be read from the configuration if not already done so
        @return                         the property set interface of the object. Usually the return value is not NULL, but
                                        if so, then the object could not be read from the configuration
        @throws                         NoSuchElementException if there is no object with the given name.
        @see    createObject
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >
                implGetByName(const ::rtl::OUString& _rName, sal_Bool _bReadIfNeccessary = sal_True) throw (::com::sun::star::container::NoSuchElementException);

    /** quickly checks if there already is an element with a given name. No access to the configuration occures, i.e.
        if there is such an object which is not already loaded, it won't be loaded now.
        @param      _rName      the object name to check
        @return                 sal_True if there already exists such an object
    */
    virtual sal_Bool checkExistence(const ::rtl::OUString& _rName);

    /** get
    */

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
        const ::rtl::OUString& _rName,
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
    void implRemove(const ::rtl::OUString& _rName);

    /** remove a object in the container. No plausibility checks are done, e.g. whether
        or not there exists an object with the given name or the object is non-NULL. This is the responsibility of the caller.<BR>
        Additionally all object-related informations within the registry will be deleted. The new object config node,
        where the caller may want to store the new objects information, is returned.<BR>
        The old component will not be disposed, this is the callers responsibility, too.
        @param          _rName              the objects name
        @param          _rxNewObject        the new object
        @param          _rNewObjectNode     the configuration node where the new object may be stored
        @see            implAppend
        @see            implRemove
    */
    void implReplace(
        const ::rtl::OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxNewObject
        );

    void notifyByName( ::osl::ClearableMutexGuard& _rGuard
                    ,const ::rtl::OUString& _rName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewElement
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& xOldElement
                    ,ENotifyKind _eHowToNotify);
private:

    void    implInsert(const ::rtl::OUString& _rName, const ::com::sun::star::uno::Any& aElement);
    void    addObjectListener(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewObject);
    void    removeObjectListener(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xNewObject);
};
//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_DEFINITIONCONTAINER_HXX_

