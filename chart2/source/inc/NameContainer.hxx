/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NameContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:19:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CHART2_NAMECONTAINER_HXX
#define _CHART2_NAMECONTAINER_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#include <map>

//.............................................................................
namespace chart
{
//.............................................................................

class NameContainer : public ::cppu::WeakImplHelper3<
          ::com::sun::star::container::XNameContainer
        , ::com::sun::star::lang::XServiceInfo
        , ::com::sun::star::util::XCloneable
        >
{
public:
    NameContainer( const ::com::sun::star::uno::Type& rType, const rtl::OUString& rServicename, const rtl::OUString& rImplementationName );
    explicit NameContainer( const NameContainer & rOther );
    virtual ~NameContainer();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  rtl::OUString& aName, const  com::sun::star::uno::Any& aElement ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::ElementExistException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  rtl::OUString& Name ) throw( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  rtl::OUString& aName, const  com::sun::star::uno::Any& aElement ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual com::sun::star::uno::Any SAL_CALL getByName( const  rtl::OUString& aName ) throw( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence<  rtl::OUString > SAL_CALL getElementNames(  ) throw( com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  rtl::OUString& aName ) throw( com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Type SAL_CALL getElementType(  ) throw( com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone() throw (::com::sun::star::uno::RuntimeException);

private: //methods
    NameContainer();//no default contructor

private: //member
    const ::com::sun::star::uno::Type   m_aType;
    const rtl::OUString                 m_aServicename;
    const rtl::OUString                 m_aImplementationName;

    typedef ::std::map< ::rtl::OUString, com::sun::star::uno::Any > tContentMap;

    tContentMap m_aMap;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
