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
#ifndef INCLUDED_unotools_INTERNALOPTIONS_HXX
#define INCLUDED_unotools_INTERNALOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

#define MUTEX           ::osl::Mutex

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtInternalOptions_Impl;

/*-************************************************************************************************************//**
    @short          collect information about internal features
    @descr          -

    @implements     -
    @base           -

    @ATTENTION      This class is partially threadsafe.

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtInternalOptions : public utl::detail::Options
{
    public:

        /*-****************************************************************************************************//**
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         SvtInternalOptions();
        virtual ~SvtInternalOptions();

        /*-****************************************************************************************************//**
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Internal/..."
            @descr      These options describe internal states to enable/disable features of installed office.
                        The values are fixed at runtime - and implemented as readonly!

                        SlotCFGEnabled()    =>  If this option is set (true), StarOffice is searching for the slot.cfg.
                                                If the slot.cfg cannot be found, the start is aborted.
                                                If this option is not set (false), the slot.cfg must not be available,
                                                otherwise the start is also aborted.

                        CrashMailEnabled()  =>  Crash-Mail-Feature to document program crashes. After a crash,
                                                an e-mail with information about the system used is generated
                                                automatically when starting StarOffice.

            @seealso    configuration package "org.openoffice.Office.Common/Internal"
        *//*-*****************************************************************************************************/

        sal_Bool    SlotCFGEnabled      () const;
        sal_Bool    MailUIEnabled      () const;

        ::rtl::OUString GetCurrentTempURL() const;
        void            SetCurrentTempURL( const ::rtl::OUString& aNewCurrentTempURL );

    private:

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static MUTEX& GetOwnStaticMutex();

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtInternalOptions_Impl* m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtInternalOptions

#endif  // #ifndef INCLUDED_unotools_INTERNALOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
