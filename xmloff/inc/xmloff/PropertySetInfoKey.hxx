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


#ifndef _XMLOFF_PROPERTYSETINFOKEY_HXX
#define _XMLOFF_PROPERTYSETINFOKEY_HXX

#include <osl/diagnose.h>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySetInfo; }
} } }


struct PropertySetInfoKey
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySetInfo > xPropInfo;
    ::com::sun::star::uno::Sequence < sal_Int8 > aImplementationId;

    inline PropertySetInfoKey();
    inline PropertySetInfoKey(
            const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySetInfo >& rPropInfo,
            const ::com::sun::star::uno::Sequence < sal_Int8 >& rImplId );
};

inline PropertySetInfoKey::PropertySetInfoKey()
{
    OSL_ENSURE( aImplementationId.getLength()==16, "illegal constructor call" );
}

inline PropertySetInfoKey::PropertySetInfoKey(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo >& rPropInfo,
        const ::com::sun::star::uno::Sequence < sal_Int8 >& rImplId ) :
    xPropInfo( rPropInfo ),
    aImplementationId( rImplId )
{
    OSL_ENSURE( rPropInfo.is(), "prop info missing" );
    OSL_ENSURE( aImplementationId.getLength()==16, "invalid implementation id" );
}

#endif
