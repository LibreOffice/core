/*************************************************************************
 *
 *  $RCSfile: documentcontainer.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:10:25 $
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

#ifndef _DBA_COREDATAACCESS_DOCUMENTCONTAINER_HXX_
#define _DBA_COREDATAACCESS_DOCUMENTCONTAINER_HXX_

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include <comphelper/propertystatecontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................
typedef ::cppu::ImplHelper4<        ::com::sun::star::frame::XComponentLoader
                                ,   ::com::sun::star::lang::XMultiServiceFactory
                                ,   ::com::sun::star::container::XHierarchicalNameContainer
                                ,   ::com::sun::star::embed::XTransactedObject
                                >   ODocumentContainer_Base;
//==========================================================================
//= ODocumentContainer -    collections of database documents (reports/forms)
//==========================================================================
class ODocumentContainer    : public ODefinitionContainer
                            , public ODocumentContainer_Base
                            , public ::comphelper::OPropertyStateContainer
                            , public ::comphelper::OPropertyArrayUsageHelper< ODocumentContainer >
{
    DECLARE_STL_USTRINGACCESS_MAP(bool, PersistentNames);
    PersistentNames m_aObjectList;
    sal_Bool        m_bFormsContainer;
public:
    /** constructs the container.<BR>
    */
    ODocumentContainer(
          const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  _xParentContainer
        ,const TContentPtr& _pImpl
        , sal_Bool _bFormsContainer
        );

    // ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )
    // com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );
    // ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

    // XComponentLoader
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL loadComponentFromURL( const ::rtl::OUString& URL, const ::rtl::OUString& TargetFrameName, sal_Int32 SearchFlags, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XCommandProcessor
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException) ;

    // XHierarchicalNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByHierarchicalName( const ::rtl::OUString& _sName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const ::rtl::OUString& _sName ) throw (::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameContainer
    virtual void SAL_CALL insertByHierarchicalName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByHierarchicalName( const ::rtl::OUString& Name ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameReplace
    virtual void SAL_CALL replaceByHierarchicalName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // XTransactedObject
    virtual void SAL_CALL commit(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revert(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // helper
    ::rtl::Reference<OContentHelper> getContent(const ::rtl::OUString& _sName) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage() const;

protected:
    virtual ~ODocumentContainer();
// ODefinitionContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(
        const ::rtl::OUString& _rName
    );

    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
    /** approve that the object given may be inserted into the container. Should be overloaded by derived classes,
        the default implementation just checks the object to be non-void.
    */
    virtual sal_Bool approveNewObject(const ::rtl::OUString& _sName,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxObject) const;
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DOCUMENTCONTAINER_HXX_

