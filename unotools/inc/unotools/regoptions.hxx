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

