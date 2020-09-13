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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <optional>

#include <vector>

namespace desktop
{

/*
    Class for controls dispatching of command URL through office command line. There
    are "dangerous" command URLs, that can result in a running office without UI. To prevent
    this situation the implementation monitors all dispatches and looks for an open task if
    there is arose a problem. If there is none the office will be shutdown to prevent a
    running office without UI.
*/
class DispatchWatcher : public ::cppu::WeakImplHelper< css::frame::XDispatchResultListener >
{
    public:
        enum RequestType
        {
            REQUEST_OPEN,
            REQUEST_VIEW,
            REQUEST_START,
            REQUEST_PRINT,
            REQUEST_PRINTTO,
            REQUEST_FORCEOPEN,
            REQUEST_FORCENEW,
            REQUEST_CONVERSION,
            REQUEST_INFILTER,
            REQUEST_BATCHPRINT,
            REQUEST_CAT,
            REQUEST_SCRIPT_CAT
        };

        struct DispatchRequest
        {
            RequestType aRequestType;
            OUString    aURL;
            std::optional< OUString > aCwdUrl;
            OUString    aPrinterName;  // also conversion params
            OUString    aPreselectedFactory;
        };

        DispatchWatcher();

        virtual ~DispatchWatcher() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XDispachResultListener
        virtual void SAL_CALL dispatchFinished( const css::frame::DispatchResultEvent& aEvent ) override;

        // execute new dispatch request
        bool executeDispatchRequests( const std::vector<DispatchRequest>& aDispatches, bool bNoTerminate );

    private:
        osl::Mutex m_mutex;

        sal_Int16                   m_nRequestCount;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
