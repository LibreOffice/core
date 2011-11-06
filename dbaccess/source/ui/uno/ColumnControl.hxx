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


#ifndef DBAUI_COLUMNCONTROL_HXX
#define DBAUI_COLUMNCONTROL_HXX

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

namespace dbaui
{
    class OColumnControl :  public UnoControl
    {
    public:
        OColumnControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

        // UnoControl
        virtual ::rtl::OUString GetComponentServiceName();

        // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

        // ::com::sun::star::awt::XControl
        virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException);
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_COLUMNCONTROL_HXX
