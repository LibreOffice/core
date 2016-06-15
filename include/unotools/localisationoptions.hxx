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
#ifndef INCLUDED_UNOTOOLS_LOCALISATIONOPTIONS_HXX
#define INCLUDED_UNOTOOLS_LOCALISATIONOPTIONS_HXX

#include <sal/types.h>
#include <unotools/unotoolsdllapi.h>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>
#include <memory>

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtLocalisationOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about localisation features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtLocalisationOptions : public utl::detail::Options
{
    public:
         SvtLocalisationOptions();
        virtual ~SvtLocalisationOptions();

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/View/Localisation/AutoMnemonic"
            @descr      These value specifies if shortcuts should be assigned automatically.
            @param      "bState", new value to set it in configuration.
            @return     The value which represent current state of internal variable.

            @onerror    No error should occur!
        *//*-*****************************************************************************************************/

        bool    IsAutoMnemonic  (                   ) const;

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/View/Localisation/DialogScale"
            @descr      These value specifies the factor for increasing controls.
                        Value from [0..100] are allowed.

            @ATTENTION  These methods don't check for valid or invalid values!
                        Our configuration server can do it ... but these implementation don't get any notifications
                        about wrong commits ...!
                        => If you set an invalid value - nothing will be changed. The information will lost.

            @seealso    baseclass ConfigItem

            @param      "nScale" new value to set it in configuration.
            @return     The value which represent current state of internal variable.

            @onerror    No error should occur!
        *//*-*****************************************************************************************************/

        sal_Int32   GetDialogScale(                     ) const;
    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:
        std::shared_ptr<SvtLocalisationOptions_Impl> m_pImpl;

};      // class SvtLocalisationOptions

#endif // INCLUDED_UNOTOOLS_LOCALISATIONOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
