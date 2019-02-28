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

#ifndef INCLUDED_SVTOOLS_EDITBROWSEBOX_HXX
#define INCLUDED_SVTOOLS_EDITBROWSEBOX_HXX
#define SVTOOLS_IN_EDITBROWSEBOX_HXX

#include <memory>
#include <svtools/svtdllapi.h>
#include <tools/ref.hxx>
#include <vcl/window.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/spinfld.hxx>

#include <vcl/button.hxx>
#include <svtools/brwbox.hxx>
#include <svtools/brwhead.hxx>
#include <svtools/svmedit.hxx>
#include <o3tl/typed_flags_set.hxx>


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


class Edit;
class FormattedField;


namespace svt
{
    class SVT_DLLPUBLIC CellController : public SvRefBase
    {
        friend class EditBrowseBox;
        Link<LinkParamNone*, void> maModifyHdl;

        VclPtr<Control>            pWindow;
        bool                       bSuspended;     // <true> if the window is hidden and disabled

    public:

        CellController(Control* pW);
        virtual ~CellController() override;

        Control& GetWindow() const { return *const_cast< CellController* >( this )->pWindow; }

        virtual void SetModified();
        virtual void ClearModified() = 0;
        virtual bool IsModified() const = 0;

        // commit any current changes. Especially, do any reformatting you need (from input formatting
        // to output formatting) here
        virtual void CommitModifications();

        // suspending the controller is not culmulative!
                void        suspend( );
                void        resume( );
        bool        isSuspended( ) const { return bSuspended; }

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        void SetModifyHdl(const Link<LinkParamNone*,void>& rLink) { maModifyHdl = rLink; }
        virtual bool WantMouseEvent() const;
        virtual void callModifyHdl() { maModifyHdl.Call(nullptr); }
    };

    typedef tools::SvRef<CellController> CellControllerRef;


    //= IEditImplementation

    class SVT_DLLPUBLIC IEditImplementation
    {
    public:
        virtual ~IEditImplementation() = 0;

        virtual Control&            GetControl() = 0;

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

        virtual void                SetModified() = 0;
        virtual bool                IsModified() const = 0;
        virtual void                ClearModified() = 0;
        virtual void                SetModifyHdl( const Link<Edit&,void>& _rLink ) = 0;
    };


    //= GenericEditImplementation

    template <class EDIT>
    class GenericEditImplementation : public IEditImplementation
    {
        EDIT&   m_rEdit;
    public:
        GenericEditImplementation( EDIT& _rEdit );

        EDIT& GetEditWindow() { return static_cast< EDIT& >( GetControl() ); }

        virtual Control&            GetControl() override;

        virtual OUString            GetText( LineEnd aSeparator ) const override;
        virtual void                SetText( const OUString& _rStr ) override;

        virtual bool                IsReadOnly() const override;
        virtual void                SetReadOnly( bool bReadOnly ) override;

        virtual sal_Int32           GetMaxTextLen() const override;
        virtual void                SetMaxTextLen( sal_Int32 _nMaxLen ) override;

        virtual Selection           GetSelection() const override;
        virtual void                SetSelection( const Selection& _rSelection ) override;

        virtual void                ReplaceSelected( const OUString& _rStr ) override;
        virtual OUString            GetSelected( LineEnd aSeparator ) const override;

        virtual void                SetModified() override;
        virtual bool                IsModified() const override;
        virtual void                ClearModified() override;
        virtual void                SetModifyHdl( const Link<Edit&,void>& _rLink ) override;
    };

    #include <svtools/editimplementation.hxx>


    //= MultiLineTextCell

    /** a multi line edit which can be used in a cell of a EditBrowseBox
    */
    class SVT_DLLPUBLIC MultiLineTextCell : public MultiLineEdit
    {
    public:
        MultiLineTextCell( vcl::Window* _pParent, WinBits _nStyle )
            :MultiLineEdit( _pParent, _nStyle )
        {
        }

    protected:
        // Window overridables
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;

        // MultiLineEdit overridables
        virtual void Modify() override;

    private:
        bool         dispatchKeyEvent( const KeyEvent& _rEvent );
    };


    //= concrete edit implementations

    typedef GenericEditImplementation< Edit >             EditImplementation;

