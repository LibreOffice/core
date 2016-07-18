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
#ifndef INCLUDED_VCL_COMMANDINFOPROVIDER_HXX
#define INCLUDED_VCL_COMMANDINFOPROVIDER_HXX

#include <vcl/dllapi.h>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>


namespace vcl {

/** Provides information about UNO commands like tooltip text with
    keyboard accelerator.
*/
class VCL_DLLPUBLIC CommandInfoProvider
{
public:
    /** Return the singleton instance.

        It caches some objects for the last XFrame object given to
        GetLabelForCommand.  These objects are release and created new
        when that method is called with a different XFrame from the
        last call.

        Lifetime control should work but could be more elegant.
    */
    static CommandInfoProvider& Instance();

    /** Return a label for the given command.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @return
            The command labe.
    */
    OUString GetLabelForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    OUString GetMenuLabelForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    OUString GetPopupLabelForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** Return a tooltip for the given command. Falls back to label if command has no tooltip.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @return
            The returned label contains the keyboard accelerator, if
            one is defined and bIncludeShortcut is true.
    */
    OUString GetTooltipForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** Returns the shortcut for a command in human-readable form */
    OUString GetCommandShortcut (const OUString& rCommandName,
                                 const css::uno::Reference<css::frame::XFrame>& rxFrame);

    OUString GetRealCommandForCommand( const OUString& rCommandName,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame );

    OUString GetCommandPropertyFromModule( const OUString& rCommandName, const OUString& rModuleName );

    Image GetImageForCommand(
        const OUString& rsCommandName,
        bool bLarge,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    sal_Int32 GetPropertiesForCommand(
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    bool IsRotated(const OUString& rsCommandName);
    bool IsMirrored(const OUString& rsCommandName);

    /** Do not call.  Should be part of a local and hidden interface.
    */
    void SetFrame (const css::uno::Reference<css::frame::XFrame>& rxFrame);

    void dispose();

  private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::frame::XFrame> mxCachedDataFrame;
    css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedDocumentAcceleratorConfiguration;
    css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedModuleAcceleratorConfiguration;
    css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedGlobalAcceleratorConfiguration;
    OUString msCachedModuleIdentifier;
    css::uno::Reference<css::lang::XComponent> mxFrameListener;

    CommandInfoProvider();
    ~CommandInfoProvider();

    css::uno::Reference<css::ui::XAcceleratorConfiguration> const & GetDocumentAcceleratorConfiguration();
    css::uno::Reference<css::ui::XAcceleratorConfiguration> const & GetModuleAcceleratorConfiguration();
    css::uno::Reference<css::ui::XAcceleratorConfiguration> const & GetGlobalAcceleratorConfiguration();
    OUString const & GetModuleIdentifier();
    css::uno::Sequence<css::beans::PropertyValue> GetCommandProperties (
        const OUString& rsCommandName);
    OUString GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName);
    bool ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName);
    static OUString RetrieveShortcutsFromConfiguration(
        const css::uno::Reference<css::ui::XAcceleratorConfiguration>& rxConfiguration,
        const OUString& rsCommandName);
    static vcl::KeyCode AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey);
};

} // end of namespace vcl

#endif // INCLUDED_VCL_COMMANDINFOPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
