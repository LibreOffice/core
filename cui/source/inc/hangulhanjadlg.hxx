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
        virtual ~SuggestionSet();
        virtual void dispose() override;

        virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
        void            ClearSet();
    };

    class SuggestionDisplay : public Control
    {
    public:
        SuggestionDisplay( vcl::Window* pParent, WinBits nBits );
        virtual ~SuggestionDisplay();
        virtual void dispose() override;

        void DisplayListBox( bool bDisplayListBox );

        void SetSelectHdl( const Link<SuggestionDisplay&,void>& rLink );

        void Clear();
        void InsertEntry( const OUString& rStr );
        void SelectEntryPos( sal_uInt16 nPos );

        sal_uInt16 GetEntryCount() const;

        OUString GetEntry( sal_uInt16 nPos ) const;
        OUString GetSelectEntry() const;

        virtual void StateChanged( StateChangedType nStateChange ) override;

        virtual void KeyInput( const KeyEvent& rKEvt ) override;
        virtual void KeyUp( const KeyEvent& rKEvt ) override;
        virtual void Activate() override;
        virtual void Deactivate() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual void Command( const CommandEvent& rCEvt ) override;

        DECL_LINK_TYPED( SelectSuggestionListBoxHdl, ListBox&, void );
        DECL_LINK_TYPED( SelectSuggestionValueSetHdl, ValueSet*, void );
        void SelectSuggestionHdl(Control*);

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
        virtual ~HangulHanjaConversionDialog();
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
                    bool _bOriginatesFromDocument = true
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
        DECL_LINK_TYPED( OnOption, Button*, void );
        DECL_LINK( OnSuggestionModified, void* );
        DECL_LINK_TYPED( OnSuggestionSelected, SuggestionDisplay&, void );
        DECL_LINK_TYPED( OnConversionDirectionClicked, Button*, void );
        DECL_LINK_TYPED( ClickByCharacterHdl, Button*, void );

        /// fill the suggestion list box with suggestions for the actual input
        void FillSuggestions( const css::uno::Sequence< OUString >& _rSuggestions );
    };


    typedef ::std::vector< css::uno::Reference< css::linguistic2::XConversionDictionary > > HHDictList;

    class HangulHanjaOptionsDialog : public ModalDialog
    {
    private:
        VclPtr<SvxCheckListBox>    m_pDictsLB;
        VclPtr<CheckBox>           m_pIgnorepostCB;
        VclPtr<CheckBox>           m_pShowrecentlyfirstCB;
        VclPtr<CheckBox>           m_pAutoreplaceuniqueCB;
        VclPtr<PushButton>         m_pNewPB;
        VclPtr<PushButton>         m_pEditPB;
        VclPtr<PushButton>         m_pDeletePB;
        VclPtr<OKButton>           m_pOkPB;

        SvLBoxButtonData*   m_pCheckButtonData;

        HHDictList          m_aDictList;
        css::uno::Reference< css::linguistic2::XConversionDictionaryList > m_xConversionDictionaryList;

        DECL_LINK_TYPED( OkHdl, Button*, void );
        DECL_LINK_TYPED( DictsLB_SelectHdl, SvTreeListBox*, void );
        DECL_LINK_TYPED( NewDictHdl, Button*, void );
        DECL_LINK_TYPED( EditDictHdl, Button*, void );
        DECL_LINK_TYPED( DeleteDictHdl, Button*, void );

        void                Init();       ///< reads settings from core and init controls
    public:
                            HangulHanjaOptionsDialog( vcl::Window* _pParent );
        virtual             ~HangulHanjaOptionsDialog();
        virtual void        dispose() override;

        void                AddDict( const OUString& _rName, bool _bChecked );
    };


    class HangulHanjaNewDictDialog : public ModalDialog
    {
    private:
        VclPtr<Edit> m_pDictNameED;
        VclPtr<OKButton> m_pOkBtn;

        bool m_bEntered;

        DECL_LINK_TYPED( OKHdl, Button*, void );
        DECL_LINK( ModifyHdl, void* );
    public:
        HangulHanjaNewDictDialog( vcl::Window* _pParent );
        virtual ~HangulHanjaNewDictDialog();
        virtual void dispose() override;

        bool GetName( OUString& _rRetName ) const;
    };


    class SuggestionList;

    class SuggestionEdit : public Edit
    {
    private:
        VclPtr<SuggestionEdit>     m_pPrev;
        VclPtr<SuggestionEdit>     m_pNext;
        VclPtr<ScrollBar>          m_pScrollBar;

        bool                ShouldScroll( bool _bUp ) const;
        void                DoJump( bool _bUp );
    public:
                            SuggestionEdit( vcl::Window* pParent, WinBits nBits );
        virtual             ~SuggestionEdit();
        virtual void        dispose() override;
        virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
        void init( ScrollBar* pScrollBar, SuggestionEdit* pPrev, SuggestionEdit* pNext);
    };


    class HangulHanjaEditDictDialog : public ModalDialog
    {
    private:
        const OUString  m_aEditHintText;
        HHDictList&     m_rDictList;
        sal_uInt32      m_nCurrentDict;

        OUString        m_aOriginal;
        SuggestionList* m_pSuggestions;

        VclPtr<ListBox>        m_aBookLB;
        VclPtr<ComboBox>       m_aOriginalLB;
        VclPtr<SuggestionEdit> m_aEdit1;
        VclPtr<SuggestionEdit> m_aEdit2;
        VclPtr<SuggestionEdit> m_aEdit3;
        VclPtr<SuggestionEdit> m_aEdit4;
        VclPtr<ScrollBar>      m_aScrollSB;
        VclPtr<PushButton>     m_aNewPB;
        VclPtr<PushButton>     m_aDeletePB;

        sal_uInt16      m_nTopPos;
        bool            m_bModifiedSuggestions;
        bool            m_bModifiedOriginal;

        DECL_LINK( OriginalModifyHdl, void* );
        DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );
        DECL_LINK( EditModifyHdl1, Edit* );
        DECL_LINK( EditModifyHdl2, Edit* );
        DECL_LINK( EditModifyHdl3, Edit* );
        DECL_LINK( EditModifyHdl4, Edit* );

        DECL_LINK_TYPED( BookLBSelectHdl, ListBox&, void );
        DECL_LINK_TYPED( NewPBPushHdl, Button*, void );
        DECL_LINK_TYPED( DeletePBPushHdl, Button*, void );

        void            InitEditDictDialog( sal_uInt32 _nSelDict );
        void            UpdateOriginalLB();
        void            UpdateSuggestions();
        void            UpdateButtonStates();

        void            SetEditText( Edit& _rEdit, sal_uInt16 _nEntryNum );
        void            EditModify( Edit* _pEdit, sal_uInt8 _nEntryOffset );

        bool            DeleteEntryFromDictionary( const OUString& rEntry, const css::uno::Reference< css::linguistic2::XConversionDictionary >& xDict );

    public:
                        HangulHanjaEditDictDialog( vcl::Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict );
                        virtual ~HangulHanjaEditDictDialog();
        virtual void    dispose() override;

        void            UpdateScrollbar();
    };


}


#endif // SVX_HANGUL_HANJA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
