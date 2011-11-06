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



#ifndef EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX
#define EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


class Window;
//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= OAdminDialogInvokation
    //=====================================================================
    /** outsourced from AdminDialogInvokationPage, 'cause this class here, in opposite to
        the page, needs exception handlíng to be enabled.
    */
    class OAdminDialogInvokation
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xDataSource;
        Window*         m_pMessageParent;

    public:
        OAdminDialogInvokation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _xDataSource,
            Window* _pMessageParent
        );

        sal_Bool invokeAdministration( sal_Bool _bFixedType );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX

