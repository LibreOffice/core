/*************************************************************************
 *
 *  $RCSfile: asyncfilepicker.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 08:51:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVTOOLS_SOURCE_FILEPICKER_ASYNCFILEPICKER_HXX
#define SVTOOLS_SOURCE_FILEPICKER_ASYNCFILEPICKER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

class SvtFileView;
class SvtFileDialog;
//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= AsyncPickerAction
    //====================================================================
    class AsyncPickerAction : public ::rtl::IReference
    {
    public:
        enum Action
        {
            ePrevLevel,
            eOpenURL,
            eExecuteFilter
        };

    private:
        mutable oslInterlockedCount m_refCount;
        Action                      m_eAction;
        SvtFileView*                m_pView;
        SvtFileDialog*              m_pDialog;
        String                      m_sURL;
        String                      m_sFileName;
        bool                        m_bRunning;

    public:
        AsyncPickerAction( SvtFileDialog* _pDialog, SvtFileView* _pView, const Action _eAction );

        /** executes the action

            @param _nMinTimeout
                the minimum timeout to wait, in milliseconds. If negative, the action will we done
                synchronously. If between 0 and 999, it will be corrected to 1000, means the
                smallest valid value is 1000 (which equals one second).
            @param _nMaxTimeout
                The maximum time to wait for a result, in milliseconds. If there's no result of
                the action within the given time frame, the action will be cancelled.
                If smaller than or equal to <arg>_nMinTimeout</arg>, it will be corrected to
                <arg>_nMinTimeout</arg> + 30000.
        */
        void execute( const String& _rURL, const String& _rFilter, sal_Int32 _nMinTimeout, sal_Int32 _nMaxTimeout );

        /// cancels the running action
        void cancel();

        // IReference overridables
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~AsyncPickerAction();

    private:
        DECL_LINK( OnActionDone, void* );

        AsyncPickerAction();                                        // never implemented
        AsyncPickerAction( const AsyncPickerAction& );              // never implemented
        AsyncPickerAction& operator=( const AsyncPickerAction& );   // never implemented
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVTOOLS_SOURCE_FILEPICKER_ASYNCFILEPICKER_HXX

