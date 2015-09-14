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
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#define INCLUDED_SVTOOLS_MENUOPTIONS_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtMenuOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about menu features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVT_DLLPUBLIC SAL_WARN_UNUSED SvtMenuOptions: public utl::detail::Options
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

         SvtMenuOptions();
        virtual ~SvtMenuOptions();

        void AddListenerLink( const Link<LinkParamNone*,void>& rLink );
        void RemoveListenerLink( const Link<LinkParamNone*,void>& rLink );

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/View/Menu/..."
            @descr      These options describe internal states to enable/disable features of installed office.

                        IsEntryHidingEnabled()
                        SetEntryHidingState()   =>  Activate this field for viewing all deactivated menu entries.
                                                    Menu commands that are normally not necessary are hidden by default.
                                                    Default=false

                        IsFollowMouseEnabled()
                        SetFollowMouseState()   =>  Automatic selection while moving the mouse on a menu.
                                                    Default=true

            @seealso    configuration package "org.openoffice.Office.Common/View/Menu"
        *//*-*****************************************************************************************************/

        bool IsEntryHidingEnabled() const;
        TriState GetMenuIconsState() const;
        void SetMenuIconsState(TriState eState);

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        SVT_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:

        /*Attention

            Don't initialize these static members in these headers!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtMenuOptions_Impl* m_pDataContainer    ;
        static sal_Int32            m_nRefCount         ;

};      // class SvtMenuOptions

#endif  // #ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
