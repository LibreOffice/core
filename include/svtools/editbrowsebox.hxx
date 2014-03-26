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

#include <svtools/svtdllapi.h>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <vcl/window.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>
#include <svtools/brwbox.hxx>
#include <vcl/timer.hxx>
#include <svtools/brwhead.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/svapp.hxx>


// EditBrowseBoxFlags (EBBF)

#define EBBF_NONE                       ((sal_Int32)0x0000)
/** if this bit is _not_ set, the handle column will be invalidated upon
    changing the row in the browse box.  This is for forcing the row picture to
    be repainted. If you do not have row pictures or text, you don't need this
    invalidation, then you would specify this bit to prevent flicker
*/
#define EBBF_NO_HANDLE_COLUMN_CONTENT   ((sal_Int32)0x0001)
/** set this bit to activate the cell on a MouseButtonDown, not a MouseButtonUp event
 */
#define EBBF_ACTIVATE_ON_BUTTONDOWN     ((sal_Int32)0x0002)
/** if this bit is set and EBBF_NO_HANDLE_COLUMN_CONTENT is _not_ set, the handle
    column is drawn with the text contained in column 0 instead of an image
*/
#define EBBF_HANDLE_COLUMN_TEXT         ((sal_Int32)0x0004)

/** If this bit is set, tab traveling is somewhat modified<br/>
    If the control gets the focus because the user pressed the TAB key, then the
    first or last cell (depending on whether the traveling was cycling forward or backward)
    gets activated.
    @see Window::GetGetFocusFlags
    @see GETFOCUS_*
*/
#define EBBF_SMART_TAB_TRAVEL           ((sal_Int32)0x0008)

/// @deprecated
#define EBBF_NOROWPICTURE               EBBF_NO_HANDLE_COLUMN_CONTENT



class Edit;
class SpinField;
class FormattedField;


namespace svt
{


    class CellControllerRef;


    //= CellController

    class SVT_DLLPUBLIC CellController : public SvRefBase
    {
        friend class EditBrowseBox;

    protected:
        Control*    pWindow;
        bool        bSuspended;     // <true> if the window is hidden and disabled

    public:
        TYPEINFO();

        CellController(Control* pW);
        virtual ~CellController();

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
        inline  bool        isSuspended( ) const { return bSuspended; }

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        virtual void SetModifyHdl(const Link& rLink) = 0;
        virtual bool WantMouseEvent() const;
    };

    SV_DECL_IMPL_REF(CellController);


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
        virtual void                DeleteSelected() = 0;
        virtual OUString            GetSelected( LineEnd aSeparator ) const = 0;

