/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_GRIDCTRL_HXX
#define _SVX_GRIDCTRL_HXX


#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vcl/fixed.hxx>

#include <vcl/field.hxx>




#include <svtools/editbrowsebox.hxx>


#include <comphelper/propmultiplex.hxx>

namespace binfilter {

extern XubString INVALIDTEXT;
extern XubString OBJECTTEXT;

class DbGridControl;
class CursorWrapper;

sal_Bool CompareBookmark(const ::com::sun::star::uno::Any& aLeft, const ::com::sun::star::uno::Any& aRight);

namespace svxform
{
    class DataColumn;
}
DECLARE_LIST(DbDataColumns, ::binfilter::svxform::DataColumn*)//STRIP008 DECLARE_LIST(DbDataColumns, ::svxform::DataColumn*);

enum GridRowStatus
{
    GRS_CLEAN,
    GRS_MODIFIED,
    GRS_DELETED,
    GRS_INVALID
};

//==================================================================
// DbGridRow, Zeilenbeschreibung
//==================================================================

class DbGridRow : public SvRefBase
{
    ::com::sun::star::uno::Any						m_aBookmark;		// ::com::sun::star::text::Bookmark der Zeile, kann gesetzt sein
    DbDataColumns				m_aVariants;
    GridRowStatus				m_eStatus;
    sal_Bool						m_bIsNew;
                                                    // Zeile ist nicht mehr gueltig,
                                                    // wird bei der naechsten positionierung entfernt
public:
    DbGridRow():m_eStatus(GRS_CLEAN), m_bIsNew(sal_True) { }
    DbGridRow(CursorWrapper* pCur, sal_Bool bPaintCursor);
    void SetState(CursorWrapper* pCur, sal_Bool bPaintCursor);

    ~DbGridRow();	

    // da GetField auf Geschwindigkeit getuned ist vorher immer noch hasField verwenden
    sal_Bool HasField(sal_uInt32 nPos) const {return nPos < m_aVariants.Count();}
    const ::binfilter::svxform::DataColumn& GetField(sal_uInt32 nPos) const { return *m_aVariants.GetObject(nPos); }//STRIP008 	const ::svxform::DataColumn& GetField(sal_uInt32 nPos) const { return *m_aVariants.GetObject(nPos); }

    void			SetStatus(GridRowStatus _eStat) { m_eStatus = _eStat; }
    GridRowStatus	GetStatus() const				{ return m_eStatus; }
    void			SetNew(sal_Bool _bNew)				{ m_bIsNew = _bNew; }
    sal_Bool			IsNew() const					{ return m_bIsNew; }

    const ::com::sun::star::uno::Any& GetBookmark() const { return m_aBookmark; }

    sal_Bool	IsValid() const { return m_eStatus == GRS_CLEAN || m_eStatus == GRS_MODIFIED; }
    sal_Bool	IsModified() const { return m_eStatus == GRS_MODIFIED; }
};

SV_DECL_REF(DbGridRow)//STRIP008 ;

//==================================================================
// DbGridControl
//==================================================================
class DbGridColumn;
DECLARE_LIST(DbGridColumns, DbGridColumn*)//STRIP008 DECLARE_LIST(DbGridColumns, DbGridColumn*);

//==================================================================
class FmGridSelectionListener
{
public:
    virtual void selectionChanged() = 0;
};

//==================================================================
class FmXGridSourcePropListener;
class DisposeListenerGridBridge;
typedef EditBrowseBox	DbGridControl_Base;
class DbGridControl : public DbGridControl_Base
{
    friend class FmXGridSourcePropListener;
    friend class GridFieldValueListener;
    friend class DisposeListenerGridBridge;

public:
    //==================================================================
    // NavigationBar
    //==================================================================
    class NavigationBar: public Control
    {
        class AbsolutePos : public NumericField
        {
        public:
            AbsolutePos(Window* pParent, WinBits nStyle = 0);

            virtual void KeyInput(const KeyEvent& rEvt);
            virtual void LoseFocus();
        };

        friend class NavigationBar::AbsolutePos;

        //	zusaetzliche Controls
        FixedText		m_aRecordText;
        AbsolutePos		m_aAbsolute;			// AbsolutePositionierung
        FixedText		m_aRecordOf;
        FixedText		m_aRecordCount;

