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


#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#define _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX

#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/container/XEnumeration.hpp>
#ifndef _COM_SUN_STAR_LANG_XPSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _HASH_MAPS_HXX
#include <HashMaps.hxx>
#endif

class ZipPackageFolderEnumeration : public cppu::WeakImplHelper2
<
    com::sun::star::container::XEnumeration,
    com::sun::star::lang::XServiceInfo
>
{
protected:
    ContentHash& rContents;
    ContentHash::const_iterator aIterator;
public:
    //ZipPackageFolderEnumeration (std::hash_map < rtl::OUString, com::sun::star::uno::Reference < com::sun::star::container::XNamed >, hashFunc, eqFunc > &rInput);
    ZipPackageFolderEnumeration (ContentHash &rInput);
    virtual ~ZipPackageFolderEnumeration( void );

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

};
#endif
