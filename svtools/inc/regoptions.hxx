/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regoptions.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:46:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SVTOOLS_REGOPTIONS_HXX
#define SVTOOLS_REGOPTIONS_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= RegOptions
    //====================================================================
    class RegOptionsImpl;
    /** encapsulates access to the configuration settings for registering the product
    */
    class SVL_DLLPUBLIC RegOptions
    {
    private:
        RegOptionsImpl*     m_pImpl;

        // the impl class is constructed upon need only (as this is expensive)
        SVL_DLLPRIVATE void ensureImpl( );

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
}   // namespace svt
//........................................................................

#endif // SVTOOLS_REGOPTIONS_HXX