        ImageButton		m_aFirstBtn;			// ImageButton fuer 'gehe zum ersten Datensatz'
        ImageButton		m_aPrevBtn;			// ImageButton fuer 'gehe zum vorhergehenden Datensatz'
        ImageButton		m_aNextBtn;			// ImageButton fuer 'gehe zum naechsten Datensatz'
        ImageButton		m_aLastBtn;			// ImageButton fuer 'gehe zum letzten Datensatz'
        ImageButton		m_aNewBtn;			// ImageButton fuer 'gehe zum neuen Datensatz'
        sal_uInt16			m_nDefaultWidth;
        sal_uInt32			m_nCurrentPos;

        sal_Bool			m_bPositioning;		// protect PositionDataSource against recursion

    public:
        //	StatusIds fuer Controls der Bar
        //  wichtig fuers Invalidieren
        enum State
        {
            RECORD_TEXT	= 1,
            RECORD_ABSOLUTE,
            RECORD_OF,
            RECORD_COUNT,
            RECORD_FIRST,
            RECORD_NEXT,
            RECORD_PREV,
            RECORD_LAST,
            RECORD_NEW
        };

        NavigationBar(Window* pParent, WinBits nStyle = 0);

        // Status Methoden fuer Controls
        void InvalidateAll(sal_uInt32 nCurrentPos = -1, sal_Bool bAll = sal_False);
        void InvalidateState(sal_uInt16 nWhich) {SetState(nWhich);}
        void SetState(sal_uInt16 nWhich);
        sal_Bool GetState(sal_uInt16 nWhich) const;
        sal_uInt16 GetDefaultWidth() const {return m_nDefaultWidth;}

    protected:
        virtual void Resize();
        virtual void Paint(const Rectangle& rRect);
        virtual void StateChanged( StateChangedType nType );

    private:
        DECL_LINK(OnClick, Button*);
        sal_uInt16 ArrangeControls();

        void PositionDataSource(sal_Int32 nRecord);
    };

    friend class DbGridControl::NavigationBar;

public:
    // diese Optionen werden verodert und geben an, welche der einzelnen feature
    // freigegeben werden koennen, default ist readonly also 0
    enum Option
    {
        OPT_READONLY	= 0x00,
        OPT_INSERT		= 0x01,
        OPT_UPDATE		= 0x02,
        OPT_DELETE		= 0x04
    };

private:
    Font			m_aDefaultFont;		  
    Link			m_aMasterStateProvider;
    Link			m_aMasterSlotExecutor;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > 		m_xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	m_xServiceFactory;

    DbGridColumns	m_aColumns;			// Spaltenbeschreibung
    NavigationBar	m_aBar;
    DbGridRowRef	m_xDataRow;			// Zeile die modifiziert werden kann
                                        // kommt vom DatenCursor
    DbGridRowRef	m_xSeekRow,			// Zeile die von Iterator gesetzt wird
                                        // kommt vom DatenCursor

                    m_xEmptyRow;		// Datensatz zum einfuegen

    sal_uInt32			m_nAsynAdjustEvent;

    // if we modify the row for the new record, we automatically insert a "new new row".
    // But if somebody else inserts a new record into the data source, we have to do the same.
    // For that reason we have to listen to some properties of our data source.
    ::comphelper::OPropertyChangeMultiplexer*		m_pDataSourcePropMultiplexer;
    FmXGridSourcePropListener*						m_pDataSourcePropListener;

    void*											m_pFieldListeners;
        // property listeners for field values

    DisposeListenerGridBridge*						m_pCursorDisposeListener;
        // need to know about the diposing of the seek cursor
        // construct analogous to the data source proplistener/multiplexer above :
        // DisposeListenerGridBridge is a bridge from FmXDisposeListener which I don't want to be derived from

    FmGridSelectionListener*						m_pSelectionListener;

protected:
    CursorWrapper*	m_pDataCursor;		// Cursor fuer Updates
    CursorWrapper*  m_pSeekCursor;		// Cursor zum Seeken

private:  
    // dieses sind Laufvariablen
    DbGridRowRef		m_xCurrentRow;		// Row an der aktuellen Zeile
    DbGridRowRef		m_xPaintRow;		// Row die gerade drgestellt werden soll
    sal_Int32			m_nSeekPos;			// Position des SeekCursors
    sal_Int32			m_nTotalCount;		// wird gesetzt, wenn der DatenCursor mit dem Z‰hlen der
                                            // Datens‰tze fertig ist
                                            // initial Wert ist -1
    osl::Mutex			m_aDestructionSafety;
    osl::Mutex			m_aAdjustSafety;

    ::com::sun::star::util::Date
                        m_aNullDate;		// NullDate of the Numberformatter;

