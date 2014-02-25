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

#include <vcl/dialog.hxx>

class TabPage;
class Button;
class PushButton;
class FixedLine;
struct ImplWizPageData;
struct ImplWizButtonData;

/*************************************************************************

Beschreibung
============

class WizardDialog

Diese Klasse dient als Basis fuer einen WizardDialog. Als
Basisfunktionalitaet wird das Anordnen der Controls angeboten und
Hilfesmethoden fuer das Umschalten von TabPages. Der Dialog
ordnet bei einer Groessenanderung die Controls auch wieder
entsprechend an.

--------------------------------------------------------------------------

Mit SetPageSizePixel() kann als Groesse die Groesse der groessten
TabPage vorgegeben werden. Wenn der Dialog angezeigt wird, wird
zu dem Zeitpunkt wenn noch keine Groesse gesetzt wurde, dafuer
die entsprechende Dialoggroesse berechnet und gesetzt. Wenn mit
SetPageSizePixel() keine Groesse gesetzt wurde, wird als Groesse
die maximale Groesse der zu diesem Zeitpunkt zugewiesenen TabPages
berechnet und genommen.

ShowPrevPage()/ShowNextPage() zeigt die vorherige/naechste TabPage
an. Dazu wird zuerst der Deactivate-Handler vom Dialog gerufen und
wenn dieser sal_True zurueckgegeben hat, wird der Acivate-Handler
vom Dialog gerufen und die entsprechende TabPage angezeigt.
Finnsh() kann gerufen werden, wenn der Finnish-Button betaetigt
wird. Dort wird dann auch noch der Deactivate-Page-Handler vom
Dialog und der aktuellen TabPage gerufen und dann der Dialog
beendet (Close() oder EndDialog()).

Mit AddPage()/RemovePage()/SetPage() koennen die TabPages dem Wizard
bekannt gemacht werden. Es wird immer die TabPage des aktuellen Levels
angezeigt, wenn fuer den aktuellen Level keine TabPage zugewiesen
ist, wird die TabPages des hoechsten Levels angezeigt. Somit kann auch
immer die aktuelle TabPage ausgetauscht werden, wobei zu
beruecksichtigen ist, das im Activate-Handler die aktuelle TabPage
nicht zerstoert werden darf.

Mit SetPrevButton()/SetNextButton() werden der Prev-Button und der
Next-Button dem Dialog bekannt gemacht. In dem Fall loest der
Dialog bei Ctr+Tab, Shift+Ctrl+Tab den entsprechenden Click-Handler
am zugewiesenen Button aus. Die Button werden nicht vom WizardDialog
disablte. Eine entsprechende Steuerung muss der Benutzer dieses
Dialoges selber programieren.

Mit AddButton()/RemoveButton() koennen Buttons dem Wizard bekannt
gemacht werden, die in der Reihenfolge der Hinzufuegung angeordnet
werden. Die Buttons werden unabhengig von ihrem sichtbarkeitsstatus
angeordnet, so das auch spaeter ein entsprechender Button angezeigt/
gehidet werden kann. Der Offset wird in Pixeln angegeben und bezieht
sich immer auf den nachfolgenden Button. Damit der Abstand zwischen
den Buttons bei allen Dialogen gleich ist, gibt es das Define
WIZARDDIALOG_BUTTON_STDOFFSET_X, welches als Standard-Offset genommen
werden sollte.

Mit ShowButtonFixedLine() kann gesteuert werden, ob die zwischen den
Buttons und der TabPage eine Trennlinie angezeigt werden soll.

Mit SetViewWindow() und SetViewAlign() kann ein Control gesetzt werden,
welches als Preview-Window oder fuer die Anzeige von schoenen Bitmaps
genutzt werden kann.

--------------------------------------------------------------------------

Der ActivatePage()-Handler wird gerufen, wenn eine neue TabPages
angezeigt wird. In diesem Handler kann beispielsweise die neue
TabPage erzeugt werden, wenn diese zu diesem Zeitpunkt noch nicht
erzeugt wurde. Der Handler kann auch als Link gesetzt werden. Mit
GetCurLevel() kann die aktuelle ebene abgefragt werden, wobei
Level 0 die erste Seite ist.

Der DeactivatePage()-Handler wird gerufen, wenn eine neue TabPage
angezeigt werden soll. In diesem Handler kann noch eine Fehler-
ueberprufung stattfinden und das Umschalten gegebenenfalls verhindert
werden, indem sal_False zurueckgegeben wird. Der Handler kann auch als
Link gesetzt werden. Die Defaultimplementierung ruft den Link und
gibt den Rueckgabewert des Links zurueck und wenn kein Link gesetzt
ist, wird sal_True zurueckgegeben.

--------------------------------------------------------------------------

Beispiel:

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

// Show line between Buttons and Page
ShowButtonFixedLine( sal_True );

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

IMPL_LINK( MyWizardDlg, ImplPrevHdl, PushButton*, pBtn )
{
    ShowPrevPage();
    if ( !GetCurLevel() )
        pBtn->Disable();
    return 0;
}

IMPL_LINK( MyWizardDlg, ImplNextHdl, PushButton*, pBtn )
{
    ShowNextPage();
    if ( GetCurLevel() < 3 )
        pBtn->Disable();
    return 0;
}

*************************************************************************/


