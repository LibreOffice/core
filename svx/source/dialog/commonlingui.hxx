/*************************************************************************
 *
 *  $RCSfile: commonlingui.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:41:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef SVX_COMMON_LINGUI_HXX
#define SVX_COMMON_LINGUI_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SVX_BOX_HXX
#include "svxbox.hxx"
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

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
        eChangeAll
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

    // updates the help texts for the "change" and "change all" buttons according to the currently
    // entered texts
            void UpdateChangesHelp( const String& _rNewText );
    inline  void UpdateChangesHelp( ) { UpdateChangesHelp( GetWordInputControl().GetText() ); }

    // updates the help texts for the "ignore" and "always ignore" buttons according to the currently
    // entered texts
            void UpdateIgnoreHelp( );

    String          GetNewEditWord();
    void            SetNewEditWord( const String& _rNew );
};



#endif // SVX_COMMON_LINGUI_HXX
