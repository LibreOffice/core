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

#include "RelationControl.hxx"

#include <svtools/editbrowsebox.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/builderfactory.hxx>
#include "TableConnectionData.hxx"
#include "TableConnection.hxx"
#include "TableWindow.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "UITools.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "RelControliFace.hxx"
#include "dbu_control.hrc"
#include "dbaccess_helpid.hrc"
#include <osl/diagnose.h>

#include <list>
using std::list;
#include <utility>
using std::pair;
using std::make_pair;

#define SOURCE_COLUMN   1
#define DEST_COLUMN     2

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace svt;

    typedef ::svt::EditBrowseBox ORelationControl_Base;
    class ORelationControl : public ORelationControl_Base
    {
        friend class OTableListBoxControl;

        VclPtr< ::svt::ListBoxControl>          m_pListCell;
        TTableConnectionData::value_type        m_pConnData;
        OTableListBoxControl*                   m_pBoxControl;
        long                                    m_nDataPos;
        Reference< XPropertySet>                m_xSourceDef;
        Reference< XPropertySet>                m_xDestDef;
        enum opcode { DELETE, INSERT, MODIFY };
        typedef list< pair < opcode, pair < OConnectionLineDataVec::size_type, OConnectionLineDataVec::size_type> > > ops_type;
        ops_type                                m_ops;

        void fillListBox(const Reference< XPropertySet>& _xDest,long nRow,sal_uInt16 nColumnId);
        /** returns the column id for the editbrowsebox
            @param  _nColId
                    the column id SOURCE_COLUMN or DEST_COLUMN

            @return the current column id either SOURCE_COLUMN or DEST_COLUMN depends on the connection data
        */
        sal_uInt16 getColumnIdent( sal_uInt16 _nColId ) const;
    public:
        explicit ORelationControl(vcl::Window *pParent);
        void SetController(OTableListBoxControl* pController)
        {
            m_pBoxControl = pController;
        }

        /** searches for a connection between these two tables
            @param  _pSource
                    the left table
            @param  _pDest
                    the right window
        */
        void setWindowTables(const OTableWindow* _pSource,const OTableWindow* _pDest);

        /** allows to access the connection data from outside

            @return the connection data
        */
        inline TTableConnectionData::value_type getData() const { return m_pConnData; }

        void lateInit();

    protected:
        virtual ~ORelationControl() { disposeOnce(); }
        virtual void dispose() SAL_OVERRIDE { m_pListCell.disposeAndClear(); ORelationControl_Base::dispose(); }
        virtual void Resize() SAL_OVERRIDE;
        virtual Size GetOptimalSize() const SAL_OVERRIDE;
        virtual bool PreNotify(NotifyEvent& rNEvt ) SAL_OVERRIDE;

        virtual bool IsTabAllowed(bool bForward) const SAL_OVERRIDE;

        void Init(const TTableConnectionData::value_type& _pConnData);
        virtual void Init() SAL_OVERRIDE { ORelationControl_Base::Init(); }
        virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) SAL_OVERRIDE;
        virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) SAL_OVERRIDE;
        virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const SAL_OVERRIDE;
        virtual bool SeekRow( long nRow ) SAL_OVERRIDE;
        virtual bool SaveModified() SAL_OVERRIDE;
        virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const SAL_OVERRIDE;

        virtual void CellModified() SAL_OVERRIDE;

        DECL_LINK_TYPED( AsynchDeactivate, void*, void );
    private:

        DECL_LINK_TYPED( AsynchActivate, void*, void );

    };

    // class ORelationControl
    ORelationControl::ORelationControl(vcl::Window *pParent)
        : EditBrowseBox(pParent,
            EditBrowseBoxFlags::SMART_TAB_TRAVEL | EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT,
            WB_TABSTOP | WB_BORDER,
            BrowserMode::AUTOSIZE_LASTCOL)
        , m_pBoxControl(NULL)
        , m_nDataPos(0)
        , m_xSourceDef(NULL)
        , m_xDestDef(NULL)
    {
    }

    VCL_BUILDER_FACTORY(ORelationControl)

    void ORelationControl::Init(const TTableConnectionData::value_type& _pConnData)
    {

        m_pConnData = _pConnData;
        OSL_ENSURE(m_pConnData, "No data supplied!");

        m_pConnData->normalizeLines();
    }

    void ORelationControl::lateInit()
    {
        if ( !m_pConnData.get() )
            return;
        m_xSourceDef = m_pConnData->getReferencingTable()->getTable();
        m_xDestDef = m_pConnData->getReferencedTable()->getTable();

        if ( ColCount() == 0 )
        {
            InsertDataColumn( SOURCE_COLUMN, m_pConnData->getReferencingTable()->GetWinName(), 100);
            InsertDataColumn( DEST_COLUMN, m_pConnData->getReferencedTable()->GetWinName(), 100);
            // If the Defs do not yet exits, we need to set them with SetSource-/-DestDef

            m_pListCell.reset( VclPtr<ListBoxControl>::Create( &GetDataWindow() ) );

            // set browse mode
            SetMode(    BrowserMode::COLUMNSELECTION |
                        BrowserMode::HLINES      |
                        BrowserMode::VLINES      |
                        BrowserMode::HIDECURSOR      |
                        BrowserMode::HIDESELECT      |
                        BrowserMode::AUTO_HSCROLL    |
                        BrowserMode::AUTO_VSCROLL);
        }
        else
            // not the first call
            RowRemoved(0, GetRowCount());

        RowInserted(0, m_pConnData->GetConnLineDataList().size() + 1, true); // add one extra row
    }

    void ORelationControl::Resize()
    {
        EditBrowseBox::Resize();
        long nOutputWidth = GetOutputSizePixel().Width() - 1;
        SetColumnWidth(1, (nOutputWidth / 2));
        SetColumnWidth(2, (nOutputWidth / 2));
    }

    bool ORelationControl::PreNotify(NotifyEvent& rNEvt)
    {
        if (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS && !HasChildPathFocus() )
            PostUserEvent(LINK(this, ORelationControl, AsynchDeactivate), NULL, true);
        else if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
            PostUserEvent(LINK(this, ORelationControl, AsynchActivate), NULL, true);

        return EditBrowseBox::PreNotify(rNEvt);
    }

    IMPL_LINK_NOARG_TYPED(ORelationControl, AsynchActivate, void*, void)
    {
        ActivateCell();
    }

    IMPL_LINK_NOARG_TYPED(ORelationControl, AsynchDeactivate, void*, void)
    {
        DeactivateCell();
    }

    bool ORelationControl::IsTabAllowed(bool bForward) const
    {
        long nRow = GetCurRow();
        sal_uInt16 nCol = GetCurColumnId();

        bool bRet = !((     ( bForward && (nCol == DEST_COLUMN)     && (nRow == GetRowCount() - 1)))
                        ||  (!bForward && (nCol == SOURCE_COLUMN)   && (nRow == 0)));

        return bRet && EditBrowseBox::IsTabAllowed(bForward);
    }

    bool ORelationControl::SaveModified()
    {
        long nRow = GetCurRow();
        if ( nRow != BROWSER_ENDOFSELECTION )
        {
            OUString sFieldName(m_pListCell->GetSelectEntry());
            OConnectionLineDataVec& rLines = m_pConnData->GetConnLineDataList();
            if ( rLines.size() <= static_cast<OConnectionLineDataVec::size_type>(nRow) )
            {
                rLines.push_back(new OConnectionLineData());
                nRow = rLines.size() - 1;
                // add new past-rLines row
                m_ops.push_back(make_pair(INSERT, make_pair(nRow+1, nRow+2)));
            }

            OConnectionLineDataRef pConnLineData = rLines[nRow];

            switch( getColumnIdent( GetCurColumnId() ) )
            {
            case SOURCE_COLUMN:
                pConnLineData->SetSourceFieldName( sFieldName );
                break;
            case DEST_COLUMN:
                pConnLineData->SetDestFieldName( sFieldName );
                break;
            }
            // the modification we just did does *not* need to be registered in m_ops;
            // it is already taken into account (by the codepath that called us)
            //m_ops.push_back(make_pair(MODIFY, make_pair(nRow, nRow+1)));
        }

        const OConnectionLineDataVec::size_type oldSize = m_pConnData->GetConnLineDataList().size();
        OConnectionLineDataVec::size_type line = m_pConnData->normalizeLines();
        const OConnectionLineDataVec::size_type newSize = m_pConnData->GetConnLineDataList().size();
        assert(newSize <= oldSize);
        m_ops.push_back(make_pair(MODIFY, make_pair(line, newSize)));
        m_ops.push_back(make_pair(DELETE, make_pair(newSize, oldSize)));

        return true;
    }

    sal_uInt16 ORelationControl::getColumnIdent( sal_uInt16 _nColId ) const
    {
        sal_uInt16 nId = _nColId;
        if ( m_pConnData->getReferencingTable() != m_pBoxControl->getReferencingTable() )
            nId = ( _nColId == SOURCE_COLUMN) ? DEST_COLUMN : SOURCE_COLUMN;
        return nId;
    }

    OUString ORelationControl::GetCellText( long nRow, sal_uInt16 nColId ) const
    {
        OUString sText;
        if ( m_pConnData->GetConnLineDataList().size() > static_cast<size_t>(nRow) )
        {
            OConnectionLineDataRef pConnLineData = m_pConnData->GetConnLineDataList()[nRow];
            switch( getColumnIdent( nColId ) )
            {
            case SOURCE_COLUMN:
                sText  = pConnLineData->GetSourceFieldName();
                break;
            case DEST_COLUMN:
                sText  = pConnLineData->GetDestFieldName();
                break;
            }
        }
        return sText;
    }

    void ORelationControl::InitController( CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColumnId )
    {

        OString sHelpId( HID_RELATIONDIALOG_LEFTFIELDCELL );

        Reference< XPropertySet> xDef;
        switch ( getColumnIdent(nColumnId) )
        {
            case SOURCE_COLUMN:
                xDef    = m_xSourceDef;
                sHelpId = HID_RELATIONDIALOG_LEFTFIELDCELL;
                break;
            case DEST_COLUMN:
                xDef    = m_xDestDef;
                sHelpId = HID_RELATIONDIALOG_RIGHTFIELDCELL;
                break;
            default:
                //  ?????????
                break;
        }

        if ( xDef.is() )
        {
            fillListBox(xDef,nRow,nColumnId);
            OUString sName = GetCellText( nRow, nColumnId );
            m_pListCell->SelectEntry( sName );
            if ( m_pListCell->GetSelectEntry() != sName )
            {
                m_pListCell->InsertEntry( sName );
                m_pListCell->SelectEntry( sName );
            }

            m_pListCell->SetHelpId(sHelpId);
        }
    }

    CellController* ORelationControl::GetController( long /*nRow*/, sal_uInt16 /*nColumnId*/ )
    {
        return new ListBoxCellController( m_pListCell.get() );
    }

    bool ORelationControl::SeekRow( long nRow )
    {
        m_nDataPos = nRow;
        return true;
    }

    void ORelationControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const
    {
        OUString aText = GetCellText( m_nDataPos, nColumnId );

        Point aPos( rRect.TopLeft() );
        Size aTextSize( GetDataWindow().GetTextWidth( aText ), GetDataWindow().GetTextHeight() );

        if( aPos.X() < rRect.Left() || aPos.X() + aTextSize.Width() > rRect.Right() ||
            aPos.Y() < rRect.Top() || aPos.Y() + aTextSize.Height() > rRect.Bottom() )
        {
            rDev.SetClipRegion(vcl::Region(rRect));
        }

        rDev.DrawText( aPos, aText );

        if( rDev.IsClipRegion() )
            rDev.SetClipRegion();
    }
    void ORelationControl::fillListBox(const Reference< XPropertySet>& _xDest,long /*_nRow*/,sal_uInt16 /*nColumnId*/)
    {
        m_pListCell->Clear();
        try
        {
            if ( _xDest.is() )
            {
                //sal_Int32 nRows = GetRowCount();
                Reference<XColumnsSupplier> xSup(_xDest,UNO_QUERY);
                Reference<XNameAccess> xColumns = xSup->getColumns();
                Sequence< OUString> aNames = xColumns->getElementNames();
                const OUString* pIter = aNames.getConstArray();
                const OUString* pEnd = pIter + aNames.getLength();
                for(;pIter != pEnd;++pIter)
                {
                    m_pListCell->InsertEntry( *pIter );
                }
                m_pListCell->InsertEntry(OUString(), 0);
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    void ORelationControl::setWindowTables(const OTableWindow* _pSource,const OTableWindow* _pDest)
    {
        // If I edit here, hide
        bool bWasEditing = IsEditing();
        if ( bWasEditing )
            DeactivateCell();

        if ( _pSource && _pDest )
        {
            m_xSourceDef = _pSource->GetTable();
            SetColumnTitle(1, _pSource->GetName());

            m_xDestDef = _pDest->GetTable();
            SetColumnTitle(2, _pDest->GetName());

            const OJoinTableView* pView = _pSource->getTableView();
            OTableConnection* pConn = pView->GetTabConn(_pSource,_pDest);
            if ( pConn && !m_pConnData->GetConnLineDataList().empty() )
            {
                m_pConnData->CopyFrom(*pConn->GetData());
                m_pBoxControl->getContainer()->notifyConnectionChange();
            }
            else
            {
                // no connection found so we clear our data
                OConnectionLineDataVec& rLines = m_pConnData->GetConnLineDataList();
                for( const auto& rLine : rLines )
                {
                    rLine.get()->Reset();
                }

                m_pConnData->setReferencingTable(_pSource->GetData());
                m_pConnData->setReferencedTable(_pDest->GetData());
            }
            m_pConnData->normalizeLines();

        }
        // Repaint
        Invalidate();

        if ( bWasEditing )
        {
            GoToRow(0);
            ActivateCell();
        }
    }

    void ORelationControl::CellModified()
    {
        EditBrowseBox::CellModified();
        SaveModified();
        assert(m_pBoxControl);
        m_pBoxControl->NotifyCellChange();
    }

    Size ORelationControl::GetOptimalSize() const
    {
        return LogicToPixel(Size(140, 80), MAP_APPFONT);
    }

    // class OTableListBoxControl
    OTableListBoxControl::OTableListBoxControl(VclBuilderContainer* _pParent,
        const OJoinTableView::OTableWindowMap* _pTableMap,
        IRelationControlInterface* _pParentDialog)
        : m_pTableMap(_pTableMap)
        , m_pParentDialog(_pParentDialog)
    {
        _pParent->get(m_pLeftTable, "table1");
        _pParent->get(m_pRightTable, "table2");

        _pParent->get(m_pRC_Tables, "relations");
        m_pRC_Tables->SetController(this);
        m_pRC_Tables->Init();

        lateUIInit();

        Link<ListBox&,void> aLink(LINK(this, OTableListBoxControl, OnTableChanged));
        m_pLeftTable->SetSelectHdl(aLink);
        m_pRightTable->SetSelectHdl(aLink);
    }

    OTableListBoxControl::~OTableListBoxControl()
    {
    }

    void OTableListBoxControl::fillListBoxes()
    {
        OSL_ENSURE( !m_pTableMap->empty(), "OTableListBoxControl::fillListBoxes: no table window!");
        OTableWindow* pInitialLeft = NULL;
        OTableWindow* pInitialRight = NULL;

        // Collect the names of all TabWins
        OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->begin();
        OJoinTableView::OTableWindowMap::const_iterator aEnd = m_pTableMap->end();
        for(;aIter != aEnd;++aIter)
        {
            m_pLeftTable->InsertEntry(aIter->first);
            m_pRightTable->InsertEntry(aIter->first);

            if (!pInitialLeft)
            {
                pInitialLeft = aIter->second;
                m_strCurrentLeft = aIter->first;
            }
            else if (!pInitialRight)
            {
                pInitialRight = aIter->second;
                m_strCurrentRight = aIter->first;
            }
        }

        if ( !pInitialRight )
        {
            pInitialRight = pInitialLeft;
            m_strCurrentRight = m_strCurrentLeft;
        }

        // The corresponding Defs for my Controls
        m_pRC_Tables->setWindowTables(pInitialLeft,pInitialRight);

        // The table selected in a ComboBox must not be available in the other

        if ( m_pTableMap->size() > 2 )
        {
            m_pLeftTable->RemoveEntry(m_strCurrentRight);
            m_pRightTable->RemoveEntry(m_strCurrentLeft);
        }

        // Select the first one on the left side and on the right side,
        // select the second one
        m_pLeftTable->SelectEntry(m_strCurrentLeft);
        m_pRightTable->SelectEntry(m_strCurrentRight);

        m_pLeftTable->GrabFocus();
    }

    IMPL_LINK_TYPED( OTableListBoxControl, OnTableChanged, ListBox&, rListBox, void )
    {
        OUString strSelected(rListBox.GetSelectEntry());
        OTableWindow* pLeft     = NULL;
        OTableWindow* pRight    = NULL;

        // Special treatment: If there are only two tables, we need to switch the other one too when changing in a LB
        if ( m_pTableMap->size() == 2 )
        {
            ListBox* pOther;
            if (&rListBox == m_pLeftTable)
                pOther = m_pRightTable;
            else
                pOther = m_pLeftTable;
            pOther->SelectEntryPos(1 - pOther->GetSelectEntryPos());

            OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->begin();
            OTableWindow* pFirst = aIter->second;
            ++aIter;
            OTableWindow* pSecond = aIter->second;

            if ( m_pLeftTable->GetSelectEntry() == pFirst->GetName() )
            {
                pLeft   = pFirst;
                pRight  = pSecond;
            }
            else
            {
                pLeft   = pSecond;
                pRight  = pFirst;
            }
        }
        else
        {
            // First we need the TableDef to the Table and with it the TabWin
            OJoinTableView::OTableWindowMap::const_iterator aFind = m_pTableMap->find(strSelected);
            OTableWindow* pLoop = NULL;
            if( aFind != m_pTableMap->end() )
                pLoop = aFind->second;
            OSL_ENSURE(pLoop != NULL, "ORelationDialog::OnTableChanged: invalid ListBox entry!");
                // We need to find strSelect, because we filled the ListBoxes with the table names with which we compare now
            if (&rListBox == m_pLeftTable)
            {
                // Insert the previously selected Entry on the left side on the right side
                m_pRightTable->InsertEntry(m_strCurrentLeft);
                // Remove the currently selected Entry
                m_pRightTable->RemoveEntry(strSelected);
                m_strCurrentLeft    = strSelected;

                pLeft = pLoop;

                OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->find(m_pRightTable->GetSelectEntry());
                OSL_ENSURE( aIter != m_pTableMap->end(), "Invalid name");
                if ( aIter != m_pTableMap->end() )
                    pRight = aIter->second;

                m_pLeftTable->GrabFocus();
            }
            else
            {
                // Insert the previously selected Entry on the right side on the left side
                m_pLeftTable->InsertEntry(m_strCurrentRight);
                // Remove the currently selected Entry
                m_pLeftTable->RemoveEntry(strSelected);
                m_strCurrentRight = strSelected;

                pRight = pLoop;
                OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->find(m_pLeftTable->GetSelectEntry());
                OSL_ENSURE( aIter != m_pTableMap->end(), "Invalid name");
                if ( aIter != m_pTableMap->end() )
                    pLeft = aIter->second;
            }
        }

        rListBox.GrabFocus();

        m_pRC_Tables->setWindowTables(pLeft,pRight);

        NotifyCellChange();
    }

    void OTableListBoxControl::NotifyCellChange()
    {
        // Enable/disable the OK button, depending on having a valid situation
        TTableConnectionData::value_type pConnData = m_pRC_Tables->getData();
        const OConnectionLineDataVec& rLines = pConnData->GetConnLineDataList();
        bool bValid = !rLines.empty();
        if (bValid)
        {
            OConnectionLineDataVec::const_iterator l(rLines.begin());
            const OConnectionLineDataVec::const_iterator le(rLines.end());
            for (; bValid && l!=le; ++l)
            {
                bValid = ! ((*l)->GetSourceFieldName().isEmpty() || (*l)->GetDestFieldName().isEmpty());
            }
        }
        m_pParentDialog->setValid(bValid);

        ORelationControl::ops_type::iterator i (m_pRC_Tables->m_ops.begin());
        const ORelationControl::ops_type::const_iterator e (m_pRC_Tables->m_ops.end());
        m_pRC_Tables->DeactivateCell();
        for(; i != e; ++i)
        {
            switch(i->first)
            {
            case ORelationControl::DELETE:
                m_pRC_Tables->RowRemoved(i->second.first, i->second.second - i->second.first);
                break;
            case ORelationControl::INSERT:
                m_pRC_Tables->RowInserted(i->second.first, i->second.second - i->second.first);
                break;
            case ORelationControl::MODIFY:
                for(OConnectionLineDataVec::size_type j = i->second.first; j < i->second.second; ++j)
                    m_pRC_Tables->RowModified(j);
                break;
            }
        }
        m_pRC_Tables->ActivateCell();
        m_pRC_Tables->m_ops.clear();
    }

    void fillEntryAndDisable(ListBox& _rListBox,const OUString& _sEntry)
    {
        _rListBox.InsertEntry(_sEntry);
        _rListBox.SelectEntryPos(0);
        _rListBox.Disable();
    }

    void OTableListBoxControl::fillAndDisable(const TTableConnectionData::value_type& _pConnectionData)
    {
        fillEntryAndDisable(*m_pLeftTable, _pConnectionData->getReferencingTable()->GetWinName());
        fillEntryAndDisable(*m_pRightTable, _pConnectionData->getReferencedTable()->GetWinName());
    }

    void OTableListBoxControl::Init(const TTableConnectionData::value_type& _pConnData)
    {
        m_pRC_Tables->Init(_pConnData);
    }

    void OTableListBoxControl::lateUIInit()
    {
        m_pRC_Tables->Show();
        lateInit();
    }

    void OTableListBoxControl::lateInit()
    {
        m_pRC_Tables->lateInit();
    }

    void OTableListBoxControl::Disable()
    {
        m_pLeftTable->Disable();
        m_pRightTable->Disable();
        m_pRC_Tables->Disable();
    }

    void OTableListBoxControl::Invalidate()
    {
        m_pLeftTable->Invalidate();
        m_pRightTable->Invalidate();
        m_pRC_Tables->Invalidate();
    }

    bool OTableListBoxControl::SaveModified()
    {
        return m_pRC_Tables->SaveModified();
    }

    TTableWindowData::value_type OTableListBoxControl::getReferencingTable()    const
    {
        return m_pRC_Tables->getData()->getReferencingTable();
    }

    void OTableListBoxControl::enableRelation(bool _bEnable)
    {
        if ( !_bEnable )
            m_pRC_Tables->PostUserEvent(LINK(m_pRC_Tables, ORelationControl, AsynchDeactivate));
        m_pRC_Tables->Enable(_bEnable);

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
