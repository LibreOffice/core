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

#ifndef INCLUDED_CUI_SOURCE_DIALOGS_COMMONLINGUI_HXX
#define INCLUDED_CUI_SOURCE_DIALOGS_COMMONLINGUI_HXX

#include <vcl/ctrl.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/dialog.hxx>


// SvxClickInfoCtr


class SvxClickInfoCtr: public Control
{
private:
    FixedInfo                   aFixedInfo;
    Link                        aActivateLink;

public:
    SvxClickInfoCtr( Window* pParent, const ResId& rResId );
    ~SvxClickInfoCtr();

    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;

    void                SetActivateHdl( const Link& rLink ) { aActivateLink = rLink; }
    const Link&         GetActivateHdl() const { return aActivateLink; }

protected:
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

};


// SvxCommonLinguisticControl

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
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;

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

    /// control groups in this window which cannot be devided (e.g. are adjacent in the Z order)
    enum ControlGroup
    {
        eLeftRightWords,    ///< the controls for the two words (original and suggestion), including the labels
        eSuggestionLabel,   ///< the label for the suggestion
        eActionButtons,     ///< the group of "ignore(all)" / "change(all)" buttons
        eDialogButtons      ///< the group of dialog control buttons (help and close)
    };

    /** Users of this class may want to insert own controls in some places,
        where the ordinary Z-Order determined by construction time is not
        sufficient. Use the following methods for this.
     */
    void    InsertControlGroup( Window& _rFirstGroupWindow, Window& _rLastGroupWindow, ControlGroup _eInsertAfter );

    /** enlarges the window

        Some controls "stick" to the borders: The group of change/ignore buttons, for instance, sticks
        to the right, the dictionary list as well as the close/help buttons stick to the bottom of the
        window.
    */
    void    Enlarge( sal_Int32 _nX, sal_Int32 _nY );

    // control access methods
    inline void     SetCurrentText( const OUString& _rText )  { aAktWord.SetText( _rText ); }
    inline OUString GetCurrentText( ) const                 { return aAktWord.GetText(); }

    inline void     SetStatusText( const OUString& _rText )   { aStatusText.SetText( _rText ); }
    inline OUString GetStatusText( ) const                  { return aStatusText.GetText(); }

    inline Edit&        GetWordInputControl()           { return aNewWordED; }
    inline const Edit&  GetWordInputControl() const { return aNewWordED; }

    /// @return the location (upper-left corner) of the group of action buttons
    inline Point    GetActionButtonsLocation( ) const { return aIgnoreBtn.GetPosPixel(); }
};



#endif // INCLUDED_CUI_SOURCE_DIALOGS_COMMONLINGUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
