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

#ifndef INCLUDED_SVTOOLS_WIZDLG_HXX
#define INCLUDED_SVTOOLS_WIZDLG_HXX

#include <svtools/svtdllapi.h>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/idle.hxx>

class TabPage;
struct ImplWizPageData;
struct ImplWizButtonData;

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
In that handler has a optional error check and returns sal_False, if the
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

#define WIZARDDIALOG_BUTTON_STDOFFSET_X         6
#define WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X    3


class SVT_DLLPUBLIC WizardDialog : public ModalDialog
{
private:
    Idle                    maWizardLayoutIdle;
    Size                    maPageSize;
    ImplWizPageData*        mpFirstPage;
    ImplWizButtonData*      mpFirstBtn;
    VclPtr<TabPage>         mpCurTabPage;
    VclPtr<PushButton>      mpPrevBtn;
    VclPtr<PushButton>      mpNextBtn;
    VclPtr<vcl::Window>     mpViewWindow;
    sal_uInt16              mnCurLevel;
    WindowAlign             meViewAlign;
    Link<WizardDialog*,void>  maActivateHdl;
    sal_Int16               mnLeftAlignCount;
    bool                    mbEmptyViewMargin;

    DECL_DLLPRIVATE_LINK( ImplHandleWizardLayoutTimerHdl, Timer*, void );

protected:
    long                LogicalCoordinateToPixel(int iCoordinate);
    /**sets the number of buttons which should be left-aligned. Normally, buttons are right-aligned.

        only to be used during construction, before any layouting happened
    */
    void                SetLeftAlignedButtonCount( sal_Int16 _nCount );
    /** declares the view area to have an empty margin

        Normally, the view area has a certain margin to the top/left/bottom/right of the
        dialog. By calling this method, you can reduce this margin to 0.
    */
    void                SetEmptyViewMargin();

private:
    SVT_DLLPRIVATE void             ImplInitData();
    SVT_DLLPRIVATE void             ImplCalcSize( Size& rSize );
    SVT_DLLPRIVATE void             ImplPosCtrls();
    SVT_DLLPRIVATE void             ImplPosTabPage();
    SVT_DLLPRIVATE void             ImplShowTabPage( TabPage* pPage );
    SVT_DLLPRIVATE TabPage*         ImplGetPage( sal_uInt16 nLevel ) const;

public:
    WizardDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );
    virtual ~WizardDialog() override;
    virtual void dispose() override;

    virtual void        Resize() override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;

    virtual void        ActivatePage();
    virtual bool        DeactivatePage();

    virtual void        queue_resize(StateChangedType eReason = StateChangedType::Layout) override;

    bool                ShowPrevPage();
    bool                ShowNextPage();
    bool                ShowPage( sal_uInt16 nLevel );
    bool                Finish( long nResult = 0 );
    sal_uInt16          GetCurLevel() const { return mnCurLevel; }

    void                AddPage( TabPage* pPage );
    void                RemovePage( TabPage* pPage );
    void                SetPage( sal_uInt16 nLevel, TabPage* pPage );
    TabPage*            GetPage( sal_uInt16 nLevel ) const;

    void                AddButton( Button* pButton, long nOffset = 0 );
    void                RemoveButton( Button* pButton );

    void                SetPrevButton( PushButton* pButton ) { mpPrevBtn = pButton; }
    void                SetNextButton( PushButton* pButton ) { mpNextBtn = pButton; }

    void                SetViewWindow( vcl::Window* pWindow ) { mpViewWindow = pWindow; }
    void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
    const Size&         GetPageSizePixel() const { return maPageSize; }

    void                SetActivatePageHdl( const Link<WizardDialog*,void>& rLink ) { maActivateHdl = rLink; }
};

#endif // INCLUDED_SVTOOLS_WIZDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
