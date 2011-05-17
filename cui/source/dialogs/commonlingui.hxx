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

#ifndef SVX_COMMON_LINGUI_HXX
#define SVX_COMMON_LINGUI_HXX

#include <vcl/ctrl.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/edit.hxx>
#include <svtools/svxbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/dialog.hxx>

//=============================================================================
// SvxClickInfoCtr
//=============================================================================

class SvxClickInfoCtr: public Control
{
private:
    FixedInfo                   aFixedInfo;
    Link                        aActivateLink;

public:
    SvxClickInfoCtr( Window* pParent, const ResId& rResId );
    ~SvxClickInfoCtr();

    virtual void        SetText( const XubString& rStr );
    virtual XubString   GetText() const;

    void                SetActivateHdl( const Link& rLink ) { aActivateLink = rLink; }
    const Link&         GetActivateHdl() const { return aActivateLink; }

protected:
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );

};

//=============================================================================
// SvxCommonLinguisticControl
//=============================================================================
class SvxCommonLinguisticControl : public Window
{
public:
    enum ButtonType
    {
        eClose,
        eIgnore,
        eIgnoreAll,
        eChange,
        eChangeAll,
        eOptions
    };

protected:
    FixedText       aWordText;
    SvxClickInfoCtr aAktWord;

    FixedText       aNewWord;
    Edit            aNewWordED;

    FixedText       aSuggestionFT;

    PushButton      aIgnoreBtn;
    PushButton      aIgnoreAllBtn;
    PushButton      aChangeBtn;
    PushButton      aChangeAllBtn;
    PushButton      aOptionsBtn;

    FixedInfo       aStatusText;
    HelpButton      aHelpBtn;
    CancelButton    aCancelBtn;

    GroupBox        aAuditBox;

protected:
    virtual void    Paint( const Rectangle& rRect );

private:
    PushButton* implGetButton( ButtonType _eType  ) const;

public:
    SvxCommonLinguisticControl( ModalDialog* _pParent );

    // handlers
    inline void         SetResetWordHdl( const Link& _rLink )   { aAktWord.SetActivateHdl( _rLink ); }
    inline const Link&  GetResetWordHdl() const                 { return aAktWord.GetActivateHdl(); }

    void    SetButtonHandler( ButtonType _eType, const Link& _rHandler );
    void    EnableButton( ButtonType _eType, sal_Bool _bEnable );

    inline  PushButton*         GetButton( ButtonType _eType )          { return implGetButton( _eType  ); }
    inline  const PushButton*   GetButton( ButtonType _eType ) const    { return implGetButton( _eType  ); }

    // users of this class may want to insert own controls in some places, where the ordinary
    // Z-Order determined by construction time is not sufficient
    // Use the following methods for this
    enum ControlGroup // control groups in this window which cannot be devided (e.g. are adjacent in the Z order)
    {
        eLeftRightWords,    // the controls for the two words (original and suggestion), including the labels
        eSuggestionLabel,   // the label for the suggestion
        eActionButtons,     // the group of "ignore(all)" / "change(all)" buttons
        eDialogButtons      // the group of dialog control buttons (help and close)
    };
    void    InsertControlGroup( Window& _rFirstGroupWindow, Window& _rLastGroupWindow, ControlGroup _eInsertAfter );

    /** enlarges the window

        Some controls "stick" to the borders: The group of change/ignore buttons, for instance, sticks
        to the right, the dictionary list as well as the close/help buttons stick to the bottom of the
        window.
    */
    void    Enlarge( sal_Int32 _nX, sal_Int32 _nY );

    // control access methods
    inline void     SetCurrentText( const String& _rText )  { aAktWord.SetText( _rText ); }
    inline String   GetCurrentText( ) const                 { return aAktWord.GetText(); }

    inline void     SetStatusText( const String& _rText )   { aStatusText.SetText( _rText ); }
    inline String   GetStatusText( ) const                  { return aStatusText.GetText(); }

    inline Edit&        GetWordInputControl()           { return aNewWordED; }
    inline const Edit&  GetWordInputControl() const { return aNewWordED; }

    // returns the location (upper-left corner) of the group of action buttons
    inline Point    GetActionButtonsLocation( ) const { return aIgnoreBtn.GetPosPixel(); }
};



#endif // SVX_COMMON_LINGUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
