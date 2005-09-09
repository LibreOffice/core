/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoatrcn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:11:29 $
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

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SvUnoAttributeContainer_CreateInstance();

class SvXMLAttrContainerData;

class XMLOFF_DLLPUBLIC SvUnoAttributeContainer:
    public ::cppu::WeakAggImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XUnoTunnel,
        ::com::sun::star::container::XNameContainer >
{
private:
    SvXMLAttrContainerData* mpContainer;

    SAL_DLLPRIVATE sal_uInt16 getIndexByName(const ::rtl::OUString& aName )
        const;

public:
    SvUnoAttributeContainer( SvXMLAttrContainerData* pContainer = NULL );
    virtual ~SvUnoAttributeContainer();

    SvXMLAttrContainerData* GetContainerImpl() const { return mpContainer; }

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    static SvUnoAttributeContainer* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt ) throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type  SAL_CALL getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString& aName) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName(const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName(const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByName(const ::rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );

    friend  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SvUnoAttributeContainer_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr ) throw( ::com::sun::star::uno::Exception );
};

