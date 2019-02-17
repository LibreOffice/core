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
#ifndef INCLUDED_CUI_SOURCE_INC_HANGULHANJADLG_HXX
#define INCLUDED_CUI_SOURCE_INC_HANGULHANJADLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/weld.hxx>
#include <svx/checklbx.hxx>
#include <editeng/hangulhanja.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <svtools/valueset.hxx>

#include <vector>
#include <memory>

namespace svx
{

    class SuggestionSet : public ValueSet
    {
    public:
        SuggestionSet( vcl::Window* pParent );
        virtual ~SuggestionSet() override;
        virtual void dispose() override;

        virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
        void            ClearSet();
    };

    class SuggestionDisplay : public Control
    {
    public:
        SuggestionDisplay( vcl::Window* pParent, WinBits nBits );
        virtual ~SuggestionDisplay() override;
        virtual void dispose() override;

        void DisplayListBox( bool bDisplayListBox );

        void SetSelectHdl( const Link<SuggestionDisplay&,void>& rLink );

        void Clear();
        void InsertEntry( const OUString& rStr );
        void SelectEntryPos( sal_uInt16 nPos );

        sal_uInt16 GetEntryCount() const;

        OUString GetEntry( sal_uInt16 nPos ) const;
        OUString GetSelectedEntry() const;

        virtual void StateChanged( StateChangedType nStateChange ) override;

        virtual void KeyInput( const KeyEvent& rKEvt ) override;
        virtual void KeyUp( const KeyEvent& rKEvt ) override;
        virtual void Activate() override;
        virtual void Deactivate() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual void Command( const CommandEvent& rCEvt ) override;

        DECL_LINK( SelectSuggestionListBoxHdl, ListBox&, void );
        DECL_LINK( SelectSuggestionValueSetHdl, ValueSet*, void );
        void SelectSuggestionHdl(Control const *);

        void SetHelpIds();

    private:
        void implUpdateDisplay();
        bool hasCurrentControl();
        Control& implGetCurrentControl();

    private:
        bool          m_bDisplayListBox;//otherwise ValueSet
        VclPtr<SuggestionSet> m_aValueSet;
        VclPtr<ListBox>  m_aListBox;

        Link<SuggestionDisplay&,void> m_aSelectLink;
        bool          m_bInSelectionUpdate;
    };

    class RubyRadioButton;

    class HangulHanjaConversionDialog : public  ModalDialog
    {
    private:

        VclPtr<PushButton>         m_pFind;
        VclPtr<PushButton>         m_pIgnore;
        VclPtr<PushButton>         m_pIgnoreAll;
        VclPtr<PushButton>         m_pReplace;
        VclPtr<PushButton>         m_pReplaceAll;
        VclPtr<PushButton>         m_pOptions;
        VclPtr<SuggestionDisplay>  m_pSuggestions;
        VclPtr<RadioButton>        m_pSimpleConversion;
        VclPtr<RadioButton>        m_pHangulBracketed;
        VclPtr<RadioButton>        m_pHanjaBracketed;
        VclPtr<Edit>               m_pWordInput;
        VclPtr<FixedText>          m_pOriginalWord;
        VclPtr<RubyRadioButton>    m_pHanjaAbove;
        VclPtr<RubyRadioButton>    m_pHanjaBelow;
        VclPtr<RubyRadioButton>    m_pHangulAbove;
        VclPtr<RubyRadioButton>    m_pHangulBelow;
        VclPtr<CheckBox>           m_pHangulOnly;
        VclPtr<CheckBox>           m_pHanjaOnly;
        VclPtr<CheckBox>           m_pReplaceByChar;

        VclPtr<CheckBox>           m_pIgnoreNonPrimary;
        /** are we working for a document? This is normally true, but in case
            the user uses the "find" functionality, we switch to working
            with what the user entered, which then does not have any relation to
            the document anymore. Some functionality must be disabled then */
        bool                       m_bDocumentMode;

        Link<LinkParamNone*,void>  m_aOptionsChangedLink;
        Link<CheckBox*,void>       m_aClickByCharacterLink;

