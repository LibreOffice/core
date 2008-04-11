/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: srchdlg.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SFX_SRCHDLG_HXX_
#define _SFX_SRCHDLG_HXX_

#ifndef _VCL_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/combobox.hxx>
#ifndef _VCL_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _VCL_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#include <sfx2/basedlgs.hxx>

// ============================================================================

namespace sfx2 {

// ============================================================================
// SearchDialog
// ============================================================================

class SearchDialog : public ModelessDialog
{
private:
    FixedText           m_aSearchLabel;
    ComboBox            m_aSearchEdit;
    CheckBox            m_aWholeWordsBox;
    CheckBox            m_aMatchCaseBox;
    CheckBox            m_aWrapAroundBox;
    CheckBox            m_aBackwardsBox;
    PushButton          m_aFindBtn;
    CancelButton        m_aCancelBtn;

    Link                m_aFindHdl;
    Link                m_aCloseHdl;

    String              m_sToggleText;
    ::rtl::OUString     m_sConfigName;
    ByteString          m_sWinState;

    bool                m_bIsConstructed;

    void                LoadConfig();
    void                SaveConfig();

    DECL_LINK(          FindHdl, PushButton* );
    DECL_LINK(          ToggleHdl, CheckBox* );

public:
    SearchDialog( Window* pWindow, const ::rtl::OUString& rConfigName );
    ~SearchDialog();

    inline void         SetFindHdl( const Link& rLink ) { m_aFindHdl = rLink; }
    inline void         SetCloseHdl( const Link& rLink ) { m_aCloseHdl = rLink; }

    inline String       GetSearchText() const { return m_aSearchEdit.GetText(); }
    inline void         SetSearchText( const String& _rText ) { m_aSearchEdit.SetText( _rText ); }
    inline bool         IsOnlyWholeWords() const { return ( m_aWholeWordsBox.IsChecked() != FALSE ); }
    inline bool         IsMarchCase() const { return ( m_aMatchCaseBox.IsChecked() != FALSE ); }
    inline bool         IsWrapAround() const { return ( m_aWrapAroundBox.IsChecked() != FALSE ); }
    inline bool         IsSearchBackwards() const { return ( m_aBackwardsBox.IsChecked() != FALSE ); }

    void                SetFocusOnEdit();

    virtual BOOL        Close();
    virtual void        Move();
    virtual void        StateChanged( StateChangedType nStateChange );
};

// ============================================================================

} // namespace sfx2

// ============================================================================

#endif // _SFX_SRCHDLG_HXX_

