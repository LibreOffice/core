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
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <comphelper/propertysequence.hxx>
#include <strings.hrc>
#include <componentmodule.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>

namespace abp
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;

    OAdminDialogInvokation::OAdminDialogInvokation(const Reference< XComponentContext >& _rxContext,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxDataSource,
                    weld::Window* _pMessageParent)
        :m_xContext(_rxContext)
        ,m_xDataSource(_rxDataSource)
        ,m_pMessageParent(_pMessageParent)
    {
        DBG_ASSERT(m_xContext.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid service factory!");
        DBG_ASSERT(m_xDataSource.is(), "OAdminDialogInvokation::OAdminDialogInvokation: invalid preferred name!");
        assert(m_pMessageParent && "OAdminDialogInvokation::OAdminDialogInvokation: invalid message parent!");
    }


    bool OAdminDialogInvokation::invokeAdministration()
    {
        if (!m_xContext.is())
            return false;

        try
        {
            // the service name of the administration dialog
            static const char s_sAdministrationServiceName[] = "com.sun.star.sdb.DatasourceAdministrationDialog";
            static const char s_sDataSourceTypeChangeDialog[] = "com.sun.star.sdb.DataSourceTypeChangeDialog";

            // the parameters for the call
            Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
            {
                {"ParentWindow", Any(m_pMessageParent->GetXWindow())},
                {"Title", Any(compmodule::ModuleRes(RID_STR_ADMINDIALOGTITLE))},
                {"InitialSelection", Any(m_xDataSource)}, // the name of the new data source
            }));


            // create the dialog
            Reference< XExecutableDialog > xDialog;
            {
                // creating the dialog service is potentially expensive (if all the libraries invoked need to be loaded)
                // so we display a wait cursor
                weld::WaitObject aWaitCursor(m_pMessageParent);
                Reference<XInterface> x = m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(s_sDataSourceTypeChangeDialog, aArguments, m_xContext);
                xDialog.set( x, UNO_QUERY );

                // just for a smoother UI: What the dialog does upon execution, is (amongst other things) creating
                // the DriverManager service
                // If this context has never been accessed before, this may be expensive (it includes loading of
                // at least one library).
                // As this wizard is intended to run on the first office start, it is very likely that the
                // context needs to be freshly created
                // Thus, we access the context here (within the WaitCursor), which means the user sees a waitcursor
                // while his/her office blocks a few seconds...
                DriverManager::create( m_xContext );
            }

            if (xDialog.is())
            {   // execute it
                if (xDialog->execute())
                    return true;
            }
            else
                ShowServiceNotAvailableError(m_pMessageParent, s_sAdministrationServiceName, true);
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.abpilot",
                                 "caught an exception while executing the dialog!");
        }
        return false;
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
