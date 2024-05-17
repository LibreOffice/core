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

#pragma once

#include <memory>
#include <svtools/svtdllapi.h>
#include <tools/ref.hxx>

#include <svtools/brwbox.hxx>
#include <svtools/brwhead.hxx>
#include <tools/lineend.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/weldutils.hxx>
#include <o3tl/typed_flags_set.hxx>

class BrowserDataWin;

// EditBrowseBoxFlags (EBBF)
enum class EditBrowseBoxFlags
{
    NONE                       = 0x0000,
/** if this bit is _not_ set, the handle column will be invalidated upon
    changing the row in the browse box.  This is for forcing the row picture to
    be repainted. If you do not have row pictures or text, you don't need this
    invalidation, then you would specify this bit to prevent flicker
*/
    NO_HANDLE_COLUMN_CONTENT   = 0x0001,
/** set this bit to activate the cell on a MouseButtonDown, not a MouseButtonUp event
 */
    ACTIVATE_ON_BUTTONDOWN     = 0x0002,
/** if this bit is set and EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT is _not_ set, the handle
    column is drawn with the text contained in column 0 instead of an image
*/
    HANDLE_COLUMN_TEXT         = 0x0004,

/** If this bit is set, tab traveling is somewhat modified<br/>
    If the control gets the focus because the user pressed the TAB key, then the
    first or last cell (depending on whether the traveling was cycling forward or backward)
    gets activated.
    @see Window::GetGetFocusFlags
    @see GETFOCUS_*
*/
    SMART_TAB_TRAVEL           = 0x0008,

};
namespace o3tl
{
    template<> struct typed_flags<EditBrowseBoxFlags> : is_typed_flags<EditBrowseBoxFlags, 0x0f> {};
}

namespace svt
{
    class ControlBase;

    class SVT_DLLPUBLIC CellController : public SvRefBase
    {
        friend class EditBrowseBox;
        Link<LinkParamNone*, void> maModifyHdl;

        VclPtr<ControlBase>        pWindow;
        bool                       bSuspended;     // <true> if the window is hidden and disabled

    public:

        CellController(ControlBase* pW);
        virtual ~CellController() override;

        ControlBase& GetWindow() const { return *const_cast<CellController*>(this)->pWindow; }

        virtual void SaveValue() = 0;
        virtual bool IsValueChangedFromSaved() const = 0;

        // commit any current changes. Especially, do any reformatting you need (from input formatting
        // to output formatting) here
        virtual void CommitModifications();

        // suspending the controller is not cumulative!
                void        suspend( );
                void        resume( );
        bool        isSuspended( ) const { return bSuspended; }

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        void SetModifyHdl(const Link<LinkParamNone*,void>& rLink) { maModifyHdl = rLink; }
        virtual void ActivatingMouseEvent(const BrowserMouseEvent& rEvt, bool bUp);
        virtual void callModifyHdl() { maModifyHdl.Call(nullptr); }
    };

    typedef tools::SvRef<CellController> CellControllerRef;


    //= IEditImplementation

    class SVT_DLLPUBLIC IEditImplementation
    {
    public:
        virtual ~IEditImplementation() = 0;

        virtual ControlBase&        GetControl() = 0;

        virtual OUString            GetText( LineEnd aSeparator ) const = 0;
        virtual void                SetText( const OUString& _rStr ) = 0;

        virtual bool                IsReadOnly() const = 0;
        virtual void                SetReadOnly( bool bReadOnly ) = 0;

        virtual sal_Int32           GetMaxTextLen() const = 0;
        virtual void                SetMaxTextLen( sal_Int32 _nMaxLen ) = 0;

        virtual Selection           GetSelection() const = 0;
        virtual void                SetSelection( const Selection& _rSelection ) = 0;

        virtual void                ReplaceSelected( const OUString& _rStr ) = 0;
        virtual OUString            GetSelected( LineEnd aSeparator ) const = 0;

        virtual bool                IsValueChangedFromSaved() const = 0;
        virtual void                SaveValue() = 0;

        virtual bool                CanUp() const = 0;
        virtual bool                CanDown() const = 0;

        virtual void                Cut() = 0;
        virtual void                Copy() = 0;
        virtual void                Paste() = 0;

        // sets a link to call when the text is changed by the user
        void SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
        {
            m_aModify1Hdl = rLink;
        }

        // sets an additional link to call when the text is changed by the user
        void SetAuxModifyHdl(const Link<LinkParamNone*,void>& rLink)
        {
            m_aModify2Hdl = rLink;
        }

