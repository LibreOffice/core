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
#ifndef _SVX_FMGRIDCL_HXX
#define _SVX_FMGRIDCL_HXX

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <svx/gridctrl.hxx>
#include <svtools/transfer.hxx>
#include "svx/svxdllapi.h"

//==================================================================
// FmGridHeader
//==================================================================
struct FmGridHeaderData;
class SVX_DLLPUBLIC FmGridHeader
            :public ::svt::EditBrowserHeader
            ,public DropTargetHelper
{
protected:
    FmGridHeaderData*       m_pImpl;

    // trigger context menu execution
    void    triggerColumnContextMenu( const ::Point& _rPreferredPos );

public:
    FmGridHeader( BrowseBox* pParent, WinBits nWinBits = WB_STDHEADERBAR | WB_DRAG );
    ~FmGridHeader();

public:
    struct AccessControl { friend class FmGridControl; private: AccessControl() { } };

    inline  void    triggerColumnContextMenu( const ::Point& _rPreferredPos, const AccessControl& )
    {
        triggerColumnContextMenu( _rPreferredPos );
    }

protected:
    virtual void Command( const CommandEvent& rCEvt );
    virtual void RequestHelp( const HelpEvent& rHEvt );
    virtual void Select();

    /** the value returned by GetItemPos is meaningless for the grid model if there are hidden columns,
        so use GetModelColumnPos instead
    */
    sal_uInt16 GetModelColumnPos(sal_uInt16 nId) const;

    /** This is called before executing a context menu for a column. rMenu contains the initial entries
        handled by this base class' method (which always has to be called).
        Derived classes may alter the menu in any way and handle any additional entries in
        PostExecuteColumnContextMenu.
        All disabled entries will be removed before executing the menu, so be careful with separators
        near entries you probably wish to disable ...
    */
    virtual void    PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu);
    /** After executing the context menu for a column this method is called.
    */
    virtual void    PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    /** selects the column at the selection supplier.
        @param  nColumnId
            The column id.
    */
    void notifyColumnSelect(sal_uInt16 nColumnId);
private:
    DECL_LINK( OnAsyncExecuteDrop, void* );
};

//==================================================================
// FmGridControl
//==================================================================
class FmXGridPeer;
class SVX_DLLPUBLIC FmGridControl : public DbGridControl

{
    friend class FmGridHeader;
    friend class FmXGridPeer;

    FmXGridPeer*        m_pPeer;
    sal_Int32           m_nCurrentSelectedColumn;   // this is the column model (not the view) posisition ...
    sal_uInt16          m_nMarkedColumnId;
    sal_Bool            m_bSelecting;
    sal_Bool            m_bInColumnMove : 1;

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

    void markColumn(sal_uInt16 nId);
    sal_Bool isColumnMarked(sal_uInt16 nId) const;

    sal_Int32   GetSelectedColumn() const;

    /** return the name of the specified object.
        @param  eObjType
            The type to ask for
        @param  _nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return
            The name of the specified object.
    */
    virtual OUString GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

    /** return the description of the specified object.
        @param  eObjType
            The type to ask for
        @param  _nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return
            The description of the specified object.
    */
    virtual OUString GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

protected:
    virtual void Command(const CommandEvent& rEvt);

    virtual BrowserHeader* imp_CreateHeaderBar(BrowseBox* pParent);
    virtual long QueryMinimumRowHeight();
    virtual void RowHeightChanged();
    virtual void ColumnResized(sal_uInt16 nId);
    virtual void ColumnMoved(sal_uInt16 nId);
    virtual void DeleteSelectedRows();
    virtual void SetDesignMode(sal_Bool bMode);
    virtual void CellModified();
    virtual void HideColumn(sal_uInt16 nId);
    virtual void ShowColumn(sal_uInt16 nId);

    sal_Bool    IsInColumnMove() const {return m_bInColumnMove;}

    virtual void BeginCursorAction();
    virtual void EndCursorAction();
    virtual void Select();

    // Initialize columns
    // a.) only by column description
    virtual void InitColumnsByModels(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& xColumns);
    // b.) during alivemode by database fields
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
        @param  nColumnId
            The column id.
        @param  _pColumn
            The column to compare with.
        @return
            <TRUE/> if the column is selected, otherwise <FALSE/>
    */
    sal_Bool isColumnSelected(sal_uInt16 nColumnId,DbGridColumn* _pColumn);
};

#endif // _SVX_FMGRIDCL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
