/*************************************************************************
 *
 *  $RCSfile: storageholder.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2004-12-07 13:18:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_
#define __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

//===============================================
// own includes

#ifndef __FRAMEWORK_ACCELERATORS_ISTORAGELISTENER_HXX_
#include <accelerators/istoragelistener.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

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
class StorageHolder : private ThreadHelpBase // attention! Must be the first base class to guarentee right initialize lock ...
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
        /** @short  TODO
         */
        static css::uno::Reference< css::embed::XStorage > createTempStorageBasedOnFolder(const ::rtl::OUString&                                        sFolder  ,
                                                                                          const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR    ,
                                                                                                sal_Bool                                                bReadOnly);

        //---------------------------------------
        /** @short  opens a sub element of the specified base storage.

            @descr  First this method try to open the requested sub element
                    using the given open mode. If it failed there is second step,
                    which tries to do the same again ... but removing a might existing
                    WRITE flag from the open mode. The user can supress this fallback
                    handling by setting the parameter bAllowFallback to FALSE.

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
