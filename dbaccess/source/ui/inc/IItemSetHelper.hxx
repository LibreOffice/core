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



#ifndef DBAUI_ITEMSETHELPER_HXX
#define DBAUI_ITEMSETHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(sdbc,XDriver)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

class SfxItemSet;
namespace dbaui
{
    class SAL_NO_VTABLE IItemSetHelper
    {
    public:
        virtual const SfxItemSet* getOutputSet() const = 0;
        virtual SfxItemSet* getWriteOutputSet() = 0;
    };

    class SAL_NO_VTABLE IDatabaseSettingsDialog
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const = 0;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver() = 0;
        virtual ::rtl::OUString getDatasourceType(const SfxItemSet& _rSet) const = 0;
        virtual void clearPassword() = 0;
        virtual sal_Bool saveDatasource() = 0;
        virtual void setTitle(const ::rtl::OUString& _sTitle) = 0;

        /** enables or disables the user's possibility to confirm the settings

            In a wizard, disabling this will usually disable the "Finish" button.
            In a normal tab dialog, this will usually disable the "OK" button.
        */
        virtual void enableConfirmSettings( bool _bEnable ) = 0;
    };
}
#endif // DBAUI_ITEMSETHELPER_HXX

