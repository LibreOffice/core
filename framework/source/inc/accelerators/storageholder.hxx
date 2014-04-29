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



#ifndef __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_
#define __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

//===============================================
// own includes

#include <accelerators/istoragelistener.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

//===============================================
// interface includes

#ifndef __COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef __COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//===============================================
// other includes


//===============================================
// namespace

namespace framework
{

#ifdef css
    #error "Who defines css? I will use it as namespace alias inside header."
#else
    #define css ::com::sun::star
#endif

//===============================================
// definitions

//-----------------------------------------------
/**
    TODO document me
 */
class StorageHolder : private ThreadHelpBase // attention! Must be the first base class to guarantee right initialize lock ...
{
    //-------------------------------------------
    // types
    public:

        /** @short  TODO */
        typedef ::std::vector< css::uno::Reference< css::embed::XStorage > > TStorageList;

        typedef ::std::vector< IStorageListener* > TStorageListenerList;

        struct TStorageInfo
        {
            public:
                css::uno::Reference< css::embed::XStorage > Storage;
                sal_Int32 UseCount;
                TStorageListenerList Listener;

                TStorageInfo()
                    : UseCount(0)
                {}
        };

        /** @short  TODO */
        typedef ::std::hash_map< ::rtl::OUString                    ,
                                 TStorageInfo                       ,
                                 ::rtl::OUStringHash                ,
                                 ::std::equal_to< ::rtl::OUString > > TPath2StorageInfo;

    //-------------------------------------------
    // member
    private:

        /** @short  TODO */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  TODO */
        css::uno::Reference< css::embed::XStorage > m_xRoot;

        /** @short  TODO */
        TPath2StorageInfo m_lStorages;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  TODO
         */
        StorageHolder();

        //---------------------------------------
        /** @short  TODO
         */
        StorageHolder(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** @short  TODO
         */
        virtual ~StorageHolder();

        //---------------------------------------
        /** @short  TODO
         */
        virtual void forgetCachedStorages();

        //---------------------------------------
        /** @short  TODO
         */
        virtual void setRootStorage(const css::uno::Reference< css::embed::XStorage >& xRoot);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getRootStorage() const;

        //---------------------------------------
        /** @short  TODO
                    open or get!
         */
        virtual css::uno::Reference< css::embed::XStorage > openPath(const ::rtl::OUString& sPath    ,
                                                                           sal_Int32        nOpenMode);

        //---------------------------------------
        /** @short  TODO
         */
        virtual StorageHolder::TStorageList getAllPathStorages(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void commitPath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void closePath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void notifyPath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void addStorageListener(      IStorageListener* pListener,
                                        const ::rtl::OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual void removeStorageListener(      IStorageListener* pListener,
                                           const ::rtl::OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual ::rtl::OUString getPathOfStorage(const css::uno::Reference< css::embed::XStorage >& xStorage);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const css::uno::Reference< css::embed::XStorage >& xChild);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const ::rtl::OUString& sChildPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void operator=(const StorageHolder& rCopy);

        //---------------------------------------
        /** @short  opens a sub element of the specified base storage.

            @descr  First this method try to open the requested sub element
                    using the given open mode. If it failed there is second step,
                    which tries to do the same again ... but removing a might existing
                    WRITE flag from the open mode. The user can suppress this fallback
                    handling by setting the parameter bAllowFallback to sal_False.

            @param  xBaseStorage
                    the storage, where the sub element should be searched.

            @param  sSubElement
                    the full name of the sub element.
                    e.g. "default.xml"

            @param  eOpenMode
                    a flag field, which set the open mode for this operation.

            @param  bAllowFallback
                    if eOpenMode contains an ELEMENT_WRITE flag this parameter
                    allow to remove it and try it with the rest of eOpenMode flags
                    again.
         */
        static css::uno::Reference< css::embed::XStorage > openSubStorageWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                      const ::rtl::OUString&                             sSubStorage   ,
                                                                                            sal_Int32                                    eOpenMode     ,
                                                                                            sal_Bool                                     bAllowFallback);

        static css::uno::Reference< css::io::XStream > openSubStreamWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                 const ::rtl::OUString&                             sSubStream    ,
                                                                                       sal_Int32                                    eOpenMode     ,
                                                                                       sal_Bool                                     bAllowFallback);

        //---------------------------------------
        // helper
        private:

            //-----------------------------------
            /** @short  TODO
             */
            static ::rtl::OUString impl_st_normPath(const ::rtl::OUString& sPath);

            //-----------------------------------
            /** @short  TODO
             */
            static OUStringList impl_st_parsePath(const ::rtl::OUString& sPath);
};

#undef css // dont let it out!

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_
