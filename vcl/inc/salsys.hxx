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

#ifndef INCLUDED_VCL_INC_SALSYS_HXX
#define INCLUDED_VCL_INC_SALSYS_HXX

#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>

// Button identifier for ShowNativeMessageBox
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK     = 1;

class VCL_PLUGIN_PUBLIC SalSystem
{
public:
            SalSystem() {}
    virtual ~SalSystem();

    // get info about the display

    /*  Gets the number of active screens attached to the display

        @returns the number of active screens
    */
    virtual unsigned int GetDisplayScreenCount() = 0;
    /*  Queries whether multiple screens are part of one bigger display

        @returns true if screens form one big display
                 false if screens are distinct and windows cannot
                 be moved between, or span multiple screens
    */
    virtual bool IsUnifiedDisplay() { return true; }
    /*  Queries the default screen number. The default screen is the
        screen on which windows will appear if no special positioning
        is made.

        @returns the default screen number
    */
    virtual unsigned int GetDisplayBuiltInScreen() { return 0; }
    /*  Gets relative position and size of the screens attached to the display

        @param nScreen
        The screen number to be queried

        @returns position: (0,0) in case of IsMultiscreen() == true
                           else position relative to whole display
                 size: size of the screen
    */
    virtual tools::Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen ) = 0;

    /*  Shows a native message box with the specified title, message and button
        combination.

        @param  rTitle
        The title to be shown by the dialog box.

        @param  rMessage
        The message to be shown by the dialog box.

        @returns the identifier of the button that was pressed by the user.
        See button identifier above. If the function fails the
        return value is 0.
    */
    virtual int ShowNativeMessageBox( const OUString& rTitle,
                                      const OUString& rMessage ) = 0;

};

VCL_DLLPUBLIC SalSystem* ImplGetSalSystem();

#endif // INCLUDED_VCL_INC_SALSYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
