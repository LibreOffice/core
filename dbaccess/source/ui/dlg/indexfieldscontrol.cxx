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

#include <core_resource.hxx>
#include <indexfieldscontrol.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <osl/diagnose.h>
#include <helpids.h>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>

namespace dbaui
{

static constexpr auto BROWSER_STANDARD_FLAGS = BrowserMode::COLUMNSELECTION | BrowserMode::HLINES | BrowserMode::VLINES |
                                    BrowserMode::HIDECURSOR | BrowserMode::HIDESELECT | BrowserMode::AUTO_HSCROLL | BrowserMode::AUTO_VSCROLL;

#define COLUMN_ID_FIELDNAME     1
#define COLUMN_ID_ORDER         2

    using namespace ::com::sun::star::uno;
    using namespace ::svt;

    // DbaMouseDownListBoxController
    class DbaMouseDownListBoxController : public ListBoxCellController
    {
    protected:
        Link<DbaMouseDownListBoxController&,void>  m_aAdditionalModifyHdl;

    public:
        explicit DbaMouseDownListBoxController(ListBoxControl* _pParent)
            :ListBoxCellController(_pParent)
        {
        }

        void SetAdditionalModifyHdl(const Link<DbaMouseDownListBoxController&,void>& _rHdl);

    protected:
        virtual bool WantMouseEvent() const override { return true; }
        virtual void callModifyHdl() override;
    };

    void DbaMouseDownListBoxController::SetAdditionalModifyHdl(const Link<DbaMouseDownListBoxController&,void>& _rHdl)
    {
        m_aAdditionalModifyHdl = _rHdl;
    }

    void DbaMouseDownListBoxController::callModifyHdl()
    {
        m_aAdditionalModifyHdl.Call(*this);
        ListBoxCellController::callModifyHdl();
    }

    // IndexFieldsControl
    IndexFieldsControl::IndexFieldsControl( vcl::Window* _pParent, WinBits nWinStyle)
        : EditBrowseBox(_pParent, EditBrowseBoxFlags::SMART_TAB_TRAVEL | EditBrowseBoxFlags::ACTIVATE_ON_BUTTONDOWN, nWinStyle, BROWSER_STANDARD_FLAGS)
        , m_aSeekRow(m_aFields.end())
        , m_pSortingCell(nullptr)
        , m_pFieldNameCell(nullptr)
        , m_bAddIndexAppendix(false)
    {
    }

