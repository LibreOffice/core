/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    inline bool         IsOnlyWholeWords() const { return ( m_aWholeWordsBox.IsChecked() != sal_False ); }
    inline bool         IsMarchCase() const { return ( m_aMatchCaseBox.IsChecked() != sal_False ); }
    inline bool         IsWrapAround() const { return ( m_aWrapAroundBox.IsChecked() != sal_False ); }
    inline bool         IsSearchBackwards() const { return ( m_aBackwardsBox.IsChecked() != sal_False ); }

    void                SetFocusOnEdit();

    virtual sal_Bool        Close();
    virtual void        Move();
    virtual void        StateChanged( StateChangedType nStateChange );
};

// ============================================================================

} // namespace sfx2

// ============================================================================

#endif // _SFX_SRCHDLG_HXX_