    public:
        HangulHanjaConversionDialog(
                vcl::Window* _pParent,
                editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection );
        virtual ~HangulHanjaConversionDialog() override;
        virtual void dispose() override;

    public:
        void    SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl );
        void    SetIgnoreHdl( const Link<Button*,void>& _rHdl );
        void    SetIgnoreAllHdl( const Link<Button*,void>& _rHdl );
        void    SetChangeHdl( const Link<Button*,void>& _rHdl );
        void    SetChangeAllHdl( const Link<Button*,void>& _rHdl );

        void    SetClickByCharacterHdl( const Link<CheckBox*,void>& _rHdl );
        void    SetConversionFormatChangedHdl( const Link<Button*,void>& _rHdl );
        void    SetFindHdl( const Link<Button*,void>& _rHdl );

        OUString  GetCurrentString( ) const;
        void    SetCurrentString(
                    const OUString& _rNewString,
                    const css::uno::Sequence< OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument
                );

        void    FocusSuggestion( );

        /// retrieves the current suggestion
        OUString  GetCurrentSuggestion( ) const;

        void        SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType );
        editeng::HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const;

        void            SetByCharacter( bool _bByCharacter );
        void            SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection );

        /// should text which does not match the primary conversion direction be ignored?
        bool            GetUseBothDirections( ) const;

        /** get current conversion direction to use
            (return argument if GetUseBothDirections is true) */
        editeng::HangulHanjaConversion::ConversionDirection GetDirection( editeng::HangulHanjaConversion::ConversionDirection eDefaultDirection ) const;

        /// enables or disables the checkboxes for ruby formatted replacements
        void            EnableRubySupport( bool bVal );

    private:
        DECL_LINK( OnOption, Button*, void );
        DECL_LINK( OnSuggestionModified, Edit&, void );
        DECL_LINK( OnSuggestionSelected, SuggestionDisplay&, void );
        DECL_LINK( OnConversionDirectionClicked, Button*, void );
        DECL_LINK( ClickByCharacterHdl, Button*, void );

        /// fill the suggestion list box with suggestions for the actual input
        void FillSuggestions( const css::uno::Sequence< OUString >& _rSuggestions );
    };


    typedef std::vector< css::uno::Reference< css::linguistic2::XConversionDictionary > > HHDictList;

    class HangulHanjaOptionsDialog : public weld::GenericDialogController
    {
    private:
        HHDictList          m_aDictList;
        css::uno::Reference< css::linguistic2::XConversionDictionaryList > m_xConversionDictionaryList;

        std::unique_ptr<weld::TreeView> m_xDictsLB;
        std::unique_ptr<weld::CheckButton> m_xIgnorepostCB;
        std::unique_ptr<weld::CheckButton> m_xShowrecentlyfirstCB;
        std::unique_ptr<weld::CheckButton> m_xAutoreplaceuniqueCB;
        std::unique_ptr<weld::Button> m_xNewPB;
        std::unique_ptr<weld::Button> m_xEditPB;
        std::unique_ptr<weld::Button> m_xDeletePB;
        std::unique_ptr<weld::Button> m_xOkPB;

        DECL_LINK( OkHdl, weld::Button&, void );
        DECL_LINK( DictsLB_SelectHdl, weld::TreeView&, void );
        DECL_LINK( NewDictHdl, weld::Button&, void );
        DECL_LINK( EditDictHdl, weld::Button&, void );
        DECL_LINK( DeleteDictHdl, weld::Button&, void );

        void                Init();       ///< reads settings from core and init controls
    public:
        HangulHanjaOptionsDialog(weld::Window* pParent);
        virtual ~HangulHanjaOptionsDialog() override;

        void                AddDict( const OUString& _rName, bool _bChecked );
    };

    class HangulHanjaNewDictDialog : public weld::GenericDialogController
    {
    private:
        bool m_bEntered;

        std::unique_ptr<weld::Button> m_xOkBtn;
        std::unique_ptr<weld::Entry> m_xDictNameED;

        DECL_LINK(OKHdl, weld::Button&, void);
        DECL_LINK(ModifyHdl, weld::Entry&, void);
    public:
        HangulHanjaNewDictDialog(weld::Window* pParent);
        virtual ~HangulHanjaNewDictDialog() override;

        bool GetName( OUString& _rRetName ) const;
    };

    class SuggestionList;
    class HangulHanjaEditDictDialog;

    class SuggestionEdit
    {
    private:
        HangulHanjaEditDictDialog* m_pParent;
        SuggestionEdit*     m_pPrev;
        SuggestionEdit*     m_pNext;
        weld::ScrolledWindow* m_pScrollBar;
        std::unique_ptr<weld::Entry> m_xEntry;

        bool                ShouldScroll( bool _bUp ) const;
        void                DoJump( bool _bUp );
    public:
        SuggestionEdit(std::unique_ptr<weld::Entry> xEntry, HangulHanjaEditDictDialog* pParent);
        DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
        void init(weld::ScrolledWindow* pScrollBar, SuggestionEdit* pPrev, SuggestionEdit* pNext);

        void grab_focus() { m_xEntry->grab_focus(); }
        OUString get_text() const { return m_xEntry->get_text(); }
        void set_text(const OUString& rText) { m_xEntry->set_text(rText); }
        void connect_changed(const Link<weld::Entry&, void>& rLink) { m_xEntry->connect_changed(rLink); }
    };

    class HangulHanjaEditDictDialog : public weld::GenericDialogController
    {
    private:
        const OUString  m_aEditHintText;
        HHDictList&     m_rDictList;
        sal_uInt32      m_nCurrentDict;

        OUString        m_aOriginal;
        std::unique_ptr<SuggestionList> m_pSuggestions;

        sal_uInt16      m_nTopPos;
        bool            m_bModifiedSuggestions;
        bool            m_bModifiedOriginal;

        std::unique_ptr<weld::ComboBox> m_xBookLB;
        std::unique_ptr<weld::ComboBox> m_xOriginalLB;
        std::unique_ptr<SuggestionEdit> m_xEdit1;
        std::unique_ptr<SuggestionEdit> m_xEdit2;
        std::unique_ptr<SuggestionEdit> m_xEdit3;
        std::unique_ptr<SuggestionEdit> m_xEdit4;
        std::unique_ptr<weld::Widget> m_xContents;
        std::unique_ptr<weld::ScrolledWindow> m_xScrollSB;
        std::unique_ptr<weld::Button> m_xNewPB;
        std::unique_ptr<weld::Button> m_xDeletePB;

        DECL_LINK( OriginalModifyHdl, weld::ComboBox&, void );
        DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );
        DECL_LINK( EditModifyHdl1, weld::Entry&, void );
        DECL_LINK( EditModifyHdl2, weld::Entry&, void );
        DECL_LINK( EditModifyHdl3, weld::Entry&, void );
        DECL_LINK( EditModifyHdl4, weld::Entry&, void );

        DECL_LINK( BookLBSelectHdl, weld::ComboBox&, void );
        DECL_LINK( NewPBPushHdl, weld::Button&, void );
        DECL_LINK( DeletePBPushHdl, weld::Button&, void );

        void            InitEditDictDialog(sal_uInt32 nSelDict);
        void            UpdateOriginalLB();
        void            UpdateSuggestions();
        void            UpdateButtonStates();

        void            SetEditText( SuggestionEdit& rEdit, sal_uInt16 nEntryNum );
        void            EditModify( const weld::Entry* pEdit, sal_uInt8 nEntryOffset );

        bool            DeleteEntryFromDictionary( const css::uno::Reference< css::linguistic2::XConversionDictionary >& xDict );

    public:
        HangulHanjaEditDictDialog(weld::Window* pParent, HHDictList& rDictList, sal_uInt32 nSelDict);
        virtual ~HangulHanjaEditDictDialog() override;

        void            UpdateScrollbar();
    };
}

#endif // SVX_HANGUL_HANJA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