    extern "C" SAL_DLLPUBLIC_EXPORT void makeDbaIndexFieldsControl(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
    {
        rRet = VclPtr<IndexFieldsControl>::Create(pParent, WB_BORDER | WB_NOTABSTOP);
    }

    IndexFieldsControl::~IndexFieldsControl()
    {
        disposeOnce();
    }

    void IndexFieldsControl::dispose()
    {
        m_pSortingCell.disposeAndClear();
        m_pFieldNameCell.disposeAndClear();
        ::svt::EditBrowseBox::dispose();
    }

    bool IndexFieldsControl::SeekRow(long nRow)
    {
        if (!EditBrowseBox::SeekRow(nRow))
            return false;

        if (nRow < 0)
        {
            m_aSeekRow = m_aFields.end();
        }
        else
        {
            m_aSeekRow = m_aFields.begin() + nRow;
            OSL_ENSURE(m_aSeekRow <= m_aFields.end(), "IndexFieldsControl::SeekRow: invalid row!");
        }

        return true;
    }

    void IndexFieldsControl::PaintCell( OutputDevice& _rDev, const tools::Rectangle& _rRect, sal_uInt16 _nColumnId ) const
    {
        Point aPos(_rRect.TopLeft());
        aPos.AdjustX(1 );

        OUString aText = GetRowCellText(m_aSeekRow,_nColumnId);
        Size TxtSize(GetDataWindow().GetTextWidth(aText), GetDataWindow().GetTextHeight());

        // clipping
        if (aPos.X() < _rRect.Right() || aPos.X() + TxtSize.Width() > _rRect.Right() ||
            aPos.Y() < _rRect.Top() || aPos.Y() + TxtSize.Height() > _rRect.Bottom())
            _rDev.SetClipRegion(vcl::Region(_rRect));

        // allow for a disabled control ...
        bool bEnabled = IsEnabled();
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

    void IndexFieldsControl::initializeFrom(const IndexFields& _rFields)
    {
        // copy the field descriptions
        m_aFields = _rFields;
        m_aSeekRow = m_aFields.end();

        SetUpdateMode(false);
        // remove all rows
        RowRemoved(1, GetRowCount());
        // insert rows for the fields
        RowInserted(GetRowCount(), m_aFields.size(), false);
        // insert an additional row for a new field for that index
        RowInserted(GetRowCount(), 1, false);
        SetUpdateMode(true);

        GoToRowColumnId(0, COLUMN_ID_FIELDNAME);
    }

    void IndexFieldsControl::commitTo(IndexFields& _rFields)
    {
        // do not just copy the array, we may have empty field names (which should not be copied)
        _rFields.resize(m_aFields.size());
        IndexFields::iterator aDest = std::copy_if(m_aFields.begin(), m_aFields.end(), _rFields.begin(),
            [](const OIndexField& source) { return !source.sFieldName.isEmpty(); });

        _rFields.resize(aDest - _rFields.begin());
    }

    sal_uInt32 IndexFieldsControl::GetTotalCellWidth(long _nRow, sal_uInt16 _nColId)
    {
        if (COLUMN_ID_ORDER == _nColId)
        {
            sal_Int32 nWidthAsc = GetTextWidth(m_sAscendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            sal_Int32 nWidthDesc = GetTextWidth(m_sDescendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            // maximum plus some additional space
            return std::max(nWidthAsc, nWidthDesc) + GetTextWidth(OUString('0')) * 2;
        }
        return EditBrowseBox::GetTotalCellWidth(_nRow, _nColId);
    }

    void IndexFieldsControl::Init(const Sequence< OUString >& _rAvailableFields, bool _bAddIndexAppendix)
    {
        m_bAddIndexAppendix = _bAddIndexAppendix;

        RemoveColumns();

        // for the width: both columns together should be somewhat smaller than the whole window (without the scrollbar)
        sal_Int32 nFieldNameWidth = GetSizePixel().Width();

        if ( m_bAddIndexAppendix )
        {
            m_sAscendingText = DBA_RES(STR_ORDER_ASCENDING);
            m_sDescendingText = DBA_RES(STR_ORDER_DESCENDING);

            // the "sort order" column
            OUString sColumnName = DBA_RES(STR_TAB_INDEX_SORTORDER);
            // the width of the order column is the maximum widths of the texts used
            // (the title of the column)
            sal_Int32 nSortOrderColumnWidth = GetTextWidth(sColumnName);
            // ("ascending" + scrollbar width)
            sal_Int32 nOther = GetTextWidth(m_sAscendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            nSortOrderColumnWidth = std::max(nSortOrderColumnWidth, nOther);
            // ("descending" + scrollbar width)
            nOther = GetTextWidth(m_sDescendingText) + GetSettings().GetStyleSettings().GetScrollBarSize();
            nSortOrderColumnWidth = std::max(nSortOrderColumnWidth, nOther);
            // (plus some additional space)
            nSortOrderColumnWidth += GetTextWidth(OUString('0')) * 2;
            InsertDataColumn(COLUMN_ID_ORDER, sColumnName, nSortOrderColumnWidth, HeaderBarItemBits::STDSTYLE, 1);

            m_pSortingCell = VclPtr<ListBoxControl>::Create(&GetDataWindow());
            m_pSortingCell->InsertEntry(m_sAscendingText);
            m_pSortingCell->InsertEntry(m_sDescendingText);
            m_pSortingCell->SetHelpId( HID_DLGINDEX_INDEXDETAILS_SORTORDER );

            nFieldNameWidth -= nSortOrderColumnWidth;
        }
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        nFieldNameWidth -= aSystemStyle.GetScrollBarSize();
        nFieldNameWidth -= 8;
        // the "field name" column
        OUString sColumnName = DBA_RES(STR_TAB_INDEX_FIELD);
        InsertDataColumn(COLUMN_ID_FIELDNAME, sColumnName, nFieldNameWidth, HeaderBarItemBits::STDSTYLE, 0);

        // create the cell controllers
        // for the field name cell
        m_pFieldNameCell = VclPtr<ListBoxControl>::Create(&GetDataWindow());
        m_pFieldNameCell->InsertEntry(OUString());
        m_pFieldNameCell->SetHelpId( HID_DLGINDEX_INDEXDETAILS_FIELD );
        const OUString* pFields = _rAvailableFields.getConstArray();
        const OUString* pFieldsEnd = pFields + _rAvailableFields.getLength();
        for (;pFields < pFieldsEnd; ++pFields)
            m_pFieldNameCell->InsertEntry(*pFields);
    }

    CellController* IndexFieldsControl::GetController(long _nRow, sal_uInt16 _nColumnId)
    {
        if (!IsEnabled())
            return nullptr;

        IndexFields::const_iterator aRow;
        bool bNewField = !implGetFieldDesc(_nRow, aRow);

        DbaMouseDownListBoxController* pReturn = nullptr;
        switch (_nColumnId)
        {
            case COLUMN_ID_ORDER:
                if (!bNewField && m_pSortingCell && !aRow->sFieldName.isEmpty())
                    pReturn = new DbaMouseDownListBoxController(m_pSortingCell);
                break;

            case COLUMN_ID_FIELDNAME:
                pReturn = new DbaMouseDownListBoxController(m_pFieldNameCell);
                break;

            default:
                OSL_FAIL("IndexFieldsControl::GetController: invalid column id!");
        }

        if (pReturn)
            pReturn->SetAdditionalModifyHdl(LINK(this, IndexFieldsControl, OnListEntrySelected));

        return pReturn;
    }

    bool IndexFieldsControl::implGetFieldDesc(long _nRow, IndexFields::const_iterator& _rPos)
    {
        _rPos = m_aFields.end();
        if ((_nRow < 0) || (_nRow >= static_cast<sal_Int32>(m_aFields.size())))
            return false;
        _rPos = m_aFields.begin() + _nRow;
        return true;
    }

    bool IndexFieldsControl::SaveModified()
    {
        if (!IsModified())
            return true;

        switch (GetCurColumnId())
        {
            case COLUMN_ID_FIELDNAME:
            {
                OUString sFieldSelected = m_pFieldNameCell->GetSelectedEntry();
                bool bEmptySelected = sFieldSelected.isEmpty();
                if (isNewField())
                {
                    if (!bEmptySelected)
                    {
                        // add a new field to the collection
                        OIndexField aNewField;
                        aNewField.sFieldName = sFieldSelected;
                        m_aFields.push_back(aNewField);
                        RowInserted(GetRowCount());
                    }
                }
                else
                {
                    sal_Int32 nRow = GetCurRow();
                    OSL_ENSURE(nRow < static_cast<sal_Int32>(m_aFields.size()), "IndexFieldsControl::SaveModified: invalid current row!");
                    if (nRow >= 0)  // may be -1 in case the control was empty
                    {
                        // remove the field from the selection
                        IndexFields::iterator aPos = m_aFields.begin() + nRow;

                        if (bEmptySelected)
                        {
                            aPos->sFieldName.clear();

                            // invalidate the row to force repaint
                            Invalidate(GetRowRectPixel(nRow));
                            return true;
                        }

                        if (sFieldSelected == aPos->sFieldName)
                            // nothing changed
                            return true;

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
                sal_Int32 nPos = m_pSortingCell->GetSelectedEntryPos();
                OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND != nPos, "IndexFieldsControl::SaveModified: how did you get this selection??");
                // adjust the sort flag in the index field description
                OIndexField& rCurrentField = m_aFields[GetCurRow()];
                rCurrentField.bSortAscending = (0 == nPos);

            }
            break;
            default:
                OSL_FAIL("IndexFieldsControl::SaveModified: invalid column id!");
        }
        return true;
    }

    void IndexFieldsControl::InitController(CellControllerRef& /*_rController*/, long _nRow, sal_uInt16 _nColumnId)
    {
        IndexFields::const_iterator aFieldDescription;
        bool bNewField = !implGetFieldDesc(_nRow, aFieldDescription);

        switch (_nColumnId)
        {
            case COLUMN_ID_FIELDNAME:
                m_pFieldNameCell->SelectEntry(bNewField ? OUString() : aFieldDescription->sFieldName);
                m_pFieldNameCell->SaveValue();
                break;

            case COLUMN_ID_ORDER:
                m_pSortingCell->SelectEntry(aFieldDescription->bSortAscending ? m_sAscendingText : m_sDescendingText);
                m_pSortingCell->SaveValue();
                break;

            default:
                OSL_FAIL("IndexFieldsControl::InitController: invalid column id!");
        }
    }

    IMPL_LINK( IndexFieldsControl, OnListEntrySelected, DbaMouseDownListBoxController&, rController, void )
    {
        ListBoxControl& rListBox = rController.GetListBox();
        if (!rListBox.IsTravelSelect())
            m_aModifyHdl.Call(*this);

        if (&rListBox == m_pFieldNameCell.get())
        {   // a field has been selected
            if (GetCurRow() >= GetRowCount() - 2)
            {   // and we're in one of the last two rows
                OUString sSelectedEntry = m_pFieldNameCell->GetSelectedEntry();
                sal_Int32 nCurrentRow = GetCurRow();
                sal_Int32 rowCount = GetRowCount();

                OSL_ENSURE((static_cast<sal_Int32>(m_aFields.size() + 1)) == rowCount, "IndexFieldsControl::OnListEntrySelected: inconsistence!");

                if (!sSelectedEntry.isEmpty() && (nCurrentRow == rowCount - 1) /*&& (!m_nMaxColumnsInIndex || rowCount < m_nMaxColumnsInIndex )*/ )
                {   // in the last row, an non-empty string has been selected
                    // -> insert a new row
                    m_aFields.emplace_back();
                    RowInserted(GetRowCount());
                    Invalidate(GetRowRectPixel(nCurrentRow));
                }
                else if (sSelectedEntry.isEmpty() && (nCurrentRow == rowCount - 2))
                {   // in the (last-1)th row, an empty entry has been selected
                    // -> remove the last row
                    m_aFields.pop_back();
                    RowRemoved(GetRowCount() - 1);
                    Invalidate(GetRowRectPixel(nCurrentRow));
                }
            }

            SaveModified();
        }
    }
    OUString IndexFieldsControl::GetCellText(long _nRow,sal_uInt16 nColId) const
    {
        IndexFields::const_iterator aRow = m_aFields.end();
        if ( _nRow >= 0 )
        {
            aRow = m_aFields.begin() + _nRow;
            OSL_ENSURE(aRow <= m_aFields.end(), "IndexFieldsControl::SeekRow: invalid row!");
        }
        return GetRowCellText(aRow,nColId);
    }
    OUString IndexFieldsControl::GetRowCellText(const IndexFields::const_iterator& _rRow,sal_uInt16 nColId) const
    {
        if (_rRow < m_aFields.end())
        {
            switch (nColId)
            {
                case COLUMN_ID_FIELDNAME:
                    return _rRow->sFieldName;
                case COLUMN_ID_ORDER:
                    if (_rRow->sFieldName.isEmpty())
                        return OUString();
                    else
                        return _rRow->bSortAscending ? m_sAscendingText : m_sDescendingText;
                default:
                    OSL_FAIL("IndexFieldsControl::GetCurrentRowCellText: invalid column id!");
            }
        }
        return OUString();
    }
    bool IndexFieldsControl::IsTabAllowed(bool /*bForward*/) const
    {
        return false;
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
