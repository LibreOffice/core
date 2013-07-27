/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "admininvokationimpl.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <vcl/stdtext.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "abpresid.hrc"
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
    using namespace ::com::sun::star::sdbc;

    //=====================================================================
    //= OAdminDialogInvokation
    //=====================================================================
    //---------------------------------------------------------------------
    OAdminDialogInvokation::OAdminDialogInvokation(const Reference< XComponentContext >& _rxContext
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _xDataSource
                    , Window* _pMessageParent)
        :m_xContext(_rxContext)
        ,m_xDataSource(_xDataSource)
        ,m_pMessageParent(_pMessageParent)
    {
        DBG_ASSERT(m_xContext.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid service factory!");
        DBG_ASSERT(m_xDataSource.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid preferred name!");
        DBG_ASSERT(m_pMessageParent, "OAdminDialogInvokation::OAdminDialogInvokation: invalid message parent!");
    }

    //---------------------------------------------------------------------
    sal_Bool OAdminDialogInvokation::invokeAdministration( sal_Bool _bFixedType )
    {
        if (!m_xContext.is())
            return sal_False;

        try
        {
            // the service name of the administration dialog
            const static OUString s_sAdministrationServiceName = OUString("com.sun.star.sdb.DatasourceAdministrationDialog");
            const static OUString s_sDataSourceTypeChangeDialog = OUString("com.sun.star.sdb.DataSourceTypeChangeDialog");

            // the parameters for the call
            Sequence< Any > aArguments(3);
            Any* pArguments = aArguments.getArray();

            // the parent window
            Reference< XWindow > xDialogParent = VCLUnoHelper::GetInterface(m_pMessageParent);
            *pArguments++ <<= PropertyValue(OUString("ParentWindow"), -1, makeAny(xDialogParent), PropertyState_DIRECT_VALUE);

            // the title of the dialog
            OUString sAdminDialogTitle(ModuleRes(RID_STR_ADMINDIALOGTITLE).toString());
            *pArguments++ <<= PropertyValue(OUString("Title"), -1, makeAny(sAdminDialogTitle), PropertyState_DIRECT_VALUE);

            // the name of the new data source
            *pArguments++ <<= PropertyValue(OUString("InitialSelection"), -1, makeAny(m_xDataSource), PropertyState_DIRECT_VALUE);

            // create the dialog
            Reference< XExecutableDialog > xDialog;
            {
                // creating the dialog service is potentially expensive (if all the libraries invoked need to be loaded)
                // so we display a wait cursor
                WaitObject aWaitCursor(m_pMessageParent);
                xDialog = Reference< XExecutableDialog >( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(_bFixedType ? s_sAdministrationServiceName : s_sDataSourceTypeChangeDialog, aArguments, m_xContext), UNO_QUERY );

                // just for a smoother UI: What the dialog does upon execution, is (amongst other things) creating
                // the DriverManager service
                // If this context has never been accessed before, this may be expensive (it includes loading of
                // at least one library).
                // As this wizard is intended to run on the first office start, it is very likely that the
                // context needs to be freshly created
                // Thus, we access the context here (within the WaitCursor), which means the user sees a waitcursor
                // while his/her office blocks a few seconds ....
                DriverManager::create( m_xContext );
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
            OSL_FAIL("OAdminDialogInvokation::invokeAdministration: caught an exception while executing the dialog!");
        }
        return sal_False;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
