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
    static OUString GetLabelForCommand (
        const OUString& rsCommandName);

    static OUString GetMenuLabelForCommand (const OUString& rsCommandName);

    static OUString GetPopupLabelForCommand (const OUString& rsCommandName);

    /** Return a tooltip for the given command. Falls back to label if command has no tooltip.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @param bIncludeShortcut
            Whether the shortcut should be appended in brackets
        @return
            The returned label contains the keyboard accelerator, if
            one is defined and bIncludeShortcut is true.
    */
   static OUString GetTooltipForCommand (const OUString& rsCommandName,
        bool bIncludeShortcut = true);

    /** Returns the shortcut for a command in human-readable form */
  static  OUString GetCommandShortcut (const OUString& rCommandName);

  static Image GetImageForCommand(
        const OUString& rsCommandName,
        bool bLarge,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

   static  sal_Int32 GetPropertiesForCommand(const OUString& rsCommandName);

   static  bool IsRotated(const OUString& rsCommandName);
   static  bool IsMirrored(const OUString& rsCommandName);

    /** Do not call.  Should be part of a local and hidden interface.
    */
    void SetFrame (const css::uno::Reference<css::frame::XFrame>& rxFrame);

    void dispose();

  private:
   static css::uno::Reference<css::uno::XComponentContext> mxContext;
   static  css::uno::Reference<css::frame::XFrame> mxCachedDataFrame;
   static css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedDocumentAcceleratorConfiguration;
   static css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedModuleAcceleratorConfiguration;
   static css::uno::Reference<css::ui::XAcceleratorConfiguration> mxCachedGlobalAcceleratorConfiguration;
   static OUString msCachedModuleIdentifier;
   static css::uno::Reference<css::lang::XComponent> mxFrameListener;

    CommandInfoProvider();
    ~CommandInfoProvider();

   static css::uno::Reference<css::ui::XAcceleratorConfiguration> GetDocumentAcceleratorConfiguration();
   static  css::uno::Reference<css::ui::XAcceleratorConfiguration> GetModuleAcceleratorConfiguration();
   static css::uno::Reference<css::ui::XAcceleratorConfiguration> GetGlobalAcceleratorConfiguration();
   static OUString GetModuleIdentifier();
   static css::uno::Sequence<css::beans::PropertyValue> GetCommandProperties (
        const OUString& rsCommandName);
   static OUString GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName);
   static bool ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName);
   static OUString RetrieveShortcutsFromConfiguration(
        const css::uno::Reference<css::ui::XAcceleratorConfiguration>& rxConfiguration,
        const OUString& rsCommandName);
    static vcl::KeyCode AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey);
};

} // end of namespace vcl

#endif // INCLUDED_VCL_COMMANDINFOPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
