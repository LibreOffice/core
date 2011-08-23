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
#ifndef _SVX_FMGRIDCL_HXX
#define _SVX_FMGRIDCL_HXX

#include <com/sun/star/container/XIndexContainer.hpp>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

// alles nur fuer stl
#include <bf_svx/gridctrl.hxx>
#include <svtools/transfer.hxx>
namespace binfilter {

//==================================================================
// FmGridHeader
//==================================================================
struct FmGridHeaderData;
class FmGridHeader
            :public EditBrowserHeader
            ,public DropTargetHelper
{
protected:
    FmGridHeaderData*		m_pImpl;

    // trigger context menu execution
    void	triggerColumnContextMenu( const ::Point& _rPreferredPos );

public:
    FmGridHeader( BrowseBox* pParent, WinBits nWinBits = WB_STDHEADERBAR | WB_DRAG );
    ~FmGridHeader();

public:
    struct AccessControl { friend class FmGridControl; private: AccessControl() { } };

    inline	void	triggerColumnContextMenu( const ::Point& _rPreferredPos, const AccessControl& )
    {
        triggerColumnContextMenu( _rPreferredPos );
    }

protected:
    virtual void Command( const CommandEvent& rCEvt );
    virtual void RequestHelp( const HelpEvent& rHEvt );
    virtual void Select();

    /**	the value returned by GetItemPos is meaningless for the grid model if there are hidden columns,
        so use GetModelColumnPos instead
    */
    sal_uInt16 GetModelColumnPos(sal_uInt16 nId) const;

    /**	This is called before executing a context menu for a column. rMenu contains the initial entries
        handled by this base class' method (which always has to be called).
        Derived classes may alter the menu in any way and handle any additional entries in
        PostExecuteColumnContextMenu.
        All disabled entries will be removed before executing the menu, so be careful with separators
        near entries you probably wish to disable ...
    */
    virtual void	PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu);
    /**	After executing the context menu for a column this method is called.
    */
    virtual	void	PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

    // DropTargetHelper
    virtual sal_Int8	AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8	ExecuteDrop( const ExecuteDropEvent& rEvt );

    /** selects the column at the selection supplier.
        @param	nColumnId
            The column id.
    */
    void notifyColumnSelect(sal_uInt16 nColumnId);
private:
    DECL_LINK( OnAsyncExecuteDrop, void* );
};

//==================================================================
// FmGridControl
//==================================================================
class FmGridControl : public DbGridControl

{
    friend class FmGridHeader;
    friend class FmXGridPeer;

    FmXGridPeer*		m_pPeer;
    sal_Int32			m_nCurrentSelectedColumn;	// this is the column model (not the view) posisition ...
    sal_uInt16			m_nMarkedColumnId;
    sal_Bool			m_bSelecting;
    sal_Bool			m_bInColumnMove	: 1;

public:
    FmGridControl(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >,
        Window* pParent,
        FmXGridPeer* _pPeer,
        WinBits nBits);

    // Window
    virtual void KeyInput( const KeyEvent& rKEvt );

    // ::com::sun::star::beans::XPropertyChangeListener
    void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

    // ::com::sun::star::form::XPositioningListener
    void positioned(const ::com::sun::star::lang::EventObject& rEvent);

    // XBound
    sal_Bool commit();

    // ::com::sun::star::form::XInsertListener
    void inserted(const ::com::sun::star::lang::EventObject& rEvent);

    // ::com::sun::star::form::XRestoreListener
    void restored(const ::com::sun::star::lang::EventObject& rEvent);

    void markColumn(sal_uInt16 nId);
    sal_Bool isColumnMarked(sal_uInt16 nId) const;

    sal_Int32	GetSelectedColumn() const;

    /** return the name of the specified object. 
        @param	eObjType
            The type to ask for
        @param	_nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return  
            The name of the specified object. 
    */
    virtual ::rtl::OUString GetAccessibleName( AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

    /** return the description of the specified object. 
        @param	eObjType
            The type to ask for
        @param	_nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return  
            The description of the specified object. 
    */
    virtual ::rtl::OUString GetAccessibleDescription( AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

protected:
    virtual void Command(const CommandEvent& rEvt);

    virtual BrowserHeader* imp_CreateHeaderBar(BrowseBox* pParent);
    virtual void ColumnResized(sal_uInt16 nId);
    virtual void ColumnMoved(sal_uInt16 nId);
    virtual void DeleteSelectedRows();
    virtual void SetDesignMode(sal_Bool bMode);
    virtual void CellModified();
    virtual void HideColumn(sal_uInt16 nId);
    virtual void ShowColumn(sal_uInt16 nId);

    sal_Bool	IsInColumnMove() const {return m_bInColumnMove;}

    virtual void BeginCursorAction();
    virtual void EndCursorAction();
    virtual void Select();

    // Initialisieren der Spalten
    // a.) nur ueber Spaltenbeschreibung
    virtual void InitColumnsByModels(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& xColumns);
    // b.) im alivemode ueber Datenbankfelder
    virtual void InitColumnsByFields(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xFields);

            // some kind of impl version (for one single column) of our version of InitColumnsByFields
            void InitColumnByField(
                    DbGridColumn* _pColumn,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumnModel,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxFieldsByNames,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxFieldsByIndex
                );

    FmXGridPeer* GetPeer() const {return m_pPeer;}

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>
            getSelectionBookmarks();
    sal_Bool selectBookmarks(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& _rBookmarks);

    /** returns if a column is selected
        @param	nColumnId
            The column id.
        @param	_pColumn
            The column to compare with.
        @return
            <TRUE/> if the column is selected, otherwise <FALSE/>
    */
    sal_Bool isColumnSelected(sal_uInt16 nColumnId,DbGridColumn* _pColumn);
};

}//end of namespace binfilter
#endif // _SVX_FMGRIDCL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
