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



#ifndef EXTENSIONS_DEFAULTHELPPROVIDER_HXX
#define EXTENSIONS_DEFAULTHELPPROVIDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XPropertyControlObserver.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>
#include <comphelper/componentcontext.hxx>

class Window;

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= DefaultHelpProvider
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::inspection::XPropertyControlObserver
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   DefaultHelpProvider_Base;
    class DefaultHelpProvider : public DefaultHelpProvider_Base
    {
    private:
        ::comphelper::ComponentContext  m_aContext;
        bool                            m_bConstructed;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >
                                        m_xInspectorUI;

    public:
        DefaultHelpProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        ~DefaultHelpProvider();

        // XPropertyControlObserver
        virtual void SAL_CALL focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    protected:
        // Service constructors
        void    create( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxUI );

    private:
        Window* impl_getVclControlWindow_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
        ::rtl::OUString impl_getHelpText_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_DEFAULTHELPPROVIDER_HXX
