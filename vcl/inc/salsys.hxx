/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALSYS_HXX
#define _SV_SALSYS_HXX

#include <tools/gen.hxx>
#include <vcl/dllapi.h>

namespace rtl
{
    class OUString;
}

/* Button combinations for ShowNativeMessageBox
*/
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK                 = 0;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL          = 1;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE = 2;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL      = 3;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO             = 4;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL       = 5;

/* Button identifier for ShowNativeMessageBox
*/
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK     = 1;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL = 2;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT  = 3;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY  = 4;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE = 5;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES    = 6;
const int SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO     = 7;


// -------------
// - SalSystem -
// -------------
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
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen ) = 0;
    /*  Gets position and size of the work area of a screen attached to the display

        @param nScreen
        The screen number to be queried

        @returns position and size relative to the scree
    */
    virtual Rectangle GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen ) = 0;
    /* Gets the name of a screen

       @param nScreen
       The screen number to be queried

       @returns the name of the screen
    */
    virtual rtl::OUString GetDisplayScreenName( unsigned int nScreen ) = 0;

    /*  Shows a native message box with the specified title, message and button
        combination.

        @param  rTitle
        The title to be shown by the dialog box.

        @param  rMessage
        The message to be shown by the dialog box.

        @param  nButtonCombination
        Specify which button combination the message box should display.
        See button combinations above.

        @param  nDefaultButton
        Specifies which button has the focus initially.
        See button identifiers above.
        The effect of specifying a button that doesn't belong
        to the specified button combination is undefined.

        @param  bUseResources
        If false, assume initialization of the application failed early and do
        not try to access any resources.

        @returns the identifier of the button that was pressed by the user.
        See button identifier above. If the function fails the
        return value is 0.
    */
    virtual int ShowNativeMessageBox( const rtl::OUString& rTitle,
                                      const rtl::OUString& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton,
                                      bool bUseResources ) = 0;
};

SalSystem* ImplGetSalSystem();

#endif // _SV_SALSYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
