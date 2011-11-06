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



#ifndef _SFX_SFXURLRELOCATOR_HXX_
#define _SFX_SFXURLRELOCATOR_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

class SfxURLRelocator_Impl
{
    ::osl::Mutex maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >           mxFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XOfficeInstallationDirectories > mxOfficeInstDirs;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >                 mxMacroExpander;

public:
    static bool                 propertyCanContainOfficeDir( const rtl::OUString & rPropName );
    void                        initOfficeInstDirs();
    void                        makeRelocatableURL( rtl::OUString & rURL );
    void                        makeAbsoluteURL( rtl::OUString & rURL );

    SfxURLRelocator_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );
    ~SfxURLRelocator_Impl();

private:
    void implExpandURL( ::rtl::OUString& io_url );
};

#endif