        virtual void                SetModified() = 0;
        virtual bool                IsModified() const = 0;
        virtual void                ClearModified() = 0;
        virtual void                SetModifyHdl( const Link& _rLink ) = 0;
    };


    //= GenericEditImplementation

    template <class EDIT>
    class GenericEditImplementation : public IEditImplementation
    {
        EDIT&   m_rEdit;
    public:
        GenericEditImplementation( EDIT& _rEdit );

        EDIT& GetEditWindow() { return static_cast< EDIT& >( GetControl() ); }

        virtual Control&            GetControl();

        virtual OUString            GetText( LineEnd aSeparator ) const;
        virtual void                SetText( const OUString& _rStr );

        virtual bool                IsReadOnly() const;
        virtual void                SetReadOnly( bool bReadOnly );

        virtual sal_Int32           GetMaxTextLen() const;
        virtual void                SetMaxTextLen( sal_Int32 _nMaxLen );

        virtual Selection           GetSelection() const;
        virtual void                SetSelection( const Selection& _rSelection );

        virtual void                ReplaceSelected( const OUString& _rStr );
        virtual void                DeleteSelected();
        virtual OUString            GetSelected( LineEnd aSeparator ) const;

        virtual void                SetModified();
        virtual bool                IsModified() const;
        virtual void                ClearModified();
        virtual void                SetModifyHdl( const Link& _rLink );
    };

    #include <svtools/editimplementation.hxx>


    //= MultiLineTextCell

    /** a multi line edit which can be used in a cell of a EditBrowseBox
    */
    class SVT_DLLPUBLIC MultiLineTextCell : public MultiLineEdit
    {
    public:
        MultiLineTextCell( Window* _pParent, WinBits _nStyle )
            :MultiLineEdit( _pParent, _nStyle )
        {
        }

    protected:
        // Window overridables
        virtual bool PreNotify( NotifyEvent& rNEvt );

        // MultiLineEdit overridables
        virtual void Modify();

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

        virtual OUString GetText( LineEnd aSeparator ) const;
        virtual OUString GetSelected( LineEnd aSeparator ) const;
    };


    //= EditCellController

    class SVT_DLLPUBLIC EditCellController : public CellController
    {
        IEditImplementation*    m_pEditImplementation;
        bool                    m_bOwnImplementation;   // did we create m_pEditImplementation?

    public:
        TYPEINFO_OVERRIDE();
        EditCellController( Edit* _pEdit );
        EditCellController( IEditImplementation* _pImplementation );
        ~EditCellController( );

        const IEditImplementation* GetEditImplementation( ) const { return m_pEditImplementation; }
              IEditImplementation* GetEditImplementation( )       { return m_pEditImplementation; }

        virtual void SetModified();
        virtual bool IsModified() const;
        virtual void ClearModified();

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        virtual void SetModifyHdl(const Link& rLink);
    };


    //= SpinCellController

    class SVT_DLLPUBLIC SpinCellController : public CellController
    {
    public:
        TYPEINFO_OVERRIDE();
        SpinCellController(SpinField* pSpinField);
        SpinField& GetSpinWindow() const {return (SpinField &)GetWindow();}

        virtual void SetModified();
        virtual bool IsModified() const;
        virtual void ClearModified();

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        virtual void SetModifyHdl(const Link& rLink);
    };


    //= CheckBoxControl

    class SVT_DLLPUBLIC CheckBoxControl : public Control
    {
        CheckBox*   pBox;
        Rectangle   aFocusRect;
        Link        m_aClickLink,m_aModifyLink;

    public:
        CheckBoxControl(Window* pParent, WinBits nWinStyle = 0);
        ~CheckBoxControl();

        virtual void GetFocus();
        virtual bool PreNotify(NotifyEvent& rEvt);
        virtual void Paint(const Rectangle& rClientRect);
        virtual void Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& _rEvent );
        virtual void Resize();

        void SetClickHdl(const Link& rHdl) {m_aClickLink = rHdl;}
        const Link& GetClickHdl() const {return m_aClickLink;}

        void SetModifyHdl(const Link& rHdl) {m_aModifyLink = rHdl;}
        const Link& GetModifyHdl() const {return m_aModifyLink;}

        CheckBox&   GetBox() {return *pBox;};

    private:
        DECL_LINK( OnClick, void* );
    };


    //= CheckBoxCellController

    class SVT_DLLPUBLIC CheckBoxCellController : public CellController
    {
    public:
        TYPEINFO_OVERRIDE();

        CheckBoxCellController(CheckBoxControl* pWin):CellController(pWin){}
        CheckBox& GetCheckBox() const;

        virtual bool IsModified() const;
        virtual void ClearModified();

    protected:
        virtual void SetModifyHdl(const Link& rLink);
        virtual bool WantMouseEvent() const;
    };


    //= ComboBoxControl

    class SVT_DLLPUBLIC ComboBoxControl : public ComboBox
    {
        friend class ComboBoxCellController;

    public:
        ComboBoxControl(Window* pParent, WinBits nWinStyle = 0);

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt );
    };


    //= ComboBoxCellController

    class SVT_DLLPUBLIC ComboBoxCellController : public CellController
    {
    public:
        TYPEINFO_OVERRIDE();

        ComboBoxCellController(ComboBoxControl* pParent);
        ComboBoxControl& GetComboBox() const {return (ComboBoxControl &)GetWindow();}

        virtual bool IsModified() const;
        virtual void ClearModified();

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        virtual void SetModifyHdl(const Link& rLink);
    };


    //= ListBoxControl

    class SVT_DLLPUBLIC ListBoxControl : public ListBox
    {
        friend class ListBoxCellController;

    public:
        ListBoxControl(Window* pParent, WinBits nWinStyle = 0);

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt );
    };


    //= ListBoxCellController

    class SVT_DLLPUBLIC ListBoxCellController : public CellController
    {
    public:
        TYPEINFO_OVERRIDE();

        ListBoxCellController(ListBoxControl* pParent);
        ListBoxControl& GetListBox() const {return (ListBoxControl &)GetWindow();}

        virtual bool IsModified() const;
        virtual void ClearModified();

    protected:
        virtual bool MoveAllowed(const KeyEvent& rEvt) const;
        virtual void SetModifyHdl(const Link& rLink);
    };


    //= FormattedFieldCellController

    class SVT_DLLPUBLIC FormattedFieldCellController : public EditCellController
    {
    public:
        TYPEINFO_OVERRIDE();
        FormattedFieldCellController( FormattedField* _pFormatted );

        virtual void CommitModifications();
    };


    //= EditBrowserHeader

    class SVT_DLLPUBLIC EditBrowserHeader : public BrowserHeader
    {
    public:
        EditBrowserHeader( BrowseBox* pParent, WinBits nWinBits = WB_BUTTONSTYLE )
            :BrowserHeader(pParent, nWinBits){}

    protected:
        virtual void DoubleClick();
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
        // forbid these ones
        EditBrowseBox(EditBrowseBox&);
        EditBrowseBox& operator=(EditBrowseBox&);

        class BrowserMouseEventPtr
        {
            BrowserMouseEvent* pEvent;
            bool               bDown;

        public:
            BrowserMouseEventPtr():pEvent(NULL){}
            ~BrowserMouseEventPtr(){Clear();}

            bool Is() const {return pEvent != NULL;}
            bool IsDown() const {return bDown;}
            const BrowserMouseEvent* operator->() const {return pEvent;}
            const BrowserMouseEvent& operator*() const {return *pEvent;}

            SVT_DLLPUBLIC void Clear();
            void Set(const BrowserMouseEvent* pEvt, bool bIsDown);
        } aMouseEvent;

        CellControllerRef        aController,
                                 aOldController;

        sal_uLong   nStartEvent, nEndEvent, nCellModifiedEvent;     // event ids
        Window* m_pFocusWhileRequest;
            // In ActivateCell, we grab the focus asynchronously, but if between requesting activation
            // and the asynchornous event the focus has changed, we won't grab it for ourself.

        long    nPaintRow;  // row beeing painted
        long    nEditRow, nOldEditRow;
        sal_uInt16  nEditCol, nOldEditCol;

        bool            bHasFocus : 1;
        mutable bool    bPaintStatus : 1;   // paint a status (image) in the handle column
        bool            bActiveBeforeTracking;

        CheckBoxControl* pCheckBoxPaint;

        sal_Int32   m_nBrowserFlags;
        ImageList   m_aStatusImages;
        ::std::auto_ptr< EditBrowseBoxImpl> m_aImpl;

    protected:
        BrowserHeader*  pHeader;

        bool isGetCellFocusPending() const { return nStartEvent != 0; }
        void cancelGetCellFocus() { if (nStartEvent) Application::RemoveUserEvent(nStartEvent); nStartEvent = 0; }
        void forceGetCellFocus() { cancelGetCellFocus(); LINK(this, EditBrowseBox, StartEditHdl).Call((void*)NULL); }

        BrowserMouseEventPtr& getMouseEvent() { return aMouseEvent; }

    protected:
        BrowserHeader*  GetHeaderBar() const {return pHeader;}

        virtual BrowserHeader* CreateHeaderBar(BrowseBox* pParent);

        // if you want to have an own header ...
        virtual BrowserHeader* imp_CreateHeaderBar(BrowseBox* pParent);

        virtual void ColumnMoved(sal_uInt16 nId);
        virtual void ColumnResized(sal_uInt16 nColId);
        virtual void Resize();
        virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);
        virtual bool SeekRow(long nRow);

        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void KeyInput(const KeyEvent& rEvt);
        virtual void MouseButtonDown(const BrowserMouseEvent& rEvt);
        virtual void MouseButtonUp(const BrowserMouseEvent& rEvt);
        virtual void StateChanged( StateChangedType nType );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        using BrowseBox::MouseButtonUp;
        using BrowseBox::MouseButtonDown;

        virtual bool PreNotify(NotifyEvent& rNEvt );
        virtual bool Notify(NotifyEvent& rNEvt);

        virtual void EndScroll();

        // should be used instead of GetFieldRectPixel, 'cause this method here takes into account the borders
        Rectangle GetCellRect(long nRow, sal_uInt16 nColId, bool bRelToBrowser = true) const;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);
        virtual sal_uInt32 GetAutoColumnWidth(sal_uInt16 nColId);

        virtual void PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const;
        virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const = 0;

        virtual RowStatus GetRowStatus(long nRow) const;

        virtual void    RowHeightChanged();

        // callbacks for the data window
        virtual void    ImplStartTracking();
        virtual void    ImplTracking();
        virtual void    ImplEndTracking();

        // when changing a row:
        // CursorMoving:    cursor is beeing moved, but GetCurRow() still provides the old row
        virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);

        // cursor has been moved
        virtual void CursorMoved();

        virtual void CellModified();        // called whenever a cell has been modified
        virtual bool SaveModified();    // called whenever a cell should be left, and it's content should be saved
                                            // return sal_False prevents leaving the cell
        virtual bool SaveRow();         // commit the current row

        virtual bool IsModified() const {return aController.Is() && aController->IsModified();}

        virtual CellController* GetController(long nRow, sal_uInt16 nCol);
        virtual void InitController(CellControllerRef& rController, long nRow, sal_uInt16 nCol);
        virtual void ResizeController(CellControllerRef& rController, const Rectangle&);
        virtual void ReleaseController(CellControllerRef& pController, long nRow, sal_uInt16 nCol);
        virtual void DoubleClick(const BrowserMouseEvent&);

        void ActivateCell() { ActivateCell(GetCurRow(), GetCurColumnId()); }

        // retrieve the image for the row status
        virtual Image GetImage(RowStatus) const;

        // inserting columns
        // if you don't set a width, this will be calculated automatically
        // if the id isn't set the smallest unused will do it ...
        virtual sal_uInt16 AppendColumn(const OUString& rName, sal_uInt16 nWidth = 0, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = (sal_uInt16)-1);

        // called whenever (Shift)Tab or Enter is pressed. If true is returned, these keys
        // result in traveling to the next or to th previous cell
        virtual bool IsTabAllowed(bool bForward) const;

        virtual bool IsCursorMoveAllowed(long nNewRow, sal_uInt16 nNewColId) const;

        void    PaintTristate(OutputDevice& rDev, const Rectangle& rRect, const TriState& eState, bool _bEnabled=true) const;

        void AsynchGetFocus();
            // secure starting of StartEditHdl

    public:
        EditBrowseBox(Window* pParent, sal_Int32 nBrowserFlags = EBBF_NONE, WinBits nBits = WB_TABSTOP, BrowserMode nMode = 0 );
        EditBrowseBox(Window* pParent, const ResId& rId, sal_Int32 nBrowserFlags = EBBF_NONE, BrowserMode nMode = 0 );
        ~EditBrowseBox();

        bool IsEditing() const {return aController.Is();}
        void InvalidateStatusCell(long nRow) {RowModified(nRow, 0);}
        void InvalidateHandleColumn();

        // late construction
        virtual void Init();
        virtual void RemoveRows();
        virtual void Dispatch(sal_uInt16 nId);

        CellControllerRef Controller() const { return aController; }
        sal_Int32   GetBrowserFlags() const { return m_nBrowserFlags; }
        void    SetBrowserFlags(sal_Int32 nFlags);

        virtual void ActivateCell(long nRow, sal_uInt16 nCol, bool bSetCellFocus = true);
        virtual void DeactivateCell(bool bUpdate = true);
        // Children ---------------------------------------------------------------

        /** Creates the accessible object of a data table cell.
        @param nRow
            The row index of the cell.
        @param nColumnId
            The column ID of the cell.
        @return
            The XAccessible interface of the specified cell. */
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnPos );

        /** @return  The count of additional controls of the control area. */
        virtual sal_Int32 GetAccessibleControlCount() const;

        /** Creates the accessible object of an additional control.
            @param nIndex
                The 0-based index of the control.
            @return
                The XAccessible interface of the specified control. */
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        CreateAccessibleControl( sal_Int32 nIndex );

        /** Creates the accessible object of a column header.
            @param nColumnId
                The column ID of the header.
            @return
                The XAccessible interface of the specified column header. */
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        CreateAccessibleRowHeader( sal_Int32 _nRow );

        /** Sets focus to current cell of the data table. */
        virtual void GrabTableFocus();

        virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);

        ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible > CreateAccessibleCheckBoxCell(long _nRow, sal_uInt16 _nColumnPos,const TriState& eState);
    protected:
        // creates the accessible which wraps the active cell
        void    implCreateActiveAccessible( );

    private:
        virtual void PaintField(OutputDevice& rDev, const Rectangle& rRect,
                                sal_uInt16 nColumnId ) const;
        using Control::ImplInitSettings;
        SVT_DLLPRIVATE void ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
        SVT_DLLPRIVATE void DetermineFocus( const sal_uInt16 _nGetFocusFlags = 0);
        inline void HideAndDisable(CellControllerRef& rController);
        inline void EnableAndShow() const;

        SVT_DLLPRIVATE void implActivateCellOnMouseEvent(const BrowserMouseEvent& _rEvt, bool _bUp);
        SVT_DLLPRIVATE void impl_construct();

        DECL_DLLPRIVATE_LINK(ModifyHdl, void* );
        DECL_DLLPRIVATE_LINK(StartEditHdl, void* );
        DECL_DLLPRIVATE_LINK(EndEditHdl, void* );
        DECL_DLLPRIVATE_LINK(CellModifiedHdl, void* );
    };


}   // namespace svt


#undef SVTOOLS_IN_EDITBROWSEBOX_HXX
#endif // INCLUDED_SVTOOLS_EDITBROWSEBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
