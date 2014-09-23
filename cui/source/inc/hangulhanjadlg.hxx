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

        virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;
        void            ClearSet();
    };

    class SuggestionDisplay : public Control
    {
    public:
        SuggestionDisplay( vcl::Window* pParent, WinBits nBits );
        virtual ~SuggestionDisplay();

        void DisplayListBox( bool bDisplayListBox );

        void SetSelectHdl( const Link& rLink );

        void Clear();
        void InsertEntry( const OUString& rStr );
        void SelectEntryPos( sal_uInt16 nPos );

        sal_uInt16 GetEntryCount() const;

        OUString GetEntry( sal_uInt16 nPos ) const;
        OUString GetSelectEntry() const;

        virtual void StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;

        virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void KeyUp( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void Activate() SAL_OVERRIDE;
        virtual void Deactivate() SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual void LoseFocus() SAL_OVERRIDE;
        virtual void Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

        DECL_LINK( SelectSuggestionHdl, Control* );

        void SetHelpIds();

    private:
        void implUpdateDisplay();
        Control& implGetCurrentControl();

    private:
        bool          m_bDisplayListBox;//otherwise ValueSet
        SuggestionSet m_aValueSet;
        ListBox       m_aListBox;

        Link          m_aSelectLink;
        bool          m_bInSelectionUpdate;
    };

    class RubyRadioButton;

    //= HangulHanjaConversionDialog

    class HangulHanjaConversionDialog : public  ModalDialog
    {
    private:

        PushButton*         m_pFind;
        PushButton*         m_pIgnore;
        PushButton*         m_pIgnoreAll;
        PushButton*         m_pReplace;
        PushButton*         m_pReplaceAll;
        PushButton*         m_pOptions;
        SuggestionDisplay*  m_pSuggestions;
        RadioButton*        m_pSimpleConversion;
        RadioButton*        m_pHangulBracketed;
        RadioButton*        m_pHanjaBracketed;
        Edit*               m_pWordInput;
        FixedText*          m_pOriginalWord;
        RubyRadioButton*    m_pHanjaAbove;
        RubyRadioButton*    m_pHanjaBelow;
        RubyRadioButton*    m_pHangulAbove;
        RubyRadioButton*    m_pHangulBelow;
        CheckBox*           m_pHangulOnly;
        CheckBox*           m_pHanjaOnly;
        CheckBox*           m_pReplaceByChar;

        CheckBox*       m_pIgnoreNonPrimary;
        /** are we working for a document? This is normally true, but in case
            the user uses the "find" functionality, we switch to working
            with what the user entered, which then does not have any relation to
            the document anymore. Some functionality must be disabled then */
        bool            m_bDocumentMode;

        Link            m_aOptionsChangedLink;
        Link            m_aClickByCharacterLink;

    public:
        HangulHanjaConversionDialog(
                vcl::Window* _pParent,
                editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection );
        virtual ~HangulHanjaConversionDialog( );

    public:
        void    SetOptionsChangedHdl( const Link& _rHdl );
        void    SetIgnoreHdl( const Link& _rHdl );
        void    SetIgnoreAllHdl( const Link& _rHdl );
        void    SetChangeHdl( const Link& _rHdl );
        void    SetChangeAllHdl( const Link& _rHdl );

        void    SetClickByCharacterHdl( const Link& _rHdl );
        void    SetConversionFormatChangedHdl( const Link& _rHdl );
        void    SetFindHdl( const Link& _rHdl );

        OUString  GetCurrentString( ) const;
        void    SetCurrentString(
                    const OUString& _rNewString,
                    const ::com::sun::star::uno::Sequence< OUString >& _rSuggestions,
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
        DECL_LINK( OnClose, void* );
        DECL_LINK( OnOption, void* );
        DECL_LINK( OnSuggestionModified, void* );
        DECL_LINK( OnSuggestionSelected, void* );
        DECL_LINK( OnConversionDirectionClicked, CheckBox* );
        DECL_LINK( ClickByCharacterHdl, CheckBox* );

        /// fill the suggestion list box with suggestions for the actual input
        void FillSuggestions( const ::com::sun::star::uno::Sequence< OUString >& _rSuggestions );
    };


    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > > HHDictList;

    class HangulHanjaOptionsDialog : public ModalDialog
    {
    private:
        SvxCheckListBox*    m_pDictsLB;
        CheckBox*           m_pIgnorepostCB;
        CheckBox*           m_pShowrecentlyfirstCB;
        CheckBox*           m_pAutoreplaceuniqueCB;
        PushButton*         m_pNewPB;
        PushButton*         m_pEditPB;
        PushButton*         m_pDeletePB;
        OKButton*           m_pOkPB;

        SvLBoxButtonData*   m_pCheckButtonData;

        HHDictList          m_aDictList;
        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionaryList > m_xConversionDictionaryList;

        DECL_LINK( OkHdl, void* );
        DECL_LINK( OnNew, void* );
        DECL_LINK( OnEdit, void* );
        DECL_LINK( OnDelete, void* );
        DECL_LINK( DictsLB_SelectHdl, void* );
        DECL_LINK( NewDictHdl, void* );
        DECL_LINK( EditDictHdl, void* );
        DECL_LINK( DeleteDictHdl, void* );

        void                Init( void );       ///< reads settings from core and init controls
    public:
                            HangulHanjaOptionsDialog( vcl::Window* _pParent );
        virtual             ~HangulHanjaOptionsDialog();

        void                AddDict( const OUString& _rName, bool _bChecked );
    };


    class HangulHanjaNewDictDialog : public ModalDialog
    {
    private:
        Edit* m_pDictNameED;
        OKButton* m_pOkBtn;

        bool m_bEntered;

        DECL_LINK( OKHdl, void* );
        DECL_LINK( ModifyHdl, void* );
    public:
        HangulHanjaNewDictDialog( vcl::Window* _pParent );

        bool GetName( OUString& _rRetName ) const;
    };


    class SuggestionList;

    class SuggestionEdit : public Edit
    {
    private:
        SuggestionEdit*     m_pPrev;
        SuggestionEdit*     m_pNext;
        ScrollBar*          m_pScrollBar;

        bool                ShouldScroll( bool _bUp ) const;
        void                DoJump( bool _bUp );
    public:
                            SuggestionEdit( vcl::Window* pParent, WinBits nBits );
        virtual             ~SuggestionEdit();
        virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
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

        ListBox*        m_aBookLB;
        ComboBox*       m_aOriginalLB;
        SuggestionEdit* m_aEdit1;
        SuggestionEdit* m_aEdit2;
        SuggestionEdit* m_aEdit3;
        SuggestionEdit* m_aEdit4;
        ScrollBar*      m_aScrollSB;
        PushButton*     m_aNewPB;
        PushButton*     m_aDeletePB;

        sal_uInt16      m_nTopPos;
        bool            m_bModifiedSuggestions;
        bool            m_bModifiedOriginal;

        DECL_LINK( OriginalModifyHdl, void* );
        DECL_LINK( ScrollHdl, void* );
        DECL_LINK( EditModifyHdl1, Edit* );
        DECL_LINK( EditModifyHdl2, Edit* );
        DECL_LINK( EditModifyHdl3, Edit* );
        DECL_LINK( EditModifyHdl4, Edit* );

        DECL_LINK( BookLBSelectHdl, void* );
        DECL_LINK( NewPBPushHdl, void* );
        DECL_LINK( DeletePBPushHdl, void* );

        void            InitEditDictDialog( sal_uInt32 _nSelDict );
        void            UpdateOriginalLB( void );
        void            UpdateSuggestions( void );
        void            UpdateButtonStates();

        void            SetEditText( Edit& _rEdit, sal_uInt16 _nEntryNum );
        void            EditModify( Edit* _pEdit, sal_uInt8 _nEntryOffset );
        void            EditFocusLost( Edit* _pEdit, sal_uInt8 _nEntryOffset );

        bool            DeleteEntryFromDictionary( const OUString& rEntry, const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary >& xDict );

    public:
                        HangulHanjaEditDictDialog( vcl::Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict );
                        virtual ~HangulHanjaEditDictDialog();

        void            UpdateScrollbar( void );
    };


}   // namespace svx


#endif // SVX_HANGUL_HANJA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
