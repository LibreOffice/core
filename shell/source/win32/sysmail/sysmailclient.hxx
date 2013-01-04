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

#ifndef _SYSMAILCLIENT_HXX_
#define _SYSMAILCLIENT_HXX_

#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/system/XMailClient.hpp>

namespace css = ::com::sun::star;

namespace shell
{
    typedef cppu::WeakImplHelper1<css::system::XMailClient> WinSysMailClient_Base;

    class WinSysMailClient : public WinSysMailClient_Base
    {
    public:
        virtual css::uno::Reference<css::system::XMailMessage> SAL_CALL createMailMessage()
            throw (css::uno::RuntimeException);

        virtual void SAL_CALL sendMailMessage(const css::uno::Reference<css::system::XMailMessage>& xMailMessage, sal_Int32 aFlag)
            throw (css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException);

    private:
        void validateParameter(const css::uno::Reference<css::system::XMailMessage>& xMailMessage, sal_Int32 aFlag);
        void assembleCommandLine(const css::uno::Reference<css::system::XMailMessage>& xMailMessage, sal_Int32 aFlag, std::vector<rtl::OUString>& rCommandArgs);
    };
}

#endif
