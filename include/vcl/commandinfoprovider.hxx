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
#include <vcl/image.hxx>

#include <com/sun/star/frame/XFrame.hpp>

namespace vcl { namespace CommandInfoProvider {

    /** Return a label for the given command.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rsModuleName
            The current application module.
        @return
            The command label.
    */
    VCL_DLLPUBLIC OUString GetLabelForCommand (
        const OUString& rsCommandName,
        const OUString& rsModuleName);

    VCL_DLLPUBLIC OUString GetMenuLabelForCommand (
        const OUString& rsCommandName,
        const OUString& rsModuleName);

    VCL_DLLPUBLIC OUString GetPopupLabelForCommand (
        const OUString& rsCommandName,
        const OUString& rsModuleName);

    /** Return a tooltip for the given command. Falls back to label if command has no tooltip.
        @param rsCommandName
            The command name is expected to start with .uno:
        @param rxFrame
            The frame is used to identify the module and document.
        @return
            The returned label contains the keyboard accelerator, if
            one is defined and bIncludeShortcut is true.
    */
    VCL_DLLPUBLIC OUString GetTooltipForCommand (
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** Returns the shortcut for a command in human-readable form */
    VCL_DLLPUBLIC OUString GetCommandShortcut (const OUString& rCommandName,
                                               const css::uno::Reference<css::frame::XFrame>& rxFrame);

    VCL_DLLPUBLIC OUString GetRealCommandForCommand( const OUString& rCommandName,
                                                     const OUString& rsModuleName );

    VCL_DLLPUBLIC Image GetImageForCommand(
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        vcl::ImageType eImageType = vcl::ImageType::Small);

    VCL_DLLPUBLIC sal_Int32 GetPropertiesForCommand(
        const OUString& rsCommandName,
        const OUString& rsModuleName);

    VCL_DLLPUBLIC bool IsRotated(const OUString& rsCommandName, const OUString& rsModuleName);
    VCL_DLLPUBLIC bool IsMirrored(const OUString& rsCommandName, const OUString& rsModuleName);

    /** Returns whether the command is experimental. */
    VCL_DLLPUBLIC bool IsExperimental(
        const OUString& rsCommandName,
        const OUString& rModuleName);

    VCL_DLLPUBLIC OUString const GetModuleIdentifier(const css::uno::Reference<css::frame::XFrame>& rxFrame);
} }

#endif // INCLUDED_VCL_COMMANDINFOPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
