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

#ifndef unotools_REGOPTIONS_HXX
#define unotools_REGOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <tools/string.hxx>

//........................................................................
namespace utl
{
//........................................................................

    //====================================================================
    //= RegOptions
    //====================================================================
    class RegOptionsImpl;
    /** encapsulates access to the configuration settings for registering the product
    */
    class UNOTOOLS_DLLPUBLIC RegOptions
    {
    private:
        RegOptionsImpl*     m_pImpl;

        // the impl class is constructed upon need only (as this is expensive)
        UNOTOOLS_DLLPRIVATE void ensureImpl( );

    public:
        RegOptions( );
        ~RegOptions( );

        // retrieves the URL which should be used for online registration
        String              getRegistrationURL( ) const;

        // checks if the menu item which can be used to trigger the online registration should be available
        sal_Bool            allowMenu( ) const;

        enum DialogPermission
        {                       // the registration dialog shall be executed ....
            dpDisabled,         // ... never - it has been disabled
            dpNotThisSession,   // ... later - no statement when, but _not now_
            dpRemindLater,      // ... later at a reminder date
            dpThisSession       // ... during this session
        };

        /** checks if the registration dialog is allowed
            <p>A return value of <TRUE/> does not mean that during this session, the dialog is allowed to
            be executed. It simply means executing it is allowed <em>in general</em></p>
        */
        DialogPermission    getDialogPermission( ) const;

        /** claims that the current session is done with respect to the registration dialog.

            <p>This is not to be called if <method>getDialogPermission</method> returned dpDisabled previously<p>
            <p>The behaviour changes as follows:
            <ul>
                <li>any subsequent calls to <method>getDialogPermission</method> will return dpNotThisSession, given
                    that it previously returned dpNotThisSession</li>
                <li>any subsequent calls to <method>getDialogPermission</method> will return dpDisabled, given
                    that it previously returned dpThisSession</li>
                <li>an internal (persistent) session counter is updated, so that during the next session,
                    <method>getDialogPermission</method> <em>may</em> return dpThisSession</li>
            </ul></p>
        */
        void                markSessionDone( );

        /** activates the reminder

            <p>If this method is called, the reminder for the dialog will be activated.
            This means that during the next <arg>_nDaysFromNow</arg>-1 days,
            <method>getDialogPermission</method> will return dpRemindLater</p>,
            at the <arg>_nDaysFromNow</arg>th day from today onwards it will
            return dpThisSession.</p>

            <p>It is not allowed to call this method if the dialog is currently disabled.</p>

            <p>If the current session has not been marked as done already
            (i.e., <method>markSessionDone</method> has not been called, yet), this is
            done implicitly when you call <method>activateReminder</method>.</p>
        */
        void                activateReminder( sal_Int32 _nDaysFromNow );

        /** removes the reminder

            <p>If this method is called, the reminder for the dialog will be removed.
        */
        void                removeReminder();

        /** checks if the reminder date has come

            <p>Returns <true/> if the current date is greater or equal the reminder date.
        */
        bool                hasReminderDateCome() const;
    };

//........................................................................
}   // namespace utl
//........................................................................

#endif // unotools_REGOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