    BrowserMode			m_nMode;
    sal_Int32			m_nCurrentPos;		// Aktuelle Position;
    sal_uInt32			m_nDeleteEvent;		// EventId fuer asychrone Loeschen von Zeilen
    sal_uInt16			m_nOptions;			// Was kann das Control (Insert, Update, Delete)
                                        // default readonly
    sal_uInt16			m_nOptionMask;		// the mask of options to be enabled in setDataSource
                                        // (with respect to the data source capabilities)
                                        // defaults to (insert | update | delete)

    sal_Bool			m_bFrozen : 1;			// Anzeige ist eingefroren
    sal_Bool			m_bDesignMode : 1;		// default = sal_False
    sal_Bool			m_bRecordCountFinal : 1;
    sal_Bool			m_bMultiSelection   : 1;
    sal_Bool			m_bNavigationBar	  : 1;

    sal_Bool			m_bSynchDisplay : 1;
    sal_Bool			m_bForceROController : 1;
    sal_Bool			m_bHandle : 1;
    sal_Bool			m_bFilterMode : 1;
    sal_Bool			m_bWantDestruction : 1;
    sal_Bool			m_bInAdjustDataSource : 1;
    sal_Bool			m_bPendingAdjustRows : 1;	// if an async adjust is pending, is it for AdjustRows or AdjustDataSource ?

protected:
    sal_Bool			m_bUpdating : 1;			// werden gerade updates durchgefuehrt

protected:
    virtual sal_Bool SeekRow(long nRow);
    virtual void VisibleRowsChanged( long nNewTopRow, sal_uInt16 nNumRows);
    virtual void PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const;
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const;
    virtual RowStatus GetRowStatus(long nRow) const;
    virtual sal_Bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);
    virtual void CursorMoved();
    virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);
    virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);
    virtual void Command(const CommandEvent& rEvt);
    virtual long PreNotify(NotifyEvent& rEvt);
    virtual void KeyInput(const KeyEvent& rEvt);
    virtual void StateChanged( StateChangedType nType );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    virtual void Select();

    virtual CellController* GetController(long nRow, sal_uInt16 nCol);

    virtual void CellModified();
    virtual sal_Bool SaveModified();
    virtual sal_Bool IsModified() const;

    virtual sal_uInt16 AppendColumn(const String& rName, sal_uInt16 nWidth = 0, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = (sal_uInt16)-1);
    virtual void RemoveColumn(sal_uInt16 nId);
    virtual DbGridColumn* CreateColumn(sal_uInt16 nId, const DbGridRowRef& _xRow) const;
    virtual void ColumnMoved(sal_uInt16 nId);
    virtual sal_Bool SaveRow();
    virtual sal_Bool IsTabAllowed(sal_Bool bForward) const;

    /// hide a column
    virtual void	HideColumn(sal_uInt16 nId);
    /// show a column
    virtual void	ShowColumn(sal_uInt16 nId);

    /**	This is called before executing a context menu for a row. rMenu contains the initial entries
        handled by this base class' method (which always has to be called).
        Derived classes may alter the menu in any way and handle any additional entries in
        PostExecuteColumnContextMenu.
        All disabled entries will be removed before executing the menu, so be careful with separators
        near entries you probably wish to disable ...
    */
    virtual void PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu);
    /**	After executing the context menu for a row this method is called.
    */
    virtual void PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

    virtual void DataSourcePropertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    virtual void FieldValueChanged(sal_uInt16 _nId, const ::com::sun::star::beans::PropertyChangeEvent& _evt);
    virtual void FieldListenerDisposing(sal_uInt16 _nId);

    virtual void disposing(sal_uInt16 _nId, const ::com::sun::star::lang::EventObject& _rEvt);

    // DragSourceHelper overridables
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    void	executeRowContextMenu( long _nRow, const Point& _rPreferredPos );

