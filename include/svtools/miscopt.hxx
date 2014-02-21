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
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#define INCLUDED_SVTOOLS_MISCOPT_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtMiscOptions_Impl;
class Link;

/*-************************************************************************************************************//**
    @short          collect information about misc group
    @descr          -

    @implements     -
    @base           -

    @ATTENTION      This class is partially threadsafe.

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVT_DLLPUBLIC SvtMiscOptions: public utl::detail::Options
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

         SvtMiscOptions();
        virtual ~SvtMiscOptions();

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );

        sal_Bool    UseSystemFileDialog() const;
        void        SetUseSystemFileDialog( sal_Bool bSet );
        sal_Bool    IsUseSystemFileDialogReadOnly() const;

        sal_Bool    DisableUICustomization() const;

        sal_Bool    IsPluginsEnabled() const;

        sal_Int16   GetSymbolsSize() const;
        void        SetSymbolsSize( sal_Int16 eSet );
        sal_Int16   GetCurrentSymbolsSize() const;
        bool        AreCurrentSymbolsLarge() const;

        OUString    GetIconTheme() const;
        void        SetIconTheme(const OUString&);
        void        SetIconThemeAutomatically();
        bool        IconThemeWasSetAutomatically();

        sal_Int16   GetToolboxStyle() const;
        void        SetToolboxStyle( sal_Int16 nStyle );

        sal_Bool    IsModifyByPrinting() const;
        void        SetModifyByPrinting(sal_Bool bSet );

        sal_Bool    UseSystemPrintDialog() const;
        void        SetUseSystemPrintDialog( sal_Bool bSet );

        sal_Bool    ShowLinkWarningDialog() const;
        void        SetShowLinkWarningDialog( sal_Bool bSet );
        sal_Bool    IsShowLinkWarningDialogReadOnly() const;

        void        SetSaveAlwaysAllowed( sal_Bool bSet );
        sal_Bool    IsSaveAlwaysAllowed() const;

        void        SetExperimentalMode( sal_Bool bSet );
        sal_Bool    IsExperimentalMode() const;

        void        SetMacroRecorderMode( sal_Bool bSet );
        sal_Bool    IsMacroRecorderMode() const;

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

        SVT_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtMiscOptions_Impl* m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtMiscOptions

#endif  // #ifndef INCLUDED_SVTOOLS_MISCOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
