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
#ifndef INCLUDED_UNOTOOLS_FONTOPTIONS_HXX
#define INCLUDED_UNOTOOLS_FONTOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtFontOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about font features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtFontOptions : public utl::detail::Options
{
    public:
        /*-****************************************************************************************************
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer
        *//*-*****************************************************************************************************/

         SvtFontOptions();
        virtual ~SvtFontOptions();

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Font"
            @descr      These values defines different states of font handling.

                        ..ReplacementTable..()  =>  Determines if the list of font replacements is applied or not.

                        ..FontHistory..()       =>  The last 5 fonts will be shown and the last one will be the
                                                    first name on the list. These will be displayed in the Font name
                                                    combo box on the Object bar.

                        ..FontWYSIWYG..()       =>  With this option the names of the selectable fonts
                                                    (for example, the fonts in the Font field in the object bar)
                                                    will be formatted as the current font.
            @onerror    No error should occur!
        *//*-*****************************************************************************************************/

        bool        IsFontHistoryEnabled        (                   ) const;

        bool        IsFontWYSIWYGEnabled        (                   ) const;
        void        EnableFontWYSIWYG           ( bool bState   );
    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class use his own static mutex to be threadsafe.
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& impl_GetOwnStaticMutex();
    private:

        /*Attention

            Don't initialize these static members in these headers!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtFontOptions_Impl*         m_pDataContainer;
        static sal_Int32                    m_nRefCount;

};      // class SvtFontOptions

#endif // INCLUDED_UNOTOOLS_FONTOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