public:
    DbGridControl(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >,
        Window* pParent,
        WinBits nBits = WB_BORDER);
    DbGridControl(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >,
        Window* pParent,
        const ResId& rId);

    virtual ~DbGridControl();

    virtual void Init();
    virtual void InitColumnsByFields(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xFields) = 0;
    virtual void RemoveRows();

    /** GetCellText returns the text at the given position
        @param	_nRow
            the number of the row
        @param	_nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual String	GetCellText(long _nRow, USHORT _nColId) const;

    void RemoveRows(sal_Bool bNewCursor);

    void InvalidateStatus();

    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& getNumberFormatter() const {return m_xFormatter;}

    // die Datenquelle
    // die Optionen koennen die Updatefaehigkeiten noch einschraenken, nicht erweitern
    virtual void setDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& rCursor,
        sal_uInt16 nOpts = OPT_INSERT | OPT_UPDATE | OPT_DELETE);
    virtual void Dispatch(sal_uInt16 nId);

    CursorWrapper* getDataSource() const {return m_pDataCursor;}
    const DbGridColumns& GetColumns() const {return m_aColumns;}

    void EnableHandle(sal_Bool bEnable);
    sal_Bool HasHandle() const {return m_bHandle;}
    void InsertHandleColumn();

    /// welche Position hat die Spalte mit der Id in der ::com::sun::star::sdbcx::View, die Handle-Column zahelt nicht mit
    sal_uInt16 GetViewColumnPos( sal_uInt16 nId ) const { sal_uInt16 nPos = GetColumnPos(nId); return (nPos==(sal_uInt16)-1) ? -1 : nPos-1; }
    /**	welche Position hat die Spalte mit der Id in m_aColumns, also dem von GetColumns gelieferten ::com::sun::star::sdbcx::Container ?
        (unterscheidet sich u.U. von der von GetViewColumnPos gelieferten, wenn es versteckt Spalten gibt)
    */
    sal_uInt16 GetModelColumnPos( sal_uInt16 nId ) const;

    /// Anzahl der Spalten im Model
    sal_uInt16 GetViewColCount() const { return ColCount() - 1; }
    sal_uInt16 GetModelColCount() const { return (sal_uInt16)m_aColumns.Count(); }
    /// umgekehrt zu GetViewColumnPos : Id zu Position, die erste Nicht-Handle-Column hat die Position 0
    sal_uInt16 GetColumnIdFromViewPos( sal_uInt16 nPos ) const { return GetColumnId(nPos + 1); }
    sal_uInt16 GetColumnIdFromModelPos( sal_uInt16 nPos ) const;

    virtual void SetDesignMode(sal_Bool bMode);
    sal_Bool IsDesignMode() const {return m_bDesignMode;}
    sal_Bool IsOpen() const {return m_pSeekCursor != NULL;}

    virtual void SetFilterMode(sal_Bool bMode);
    sal_Bool IsFilterMode() const {return m_bFilterMode;}
    sal_Bool IsFilterRow(long nRow) const {return m_bFilterMode && nRow == 0;}

    void EnableNavigationBar(sal_Bool bEnable);
    sal_Bool HasNavigationBar() const {return m_bNavigationBar;}

    sal_uInt16 GetOptions() const {return m_nOptions;}
    NavigationBar& GetNavigationBar() {return m_aBar;}
    sal_uInt16 SetOptions(sal_uInt16 nOpt);
        // The new options are interpreted with respect to the current data source. If it is unable
        // to update, to insert or to restore, the according options are ignored. If the grid isn't
        // connected to a data source, all options except OPT_READONLY are ignored.

    void SetMultiSelection(sal_Bool bMulti);
    sal_Bool GetMultiSelection() const {return m_bMultiSelection;}

    const ::com::sun::star::util::Date&	getNullDate() const {return m_aNullDate;}

    // Positionierung
    void MoveToPosition(sal_uInt32 nPos);
    void MoveToFirst();
    void MoveToNext();
    void MoveToPrev();
    void MoveToLast();
    void AppendNew();

    // Abgleich der Cursor falls von auﬂen der DatenCursor verschoben wurde
    // Flag bedeutet ob ein Abgleich es Rowcounts vorgenommen werden soll
    void AdjustDataSource(sal_Bool bFull = sal_False);
    void Undo();

    virtual void BeginCursorAction();
    virtual void EndCursorAction();

    // wird die aktuelle Zeile neu gesetzt
    sal_Bool IsUpdating() const {return m_bUpdating;}

    virtual void RowRemoved( long nRow, long nNumRows = 1, sal_Bool bDoPaint = sal_True );
    virtual void RowInserted( long nRow, long nNumRows = 1, sal_Bool bDoPaint = sal_True );
    virtual void RowModified( long nRow, sal_uInt16 nColId = USHRT_MAX );

    void resetCurrentRow();

    sal_Bool getDisplaySynchron() const { return m_bSynchDisplay; }
    void setDisplaySynchron(sal_Bool bSync);
    void forceSyncDisplay();
        // wenn das auf sal_False gesetzt wird, laeuft die Anzeige nicht mehr mit der aktuellen Cursor-Position synchron
        // (soll heissen, in AdjustDataSource wird nicht in die zur CursorPosition gehoerenden Zeile gesprungen)
        // wer das benutzt, sollte bitte wissen, was er tut, da zum Beispiel die Eingabe von Daten in einer Zeile auf der Anzeige,
        // die gar nicht mit der Position des Cursors synchron ist, ziemlich kritisch sein koennte

    sal_Bool isForcedROController() const { return m_bForceROController; }
    void forceROController(sal_Bool bForce);
        // Setzt man das auf sal_True, hat das GridControl immer einen ::com::sun::star::frame::Controller, der allerdings read-only ist. Ausserdem
        // wird die Edit-Zeile des Controllers so eingestellt, dass sie ihre Selektion bei Fokus-Verlust weiterhin anzeigt.

    const DbGridRowRef& GetCurrentRow() const {return m_xCurrentRow;}

    void SetStateProvider(const Link& rProvider) { m_aMasterStateProvider = rProvider; }
        // if this link is set the given provider will be asked for the state of my items.
        // the return values are interpreted as follows :
        // <0 -> not specified (use default mechanism to determine the state)
        // ==0 -> the item is disabled
        // >0 -> the item is enabled
    void SetSlotExecutor(const Link& rExecutor) { m_aMasterSlotExecutor = rExecutor; }
        // analogous : if this link is set, all nav-bar slots will be routed through it when executed
        // if the handler returns nonzero, no further handling of the slot occurs

    void EnablePermanentCursor(sal_Bool bEnable);
    sal_Bool IsPermanentCursorEnabled() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        getServiceManager() const { return m_xServiceFactory; }

    /// returns <TRUE/> if the text of the given cell can be copied into the clipboard
    sal_Bool	canCopyCellText(sal_Int32 _nRow, sal_Int16 _nColId);
    /// copies the text of the given cell into the clipboard
    void		copyCellText(sal_Int32 _nRow, sal_Int16 _nColId);

    // selectin listener handling
    FmGridSelectionListener*	getSelectionListener() const { return m_pSelectionListener; }
    void						setSelectionListener(FmGridSelectionListener* _pListener) { m_pSelectionListener = _pListener; }

    // helper class to grant access to selected methods from within the DbCellControl class
    struct GrantCellControlAccess
    {
        friend class DbCellControl;
    protected:
        GrantCellControlAccess() { }
    };

    /// called when a controller needs to be re-initialized
    void refreshController(sal_uInt16 _nColId, GrantCellControlAccess _aAccess);

    /** 
        @return  
            The count of additional controls of the control area. 
    */
    virtual sal_Int32 GetAccessibleControlCount() const;

    /** Creates the accessible object of an additional control.
        @param _nIndex  
            The 0-based index of the control.
        @return  
            The XAccessible interface of the specified control. 
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleControl( sal_Int32 _nIndex );

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId );

protected:
    void RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, sal_Bool bUpdateCursor);
    sal_Bool SeekCursor(long nRow, sal_Bool bAbsolute = sal_False);
    void RemoveColumns();		// aufraeumen eigener strukturen
    void AdjustRows();
    sal_Int32 AlignSeekCursor();
    sal_Bool SetCurrent(long nNewRow, sal_Bool bCancelInsert);

    String GetCurrentRowCellText(DbGridColumn* pCol,const DbGridRowRef& _rRow) const;
    virtual void DeleteSelectedRows();
    sal_Bool IsValid(const DbGridRowRef& _xRow) const {return _xRow && _xRow->IsValid();}

    // Zeile an der gerade eingefuegt wird
    sal_Bool IsCurrentAppending() const;

    // Leerzeile zum Einfuegen
    sal_Bool IsEmptyRow(long nRow) const;

    void  SetSeekPos(sal_Int32 nPos) {m_nSeekPos = nPos;}
    sal_Int32 GetCurrentPos() const {return m_nCurrentPos;}
    sal_Int32 GetSeekPos() const {return m_nSeekPos;}
    sal_Int32 GetTotalCount() const {return m_nTotalCount;}

    const DbGridRowRef& GetEmptyRow() const {return m_xEmptyRow;}
    const DbGridRowRef& GetSeekRow() const {return m_xSeekRow;}
    CursorWrapper* GetSeekCursor() const {return m_pSeekCursor;}

    void ConnectToFields();
    void DisconnectFromFields();

    void implAdjustInSolarThread(sal_Bool _bRows);
        // calls AdjustRows or AdjustDataSource, synchron if the caller is running in the solar thread, else asynchron

protected:
    virtual void InitController(CellControllerRef& rController, long nRow, sal_uInt16 nCol);
    void Construct();
    void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    DECL_LINK(OnDelete, void*);

    DECL_LINK(OnAsyncAdjust, void*);
        // if the param is != NULL, AdjustRows will be called, else AdjustDataSource
};

SV_IMPL_REF(DbGridRow);

}//end of namespace binfilter
#endif // _SVX_GRIDCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
