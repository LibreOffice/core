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
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

namespace osl { class Mutex; }
template <typename Arg, typename Ret> class Link;
class LinkParamNone;

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtMiscOptions_Impl;
enum class ToolBoxButtonSize;

/*-************************************************************************************************************
    @short          collect information about misc group
    @ATTENTION      This class is partially threadsafe.

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVT_DLLPUBLIC SvtMiscOptions: public utl::detail::Options
{
    public:
         SvtMiscOptions();
        virtual ~SvtMiscOptions() override;

        void        AddListenerLink( const Link<LinkParamNone*,void>& rLink );
        void        RemoveListenerLink( const Link<LinkParamNone*,void>& rLink );

        bool        UseSystemFileDialog() const;
        void        SetUseSystemFileDialog( bool bSet );
        bool        IsUseSystemFileDialogReadOnly() const;

        bool        DisableUICustomization() const;

        bool        IsPluginsEnabled() const;

        sal_Int16   GetSymbolsSize() const;
        void        SetSymbolsSize( sal_Int16 eSet );
        ToolBoxButtonSize   GetSidebarIconSize() const;
        void        SetSidebarIconSize( ToolBoxButtonSize eSet );
        ToolBoxButtonSize   GetNotebookbarIconSize() const;
        void        SetNotebookbarIconSize( ToolBoxButtonSize eSet );
        sal_Int16   GetCurrentSymbolsSize() const;
        bool        AreCurrentSymbolsLarge() const;

        OUString    GetIconTheme() const;
        void        SetIconTheme(const OUString&);
        bool        IconThemeWasSetAutomatically();

        sal_Int16   GetToolboxStyle() const;
        void        SetToolboxStyle( sal_Int16 nStyle );

        bool        UseSystemPrintDialog() const;
        void        SetUseSystemPrintDialog( bool bSet );

        bool        ShowLinkWarningDialog() const;
        void        SetShowLinkWarningDialog( bool bSet );
        bool        IsShowLinkWarningDialogReadOnly() const;

        void        SetExperimentalMode( bool bSet );
        bool        IsExperimentalMode() const;

        void        SetMacroRecorderMode( bool bSet );
        bool        IsMacroRecorderMode() const;

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods aren't safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        SVT_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    private:
        std::shared_ptr<SvtMiscOptions_Impl> m_pImpl;

};      // class SvtMiscOptions

#endif  // #ifndef INCLUDED_SVTOOLS_MISCOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
