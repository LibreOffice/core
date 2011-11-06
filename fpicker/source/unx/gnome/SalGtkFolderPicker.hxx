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



#ifndef _SALGTKFOLDERPICKER_HXX_
#define _SALGTKFOLDERPICKER_HXX_

//_______________________________________________________________________________________________________________________
//  includes of other projects
//_______________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _SALGTKPICKER_HXX_
#include "SalGtkPicker.hxx"
#endif

#include <memory>

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.hxx>
#endif

#include <list>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalGtkFolderPicker :
        public SalGtkPicker,
    public cppu::WeakImplHelper3<
    ::com::sun::star::ui::dialogs::XFolderPicker,
    ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::util::XCancellable >
{
    public:

        // constructor
        SalGtkFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

        //------------------------------------------------------------------------------------
        // XExecutableDialog functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int16 SAL_CALL execute(  )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XFolderPicker functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDirectory( )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDescription( const rtl::OUString& rDescription )
            throw( com::sun::star::uno::RuntimeException );


        //------------------------------------------------
        // XServiceInfo
        //------------------------------------------------

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        //------------------------------------------------
        // XCancellable
        //------------------------------------------------

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------
        // XEventListener
        //------------------------------------------------

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::uno::RuntimeException);

    private:
        // prevent copy and assignment
        SalGtkFolderPicker( const SalGtkFolderPicker& );
        SalGtkFolderPicker& operator=( const SalGtkFolderPicker& );
    private:
        // to instanciate own services
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;

#if 0
    public:
         virtual ~SalGtkFolderPicker();
#endif
};

#endif // _SALGTKFOLDERPICKER_HXX_
