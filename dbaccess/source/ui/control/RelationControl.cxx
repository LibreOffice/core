/*************************************************************************
 *
 *  $RCSfile: RelationControl.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:35:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_RELATIONCONTROL_HXX
#include "RelationControl.hxx"
#endif
#ifndef DBACCESS_SOURCE_UI_INC_RELATIONCONTROL_HRC
#include "RelationControl.hrc"
#endif

#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include <svtools/editbrowsebox.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_RELCONTROLIFACE_HXX
#include "RelControliFace.hxx"
#endif
#ifndef _DBU_CONTROL_HRC_
#include "dbu_control.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

#include <algorithm>

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

        ::svt::ListBoxControl*                  m_pListCell;
        OTableConnectionData*                   m_pConnData;
        const OJoinTableView::OTableWindowMap*  m_pTableMap;
        OTableListBoxControl*                   m_pBoxControl;
        long                                    m_nDataPos;
        Reference< XPropertySet>                m_xSourceDef;
        Reference< XPropertySet>                m_xDestDef;


        void fillListBox(const Reference< XPropertySet>& _xDest,long nRow,USHORT nColumnId);
        /** returns the column id for the editbrowsebox
            @param  _nColId
                    the column id SOURCE_COLUMN or DEST_COLUMN

            @return the current column id eihter SOURCE_COLUMN or DEST_COLUMN depends on the connection data
        */
        USHORT getColumnIdent( USHORT _nColId ) const;
    public:
        ORelationControl( OTableListBoxControl* pParent,const OJoinTableView::OTableWindowMap* _pTableMap );
        virtual ~ORelationControl();

        /** searches for a connection between these two tables
            @param  _pSource
                    the left table
            @param  _pDest
                    the right window
        */
        void setWindowTables(const OTableWindow* _pSource,const OTableWindow* _pDest);

        /** allows to access the connection data from outside

            @return rthe connection data
        */
        inline OTableConnectionData* getData() const { return m_pConnData; }

        void lateInit();

    protected:
        virtual void Resize();

        virtual long PreNotify(NotifyEvent& rNEvt );

        virtual BOOL IsTabAllowed(BOOL bForward) const;

        virtual void Init(OTableConnectionData* _pConnData);
        virtual void Init() { ORelationControl_Base::Init(); }
        virtual void InitController( ::svt::CellControllerRef& rController, long nRow, USHORT nCol );
        virtual ::svt::CellController* GetController( long nRow, USHORT nCol );
        virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColId ) const;
        virtual BOOL SeekRow( long nRow );
        virtual BOOL SaveModified();
        virtual String GetCellText( long nRow, USHORT nColId );

        virtual void CellModified();

    private:

        DECL_LINK( AsynchActivate, void* );
        DECL_LINK( AsynchDeactivate, void* );
    };
    //========================================================================
    // class ORelationControl
    //========================================================================
    DBG_NAME(ORelationControl)
    //------------------------------------------------------------------------
    ORelationControl::ORelationControl( OTableListBoxControl* pParent ,const OJoinTableView::OTableWindowMap* _pTableMap)
        :EditBrowseBox( pParent, EBBF_SMART_TAB_TRAVEL | EBBF_NOROWPICTURE, WB_TABSTOP | WB_3DLOOK | WB_BORDER )
        ,m_pListCell( NULL )
        ,m_pConnData( NULL )
        ,m_xSourceDef( NULL )
        ,m_xDestDef( NULL )
        ,m_pTableMap(_pTableMap)
        ,m_pBoxControl(pParent)
    {
        DBG_CTOR(ORelationControl,NULL);
    }

    //------------------------------------------------------------------------
    ORelationControl::~ORelationControl()
    {
        DBG_DTOR(ORelationControl,NULL);

        delete m_pListCell;
    }

    //------------------------------------------------------------------------
    void ORelationControl::Init(OTableConnectionData* _pConnData)
    {
        DBG_CHKTHIS(ORelationControl,NULL);

        m_pConnData = _pConnData;
        OSL_ENSURE(m_pConnData, "No data supplied!");

        m_pConnData->normalizeLines();
    }
    //------------------------------------------------------------------------------
    void ORelationControl::lateInit()
    {
        OJoinTableView::OTableWindowMap::const_iterator aFind = m_pTableMap->find(m_pConnData->GetSourceWinName());
        if( aFind != m_pTableMap->end() )
            m_xSourceDef = aFind->second->GetTable();

        aFind = m_pTableMap->find(m_pConnData->GetDestWinName());
        if( aFind != m_pTableMap->end() )
            m_xDestDef = aFind->second->GetTable();

        if ( ColCount() == 0 )
        {
            InsertDataColumn( SOURCE_COLUMN, m_pConnData->GetSourceWinName(), 100);
            InsertDataColumn( DEST_COLUMN, m_pConnData->GetDestWinName(), 100);
                // wenn es die Defs noch nicht gibt, dann muessen sie noch mit SetSource-/-DestDef gesetzt werden !

            m_pListCell = new ListBoxControl( &GetDataWindow() );

            //////////////////////////////////////////////////////////////////////
            // set browse mode
            SetMode(    BROWSER_COLUMNSELECTION |
                        BROWSER_HLINESFULL      |
                        BROWSER_VLINESFULL      |
                        BROWSER_HIDECURSOR      |
                        BROWSER_HIDESELECT      |
                        BROWSER_AUTO_HSCROLL    |
                        BROWSER_AUTO_VSCROLL);
        }
        else
            // not the first call
            RowRemoved(0, GetRowCount());

        RowInserted(0, m_pConnData->GetConnLineDataList()->size(), TRUE);
    }
    //------------------------------------------------------------------------------
    void ORelationControl::Resize()
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        EditBrowseBox::Resize();
        long nOutputWidth = GetOutputSizePixel().Width();
        SetColumnWidth(1, (nOutputWidth / 2));
        SetColumnWidth(2, (nOutputWidth / 2));
    }

    //------------------------------------------------------------------------------
    long ORelationControl::PreNotify(NotifyEvent& rNEvt)
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        if (rNEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
            PostUserEvent(LINK(this, ORelationControl, AsynchDeactivate));
        else if (rNEvt.GetType() == EVENT_GETFOCUS)
            PostUserEvent(LINK(this, ORelationControl, AsynchActivate));

        return EditBrowseBox::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(ORelationControl, AsynchActivate, void*, EMPTYARG)
    {
        ActivateCell();
        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(ORelationControl, AsynchDeactivate, void*, EMPTYARG)
    {
        DeactivateCell();
        return 0L;
    }

    //------------------------------------------------------------------------------
    BOOL ORelationControl::IsTabAllowed(BOOL bForward) const
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        long nRow = GetCurRow();
        USHORT nCol = GetCurColumnId();

        BOOL bRet = !((     ( bForward && (nCol == DEST_COLUMN)     && (nRow == GetRowCount() - 1)))
                        ||  (!bForward && (nCol == SOURCE_COLUMN)   && (nRow == 0)));

        return bRet && EditBrowseBox::IsTabAllowed(bForward);
    }

    //------------------------------------------------------------------------------
    BOOL ORelationControl::SaveModified()
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        if ( GetCurRow() != BROWSER_ENDOFSELECTION )
        {
            OSL_ENSURE((sal_Int32)m_pConnData->GetConnLineDataList()->size() > GetCurRow(),"Invalid Index!");

            String sFieldName(m_pListCell->GetSelectEntry());

            OConnectionLineDataRef pConnLineData = (*m_pConnData->GetConnLineDataList())[GetCurRow()];
            switch( getColumnIdent( GetCurColumnId() ) )
            {
            case SOURCE_COLUMN:
                pConnLineData->SetSourceFieldName( sFieldName );
                break;
            case DEST_COLUMN:
                pConnLineData->SetDestFieldName( sFieldName );
                break;
            }
        }

        return TRUE;
    }
    //------------------------------------------------------------------------------
    USHORT ORelationControl::getColumnIdent( USHORT _nColId ) const
    {
        USHORT nId = _nColId;
        if ( m_pConnData->GetSourceWinName() != m_pBoxControl->getSourceWinName())
            nId = ( _nColId == SOURCE_COLUMN) ? DEST_COLUMN : SOURCE_COLUMN;
        return nId;
    }

    //------------------------------------------------------------------------------
    String ORelationControl::GetCellText( long nRow, USHORT nColId )
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        String sText;
        if ( m_pConnData->GetConnLineDataList()->size() > static_cast<size_t>(nRow) )
        {
            OConnectionLineDataRef pConnLineData = (*m_pConnData->GetConnLineDataList())[nRow];
            if( pConnLineData.isValid() )
            {
                switch( getColumnIdent( nColId ) )
                {
                case SOURCE_COLUMN:
                    sText  =pConnLineData->GetSourceFieldName();
                    break;
                case DEST_COLUMN:
                    sText  =pConnLineData->GetDestFieldName();
                    break;
                }
            }
        }
        return sText;
    }

    //------------------------------------------------------------------------------
    void ORelationControl::InitController( CellControllerRef& rController, long nRow, USHORT nColumnId )
    {
        DBG_CHKTHIS(ORelationControl,NULL);

        ULONG nHelpId = HID_RELATIONDIALOG_LEFTFIELDCELL;

        Reference< XPropertySet> xDef;
        switch ( getColumnIdent(nColumnId) )
        {
            case SOURCE_COLUMN:
                xDef    = m_xSourceDef;
                nHelpId = HID_RELATIONDIALOG_LEFTFIELDCELL;
                break;
            case DEST_COLUMN:
                xDef    = m_xDestDef;
                nHelpId = HID_RELATIONDIALOG_RIGHTFIELDCELL;
                break;
            default:
                //  ?????????
                break;
        }

        if ( xDef.is() )
        {
            fillListBox(xDef,nRow,nColumnId);
            String sName = GetCellText( nRow, nColumnId );
            m_pListCell->SelectEntry( sName );
            OSL_ENSURE(m_pListCell->GetSelectEntry() == sName,"Name was not selected!");

            m_pListCell->SetHelpId(nHelpId);
        }
    }

    //------------------------------------------------------------------------------
    CellController* ORelationControl::GetController( long nRow, USHORT nColumnId )
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        return new ListBoxCellController( m_pListCell );
    }

    //------------------------------------------------------------------------------
    BOOL ORelationControl::SeekRow( long nRow )
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        m_nDataPos = nRow;
        return TRUE;
    }

    //------------------------------------------------------------------------------
    void ORelationControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId ) const
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        String aText  =const_cast< ORelationControl*>(this)->GetCellText( m_nDataPos, nColumnId );

        Point aPos( rRect.TopLeft() );
        Size aTextSize( GetDataWindow().GetTextHeight(),GetDataWindow().GetTextWidth( aText ));

        if( aPos.X() < rRect.Right() || aPos.X() + aTextSize.Width() > rRect.Right() ||
            aPos.Y() < rRect.Top() || aPos.Y() + aTextSize.Height() > rRect.Bottom() )
            rDev.SetClipRegion( rRect );

        rDev.DrawText( aPos, aText );

        if( rDev.IsClipRegion() )
            rDev.SetClipRegion();
    }
    // -----------------------------------------------------------------------------
    void ORelationControl::fillListBox(const Reference< XPropertySet>& _xDest,long _nRow,USHORT nColumnId)
    {
        m_pListCell->Clear();
        try
        {
            if ( _xDest.is() )
            {
                sal_Int32 nRows = GetRowCount();
                Reference<XColumnsSupplier> xSup(_xDest,UNO_QUERY);
                Reference<XNameAccess> xColumns = xSup->getColumns();
                Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                const ::rtl::OUString* pBegin = aNames.getConstArray();
                const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                {
                    String sName = *pBegin;
                    sal_Int32 i = 0;
                    for (; i < nRows; ++i)
                        if(i != _nRow && GetCellText(i,nColumnId) == sName)
                            break;
                    if ( i == nRows )
                        m_pListCell->InsertEntry( *pBegin );
                }
                m_pListCell->InsertEntry(String(), 0);
            }
        }
        catch(SQLException&)
        {
            OSL_ENSURE(0,"Exception caught while compose tablename!");
        }
    }
    // -----------------------------------------------------------------------------
    void ORelationControl::setWindowTables(const OTableWindow* _pSource,const OTableWindow* _pDest)
    {
        // wenn ich hier gerade editiere, ausblenden
        BOOL bWasEditing = IsEditing();
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
            if ( pConn )
            {
                m_pConnData->CopyFrom(*pConn->GetData());
                m_pBoxControl->getContainer()->notifyConnectionChange(m_pConnData);
            }
            else
            {
                // no connection found so we clear our data
                OConnectionLineDataVec* pLines = m_pConnData->GetConnLineDataList();
                ::std::for_each(pLines->begin(),
                                pLines->end(),
                                OUnaryRefFunctor<OConnectionLineData>( ::std::mem_fun(&OConnectionLineData::Reset))
                                );

                m_pConnData->SetSourceWinName(_pSource->GetName());
                m_pConnData->SetDestWinName(_pDest->GetName());
            }
            m_pConnData->normalizeLines();

        }
        // neu zeichnen
        Invalidate();

        if ( bWasEditing )
        {
            GoToRow(0);
            ActivateCell();
        }
    }
    //------------------------------------------------------------------------
    void ORelationControl::CellModified()
    {
        DBG_CHKTHIS(ORelationControl,NULL);
        EditBrowseBox::CellModified();
        SaveModified();
        static_cast<OTableListBoxControl*>(GetParent())->NotifyCellChange();
    }
    //========================================================================
    // class OTableListBoxControl
    //========================================================================
    OTableListBoxControl::OTableListBoxControl(Window* _pParent,
                                               const ResId& _rResId,
                                               const OJoinTableView::OTableWindowMap* _pTableMap,
                                               IRelationControlInterface* _pParentDialog)
     : Window(_pParent,_rResId)
     , m_lmbLeftTable(          this, ResId(LB_LEFT_TABLE))
     , m_lmbRightTable(         this, ResId(LB_RIGHT_TABLE))
     , m_aFL_InvolvedTables(    this, ResId(FL_INVOLVED_TABLES))
     , m_aFL_InvolvedFields(    this, ResId(FL_INVOLVED_FIELDS))
     , m_pTableMap(_pTableMap)
     , m_pParentDialog(_pParentDialog)
    {
        m_pRC_Tables = new ORelationControl( this,m_pTableMap );
        m_pRC_Tables->SetHelpId(HID_RELDLG_KEYFIELDS);
        m_pRC_Tables->Init( );
        m_pRC_Tables->SetZOrder(&m_lmbRightTable, WINDOW_ZORDER_BEHIND);

        //////////////////////////////////////////////////////////////////////
        // positing BrowseBox control

        Point aDlgPoint = LogicToPixel( Point(12,43), MAP_APPFONT );
        Size aCurrentSize = GetSizePixel();
        Size aDlgSize = LogicToPixel( Size(24,60), MAP_APPFONT );
        aDlgSize.Width() = aCurrentSize.Width() - aDlgSize.Width();

        m_pRC_Tables->SetPosSizePixel( aDlgPoint, aDlgSize );
        m_pRC_Tables->Show();

        Link aLink(LINK(this, OTableListBoxControl, OnTableChanged));
        m_lmbLeftTable.SetSelectHdl(aLink);
        m_lmbRightTable.SetSelectHdl(aLink);

        FreeResource();
    }
    // -----------------------------------------------------------------------------
    OTableListBoxControl::~OTableListBoxControl()
    {
        ORelationControl* pTemp = m_pRC_Tables;
        m_pRC_Tables = NULL;
        delete pTemp;
    }
    // -----------------------------------------------------------------------------
    void OTableListBoxControl::fillListBoxes()
    {
        DBG_ASSERT(m_pTableMap->size() >= 2, "OTableListBoxControl::OTableListBoxControl : brauche mindestens zwei TabWins !");
        OTableWindow* pInitialLeft = NULL;
        OTableWindow* pInitialRight = NULL;

        // die Namen aller TabWins einsammeln
        OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->begin();
        for(;aIter != m_pTableMap->end();++aIter)
        {
            m_lmbLeftTable.InsertEntry(aIter->first);
            m_lmbRightTable.InsertEntry(aIter->first);

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

        // die entsprechenden Defs an mein Controls
        m_pRC_Tables->setWindowTables(pInitialLeft,pInitialRight);

        // die in einer ComboBox ausgewaehlte Tabelle darf nicht in der anderen zur Verfuegung stehen

        if ( m_pTableMap->size() > 2 )
        {
            m_lmbLeftTable.RemoveEntry(m_strCurrentRight);
            m_lmbRightTable.RemoveEntry(m_strCurrentLeft);
        }

        // links das erste, rechts das zweite selektieren
        m_lmbLeftTable.SelectEntry(m_strCurrentLeft);
        m_lmbRightTable.SelectEntry(m_strCurrentRight);

        m_lmbLeftTable.GrabFocus();
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( OTableListBoxControl, OnTableChanged, ListBox*, pListBox )
    {
        String strSelected(pListBox->GetSelectEntry());
        OTableWindow* pLeft     = NULL;
        OTableWindow* pRight    = NULL;

        // eine Sonderbehandlung : wenn es nur zwei Tabellen gibt, muss ich bei Wechsel in einer LB auch in der anderen umschalten
        if ( m_pTableMap->size() == 2 )
        {
            ListBox* pOther;
            if ( pListBox == &m_lmbLeftTable )
                pOther = &m_lmbRightTable;
            else
                pOther = &m_lmbLeftTable;
            pOther->SelectEntryPos(1 - pOther->GetSelectEntryPos());

            OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->begin();
            OTableWindow* pFirst = aIter->second;
            ++aIter;
            OTableWindow* pSecond = aIter->second;

            if ( m_lmbLeftTable.GetSelectEntry() == String(pFirst->GetName()) )
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
            // zuerst brauche ich die TableDef zur Tabelle, dazu das TabWin
            OJoinTableView::OTableWindowMap::const_iterator aFind = m_pTableMap->find(strSelected);
            OTableWindow* pLoop = NULL;
            if( aFind != m_pTableMap->end() )
                pLoop = aFind->second;
            DBG_ASSERT(pLoop != NULL, "ORelationDialog::OnTableChanged : ungueltiger Eintrag in ListBox !");
                // da ich die ListBoxen selber mit eben diesen Tabellennamen, mit denen ich sie jetzt vergleiche, gefuellt habe,
                // MUSS ich strSelected finden
            if (pListBox == &m_lmbLeftTable)
            {
                // den vorher links selektierten Eintrag wieder rein rechts
                m_lmbRightTable.InsertEntry(m_strCurrentLeft);
                // und den jetzt selektierten raus
                m_lmbRightTable.RemoveEntry(strSelected);
                m_strCurrentLeft    = strSelected;

                pLeft = pLoop;

                OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->find(m_lmbRightTable.GetSelectEntry());
                OSL_ENSURE( aIter != m_pTableMap->end(), "Invalid name");
                if ( aIter != m_pTableMap->end() )
                    pRight = aIter->second;

                m_lmbLeftTable.GrabFocus();
            }
            else
            {
                // den vorher rechts selektierten Eintrag wieder rein links
                m_lmbLeftTable.InsertEntry(m_strCurrentRight);
                // und den jetzt selektierten raus
                m_lmbLeftTable.RemoveEntry(strSelected);
                m_strCurrentRight = strSelected;

                pRight = pLoop;
                OJoinTableView::OTableWindowMap::const_iterator aIter = m_pTableMap->find(m_lmbLeftTable.GetSelectEntry());
                OSL_ENSURE( aIter != m_pTableMap->end(), "Invalid name");
                if ( aIter != m_pTableMap->end() )
                    pLeft = aIter->second;
            }
        }

        pListBox->GrabFocus();

        m_pRC_Tables->setWindowTables(pLeft,pRight);

        NotifyCellChange();
        return 0;
    }
    // -----------------------------------------------------------------------------
    void OTableListBoxControl::NotifyCellChange()
    {
        // den Ok-Button en- oder disablen, je nachdem, ob ich eine gueltige Situation habe
        BOOL bValid = TRUE;
        USHORT nEmptyRows = 0;
        OTableConnectionData* pConnData = m_pRC_Tables->getData();
        OConnectionLineDataVec* pLines = pConnData->GetConnLineDataList();
        OConnectionLineDataVec::iterator aIter = pLines->begin();
        for(;aIter != pLines->end();++aIter)
        {
            sal_Int32 nDestLen  = (*aIter)->GetDestFieldName().getLength();
            sal_Int32 nSrcLen   = (*aIter)->GetSourceFieldName().getLength();
            if ( (nDestLen != 0) != (nSrcLen != 0) )
                bValid = FALSE;
                // wenn nich beide leer oder beide voll sind -> ungueltig
            if ((nDestLen == 0) && (nSrcLen == 0))
                ++nEmptyRows;
        }
        m_pParentDialog->setValid(bValid && (nEmptyRows != pLines->size()));
            // nur leere Zeilen -> ungueltig

        if (nEmptyRows == 0)
        {
            pConnData->AppendConnLine(String(), String());
            m_pRC_Tables->DeactivateCell();
            m_pRC_Tables->RowInserted(m_pRC_Tables->GetRowCount(), 1, TRUE);
            m_pRC_Tables->ActivateCell();
        }
    }
    // -----------------------------------------------------------------------------
    void fillEntryAndDisable(ListBox& _rListBox,const String& _sEntry)
    {
        _rListBox.InsertEntry(_sEntry);
        _rListBox.SelectEntryPos(0);
        _rListBox.Disable();
    }
    // -----------------------------------------------------------------------------
    void OTableListBoxControl::fillAndDisable(OTableConnectionData* _pConnectionData)
    {
        fillEntryAndDisable(m_lmbLeftTable,_pConnectionData->GetSourceWinName());
        fillEntryAndDisable(m_lmbRightTable,_pConnectionData->GetDestWinName());
    }
    // -----------------------------------------------------------------------------
    void OTableListBoxControl::Init(OTableConnectionData* _pConnData)
    {
        m_pRC_Tables->Init(_pConnData);
    }
    // -----------------------------------------------------------------------------
    void OTableListBoxControl::lateInit()
    {
        m_pRC_Tables->lateInit();
    }
    // -----------------------------------------------------------------------------
    BOOL OTableListBoxControl::SaveModified()
    {
        BOOL bRet = m_pRC_Tables->SaveModified();
        m_pRC_Tables->getData()->normalizeLines();
        return bRet;
    }
    // -----------------------------------------------------------------------------
    String OTableListBoxControl::getSourceWinName() const
    {
        return m_lmbLeftTable.GetSelectEntryCount() ? m_lmbLeftTable.GetSelectEntry() : m_lmbLeftTable.GetEntry(0);
    }
    // -----------------------------------------------------------------------------
    String OTableListBoxControl::getDestWinName() const
    {
        return m_lmbRightTable.GetSelectEntryCount() ? m_lmbRightTable.GetSelectEntry() : m_lmbRightTable.GetEntry(0);
    }
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------

