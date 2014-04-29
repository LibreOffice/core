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



#ifndef __FRAMEWORK_SERVICES_LICENSE_HXX_
#define __FRAMEWORK_SERVICES_LICENSE_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <properties.h>
#include <stdtypes.h>
#include <uielement/menubarmanager.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

namespace framework
{
    class License : public  css::lang::XTypeProvider            ,
                    public  css::lang::XServiceInfo             ,
                    public  css::task::XJob                     ,
                    public  css::util::XCloseable               ,
                    // base classes
                    // Order is necessary for right initialization!
                    private ThreadHelpBase                      , // Struct for right initalization of mutex member! Must be first of baseclasses.
                    public  ::cppu::OWeakObject                   // => XWeak, XInterface
    {
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        sal_Bool m_bTerminate;
    public:
        License( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMGR );
        virtual ~License();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

#if 0
        DECL_STATIC_LINK( License, Terminate, void* );
#endif

        /*
        XJob...
        any execute([in] sequence< ::com::sun::star::beans::NamedValue > Arguments )
            raises(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::Exception );
        */
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence<css::beans::NamedValue>& args)
            throw( css::lang::IllegalArgumentException, css::uno::Exception);

        /*
        XCLoseable
        void close( [in] boolean DeliverOwnership ) raises(CloseVetoException );
        */
        virtual void SAL_CALL close(sal_Bool bDeliverOwnership) throw (css::util::CloseVetoException);

        /*
        XCloseVroadcaster
        [oneway] void addCloseListener([in] XCloseListenerListener );
        [oneway] void removeCloseListener([in] XCloseListenerListener );
        */
        virtual void SAL_CALL addCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);
        virtual void SAL_CALL removeCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);

};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
