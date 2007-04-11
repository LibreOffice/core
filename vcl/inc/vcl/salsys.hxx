/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsys.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:09:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALSYS_HXX
#define _SV_SALSYS_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif


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

class VCL_DLLPUBLIC SalSystem
{
public:
            SalSystem() {}
    virtual ~SalSystem();

    // get info about the display

    /*  Gets the number of active screens attached to the display

        @returns the number of active screens
    */
    virtual unsigned int GetDisplayScreenCount() = 0;
    /*  Queries whether multiple screens are truly separate

        @returns true if screens are distinct and windows cannot
                  be moved between them or span multiple of them
                 false if screens form up one big display
    */
    virtual bool IsMultiDisplay() = 0;
    /*  Queries the default screen number. The default screen is the
        screen on which windows will appear if no special positioning
        is made.

        @returns the default screen number
    */
    virtual unsigned int GetDefaultDisplayNumber() = 0;
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
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen ) = 0;
    /* Gets the name of a screen

       @param nScreen
       The screen number to be queried

       @returns the name of the screen
    */
    virtual rtl::OUString GetScreenName( unsigned int nScreen ) = 0;

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

        @returns the identifier of the button that was pressed by the user.
        See button identifier above. If the function fails the
        return value is 0.
    */
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton) = 0;
};

SalSystem* ImplGetSalSystem();

#endif // _SV_SALSYS_HXX
