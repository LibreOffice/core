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
#ifndef SIDEBAR_COMMAND_INFO_PROVIDER_HXX
#define SIDEBAR_COMMAND_INFO_PROVIDER_HXX

#include "sfx2/dllapi.h"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** Provide information about UNO commands like tooltip text with
    keyboard accelerator.
*/
class SFX2_DLLPUBLIC CommandInfoProvider
{
public:
    /** Return the singleton instance.

        It caches some objects for the last XFrame object given to
        GetLabelForCommand.  These objects are release and created new
        when that method is called with a different XFrame from the
        last call.

        Lifetime control should work but could be more elegant.
    */
    static CommandInfoProvider& Instance (void);

    /** Return a label for the given command.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @return
            The returned label contains the keyboard accelerator, if
            one is defined.
    */
    ::rtl::OUString GetLabelForCommand (
        const ::rtl::OUString& rsCommandName,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    /** Do not call.  Should be part of a local and hidden interface.
    */
    void SetFrame (const cssu::Reference<css::frame::XFrame>& rxFrame);

  private:
    cssu::Reference<css::uno::XComponentContext> mxContext;
    cssu::Reference<css::frame::XFrame> mxCachedDataFrame;
    cssu::Reference<css::ui::XAcceleratorConfiguration> mxCachedDocumentAcceleratorConfiguration;
    cssu::Reference<css::ui::XAcceleratorConfiguration> mxCachedModuleAcceleratorConfiguration;
    cssu::Reference<css::ui::XAcceleratorConfiguration> mxCachedGlobalAcceleratorConfiguration;
    ::rtl::OUString msCachedModuleIdentifier;
    cssu::Reference<css::lang::XComponent> mxFrameListener;

    CommandInfoProvider (void);
    ~CommandInfoProvider (void);

    cssu::Reference<css::ui::XAcceleratorConfiguration> GetDocumentAcceleratorConfiguration (void);
    cssu::Reference<css::ui::XAcceleratorConfiguration> GetModuleAcceleratorConfiguration (void);
    cssu::Reference<css::ui::XAcceleratorConfiguration> GetGlobalAcceleratorConfiguration(void);
    ::rtl::OUString GetModuleIdentifier (void);
    ::rtl::OUString GetCommandShortcut (const ::rtl::OUString& rCommandName);
    cssu::Sequence<css::beans::PropertyValue> GetCommandProperties (
        const ::rtl::OUString& rsCommandName);
    ::rtl::OUString GetCommandLabel (const ::rtl::OUString& rsCommandName);
    rtl::OUString RetrieveShortcutsFromConfiguration(
        const cssu::Reference<css::ui::XAcceleratorConfiguration>& rxConfiguration,
        const rtl::OUString& rsCommandName);
};

} } // end of namespace sfx2/framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
