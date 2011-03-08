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

#ifndef SVTOOLS_SOURCE_FILEPICKER_ASYNCFILEPICKER_HXX
#define SVTOOLS_SOURCE_FILEPICKER_ASYNCFILEPICKER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.h>

class SvtFileView;
class SvtFileDialog;

typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >  OUStringList;

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
        void execute(
            const String& _rURL,
            const String& _rFilter,
            sal_Int32 _nMinTimeout,
            sal_Int32 _nMaxTimeout,
            const OUStringList& rBlackList = OUStringList() );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
