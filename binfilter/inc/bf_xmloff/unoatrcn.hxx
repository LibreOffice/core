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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase3.hxx>
namespace binfilter {

extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SvUnoAttributeContainer_CreateInstance();

class SvXMLAttrContainerData;

class SvUnoAttributeContainer : public ::cppu::WeakAggImplHelper3<
                                            ::com::sun::star::lang::XServiceInfo,
                                            ::com::sun::star::lang::XUnoTunnel,
                                            ::com::sun::star::container::XNameContainer >
{
private:
    SvXMLAttrContainerData* mpContainer;

    sal_uInt16 getIndexByName(const ::rtl::OUString& aName ) const;

public:
    SvUnoAttributeContainer( SvXMLAttrContainerData* pContainer = NULL );
    virtual ~SvUnoAttributeContainer();

    SvXMLAttrContainerData* GetContainerImpl() const { return mpContainer; }

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
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

    friend	::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SvUnoAttributeContainer_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr ) throw( ::com::sun::star::uno::Exception );
};
}//end of namespace binfilter


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
