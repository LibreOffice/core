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



#ifndef _COMPHELPER_MASTERPROPERTYSETINFO_HXX_
#define _COMPHELPER_MASTERPROPERTYSETINFO_HXX_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <comphelper/PropertyInfoHash.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/TypeGeneration.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    class COMPHELPER_DLLPUBLIC MasterPropertySetInfo:
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::beans::XPropertySetInfo >
    {
        friend class MasterPropertySet;
    protected:
        PropertyDataHash maMap;
        com::sun::star::uno::Sequence < com::sun::star::beans::Property > maProperties;
    public:
        MasterPropertySetInfo()
            throw();
        MasterPropertySetInfo( PropertyInfo * pMap )
            throw();
        virtual ~MasterPropertySetInfo()
            throw();
        void add( PropertyInfo* pMap, sal_Int32 nCount = -1, sal_uInt8 nMapId = 0 )
            throw();
        void add( PropertyInfoHash &rHash, sal_uInt8 nMapId )
            throw();
        void remove( const rtl::OUString& aName )
            throw();

        // XPropertySetInfo
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties()
            throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif
