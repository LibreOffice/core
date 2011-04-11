/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DBAUI_ICONTROLLER_HXX
#define DBAUI_ICONTROLLER_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include "IReference.hxx"
#include "dbaccessdllapi.h"

namespace com { namespace sun { namespace star {
    namespace util {
        struct URL;
    }
    namespace frame {
        class XController;
    }
} } }

class NotifyEvent;

namespace dbaui
{
    // interface for controller depended calls like commands
    class DBACCESS_DLLPUBLIC IController : public IReference
    {
    public:
        /** executes the given command without checking if it is allowed
            @param  _rCommand   the URL of the command
        */
        virtual void executeUnChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _rCommand
                the URL of the command
        */
        virtual void executeChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command without checking if it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeUnChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;


        /** checks if the given Command is enabled
            @param  _nCommandId
                the id of the command URL

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual sal_Bool isCommandEnabled(sal_uInt16 _nCommandId) const = 0;

        /** checks if the given Command is enabled
            @param  _rCompleteCommandURL
                the URL of the command

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual sal_Bool isCommandEnabled( const ::rtl::OUString& _rCompleteCommandURL ) const = 0;

        /** registers a command URL, giving it a unique name

            If you call this with a command URL which is supported by the controller, then
            you will simply get the controller's internal numeric shortcut to this command.

            If you call this with a command URL which is not supported by the controller, then
            you will get a new ID, which is unique during the lifetime of the controller.

            If the command URL is invalid, or the controller cannot register new commands anymore,
            then 0 is returned.
        */
        virtual sal_uInt16
                        registerCommandURL( const ::rtl::OUString& _rCompleteCommandURL ) = 0;

        /** notifyHiContrastChanged will be called when the hicontrast mode changed.
            @param  _bHiContrast
                <TRUE/> when in hicontrast mode.
        */
        virtual void notifyHiContrastChanged() = 0;

        /** checks if the selected data source is read only
            @return
                <TRUE/> if read only, otherwise <FALSE/>
        */
        virtual sal_Bool isDataSourceReadOnly() const = 0;

        /** provides access to the model of the controller

            This must be the same model as returned by XController::getModel, and might be <NULL/> when
            the controller does not have an own model.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                getXController(void) throw( ::com::sun::star::uno::RuntimeException ) = 0;

        /** allows interception of user input, aka mouse clicks and key events
        */
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) = 0;
    };
}
#endif // DBAUI_ICONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