    typedef GenericEditImplementation< MultiLineTextCell >  MultiLineEditImplementation_Base;
    class SVT_DLLPUBLIC MultiLineEditImplementation : public MultiLineEditImplementation_Base
    {
    public:
        MultiLineEditImplementation( MultiLineTextCell& _rEdit ) : MultiLineEditImplementation_Base( _rEdit )
        {
        }

        virtual OUString GetText( LineEnd aSeparator ) const override;
        virtual OUString GetSelected( LineEnd aSeparator ) const override;
    };


    //= EditCellController

    class SVT_DLLPUBLIC EditCellController : public CellController
    {
        IEditImplementation*    m_pEditImplementation;
        bool const              m_bOwnImplementation;   // did we create m_pEditImplementation?

    public:
        EditCellController( Edit* _pEdit );
        EditCellController( IEditImplementation* _pImplementation );
        virtual ~EditCellController( ) override;

        const IEditImplementation* GetEditImplementation( ) const { return m_pEditImplementation; }
              IEditImplementation* GetEditImplementation( )       { return m_pEditImplementation; }

        virtual void SetModified() override;
        virtual bool IsModified() const override;
        virtual void ClearModified() override;

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_LINK(ModifyHdl, Edit&, void);
    };


    //= SpinCellController

    class SVT_DLLPUBLIC SpinCellController : public CellController
    {
    public:
        SpinCellController(SpinField* pSpinField);
        const SpinField& GetSpinWindow() const { return static_cast<const SpinField &>(GetWindow()); }
        SpinField& GetSpinWindow() { return static_cast<SpinField &>(GetWindow()); }

        virtual void SetModified() override;
        virtual bool IsModified() const override;
        virtual void ClearModified() override;

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_LINK(ModifyHdl, Edit&, void);
    };


    //= CheckBoxControl

    class SVT_DLLPUBLIC CheckBoxControl : public Control
    {
        VclPtr<CheckBox>             pBox;
        Link<VclPtr<CheckBox>,void>  m_aClickLink;
        Link<LinkParamNone*,void>    m_aModifyLink;

    public:
        CheckBoxControl(vcl::Window* pParent);
        virtual ~CheckBoxControl() override;
        virtual void dispose() override;

        virtual void GetFocus() override;
        virtual bool PreNotify(NotifyEvent& rEvt) override;
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rClientRect) override;
        virtual void Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
        virtual void StateChanged( StateChangedType nStateChange ) override;
        virtual void DataChanged( const DataChangedEvent& _rEvent ) override;
        virtual void Resize() override;

        void SetClickHdl(const Link<VclPtr<CheckBox>,void>& rHdl) {m_aClickLink = rHdl;}

        void SetModifyHdl(const Link<LinkParamNone*,void>& rHdl) {m_aModifyLink = rHdl;}

        CheckBox&   GetBox() {return *pBox;};

