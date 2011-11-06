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



#ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          a helper to merge dispatch informations of different sources together.
*//*-*************************************************************************************************************/
class DispatchInformationProvider : public  css::frame::XDispatchInformationProvider
                                  , private ThreadHelpBase
                                  , public  ::cppu::OWeakObject
{
    //_______________________
    // member
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //_______________________
    // interface
    public:

        DispatchInformationProvider(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame);

        virtual ~DispatchInformationProvider();

        FWK_DECLARE_XINTERFACE

        virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups()
            throw (css::uno::RuntimeException);

        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
            throw (css::uno::RuntimeException);

    //_______________________
    // helper
    private:

        css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > implts_getAllSubProvider();

}; // class DispatchInformationProvider

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_
