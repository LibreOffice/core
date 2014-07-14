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



// ----------------
// - SimpleResMgr -
// ----------------

// a simple resource manager : no stacks, no sharing of the impl class, only loading of strings and blobs
// but thread-safety !! :)

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#define _TOOLS_SIMPLERESMGR_HXX_

#include <vos/mutex.hxx>
#include <tools/resid.hxx>
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "tools/toolsdllapi.h"

class InternalResMgr;
class TOOLS_DLLPUBLIC SimpleResMgr
{
protected:
    vos::OMutex m_aAccessSafety;
    InternalResMgr*         m_pResImpl;

public:
                            /** creates a new SimpleResManager
                                @param pPrefixName
                                    denotes the prefix of the resource file name, in ThreadTextEncoding
                                @param rLocale
                                    denotes the locale of the resource file to load. If empty, a default locale
                                    will be used.
                            */
                            SimpleResMgr( const sal_Char* pPrefixName,
                                          const ::com::sun::star::lang::Locale& _rLocale);

                            /** creates a new SimpleResManager
                                @param _rPrefixName
                                    denotes the prefix of the resource file name
                                @param _inout_Locale
                                    denotes the locale of the resource file to load. If empty, no default handling
                                    (like in the other constructor) will happen, instead an unlocalized version will be
                                    attempted to be loaded.
                                    Upon return, the variable will contain the actual locale of the loaded resource file.
                                    For instance, if "en-US" is requested, but only "en" exists, the latter will be loaded
                                    and returned. Furthermore, if an unlocalized resource file with only the base name exists,
                                    this one will be loaded as final fallback.
                            */
                            SimpleResMgr( const ::rtl::OUString& _rPrefixName, ::com::sun::star::lang::Locale& _inout_Locale );
    virtual                 ~SimpleResMgr();

    static SimpleResMgr*    Create( const sal_Char* pPrefixName,
                                      ::com::sun::star::lang::Locale aLocale = ::com::sun::star::lang::Locale( rtl::OUString(),
                                                                                                               rtl::OUString(),
                                                                                                               rtl::OUString()));// nur in VCL

    bool                    IsValid() const { return m_pResImpl != NULL; }

    /** retrieves the locale of the resource file represented by this instance
        @precond
            IsValid returns <TRUE/>
    */
    const ::com::sun::star::lang::Locale&
                            GetLocale() const;

    /** reads the string with the given resource id
        @param  nId
            the resource id of the string to read
        @return
            the string with the given resource id, or an empty string if the id does not denote
            an existent string
        @seealso IsAvailable
    */
    UniString               ReadString( sal_uInt32 nId );

    /** checks whether a certain resource is available
        @param  _resourceType
            the type of the resource to check. Currently, only RSC_STRING (strings) and RSC_RESOURCE (blobs)
            are supported, for every other type, <FALSE/> will be returned.
        @param  _resourceId
            the id of the resource to lookup.
        @return
            <TRUE/> if and only if a resource of the given type, with the given id, is available.
    */
    bool                    IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId );

    sal_uInt32              ReadBlob( sal_uInt32 nId, void** pBuffer );
    void                    FreeBlob( void* pBuffer );
};

#endif // _TOOLS_SIMPLERESMGR_HXX_