// - WizardDialog-Types -


#define WIZARDDIALOG_BUTTON_STDOFFSET_X         6
#define WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X    3


// - WizardDialog -


class SVT_DLLPUBLIC WizardDialog : public ModalDialog
{
private:
    Timer               maWizardLayoutTimer;
    Size                maPageSize;
    ImplWizPageData*    mpFirstPage;
    ImplWizButtonData*  mpFirstBtn;
    FixedLine*          mpFixedLine;
    TabPage*            mpCurTabPage;
    PushButton*         mpPrevBtn;
    PushButton*         mpNextBtn;
    Window*             mpViewWindow;
    sal_uInt16              mnCurLevel;
    WindowAlign         meViewAlign;
    Link                maActivateHdl;
    Link                maDeactivateHdl;
    sal_Int16           mnLeftAlignCount;
    bool                mbEmptyViewMargin;

    DECL_DLLPRIVATE_LINK( ImplHandleWizardLayoutTimerHdl, void* );
    bool hasWizardPendingLayout() const;

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
    WizardDialog( Window* pParent, WinBits nStyle = WB_STDTABDIALOG );
    WizardDialog( Window* pParent, const ResId& rResId );
    ~WizardDialog();

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual bool        Notify( NotifyEvent& rNEvt );

    virtual void        ActivatePage();
    virtual long        DeactivatePage();

    virtual void        queue_resize();

    sal_Bool            ShowPrevPage();
    sal_Bool            ShowNextPage();
    sal_Bool            ShowPage( sal_uInt16 nLevel );
    sal_Bool            Finnish( long nResult = 0 );
    sal_uInt16          GetCurLevel() const { return mnCurLevel; }

    void                AddPage( TabPage* pPage );
    void                RemovePage( TabPage* pPage );
    void                SetPage( sal_uInt16 nLevel, TabPage* pPage );
    TabPage*            GetPage( sal_uInt16 nLevel ) const;

    void                AddButton( Button* pButton, long nOffset = 0 );
    void                RemoveButton( Button* pButton );

    void                SetPrevButton( PushButton* pButton ) { mpPrevBtn = pButton; }
    PushButton*         GetPrevButton() const { return mpPrevBtn; }
    void                SetNextButton( PushButton* pButton ) { mpNextBtn = pButton; }
    PushButton*         GetNextButton() const { return mpNextBtn; }

    void                ShowButtonFixedLine( sal_Bool bVisible );

    void                SetViewWindow( Window* pWindow ) { mpViewWindow = pWindow; }
    Window*             GetViewWindow() const { return mpViewWindow; }
    void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }
    WindowAlign         GetViewAlign() const { return meViewAlign; }

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
    const Size&         GetPageSizePixel() const { return maPageSize; }

    void                SetActivatePageHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&         GetActivatePageHdl() const { return maActivateHdl; }
    void                SetDeactivatePageHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&         GetDeactivatePageHdl() const { return maDeactivateHdl; }
};

#endif // INCLUDED_SVTOOLS_WIZDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
