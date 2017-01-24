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

using namespace css;
using namespace css::uno;


namespace vcl {

/** Provides information about UNO commands like tooltip text with
    keyboard accelerator.
*/
class VCL_DLLPUBLIC CommandInfoProvider
{
public:
    CommandInfoProvider();
    ~CommandInfoProvider();

    /** Return a label for the given command.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @return
            The command labe.
    */
    static OUString GetLabelForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static OUString GetMenuLabelForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static OUString GetPopupLabelForCommand (
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
    static OUString GetTooltipForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** Returns the shortcut for a command in human-readable form */
    static OUString GetCommandShortcut (const OUString& rCommandName,
                                 const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static OUString GetRealCommandForCommand( const OUString& rCommandName,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame );

    static OUString GetCommandPropertyFromModule( const OUString& rCommandName, const OUString& rModuleName );

    static BitmapEx GetBitmapForCommand(
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        vcl::ImageType eImageType = vcl::ImageType::Small);

    static Image GetImageForCommand(
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        vcl::ImageType eImageType = vcl::ImageType::Small);

    static sal_Int32 GetPropertiesForCommand(
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static bool IsRotated(const OUString& rsCommandName,const Reference<frame::XFrame>& rxFrame);
    static bool IsMirrored(const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame);

    /** Returns whether the command is experimental. */
    static bool IsExperimental(
        const OUString& rsCommandName,
        const OUString& rModuleName);

    /** Do not call.  Should be part of a local and hidden interface.
    */
    static void SetFrame (const css::uno::Reference<css::frame::XFrame>& rxFrame);

  private:
    static css::uno::Reference<css::ui::XAcceleratorConfiguration> const  GetDocumentAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame);
    static css::uno::Reference<css::ui::XAcceleratorConfiguration> const  GetModuleAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame);
    static css::uno::Reference<css::ui::XAcceleratorConfiguration> const  GetGlobalAcceleratorConfiguration();
    static OUString const GetModuleIdentifier(const Reference<frame::XFrame>& rxFrame);
    static css::uno::Sequence<css::beans::PropertyValue> GetCommandProperties (
        const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame);
    static OUString GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName,const Reference<frame::XFrame>& rxFrame);
    static bool ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame);
    static OUString RetrieveShortcutsFromConfiguration(
        const css::uno::Reference<css::ui::XAcceleratorConfiguration>& rxConfiguration,
        const OUString& rsCommandName);
    static vcl::KeyCode AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey);
};

} // end of namespace vcl

#endif // INCLUDED_VCL_COMMANDINFOPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
