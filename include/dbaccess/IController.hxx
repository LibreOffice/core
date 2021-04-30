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

#ifndef INCLUDED_DBACCESS_ICONTROLLER_HXX
#define INCLUDED_DBACCESS_ICONTROLLER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace beans {
        struct PropertyValue;
    }
    namespace util {
        struct URL;
    }
    namespace frame {
        class XController;
    }
}

class NotifyEvent;

namespace dbaui
{
    // interface for controller depended calls like commands
    class DBACCESS_DLLPUBLIC IController
    {
    public:
        /** executes the given command without checking if it is allowed
            @param  _rCommand   the URL of the command
        */
        virtual void executeUnChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _rCommand
                the URL of the command
        */
        virtual void executeChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command without checking if it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeUnChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) = 0;


        /** checks if the given Command is enabled
            @param  _nCommandId
                the id of the command URL

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual bool isCommandEnabled(sal_uInt16 _nCommandId) const = 0;

        /** checks if the given Command is enabled
            @param  _rCompleteCommandURL
                the URL of the command

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual bool isCommandEnabled( const OUString& _rCompleteCommandURL ) const = 0;

        /** checks if the selected data source is read only
            @return
                <TRUE/> if read only, otherwise <FALSE/>
        */
        virtual bool isDataSourceReadOnly() const = 0;

        /** provides access to the model of the controller

            This must be the same model as returned by XController::getModel, and might be <NULL/> when
            the controller does not have an own model.

            @throws css::uno::RuntimeException
        */
        virtual css::uno::Reference< css::frame::XController >
                getXController() = 0;

        /** allows interception of user input, aka mouse clicks and key events
        */
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) = 0;

        virtual void SAL_CALL acquire(  ) noexcept = 0;
        virtual void SAL_CALL release(  ) noexcept = 0;

    protected:
        ~IController() {}
    };
}
#endif // INCLUDED_DBACCESS_ICONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
