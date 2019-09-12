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

#ifndef INCLUDED_VCL_WIZDLG_HXX
#define INCLUDED_VCL_WIZDLG_HXX

#include <vcl/dllapi.h>

class TabPage;

/*

Description
==========

class WizardDialog

This class is the base for WizardDialog. The basic functionality is to
order the Controls. Besides it's a helper method for switching the TabPages.
The dialog orders the Controls when their size changed.

--------------------------------------------------------------------------

SetPageSizePixel() sets the biggest TabPage size. When the dialog
should be displayed, first the dialog size is calculated and set.
If there is no size set with SetPageSizePixel(), max size of the
current TabPage is set as default.

ShowPrevPage()/ShowNextPage() shows the previous/next TabPage.
First the Deactivate-Handler is called by dialog and if the return
value is sal_True the Active-Handler is called by dialog and the
corresponding TabPage is showed. Finnish() can only be called
if the Finnish-Button is activated. Then the Deactivate-Page-Handler
is called by dialog and by the current TabPage. Now the dialog ends
(Close() or EndDialog()).

AddPage()/RemovePage()/SetPage() TabPages are made known to the Wizard.
The TabPage of the current Level is always shown and if no TabPage is
available at that level then the TabPage of the highest level is used.
Because of that the current TabPage always can be swapped under the
condition that in the Activate-Handler the current TabPage cannot be
destroyed.

SetPrevButton()/SetNextButton() add the Prev-Button and the
Next-Button to the dialog. In that case the dialog emits the Click-Handler
of the assigned Button when Ctrl+Tab, Shift-Ctrl-Tab are pressed. The Buttons
are not disabled by the WizardDialog and such an action must be
programmed by the user of this Dialog.

AddButton()/RemoveButton() can add Buttons to the Wizard and they're shown
the order of adding them. The Buttons are ordered independent of their
visibility state, so that also later a Button can be shown/hidden.
The Offset is in Pixels and always refers to the following Button. So that
the distance between the Buttons is the same for all dialogs, there is the
macro WIZARDDIALOG_BUTTON_STDOFFSET_X to be used as the default offset.

With SetViewWindow() and SetViewAlign() a Control can be set, which can
be used as Preview-Window or for displaying of pretty Bitmaps.

--------------------------------------------------------------------------

The ActivatePage()-Handler is called, if a new TabPage is shown. In that
handler a new TabPage can be created if it was not created before; the
handler can be set as a Link. GetCurLevel() returns the current level and
Level 0 is the first page.

The DeactivatePage()-Handler is called if a new TabPage should be shown.
In that handler has an optional error check and returns sal_False, if the
switch should not be done. Also the Handler can be set as a Link. The
default implementation calls the Link and returns the Links value or returns
sal_True if no Link is set.

--------------------------------------------------------------------------

Example:

MyWizardDlg-Ctor
----------------

// add buttons
AddButton( &maHelpBtn, WIZARDDIALOG_BUTTON_STDOFFSET_X );
AddButton( &maCancelBtn, WIZARDDIALOG_BUTTON_STDOFFSET_X );
AddButton( &maPrevBtn );
AddButton( &maNextBtn, WIZARDDIALOG_BUTTON_STDOFFSET_X );
AddButton( &maFinnishBtn );
SetPrevButton( &maPrevBtn );
SetNextButton( &maNextBtn );

// SetHandler
maPrevBtn.SetClickHdl( LINK( this, MyWizardDlg, ImplPrevHdl ) );
maNextBtn.SetClickHdl( LINK( this, MyWizardDlg, ImplNextHdl ) );

// Set PreviewWindow
SetViewWindow( &maPreview );

// Call ActivatePage, because the first page should be created an activated
ActivatePage();


MyWizardDlg-ActivatePage-Handler
--------------------------------

void MyWizardDlg::ActivatePage()
{
    WizardDialog::ActivatePage();

    // Test, if Page is created already
    if ( !GetPage( GetCurLevel() ) )
    {
        // Create and add new page
        TabPage* pNewTabPage;
        switch ( GetCurLevel() )
        {
            case 0:
                pNewTabPage = CreateIntroPage();
                break;
            case 1:
                pNewTabPage = CreateSecondPage();
                break;
            case 2:
                pNewTabPage = CreateThirdPage();
                break;
            case 3:
                pNewTabPage = CreateFinnishedPage();
                break;

        }
        AddPage( pNewTabPage );
    }
}


MyWizardDlg-Prev/Next-Handler
-----------------------------

IMPL_LINK( MyWizardDlg, ImplPrevHdl, PushButton*, pBtn, void )
{
    ShowPrevPage();
    if ( !GetCurLevel() )
        pBtn->Disable();
}

IMPL_LINK( MyWizardDlg, ImplNextHdl, PushButton*, pBtn, void )
{
    ShowNextPage();
    if ( GetCurLevel() < 3 )
        pBtn->Disable();
}

*/

#endif // INCLUDED_VCL_WIZDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
