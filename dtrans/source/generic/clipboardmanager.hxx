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



#ifndef _DTRANS_CLIPBOARDMANAGER_HXX_
#define _DTRANS_CLIPBOARDMANAGER_HXX_

#include <cppuhelper/compbase3.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDMANAGER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

// ------------------------------------------------------------------------

#define CLIPBOARDMANAGER_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.ClipboardManager"

// ------------------------------------------------------------------------

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > > ClipboardMap;

// ------------------------------------------------------------------------

namespace dtrans
{

    class ClipboardManager : public ::cppu::WeakComponentImplHelper3 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardManager, \
    ::com::sun::star::lang::XEventListener, \
    ::com::sun::star::lang::XServiceInfo >
    {
        ClipboardMap m_aClipboardMap;
        ::osl::Mutex m_aMutex;

        const ::rtl::OUString m_aDefaultName;

        virtual ~ClipboardManager();
    protected:
        using WeakComponentImplHelperBase::disposing;
    public:

        ClipboardManager();

        /*
         * XServiceInfo
         */

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XComponent
         */

        virtual void SAL_CALL dispose()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XEventListener
         */

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardManager
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const ::rtl::OUString& aName )
            throw(::com::sun::star::container::NoSuchElementException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL addClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboard( const ::rtl::OUString& aName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL listClipboardNames(  )
            throw(::com::sun::star::uno::RuntimeException);


    };

}

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClipboardManager_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClipboardManager_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif
