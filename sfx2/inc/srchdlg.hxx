/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srchdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:19:20 $
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
#ifndef _SFX_SRCHDLG_HXX_
#define _SFX_SRCHDLG_HXX_

#ifndef _VCL_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _VCL_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _VCL_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include "basedlgs.hxx"
#endif

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