    private:
        DECL_LINK( OnClick, Button*, void );
    };


    //= CheckBoxCellController

    class SVT_DLLPUBLIC CheckBoxCellController : public CellController
    {
    public:

        CheckBoxCellController(CheckBoxControl* pWin);
        CheckBox& GetCheckBox() const;

        virtual bool IsModified() const override;
        virtual void ClearModified() override;

    protected:
        virtual bool WantMouseEvent() const override;
    private:
        DECL_LINK(ModifyHdl, LinkParamNone*, void);
    };


    //= ComboBoxControl

    class SVT_DLLPUBLIC ComboBoxControl : public ComboBox
    {
        friend class ComboBoxCellController;

    public:
        ComboBoxControl(vcl::Window* pParent);

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    };


    //= ComboBoxCellController

    class SVT_DLLPUBLIC ComboBoxCellController : public CellController
    {
    public:

        ComboBoxCellController(ComboBoxControl* pParent);
        ComboBoxControl& GetComboBox() const { return static_cast<ComboBoxControl &>(GetWindow()); }

        virtual bool IsModified() const override;
        virtual void ClearModified() override;

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_LINK(ModifyHdl, Edit&, void);
    };


    //= ListBoxControl

    class SVT_DLLPUBLIC ListBoxControl : public ListBox
    {
        friend class ListBoxCellController;

    public:
        ListBoxControl(vcl::Window* pParent);

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    };


    //= ListBoxCellController

    class SVT_DLLPUBLIC ListBoxCellController : public CellController
    {
    public:

        ListBoxCellController(ListBoxControl* pParent);
        const ListBoxControl& GetListBox() const { return static_cast<const ListBoxControl &>(GetWindow()); }
        ListBoxControl& GetListBox() { return static_cast<ListBoxControl &>(GetWindow()); }

        virtual bool IsModified() const override;
        virtual void ClearModified() override;

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const override;
    private:
        DECL_LINK(ListBoxSelectHdl, ListBox&, void);
    };


    //= FormattedFieldCellController

    class SVT_DLLPUBLIC FormattedFieldCellController : public EditCellController
    {
    public:
        FormattedFieldCellController( FormattedField* _pFormatted );

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

        class BrowserMouseEventPtr
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

        long    nPaintRow;  // row being painted
        long    nEditRow;
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
        virtual bool SeekRow(long nRow) override;

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
        tools::Rectangle GetCellRect(long nRow, sal_uInt16 nColId, bool bRelToBrowser = true) const;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);
        sal_uInt32 GetAutoColumnWidth(sal_uInt16 nColId);

        virtual void PaintStatusCell(OutputDevice& rDev, const tools::Rectangle& rRect) const;
        virtual void PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId) const = 0;

        virtual RowStatus GetRowStatus(long nRow) const;

        virtual void    RowHeightChanged() override;

        // callbacks for the data window
        virtual void    ImplStartTracking() override;
        virtual void    ImplEndTracking() override;

        // when changing a row:
        // CursorMoving:    cursor is being moved, but GetCurRow() still provides the old row
        virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);

        // cursor has been moved
        virtual void CursorMoved() override;

        virtual void CellModified();        // called whenever a cell has been modified
        virtual bool SaveModified();    // called whenever a cell should be left, and it's content should be saved
                                            // return sal_False prevents leaving the cell
        virtual bool SaveRow();         // commit the current row

        virtual bool IsModified() const {return aController.is() && aController->IsModified();}

        virtual CellController* GetController(long nRow, sal_uInt16 nCol);
        virtual void InitController(CellControllerRef& rController, long nRow, sal_uInt16 nCol);
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

        virtual bool IsCursorMoveAllowed(long nNewRow, sal_uInt16 nNewColId) const override;

        void    PaintTristate(const tools::Rectangle& rRect, const TriState& eState, bool _bEnabled=true) const;

        void AsynchGetFocus();
            // secure starting of StartEditHdl

    public:
        EditBrowseBox(vcl::Window* pParent, EditBrowseBoxFlags nBrowserFlags, WinBits nBits, BrowserMode nMode = BrowserMode::NONE );
        virtual ~EditBrowseBox() override;
        virtual void dispose() override;

        bool IsEditing() const {return aController.is();}
        void InvalidateStatusCell(long nRow) {RowModified(nRow, 0);}
        void InvalidateHandleColumn();

        // late construction
        virtual void Init();
        virtual void RemoveRows();
        virtual void Dispatch(sal_uInt16 nId);

        const CellControllerRef& Controller() const { return aController; }
        EditBrowseBoxFlags  GetBrowserFlags() const { return m_nBrowserFlags; }
        void                SetBrowserFlags(EditBrowseBoxFlags nFlags);

        virtual void ActivateCell(long nRow, sal_uInt16 nCol, bool bSetCellFocus = true);
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

        css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCheckBoxCell(long _nRow, sal_uInt16 _nColumnPos,const TriState& eState);
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
        SVT_DLLPRIVATE void impl_construct();

        DECL_DLLPRIVATE_LINK( ModifyHdl, LinkParamNone*, void );
        DECL_DLLPRIVATE_LINK( StartEditHdl, void*, void );
        DECL_DLLPRIVATE_LINK( EndEditHdl, void*, void );
        DECL_DLLPRIVATE_LINK( CellModifiedHdl, void*, void );
    };


}   // namespace svt


#undef SVTOOLS_IN_EDITBROWSEBOX_HXX
#endif // INCLUDED_SVTOOLS_EDITBROWSEBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
