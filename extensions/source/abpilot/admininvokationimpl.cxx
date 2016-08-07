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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "admininvokationimpl.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/stdtext.hxx>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#include "componentmodule.hxx"
#include <vcl/waitobj.hxx>


//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::ui::dialogs;

    //=====================================================================
    //= OAdminDialogInvokation
    //=====================================================================
    //---------------------------------------------------------------------
    OAdminDialogInvokation::OAdminDialogInvokation(const Reference< XMultiServiceFactory >& _rxORB
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _xDataSource
                    , Window* _pMessageParent)
        :m_xORB(_rxORB)
        ,m_xDataSource(_xDataSource)
        ,m_pMessageParent(_pMessageParent)
    {
        DBG_ASSERT(m_xORB.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid service factory!");
        DBG_ASSERT(m_xDataSource.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid preferred name!");
        DBG_ASSERT(m_pMessageParent, "OAdminDialogInvokation::OAdminDialogInvokation: invalid message parent!");
    }

    //---------------------------------------------------------------------
    sal_Bool OAdminDialogInvokation::invokeAdministration( sal_Bool _bFixedType )
    {
        if (!m_xORB.is())
            return sal_False;

        try
        {
            // the service name of the administration dialog
            const static ::rtl::OUString s_sAdministrationServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatasourceAdministrationDialog"));
            const static ::rtl::OUString s_sDataSourceTypeChangeDialog = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DataSourceTypeChangeDialog"));

            // the parameters for the call
            Sequence< Any > aArguments(3);
            Any* pArguments = aArguments.getArray();

            // the parent window
            Reference< XWindow > xDialogParent = VCLUnoHelper::GetInterface(m_pMessageParent);
            *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii("ParentWindow"), -1, makeAny(xDialogParent), PropertyState_DIRECT_VALUE);

            // the title of the dialog
            String sAdminDialogTitle(ModuleRes(RID_STR_ADMINDIALOGTITLE));
            *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii("Title"), -1, makeAny(::rtl::OUString(sAdminDialogTitle)), PropertyState_DIRECT_VALUE);

            // the name of the new data source
            *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii("InitialSelection"), -1, makeAny(m_xDataSource), PropertyState_DIRECT_VALUE);

            // create the dialog
            Reference< XExecutableDialog > xDialog;
            {
                // creating the dialog service is potentially expensive (if all the libraries invoked need to be loaded)
                // so we display a wait cursor
                WaitObject aWaitCursor(m_pMessageParent);
                xDialog = Reference< XExecutableDialog >( m_xORB->createInstanceWithArguments( _bFixedType ? s_sAdministrationServiceName : s_sDataSourceTypeChangeDialog, aArguments ), UNO_QUERY );

                // just for a smoother UI: What the dialog does upon execution, is (amongst other things) creating
                // the DriverManager service
                // If this context has never been accessed before, this may be expensive (it includes loading of
                // at least one library).
                // As this wizard is intended to run on the first office start, it is very likely that the
                // context needs to be freshly created
                // Thus, we access the context here (within the WaitCursor), which means the user sees a waitcursor
                // while his/her office blocks a few seconds ....
                m_xORB->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.sdbc.DriverManager" ) );
            }

            if (xDialog.is())
            {   // execute it
                if (xDialog->execute())
                    return sal_True;
            }
            else
                ShowServiceNotAvailableError(m_pMessageParent, s_sAdministrationServiceName, sal_True);
        }
        catch(const Exception&)
        {
            DBG_ERROR("OAdminDialogInvokation::invokeAdministration: caught an exception while executing the dialog!");
        }
        return sal_False;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

