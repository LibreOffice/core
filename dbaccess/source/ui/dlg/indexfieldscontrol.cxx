/*************************************************************************
 *
 *  $RCSfile: indexfieldscontrol.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 14:10:22 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXFIELDSCONTROL_HXX_
#include "indexfieldscontrol.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//......................................................................
namespace dbaui
{
//......................................................................

#define BROWSER_STANDARD_FLAGS      BROWSER_COLUMNSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL | \
                                    BROWSER_HIDECURSOR | BROWSER_HIDESELECT | BROWSER_AUTO_HSCROLL | BROWSER_AUTO_VSCROLL

#define COLUMN_ID_FIELDNAME     1
#define COLUMN_ID_ORDER         2

    using namespace ::com::sun::star::uno;

    //==================================================================
    //= DbaMouseDownListBoxController
    //==================================================================
    class DbaMouseDownListBoxController : public DbListBoxCellController
    {
    protected:
        Link    m_aOriginalModifyHdl;
        Link    m_aAdditionalModifyHdl;

    public:
        DbaMouseDownListBoxController(DbListBoxCtrl* _pParent)
            :DbListBoxCellController(_pParent)
        {
        }

        void SetAdditionalModifyHdl(const Link& _rHdl);

    protected:
        virtual sal_Bool WantMouseEvent() const { return sal_True; }
        virtual void SetModifyHdl(const Link& _rHdl);

    private:
        void implCheckLinks();
        DECL_LINK( OnMultiplexModify, void* );
    };

    //------------------------------------------------------------------
    void DbaMouseDownListBoxController::SetAdditionalModifyHdl(const Link& _rHdl)
    {
        m_aAdditionalModifyHdl = _rHdl;
        implCheckLinks();
    }

    //------------------------------------------------------------------
    void DbaMouseDownListBoxController::SetModifyHdl(const Link& _rHdl)
    {
        m_aOriginalModifyHdl = _rHdl;
        implCheckLinks();
    }

    //------------------------------------------------------------------
    IMPL_LINK( DbaMouseDownListBoxController, OnMultiplexModify, void*, _pArg )
    {
        if (m_aAdditionalModifyHdl.IsSet())
            m_aAdditionalModifyHdl.Call(_pArg);
        if (m_aOriginalModifyHdl.IsSet())
            m_aOriginalModifyHdl.Call(_pArg);
        return 0L;
    }

    //------------------------------------------------------------------
    void DbaMouseDownListBoxController::implCheckLinks()
    {
        if (m_aAdditionalModifyHdl.IsSet() || m_aOriginalModifyHdl.IsSet())
            DbListBoxCellController::SetModifyHdl(LINK(this, DbaMouseDownListBoxController, OnMultiplexModify));
        else
            DbListBoxCellController::SetModifyHdl(Link());
    }

    //==================================================================
    //= IndexFieldsControl
    //==================================================================
    //------------------------------------------------------------------
    IndexFieldsControl::IndexFieldsControl( Window* _pParent, const ResId& _rId ,sal_Int32 _nMaxColumnsInIndex)
        :DbBrowseBox(_pParent, _rId, DBBF_ACTIVATE_ON_BUTTONDOWN, BROWSER_STANDARD_FLAGS)
        ,m_aSeekRow(m_aFields.end())
        ,m_pSortingCell(NULL)
        ,m_pFieldNameCell(NULL)
        ,m_nMaxColumnsInIndex(_nMaxColumnsInIndex)
    {
    }

    //------------------------------------------------------------------
    IndexFieldsControl::~IndexFieldsControl()
    {
        delete m_pSortingCell;
        delete m_pFieldNameCell;
    }

    //------------------------------------------------------------------
    sal_Bool IndexFieldsControl::SeekRow(long nRow)
    {
        if (!DbBrowseBox::SeekRow(nRow))
            return sal_False;

        if (nRow < 0)
        {
            m_aSeekRow = m_aFields.end();
        }
        else
        {
            m_aSeekRow = m_aFields.begin() + nRow;
            OSL_ENSURE(m_aSeekRow <= m_aFields.end(), "IndexFieldsControl::SeekRow: invalid row!");
        }

        return sal_True;
    }

    //------------------------------------------------------------------
    void IndexFieldsControl::PaintCell( OutputDevice& _rDev, const Rectangle& _rRect, sal_uInt16 _nColumnId ) const
    {
        Point aPos(_rRect.TopLeft());
        aPos.X() += 1;

        String aText = GetCurrentRowCellText(_nColumnId);
        Size TxtSize(GetDataWindow().GetTextWidth(aText), GetDataWindow().GetTextHeight());

        // clipping
        if (aPos.X() < _rRect.Right() || aPos.X() + TxtSize.Width() > _rRect.Right() ||
            aPos.Y() < _rRect.Top() || aPos.Y() + TxtSize.Height() > _rRect.Bottom())
            _rDev.SetClipRegion( _rRect );

        // allow for a disabled control ...
        sal_Bool bEnabled = IsEnabled();
        Color aOriginalColor = _rDev.GetTextColor();
        if (!bEnabled)
            _rDev.SetTextColor(GetSettings().GetStyleSettings().GetDisableColor());

        // draw the text
        _rDev.DrawText(aPos, aText);

        // reset the color (if necessary)
        if (!bEnabled)
            _rDev.SetTextColor(aOriginalColor);

        if (_rDev.IsClipRegion())
            _rDev.SetClipRegion();
    }

    //------------------------------------------------------------------
    void IndexFieldsControl::initializeFrom(const IndexFields& _rFields)
    {
        // copy the field descriptions
        m_aFields = _rFields;
        m_aSeekRow = m_aFields.end();

        SetUpdateMode(sal_False);
        // remove all rows
        RowRemoved(1, GetRowCount());
        // insert rows for the the fields
        RowInserted(GetRowCount(), m_aFields.size(), sal_False);
        // insert an additional row for a new field for that index
        if(!m_nMaxColumnsInIndex || GetRowCount() < m_nMaxColumnsInIndex )
            RowInserted(GetRowCount(), 1, sal_False);
        SetUpdateMode(sal_True);

        GoToRowColumnId(0, COLUMN_ID_FIELDNAME);
    }

    //------------------------------------------------------------------
    void IndexFieldsControl::commitTo(IndexFields& _rFields)
    {
        // do not just copy the array, we may have empty field names (which should not be copied)
        _rFields.resize(m_aFields.size());
        ConstIndexFieldsIterator aSource = m_aFields.begin();
        ConstIndexFieldsIterator aSourceEnd = m_aFields.end();
        IndexFieldsIterator aDest = _rFields.begin();
        for (; aSource < aSourceEnd; ++aSource)
            if (0 != aSource->sFieldName.Len())
            {
                *aDest = *aSource;
                ++aDest;
            }

        _rFields.resize(aDest - _rFields.begin());
    }

    //------------------------------------------------------------------
    sal_uInt32 IndexFieldsControl::GetTotalCellWidth(long _nRow, sal_uInt16 _nColId)
    {
        if (COLUMN_ID_ORDER == _nColId)
        {
            sal_Int32 nWidthAsc = GetTextWidth(m_sAscendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            sal_Int32 nWidthDesc = GetTextWidth(m_sDescendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            // maximum plus some additional space
            return (nWidthAsc > nWidthDesc ? nWidthAsc : nWidthDesc) + GetTextWidth('0') * 2;
        }
        return DbBrowseBox::GetTotalCellWidth(_nRow, _nColId);
    }

    //------------------------------------------------------------------
    void IndexFieldsControl::Init(const Sequence< ::rtl::OUString >& _rAvailableFields)
    {
        RemoveColumns();

        m_sAscendingText = String(ModuleRes(STR_ORDER_ASCENDING));
        m_sDescendingText = String(ModuleRes(STR_ORDER_DESCENDING));

        // the "sort order" column
        String sColumnName = String(ModuleRes(STR_TAB_INDEX_SORTORDER));
        // the width of the order column is the maximum widths of the texts used
        // (the title of the column)
        sal_Int32 nSortOrderColumnWidth = GetTextWidth(sColumnName);
        // ("ascending" + scrollbar width)
        sal_Int32 nOther = GetTextWidth(m_sAscendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
        nSortOrderColumnWidth = nSortOrderColumnWidth > nOther ? nSortOrderColumnWidth : nOther;
        // ("descending" + scrollbar width)
        nOther = GetTextWidth(m_sDescendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
        nSortOrderColumnWidth = nSortOrderColumnWidth > nOther ? nSortOrderColumnWidth : nOther;
        // (plus some additional space)
        nSortOrderColumnWidth += GetTextWidth('0') * 2;
        InsertDataColumn(COLUMN_ID_ORDER, sColumnName, nSortOrderColumnWidth, HIB_STDSTYLE, 1);

        // for the width: both columns together should be somewhat smaller than the whole window (without the scrollbar)
        sal_Int32 nFieldNameWidth = GetSizePixel().Width();
        nFieldNameWidth -= nSortOrderColumnWidth;

        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        nFieldNameWidth -= aSystemStyle.GetScrollBarSize();
        nFieldNameWidth -= 8;

        // the "field name" column
        sColumnName = String(ModuleRes(STR_TAB_INDEX_FIELD));
        InsertDataColumn(COLUMN_ID_FIELDNAME, sColumnName, nFieldNameWidth, HIB_STDSTYLE, 0);

        // create the cell controllers
        // for the field name cell
        m_pFieldNameCell = new DbListBoxCtrl(&GetDataWindow());
        m_pFieldNameCell->InsertEntry(String());
        const ::rtl::OUString* pFields = _rAvailableFields.getConstArray();
        const ::rtl::OUString* pFieldsEnd = pFields + _rAvailableFields.getLength();
        for (;pFields < pFieldsEnd; ++pFields)
            m_pFieldNameCell->InsertEntry(*pFields);
        // for the sort cell
        m_pSortingCell = new DbListBoxCtrl(&GetDataWindow());
        m_pSortingCell->InsertEntry(m_sAscendingText);
        m_pSortingCell->InsertEntry(m_sDescendingText);
    }

    //------------------------------------------------------------------
    DbCellController* IndexFieldsControl::GetController(long _nRow, sal_uInt16 _nColumnId)
    {
        if (!IsEnabled())
            return NULL;

        ConstIndexFieldsIterator aRow;
        sal_Bool bNewField = !implGetFieldDesc(_nRow, aRow);

        DbaMouseDownListBoxController* pReturn = NULL;
        switch (_nColumnId)
        {
            case COLUMN_ID_ORDER:
                if (!bNewField && m_pSortingCell && 0 != aRow->sFieldName.Len())
                    pReturn = new DbaMouseDownListBoxController(m_pSortingCell);
                break;

            case COLUMN_ID_FIELDNAME:
                pReturn = new DbaMouseDownListBoxController(m_pFieldNameCell);
                break;

            default:
                OSL_ENSURE(sal_False, "IndexFieldsControl::GetController: invalid column id!");
        }

        if (pReturn)
            pReturn->SetAdditionalModifyHdl(LINK(this, IndexFieldsControl, OnListEntrySelected));

        return pReturn;
    }

    //------------------------------------------------------------------
    sal_Bool IndexFieldsControl::implGetFieldDesc(long _nRow, ConstIndexFieldsIterator& _rPos)
    {
        _rPos = m_aFields.end();
        if ((_nRow < 0) || (_nRow >= (sal_Int32)m_aFields.size()))
            return sal_False;
        _rPos = m_aFields.begin() + _nRow;
        return sal_True;
    }

    //------------------------------------------------------------------
    sal_Bool IndexFieldsControl::SaveModified()
    {
        switch (GetCurColumnId())
        {
            case COLUMN_ID_FIELDNAME:
            {
                String sFieldSelected = m_pFieldNameCell->GetSelectEntry();
                sal_Bool bEmptySelected = 0 == sFieldSelected.Len();
                if (isNewField())
                {
                    if (!bEmptySelected)
                    {
                        // add a new field to the collection
                        OIndexField aNewField;
                        aNewField.sFieldName = sFieldSelected;
                        m_aFields.push_back(aNewField);
                        RowInserted(GetRowCount(), 1, sal_True);
                    }
                }
                else
                {
                    sal_Int32 nRow = GetCurRow();
                    OSL_ENSURE(nRow < (sal_Int32)m_aFields.size(), "IndexFieldsControl::SaveModified: invalid current row!");
                    if (nRow >= 0)  // may be -1 in case the control was empty
                    {
                        // remove the field from the selection
                        IndexFieldsIterator aPos = m_aFields.begin() + nRow;

                        if (bEmptySelected)
                        {
                            aPos->sFieldName = String();

                            // invalidate the row to force repaint
                            Invalidate(GetRowRectPixel(nRow));
                            return sal_True;
                        }

                        if (sFieldSelected == aPos->sFieldName)
                            // nothing changed
                            return sal_True;

                        aPos->sFieldName = sFieldSelected;
                    }
                }

                Invalidate(GetRowRectPixel(GetCurRow()));
            }
            break;
            case COLUMN_ID_ORDER:
            {
                OSL_ENSURE(!isNewField(), "IndexFieldsControl::SaveModified: why the hell ...!!!");
                // selected entry
                sal_uInt16 nPos = m_pSortingCell->GetSelectEntryPos();
                OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND != nPos, "IndexFieldsControl::SaveModified: how did you get this selection??");
                // adjust the sort flag in the index field description
                OIndexField& rCurrentField = m_aFields[GetCurRow()];
                rCurrentField.bSortAscending = (0 == nPos);

            }
            break;
            default:
                OSL_ENSURE(sal_False, "IndexFieldsControl::SaveModified: invalid column id!");
        }
        return sal_True;
    }

    //------------------------------------------------------------------
    void IndexFieldsControl::InitController(DbCellControllerRef& _rController, long _nRow, sal_uInt16 _nColumnId)
    {
        ConstIndexFieldsIterator aFieldDescription;
        sal_Bool bNewField = !implGetFieldDesc(_nRow, aFieldDescription);

        switch (_nColumnId)
        {
            case COLUMN_ID_FIELDNAME:
                m_pFieldNameCell->SelectEntry(bNewField ? String() : aFieldDescription->sFieldName);
                m_pFieldNameCell->SaveValue();
                break;

            case COLUMN_ID_ORDER:
                m_pSortingCell->SelectEntry(aFieldDescription->bSortAscending ? m_sAscendingText : m_sDescendingText);
                m_pSortingCell->SaveValue();
                break;

            default:
                OSL_ENSURE(sal_False, "IndexFieldsControl::InitController: invalid column id!");
        }
    }

    //------------------------------------------------------------------
    IMPL_LINK( IndexFieldsControl, OnListEntrySelected, ListBox*, _pBox )
    {
        if (!_pBox->IsTravelSelect() && m_aModifyHdl.IsSet())
            m_aModifyHdl.Call(this);

        if (_pBox == m_pFieldNameCell)
        {   // a field has been selected
            if (GetCurRow() >= GetRowCount() - 2)
            {   // and we're in one of the last two rows
                String sSelectedEntry = m_pFieldNameCell->GetSelectEntry();
                sal_Int32 nCurrentRow = GetCurRow();
                sal_Int32 nRowCount = GetRowCount();

                OSL_ENSURE(((sal_Int32)(m_aFields.size() + 1)) == nRowCount, "IndexFieldsControl::OnListEntrySelected: inconsistence!");

                if (sSelectedEntry.Len() && (nCurrentRow == nRowCount - 1) && (!m_nMaxColumnsInIndex || nRowCount < m_nMaxColumnsInIndex ) )
                {   // in the last row, an non-empty string has been selected
                    // -> insert a new row
                    m_aFields.push_back(OIndexField());
                    RowInserted(GetRowCount(), 1);
                    Invalidate(GetRowRectPixel(nCurrentRow));
                }
                else if (!sSelectedEntry.Len() && (nCurrentRow == nRowCount - 2))
                {   // in the (last-1)th row, an empty entry has been selected
                    // -> remove the last row
                    m_aFields.erase(m_aFields.end() - 1);
                    RowRemoved(GetRowCount() - 1, 1);
                    Invalidate(GetRowRectPixel(nCurrentRow));
                }
            }

            SaveModified();
        }
        return 0L;
    }

    //------------------------------------------------------------------
    String IndexFieldsControl::GetCurrentRowCellText(sal_uInt16 nColId) const
    {
        if (m_aSeekRow < m_aFields.end())
        {
            switch (nColId)
            {
                case COLUMN_ID_FIELDNAME:
                    return m_aSeekRow->sFieldName;
                case COLUMN_ID_ORDER:
                    if (0 == m_aSeekRow->sFieldName.Len())
                        return String();
                    else
                        return m_aSeekRow->bSortAscending ? m_sAscendingText : m_sDescendingText;
                default:
                    OSL_ENSURE(sal_False, "IndexFieldsControl::GetCurrentRowCellText: invalid column id!");
            }
        }
        return String();
    }

//......................................................................
}   // namespace dbaui
//......................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/03/19 06:00:13  fs
 *  no controller if not enabled
 *
 *  Revision 1.1  2001/03/16 16:23:31  fs
 *  initial checkin - index design dialog and friends
 *
 *
 *  Revision 1.0 07.03.01 14:26:26  fs
 ************************************************************************/