    private:
        Link<LinkParamNone*,void> m_aModify1Hdl;
        Link<LinkParamNone*,void> m_aModify2Hdl;

    protected:
        void CallModifyHdls()
        {
            m_aModify1Hdl.Call(nullptr);
            m_aModify2Hdl.Call(nullptr);
        }
    };

    class SVT_DLLPUBLIC ControlBase : public InterimItemWindow
    {
    public:
        ControlBase(BrowserDataWin* pParent, const OUString& rUIXMLDescription, const OUString& rID);

        virtual void SetEditableReadOnly(bool bReadOnly);

        virtual bool ProcessKey(const KeyEvent& rKEvt);

        virtual void SetPointFont(const vcl::Font& rFont) = 0;

        // chain after the FocusInHdl
        void SetFocusInHdl(const Link<LinkParamNone*,void>& rHdl)
        {
            m_aFocusInHdl = rHdl;
        }

        // chain after the FocusOutHdl
        void SetFocusOutHdl(const Link<LinkParamNone*,void>& rHdl)
        {
            m_aFocusOutHdl = rHdl;
        }

        void SetMousePressHdl(const Link<const MouseEvent&,void>& rHdl)
        {
            m_aMousePressHdl = rHdl;
        }

        void SetMouseReleaseHdl(const Link<const MouseEvent&,void>& rHdl)
        {
            m_aMouseReleaseHdl = rHdl;
        }

        void SetMouseMoveHdl(const Link<const MouseEvent&,void>& rHdl)
        {
            m_aMouseMoveHdl = rHdl;
        }

        void SetKeyInputHdl(const Link<const KeyEvent&,void>& rHdl)
        {
            m_aKeyInputHdl = rHdl;
        }

        void SetKeyReleaseHdl(const Link<const KeyEvent&,void>& rHdl)
        {
            m_aKeyReleaseHdl = rHdl;
        }

    protected:
        DECL_DLLPRIVATE_LINK(KeyInputHdl, const KeyEvent&, bool);
        DECL_DLLPRIVATE_LINK(KeyReleaseHdl, const KeyEvent&, bool);
        DECL_DLLPRIVATE_LINK(FocusInHdl, weld::Widget&, void);
        DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
        DECL_DLLPRIVATE_LINK(MousePressHdl, const MouseEvent&, bool);
        DECL_DLLPRIVATE_LINK(MouseReleaseHdl, const MouseEvent&, bool);
        DECL_DLLPRIVATE_LINK(MouseMoveHdl, const MouseEvent&, bool);
    private:
        Link<LinkParamNone*,void> m_aFocusInHdl;
        Link<LinkParamNone*,void> m_aFocusOutHdl;
        Link<const MouseEvent&,void> m_aMousePressHdl;
        Link<const MouseEvent&,void> m_aMouseReleaseHdl;
        Link<const MouseEvent&,void> m_aMouseMoveHdl;
        Link<const KeyEvent&,void> m_aKeyInputHdl;
        Link<const KeyEvent&,void> m_aKeyReleaseHdl;
    };

    class SVT_DLLPUBLIC EditControlBase : public ControlBase
    {
    public:
        EditControlBase(BrowserDataWin* pParent);

        virtual void SetEditableReadOnly(bool bReadOnly) override
        {
            m_pEntry->set_editable(!bReadOnly);
        }

        virtual void SetPointFont(const vcl::Font& rFont) override
        {
            m_pEntry->set_font(rFont);
        }

        virtual void dispose() override;

        weld::Entry& get_widget() { return *m_pEntry; }

