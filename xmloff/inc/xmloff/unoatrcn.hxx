/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

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