        virtual void connect_changed(const Link<weld::Entry&, void>& rLink) = 0;
        virtual void connect_focus_in(const Link<weld::Widget&, void>& rLink) = 0;
        virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) = 0;
        virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) = 0;

    protected:
        void InitEditControlBase(weld::Entry* pEntry);

    private:
        weld::Entry* m_pEntry;
    };

    class SVT_DLLPUBLIC EditControl : public EditControlBase
    {
    public:
        EditControl(BrowserDataWin* pParent);

        virtual void dispose() override;

        virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override
        {
            m_xWidget->connect_changed(rLink);
        }

        virtual void connect_focus_in(const Link<weld::Widget&, void>& rLink) override
        {
            m_xWidget->connect_focus_in(rLink);
        }

        virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override
        {
            m_xWidget->connect_focus_out(rLink);
        }

        virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
        {
            m_xWidget->connect_key_press(rLink);
        }

    protected:
        std::unique_ptr<weld::Entry> m_xWidget;
    };

    class SVT_DLLPUBLIC EntryImplementation final : public IEditImplementation
    {
        EditControlBase& m_rEdit;
        int m_nMaxTextLen;

        DECL_LINK(ModifyHdl, weld::Entry&, void);
    public:
        EntryImplementation(EditControlBase& rEdit)
            : m_rEdit(rEdit)
            , m_nMaxTextLen(0)
        {
            m_rEdit.connect_changed(LINK(this, EntryImplementation, ModifyHdl));
        }

        virtual ControlBase& GetControl() override
        {
            return m_rEdit;
        }

        virtual OUString GetText( LineEnd /*aSeparator*/ ) const override
        {
            // ignore the line end - this base implementation does not support it
            return m_rEdit.get_widget().get_text();
        }

        virtual void SetText( const OUString& _rStr ) override
        {
            return m_rEdit.get_widget().set_text(_rStr);
        }

        virtual bool IsReadOnly() const override
        {
            return !m_rEdit.get_widget().get_editable();
        }

        virtual void SetReadOnly( bool bReadOnly ) override
        {
            m_rEdit.SetEditableReadOnly(bReadOnly);
        }

        virtual sal_Int32 GetMaxTextLen() const override
        {
            return m_nMaxTextLen;
        }

        virtual void SetMaxTextLen( sal_Int32 nMaxLen ) override
        {
            m_nMaxTextLen = nMaxLen;
            m_rEdit.get_widget().set_max_length(m_nMaxTextLen);
        }

        virtual Selection GetSelection() const override
        {
            int nStartPos, nEndPos;
            m_rEdit.get_widget().get_selection_bounds(nStartPos, nEndPos);
            return Selection(nStartPos, nEndPos);
        }

        virtual void SetSelection( const Selection& rSelection ) override
        {
            auto nMin = rSelection.Min();
            auto nMax = rSelection.Max();
            m_rEdit.get_widget().select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
        }

        virtual void ReplaceSelected( const OUString& rStr ) override
        {
            m_rEdit.get_widget().replace_selection(rStr);
        }

        virtual OUString GetSelected( LineEnd /*aSeparator*/ ) const override
            // ignore the line end - this base implementation does not support it
        {
            int nStartPos, nEndPos;
            weld::Entry& rEntry = m_rEdit.get_widget();
            rEntry.get_selection_bounds(nStartPos, nEndPos);
            return rEntry.get_text().copy(std::min(nStartPos, nEndPos), std::abs(nEndPos - nStartPos));
        }

        virtual bool IsValueChangedFromSaved() const override
        {
            return m_rEdit.get_widget().get_value_changed_from_saved();
        }

        virtual void SaveValue() override
        {
            m_rEdit.get_widget().save_value();
        }

        virtual bool CanUp() const override
        {
            return false;
        }

        virtual bool CanDown() const override
        {
            return false;
        }

        virtual void Cut() override
        {
            m_rEdit.get_widget().cut_clipboard();
        }

        virtual void Copy() override
        {
            m_rEdit.get_widget().copy_clipboard();
        }

        virtual void Paste() override
        {
            m_rEdit.get_widget().paste_clipboard();
        }
    };

    //= MultiLineTextCell

    /** a multi line edit which can be used in a cell of an EditBrowseBox
    */
    class SVT_DLLPUBLIC MultiLineTextCell final : public ControlBase
    {
    public:
        MultiLineTextCell(BrowserDataWin* pParent);

        virtual void SetEditableReadOnly(bool bReadOnly) override
        {
            m_xWidget->set_editable(!bReadOnly);
        }

        virtual void SetPointFont(const vcl::Font& rFont) override
        {
            m_xWidget->set_font(rFont);
        }

        virtual void GetFocus() override;

        virtual void dispose() override;

        void connect_changed(const Link<weld::TextView&, void>& rLink)
        {
            m_xWidget->connect_changed(rLink);
        }

        weld::TextView& get_widget() { return *m_xWidget; }

    private:
        std::unique_ptr<weld::TextView> m_xWidget;

        virtual bool ProcessKey(const KeyEvent& rKEvt) override;
    };

    class SVT_DLLPUBLIC MultiLineEditImplementation final : public IEditImplementation
    {
        MultiLineTextCell& m_rEdit;
        int m_nMaxTextLen;

        DECL_LINK(ModifyHdl, weld::TextView&, void);
    public:
        MultiLineEditImplementation(MultiLineTextCell& rEdit)
            : m_rEdit(rEdit)
            , m_nMaxTextLen(0)
        {
            m_rEdit.connect_changed(LINK(this, MultiLineEditImplementation, ModifyHdl));
        }

        virtual ControlBase& GetControl() override
        {
            return m_rEdit;
        }

        virtual OUString GetText(LineEnd aSeparator) const override;

        virtual void SetText(const OUString& rStr) override
        {
            return m_rEdit.get_widget().set_text(rStr);
        }

        virtual bool IsReadOnly() const override
        {
            return !m_rEdit.get_widget().get_editable();
        }

        virtual void SetReadOnly( bool bReadOnly ) override
        {
            m_rEdit.SetEditableReadOnly(bReadOnly);
        }

        virtual sal_Int32 GetMaxTextLen() const override
        {
            return m_nMaxTextLen;
        }

        virtual void SetMaxTextLen( sal_Int32 nMaxLen ) override
        {
            m_nMaxTextLen = nMaxLen;
            m_rEdit.get_widget().set_max_length(m_nMaxTextLen);
        }

        virtual Selection GetSelection() const override
        {
            int nStartPos, nEndPos;
            m_rEdit.get_widget().get_selection_bounds(nStartPos, nEndPos);
            return Selection(nStartPos, nEndPos);
        }

        virtual void SetSelection( const Selection& rSelection ) override
        {
            auto nMin = rSelection.Min();
            auto nMax = rSelection.Max();
            m_rEdit.get_widget().select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
        }

        virtual void ReplaceSelected( const OUString& rStr ) override
        {
            m_rEdit.get_widget().replace_selection(rStr);
        }

        virtual OUString GetSelected( LineEnd aSeparator ) const override;

        virtual bool IsValueChangedFromSaved() const override
        {
            return m_rEdit.get_widget().get_value_changed_from_saved();
        }

        virtual void SaveValue() override
        {
            m_rEdit.get_widget().save_value();
        }

        virtual bool CanUp() const override
        {
            return m_rEdit.get_widget().can_move_cursor_with_up();
        }

        virtual bool CanDown() const override
        {
            return m_rEdit.get_widget().can_move_cursor_with_down();
        }

        virtual void Cut() override
        {
            m_rEdit.get_widget().cut_clipboard();
        }

        virtual void Copy() override
        {
            m_rEdit.get_widget().copy_clipboard();
        }

        virtual void Paste() override
        {
            m_rEdit.get_widget().paste_clipboard();
        }
    };


    //= EditCellController
    class SVT_DLLPUBLIC EditCellController : public CellController
    {
        IEditImplementation*    m_pEditImplementation;
        bool                    m_bOwnImplementation;   // did we create m_pEditImplementation?

    public:
        EditCellController( EditControlBase* _pEdit );
        EditCellController( IEditImplementation* _pImplementation );
        virtual ~EditCellController( ) override;

        const IEditImplementation* GetEditImplementation( ) const { return m_pEditImplementation; }
              IEditImplementation* GetEditImplementation( )       { return m_pEditImplementation; }

        virtual bool IsValueChangedFromSaved() const override;
        virtual void SaveValue() override;

        void Modify()
        {
            ModifyHdl(nullptr);
        }

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_LINK(ModifyHdl, LinkParamNone*, void);
    };

    //= CheckBoxControl
    class SVT_DLLPUBLIC CheckBoxControl final : public ControlBase
    {
        std::unique_ptr<weld::CheckButton> m_xBox;
        weld::TriStateEnabled m_aModeState;
        Link<weld::CheckButton&,void> m_aToggleLink;
        Link<LinkParamNone*,void> m_aModify1Hdl;
        Link<LinkParamNone*,void> m_aModify2Hdl;

    public:
        CheckBoxControl(BrowserDataWin* pParent);
        virtual ~CheckBoxControl() override;
        virtual void dispose() override;

        virtual void SetPointFont(const vcl::Font& rFont) override;

        void SetToggleHdl(const Link<weld::CheckButton&,void>& rHdl) {m_aToggleLink = rHdl;}

        // sets a link to call when the text is changed by the user
        void SetModifyHdl(const Link<LinkParamNone*,void>& rHdl)
        {
            m_aModify1Hdl = rHdl;
        }

        // sets an additional link to call when the text is changed by the user
        void SetAuxModifyHdl(const Link<LinkParamNone*,void>& rLink)
        {
            m_aModify2Hdl = rLink;
        }

        void SetState(TriState eState);
        TriState GetState() const { return m_xBox->get_state(); }

        void EnableTriState(bool bTriState);

        weld::CheckButton&   GetBox() {return *m_xBox;};

        // for pseudo-click when initially clicking in a cell activates
        // the cell and performs a state change on the button as if
        // it was clicked on
        void Clicked();

    private:
        DECL_DLLPRIVATE_LINK(OnToggle, weld::Toggleable&, void);

        void CallModifyHdls()
        {
            m_aModify1Hdl.Call(nullptr);
            m_aModify2Hdl.Call(nullptr);
        }
    };

    //= CheckBoxCellController
    class SVT_DLLPUBLIC CheckBoxCellController final : public CellController
    {
    public:

        CheckBoxCellController(CheckBoxControl* pWin);
        weld::CheckButton& GetCheckBox() const;

        virtual bool IsValueChangedFromSaved() const override;
        virtual void SaveValue() override;

    private:
        virtual void ActivatingMouseEvent(const BrowserMouseEvent& rEvt, bool bUp) override;
        DECL_DLLPRIVATE_LINK(ModifyHdl, LinkParamNone*, void);
    };

    //= ComboBoxControl
    class SVT_DLLPUBLIC ComboBoxControl final : public ControlBase
    {
    private:
        std::unique_ptr<weld::ComboBox> m_xWidget;
        Link<LinkParamNone*,void> m_aModify1Hdl;
        Link<bool,void> m_aModify2Hdl;

        friend class ComboBoxCellController;

    public:
        ComboBoxControl(BrowserDataWin* pParent);

        virtual void SetPointFont(const vcl::Font& rFont) override;

        virtual void SetEditableReadOnly(bool bReadOnly) override
        {
            m_xWidget->set_entry_editable(!bReadOnly);
        }

        weld::ComboBox& get_widget() { return *m_xWidget; }

        // sets a link to call when the selection is changed by the user
        void SetModifyHdl(const Link<LinkParamNone*,void>& rHdl)
        {
            m_aModify1Hdl = rHdl;
        }

        // sets an additional link to call when the selection is changed by the user
        // bool arg is true when triggered interactively by the user
        void SetAuxModifyHdl(const Link<bool,void>& rLink)
        {
            m_aModify2Hdl = rLink;
        }

        void TriggerAuxModify()
        {
            m_aModify2Hdl.Call(false);
        }

        virtual void dispose() override;

    private:
        DECL_DLLPRIVATE_LINK(SelectHdl, weld::ComboBox&, void);

        void CallModifyHdls()
        {
            m_aModify1Hdl.Call(nullptr);
            m_aModify2Hdl.Call(true);
        }
    };

    //= ComboBoxCellController
    class SVT_DLLPUBLIC ComboBoxCellController final : public CellController
    {
    public:

        ComboBoxCellController(ComboBoxControl* pParent);
        weld::ComboBox& GetComboBox() const { return static_cast<ComboBoxControl&>(GetWindow()).get_widget(); }

        virtual bool IsValueChangedFromSaved() const override;
        virtual void SaveValue() override;

    private:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
        DECL_DLLPRIVATE_LINK(ModifyHdl, LinkParamNone*, void);
    };

    //= ListBoxControl
    class SVT_DLLPUBLIC ListBoxControl final : public ControlBase
    {
    private:
        std::unique_ptr<weld::ComboBox> m_xWidget;
        Link<LinkParamNone*,void> m_aModify1Hdl;
        Link<bool,void> m_aModify2Hdl;

        friend class ListBoxCellController;

    public:
        ListBoxControl(BrowserDataWin* pParent);

        virtual void SetPointFont(const vcl::Font& rFont) override;

        weld::ComboBox& get_widget() { return *m_xWidget; }

        // sets a link to call when the selection is changed by the user
        void SetModifyHdl(const Link<LinkParamNone*,void>& rHdl)
        {
            m_aModify1Hdl = rHdl;
        }

        // sets an additional link to call when the selection is changed,
        // bool arg is true when triggered interactively by the user
        void SetAuxModifyHdl(const Link<bool,void>& rLink)
        {
            m_aModify2Hdl = rLink;
        }

        void TriggerAuxModify()
        {
            m_aModify2Hdl.Call(false);
        }

        virtual void dispose() override;
    private:
        DECL_DLLPRIVATE_LINK(SelectHdl, weld::ComboBox&, void);

        void CallModifyHdls()
        {
            m_aModify1Hdl.Call(nullptr);
            m_aModify2Hdl.Call(true);
        }
    };

    //= ListBoxCellController
    class SVT_DLLPUBLIC ListBoxCellController : public CellController
    {
    public:

        ListBoxCellController(ListBoxControl* pParent);
        weld::ComboBox& GetListBox() const { return static_cast<ListBoxControl&>(GetWindow()).get_widget(); }

        virtual bool IsValueChangedFromSaved() const override;
        virtual void SaveValue() override;

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_DLLPRIVATE_LINK(ListBoxSelectHdl, LinkParamNone*, void);
    };

    class SVT_DLLPUBLIC FormattedControlBase : public EditControlBase
    {
    public:
        FormattedControlBase(BrowserDataWin* pParent, bool bSpinVariant);

        virtual void dispose() override;

        virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override;
        virtual void connect_focus_in(const Link<weld::Widget&, void>& rLink) override;
        virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override;
        virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override;

        weld::EntryFormatter& get_formatter();

    protected:
        bool m_bSpinVariant;
        std::unique_ptr<weld::Entry> m_xEntry;
        std::unique_ptr<weld::FormattedSpinButton> m_xSpinButton;
        std::unique_ptr<weld::EntryFormatter> m_xEntryFormatter;

        void InitFormattedControlBase();
    };

    class SVT_DLLPUBLIC FormattedControl final : public FormattedControlBase
    {
    public:
        FormattedControl(BrowserDataWin* pParent, bool bSpinVariant);
    };

    class SVT_DLLPUBLIC DoubleNumericControl final : public FormattedControlBase
    {
    public:
        DoubleNumericControl(BrowserDataWin* pParent, bool bSpinVariant);
    };

    class SVT_DLLPUBLIC LongCurrencyControl final : public FormattedControlBase
    {
    public:
        LongCurrencyControl(BrowserDataWin* pParent, bool bSpinVariant);
    };

    class SVT_DLLPUBLIC TimeControl final : public FormattedControlBase
    {
    public:
        TimeControl(BrowserDataWin* pParent, bool bSpinVariant);
    };

    class SVT_DLLPUBLIC DateControl final : public FormattedControlBase
    {
    public:
        DateControl(BrowserDataWin* pParent, bool bDropDown);

        void SetDate(const Date& rDate);

        virtual void SetEditableReadOnly(bool bReadOnly) override;

        virtual void dispose() override;
    private:
        std::unique_ptr<weld::MenuButton> m_xMenuButton;
        std::unique_ptr<weld::Builder> m_xCalendarBuilder;
        std::unique_ptr<weld::Widget> m_xTopLevel;
        std::unique_ptr<weld::Calendar> m_xCalendar;
        std::unique_ptr<weld::Widget> m_xExtras;
        std::unique_ptr<weld::Button> m_xTodayBtn;
        std::unique_ptr<weld::Button> m_xNoneBtn;

        DECL_DLLPRIVATE_LINK(ToggleHdl, weld::Toggleable&, void);
        DECL_DLLPRIVATE_LINK(ActivateHdl, weld::Calendar&, void);
        DECL_DLLPRIVATE_LINK(ImplClickHdl, weld::Button&, void);
    };

    class SVT_DLLPUBLIC PatternControl final : public EditControlBase
    {
    public:
        PatternControl(BrowserDataWin* pParent);

        weld::PatternFormatter& get_formatter() { return *m_xEntryFormatter; }

        virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override;
        virtual void connect_focus_in(const Link<weld::Widget&, void>& rLink) override;
        virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override;
        virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override;

        virtual void dispose() override;
    private:
        std::unique_ptr<weld::Entry> m_xWidget;
        std::unique_ptr<weld::PatternFormatter> m_xEntryFormatter;
    };

    //= FormattedFieldCellController
    class SVT_DLLPUBLIC FormattedFieldCellController final : public EditCellController
    {
    public:
        FormattedFieldCellController( FormattedControlBase* _pFormatted );

        virtual void CommitModifications() override;
    };

    //= EditBrowserHeader
    class SVT_DLLPUBLIC EditBrowserHeader : public BrowserHeader
    {
    public:
        EditBrowserHeader( BrowseBox* pParent, WinBits nWinBits = WB_BUTTONSTYLE )
            :BrowserHeader(pParent, nWinBits){}

    protected:
        virtual void DoubleClick() override;
    };


    //= EditBrowseBox
    class EditBrowseBoxImpl;
    class SVT_DLLPUBLIC EditBrowseBox: public BrowseBox
    {
        friend class EditBrowserHeader;

        enum BrowseInfo
        {
            COLSELECT   =   1,
            ROWSELECT   =   2,
            ROWCHANGE   =   4,
            COLCHANGE   =   8
        };

    public:
        enum RowStatus
        {
            CLEAN               =   0,
            CURRENT             =   1,
            CURRENTNEW          =   2,
            MODIFIED            =   3,
            NEW                 =   4,
            DELETED             =   5,
            PRIMARYKEY          =   6,
            CURRENT_PRIMARYKEY  =   7,
            FILTER              =   8,
            HEADERFOOTER        =   9
        };

    private:
        EditBrowseBox(EditBrowseBox const &) = delete;
        EditBrowseBox& operator=(EditBrowseBox const &) = delete;

        class SAL_DLLPRIVATE BrowserMouseEventPtr
        {
            std::unique_ptr<BrowserMouseEvent> pEvent;
            bool               bDown;

        public:
            BrowserMouseEventPtr()
                : bDown(false)
            {
            }

            bool Is() const {return pEvent != nullptr;}
            bool IsDown() const {return bDown;}
            const BrowserMouseEvent* operator->() const {return pEvent.get();}

            SVT_DLLPUBLIC void Clear();
            void Set(const BrowserMouseEvent* pEvt, bool bIsDown);
        } aMouseEvent;

        CellControllerRef        aController,
                                 aOldController;

        ImplSVEvent * nStartEvent, * nEndEvent, * nCellModifiedEvent;     // event ids
        VclPtr<vcl::Window> m_pFocusWhileRequest;
            // In ActivateCell, we grab the focus asynchronously, but if between requesting activation
            // and the asynchronous event the focus has changed, we won't grab it for ourself.

        sal_Int32   nPaintRow;  // row being painted
        sal_Int32   nEditRow;
        sal_uInt16  nEditCol;

        bool            bHasFocus : 1;
        mutable bool    bPaintStatus : 1;   // paint a status (image) in the handle column
        bool            bActiveBeforeTracking;

        VclPtr<CheckBoxControl> pCheckBoxPaint;

        EditBrowseBoxFlags  m_nBrowserFlags;
        std::unique_ptr< EditBrowseBoxImpl> m_aImpl;

    protected:
        VclPtr<BrowserHeader>  pHeader;

        BrowserMouseEventPtr& getMouseEvent() { return aMouseEvent; }

    protected:
        BrowserHeader*  GetHeaderBar() const {return pHeader;}

        virtual VclPtr<BrowserHeader> CreateHeaderBar(BrowseBox* pParent) override;

        // if you want to have an own header ...
        virtual VclPtr<BrowserHeader> imp_CreateHeaderBar(BrowseBox* pParent);

        virtual void ColumnMoved(sal_uInt16 nId) override;
        virtual void ColumnResized(sal_uInt16 nColId) override;
        virtual void Resize() override;
        virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);
        virtual bool SeekRow(sal_Int32 nRow) override;

        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual void KeyInput(const KeyEvent& rEvt) override;
        virtual void MouseButtonDown(const BrowserMouseEvent& rEvt) override;
        virtual void MouseButtonUp(const BrowserMouseEvent& rEvt) override;
        virtual void StateChanged( StateChangedType nType ) override;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

        using BrowseBox::MouseButtonUp;
        using BrowseBox::MouseButtonDown;

        virtual bool PreNotify(NotifyEvent& rNEvt ) override;
        virtual bool EventNotify(NotifyEvent& rNEvt) override;

        virtual void EndScroll() override;

        // should be used instead of GetFieldRectPixel, 'cause this method here takes into account the borders
        tools::Rectangle GetCellRect(sal_Int32 nRow, sal_uInt16 nColId, bool bRelToBrowser = true) const;
        virtual sal_uInt32 GetTotalCellWidth(sal_Int32 nRow, sal_uInt16 nColId);
        sal_uInt32 GetAutoColumnWidth(sal_uInt16 nColId);

        virtual void PaintStatusCell(OutputDevice& rDev, const tools::Rectangle& rRect) const;
        virtual void PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId) const = 0;

        virtual RowStatus GetRowStatus(sal_Int32 nRow) const;

        virtual void    RowHeightChanged() override;

        // callbacks for the data window
        virtual void    ImplStartTracking() override;
        virtual void    ImplEndTracking() override;

        // when changing a row:
        // CursorMoving:    cursor is being moved, but GetCurRow() still provides the old row
        virtual bool CursorMoving(sal_Int32 nNewRow, sal_uInt16 nNewCol);

        // cursor has been moved
        virtual void CursorMoved() override;

        virtual void CellModified();        // called whenever a cell has been modified
        virtual bool SaveModified();    // called whenever a cell should be left, and it's content should be saved
                                            // return sal_False prevents leaving the cell
        virtual bool SaveRow();         // commit the current row

        virtual bool IsModified() const {return aController.is() && aController->IsValueChangedFromSaved();}

        virtual CellController* GetController(sal_Int32 nRow, sal_uInt16 nCol);
        virtual void InitController(CellControllerRef& rController, sal_Int32 nRow, sal_uInt16 nCol);
        static void ResizeController(CellControllerRef const & rController, const tools::Rectangle&);
        virtual void DoubleClick(const BrowserMouseEvent&) override;

        void ActivateCell() { ActivateCell(GetCurRow(), GetCurColumnId()); }

        // retrieve the image for the row status
        Image GetImage(RowStatus) const;

        // inserting columns
        // if you don't set a width, this will be calculated automatically
        // if the id isn't set the smallest unused will do it ...
        virtual sal_uInt16 AppendColumn(const OUString& rName, sal_uInt16 nWidth, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = sal_uInt16(-1));

        // called whenever (Shift)Tab or Enter is pressed. If true is returned, these keys
        // result in traveling to the next or to th previous cell
        virtual bool IsTabAllowed(bool bForward) const;

        virtual bool IsCursorMoveAllowed(sal_Int32 nNewRow, sal_uInt16 nNewColId) const override;

        void    PaintTristate(const tools::Rectangle& rRect, const TriState& eState, bool _bEnabled=true) const;

        void AsynchGetFocus();
            // secure starting of StartEditHdl

    public:
        EditBrowseBox(vcl::Window* pParent, EditBrowseBoxFlags nBrowserFlags, WinBits nBits, BrowserMode nMode = BrowserMode::NONE );
        virtual ~EditBrowseBox() override;
        virtual void dispose() override;

        bool IsEditing() const {return aController.is();}
        void InvalidateStatusCell(sal_Int32 nRow) {RowModified(nRow, 0);}
        void InvalidateHandleColumn();

        // late construction
        virtual void Init();
        virtual void RemoveRows();
        virtual void Dispatch(sal_uInt16 nId);

        const CellControllerRef& Controller() const { return aController; }
        EditBrowseBoxFlags  GetBrowserFlags() const { return m_nBrowserFlags; }
        void                SetBrowserFlags(EditBrowseBoxFlags nFlags);

        virtual void ActivateCell(sal_Int32 nRow, sal_uInt16 nCol, bool bSetCellFocus = true);
        virtual void DeactivateCell(bool bUpdate = true);
        // Children ---------------------------------------------------------------

        /** @return  The count of additional controls of the control area. */
        virtual sal_Int32 GetAccessibleControlCount() const override;

        /** Creates the accessible object of an additional control.
            @param nIndex
                The 0-based index of the control.
            @return
                The XAccessible interface of the specified control. */
        virtual css::uno::Reference< css::accessibility::XAccessible >
        CreateAccessibleControl( sal_Int32 nIndex ) override;

        /** Sets focus to current cell of the data table. */
        virtual void GrabTableFocus() override;

        virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;

        virtual bool ProcessKey(const KeyEvent& rEvt) override;

        virtual void ChildFocusIn() override;
        virtual void ChildFocusOut() override;

        css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCheckBoxCell(sal_Int32 _nRow, sal_uInt16 _nColumnPos,const TriState& eState);
        bool ControlHasFocus() const;
    protected:
        // creates the accessible which wraps the active cell
        void    implCreateActiveAccessible( );

    private:
        virtual void PaintField(vcl::RenderContext& rDev, const tools::Rectangle& rRect,
                                sal_uInt16 nColumnId ) const override;
        using Control::ImplInitSettings;
        SVT_DLLPRIVATE void ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
        SVT_DLLPRIVATE void DetermineFocus( const GetFocusFlags _nGetFocusFlags = GetFocusFlags::NONE);
        inline void EnableAndShow() const;

        SVT_DLLPRIVATE void implActivateCellOnMouseEvent(const BrowserMouseEvent& _rEvt, bool _bUp);

        DECL_DLLPRIVATE_LINK( ModifyHdl, LinkParamNone*, void );
        DECL_DLLPRIVATE_LINK( StartEditHdl, void*, void );
        DECL_DLLPRIVATE_LINK( EndEditHdl, void*, void );
        DECL_DLLPRIVATE_LINK( CellModifiedHdl, void*, void );
    };


}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
