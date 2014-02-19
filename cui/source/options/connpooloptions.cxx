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

#include "connpooloptions.hxx"
#include <svtools/editbrowsebox.hxx>
#include <vcl/field.hxx>
#include "connpoolsettings.hxx"
#include <svl/eitem.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

using ::svt::EditBrowseBox;

namespace offapp
{
    /// Widget for the Connection Pool options page
    class DriverListControl : public EditBrowseBox
    {
        using Window::Update;
    protected:
        DriverPoolingSettings                   m_aSavedSettings;
        DriverPoolingSettings                   m_aSettings;
        DriverPoolingSettings::const_iterator   m_aSeekRow;

        OUString                                m_sYes;
        OUString                                m_sNo;

        Link                                    m_aRowChangeHandler;

    public:
        DriverListControl(Window* _pParent);

        virtual void Init();
                void Update(const DriverPoolingSettings& _rSettings);
        virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const;

        // the handler will be called with a DriverPoolingSettings::const_iterator as parameter,
        // or NULL if no valid current row exists
        void SetRowChangeHandler(const Link& _rHdl) { m_aRowChangeHandler = _rHdl; }
        Link GetRowChangeHandler() const { return m_aRowChangeHandler; }

        const DriverPooling* getCurrentRow() const;
        DriverPooling* getCurrentRow();
        void                                    updateCurrentRow();

        const DriverPoolingSettings& getSettings() const { return m_aSettings; }

        void        saveValue()             { m_aSavedSettings = m_aSettings; }
        sal_Bool    isModified() const;

    protected:
        virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol );
        virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol );

        virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const;

        virtual sal_Bool SeekRow( long nRow );
        virtual sal_Bool SaveModified();

        virtual sal_Bool IsTabAllowed(sal_Bool _bForward) const;

        virtual void StateChanged( StateChangedType nStateChange );

        virtual void CursorMoved();

    protected:
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);


    private:
        OUString implGetCellText(DriverPoolingSettings::const_iterator _rPos, sal_uInt16 _nColId) const;
    };

    //--------------------------------------------------------------------
    DriverListControl::DriverListControl(Window* _pParent)
        :EditBrowseBox(_pParent, EBBF_NOROWPICTURE, BROWSER_AUTO_VSCROLL | BROWSER_AUTO_HSCROLL | BROWSER_HIDECURSOR | BROWSER_AUTOSIZE_LASTCOL | WB_BORDER)
        ,m_aSeekRow(m_aSettings.end())
        ,m_sYes(CUI_RES(RID_SVXSTR_YES))
        ,m_sNo(CUI_RES(RID_SVXSTR_NO))
    {
        SetStyle((GetStyle() & ~WB_HSCROLL) | WB_AUTOHSCROLL);

        SetUniqueId(UID_OFA_CONNPOOL_DRIVERLIST_BACK);
    }

    extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeDriverListControl(Window *pParent, VclBuilder::stringmap &)
    {
        return new DriverListControl(pParent);
    }

    //--------------------------------------------------------------------
    sal_Bool DriverListControl::IsTabAllowed(sal_Bool /*_bForward*/) const
    {
        // no travinling within the fields via RETURN and TAB
        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool DriverListControl::isModified() const
    {
        if (m_aSettings.size() != m_aSavedSettings.size())
            return sal_True;

        DriverPoolingSettings::const_iterator aCurrent = m_aSettings.begin();
        DriverPoolingSettings::const_iterator aCurrentEnd = m_aSettings.end();
        DriverPoolingSettings::const_iterator aSaved = m_aSavedSettings.begin();
        for (;aCurrent != aCurrentEnd; ++aCurrent, ++aSaved)
        {
            if (*aCurrent != *aSaved)
                return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    void DriverListControl::Init()
    {
        EditBrowseBox::Init();

        Size aColWidth = LogicToPixel(Size(160, 0), MAP_APPFONT);
        InsertDataColumn(1, OUString(CUI_RES(RID_SVXSTR_DRIVER_NAME)), aColWidth.Width());
        aColWidth = LogicToPixel(Size(30, 0), MAP_APPFONT);
        InsertDataColumn(2, OUString(CUI_RES(RID_SVXSTR_POOLED_FLAG)), aColWidth.Width());
        aColWidth = LogicToPixel(Size(60, 0), MAP_APPFONT);
        InsertDataColumn(3, OUString(CUI_RES(RID_SVXSTR_POOL_TIMEOUT)), aColWidth.Width());
            // Attention: the resource of the string is local to the resource of the enclosing dialog!
    }

    //--------------------------------------------------------------------
    void DriverListControl::CursorMoved()
    {
        EditBrowseBox::CursorMoved();

        // call the row change handler
        if ( m_aRowChangeHandler.IsSet() )
        {
            if ( GetCurRow() >= 0 )
            {   // == -1 may happen in case the browse box has just been cleared
                m_aRowChangeHandler.Call( getCurrentRow() );
            }
        }
    }

    //--------------------------------------------------------------------
    const DriverPooling* DriverListControl::getCurrentRow() const
    {
        OSL_ENSURE( ( GetCurRow() < m_aSettings.size() ) && ( GetCurRow() >= 0 ),
            "DriverListControl::getCurrentRow: invalid current row!");

        if ( ( GetCurRow() >= 0 ) && ( GetCurRow() < m_aSettings.size() ) )
            return &(*(m_aSettings.begin() + GetCurRow()));

        return NULL;
    }

    //--------------------------------------------------------------------
    DriverPooling* DriverListControl::getCurrentRow()
    {
        OSL_ENSURE( ( GetCurRow() < m_aSettings.size() ) && ( GetCurRow() >= 0 ),
            "DriverListControl::getCurrentRow: invalid current row!");

        if ( ( GetCurRow() >= 0 ) && ( GetCurRow() < m_aSettings.size() ) )
            return &(*(m_aSettings.begin() + GetCurRow()));

        return NULL;
    }

    //--------------------------------------------------------------------
    void DriverListControl::updateCurrentRow()
    {
        Window::Invalidate( GetRowRectPixel( GetCurRow() ), INVALIDATE_UPDATE );
    }

    //--------------------------------------------------------------------
    void DriverListControl::Update(const DriverPoolingSettings& _rSettings)
    {
        m_aSettings = _rSettings;

        SetUpdateMode(sal_False);
        RowRemoved(0, GetRowCount());
        RowInserted(0, m_aSettings.size());
        SetUpdateMode(sal_True);

        ActivateCell(1, 0);
    }

    //--------------------------------------------------------------------
    sal_uInt32 DriverListControl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
    {
        return GetDataWindow().GetTextWidth(GetCellText(nRow, nColId));
    }

    //--------------------------------------------------------------------
    OUString DriverListControl::implGetCellText(DriverPoolingSettings::const_iterator _rPos, sal_uInt16 _nColId) const
    {
        OSL_ENSURE(_rPos < m_aSettings.end(), "DriverListControl::implGetCellText: invalid position!");

        OUString sReturn;
        switch (_nColId)
        {
            case 1:
                sReturn = _rPos->sName;
                break;
            case 2:
                sReturn = _rPos->bEnabled ? m_sYes : m_sNo;
                break;
            case 3:
                if (_rPos->bEnabled)
                    sReturn = OUString::number(_rPos->nTimeoutSeconds);
                break;
            default:
                OSL_FAIL("DriverListControl::implGetCellText: invalid column id!");
        }
        return sReturn;
    }

    //--------------------------------------------------------------------
    void DriverListControl::StateChanged( StateChangedType nStateChange )
    {
        if (STATE_CHANGE_ENABLE == nStateChange)
            Window::Invalidate(INVALIDATE_UPDATE);
        EditBrowseBox::StateChanged( nStateChange );
    }

    //--------------------------------------------------------------------
    OUString DriverListControl::GetCellText( long nRow, sal_uInt16 nColId ) const
    {
        OUString sReturn;
        if (nRow > m_aSettings.size())
        {
            OSL_FAIL("DriverListControl::GetCellText: don't ask me for such rows!");
        }
        else
        {
            sReturn = implGetCellText(m_aSettings.begin() + nRow, nColId);
        }
        return sReturn;
    }

    //--------------------------------------------------------------------
    void DriverListControl::InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol )
    {
        rController->GetWindow().SetText(GetCellText(nRow, nCol));
    }

    //--------------------------------------------------------------------
    ::svt::CellController* DriverListControl::GetController( long /*nRow*/, sal_uInt16 /*nCol*/ )
    {
        return NULL;
    }

    //--------------------------------------------------------------------
    sal_Bool DriverListControl::SaveModified()
    {
        return sal_True;
    }

    //--------------------------------------------------------------------
    sal_Bool DriverListControl::SeekRow( long _nRow )
    {
        EditBrowseBox::SeekRow(_nRow);

        if (_nRow < m_aSettings.size())
            m_aSeekRow = m_aSettings.begin() + _nRow;
        else
            m_aSeekRow = m_aSettings.end();

        return m_aSeekRow != m_aSettings.end();
    }

    //--------------------------------------------------------------------
    void DriverListControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const
    {
        OSL_ENSURE(m_aSeekRow != m_aSettings.end(), "DriverListControl::PaintCell: invalid row!");

        if (m_aSeekRow != m_aSettings.end())
        {
            rDev.SetClipRegion(Region(rRect));

            sal_uInt16 nStyle = TEXT_DRAW_CLIP;
            if (!IsEnabled())
                nStyle |= TEXT_DRAW_DISABLE;
            switch (nColId)
            {
                case 1: nStyle |= TEXT_DRAW_LEFT; break;
                case 2:
                case 3: nStyle |= TEXT_DRAW_CENTER; break;
            }

            rDev.DrawText(rRect, implGetCellText(m_aSeekRow, nColId), nStyle);

            rDev.SetClipRegion();
        }
    }

    //====================================================================
    //= ConnectionPoolOptionsPage
    //====================================================================
    //--------------------------------------------------------------------
    ConnectionPoolOptionsPage::ConnectionPoolOptionsPage(Window* _pParent, const SfxItemSet& _rAttrSet)
        : SfxTabPage(_pParent, "ConnPoolPage", "cui/ui/connpooloptions.ui", _rAttrSet)
    {
        get(m_pEnablePooling, "connectionpooling");
        get(m_pDriversLabel, "driverslabel");
        get(m_pDriverList, "driverlist");
        get(m_pDriverLabel, "driverlabel");
        get(m_pDriver, "driver");
        get(m_pDriverPoolingEnabled, "enablepooling");
        get(m_pTimeoutLabel, "timeoutlabel");
        get(m_pTimeout, "timeout");

        Size aControlSize(248, 100);
        aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
        m_pDriverList->set_width_request(aControlSize.Width());
        m_pDriverList->set_height_request(aControlSize.Height());
        m_pDriverList->Init();
        m_pDriverList->Show();

        m_pEnablePooling->SetClickHdl( LINK(this, ConnectionPoolOptionsPage, OnEnabledDisabled) );
        m_pDriverPoolingEnabled->SetClickHdl( LINK(this, ConnectionPoolOptionsPage, OnEnabledDisabled) );

        m_pDriverList->SetRowChangeHandler( LINK(this, ConnectionPoolOptionsPage, OnDriverRowChanged) );
    }

    //--------------------------------------------------------------------
    SfxTabPage* ConnectionPoolOptionsPage::Create(Window* _pParent, const SfxItemSet& _rAttrSet)
    {
        return new ConnectionPoolOptionsPage(_pParent, _rAttrSet);
    }

    //--------------------------------------------------------------------
    void ConnectionPoolOptionsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool /*_bFromReset*/)
    {
        // the enabled flag
        SFX_ITEMSET_GET( _rSet, pEnabled, SfxBoolItem, SID_SB_POOLING_ENABLED, sal_True );
        OSL_ENSURE(pEnabled, "ConnectionPoolOptionsPage::implInitControls: missing the Enabled item!");
        m_pEnablePooling->Check(pEnabled ? pEnabled->GetValue() : sal_True);

        m_pEnablePooling->SaveValue();

        // the settings for the single drivers
        SFX_ITEMSET_GET( _rSet, pDriverSettings, DriverPoolingSettingsItem, SID_SB_DRIVER_TIMEOUTS, sal_True );
        if (pDriverSettings)
            m_pDriverList->Update(pDriverSettings->getSettings());
        else
        {
            OSL_FAIL("ConnectionPoolOptionsPage::implInitControls: missing the DriverTimeouts item!");
            m_pDriverList->Update(DriverPoolingSettings());
        }
        m_pDriverList->saveValue();

        // reflect the new settings
        OnEnabledDisabled(m_pEnablePooling);
    }

    //--------------------------------------------------------------------
    bool ConnectionPoolOptionsPage::Notify( NotifyEvent& _rNEvt )
    {
        if (EVENT_LOSEFOCUS == _rNEvt.GetType())
            if (m_pTimeout->IsWindowOrChild(_rNEvt.GetWindow()))
                commitTimeoutField();

        return SfxTabPage::Notify(_rNEvt);
    }

    //--------------------------------------------------------------------
    sal_Bool ConnectionPoolOptionsPage::FillItemSet(SfxItemSet& _rSet)
    {
        commitTimeoutField();

        sal_Bool bModified = sal_False;
        // the enabled flag
        if (m_pEnablePooling->GetSavedValue() != TriState(m_pEnablePooling->IsChecked()))
        {
            _rSet.Put(SfxBoolItem(SID_SB_POOLING_ENABLED, m_pEnablePooling->IsChecked()), SID_SB_POOLING_ENABLED);
            bModified = sal_True;
        }

        // the settings for the single drivers
        if (m_pDriverList->isModified())
        {
            _rSet.Put(DriverPoolingSettingsItem(SID_SB_DRIVER_TIMEOUTS, m_pDriverList->getSettings()), SID_SB_DRIVER_TIMEOUTS);
            bModified = sal_True;
        }

        return bModified;
    }

    //--------------------------------------------------------------------
    void ConnectionPoolOptionsPage::ActivatePage( const SfxItemSet& _rSet)
    {
        SfxTabPage::ActivatePage(_rSet);
        implInitControls(_rSet, sal_False);
    }

    //--------------------------------------------------------------------
    void ConnectionPoolOptionsPage::Reset(const SfxItemSet& _rSet)
    {
        implInitControls(_rSet, sal_True);
    }

    //--------------------------------------------------------------------
    IMPL_LINK( ConnectionPoolOptionsPage, OnDriverRowChanged, const void*, _pRowIterator )
    {
        sal_Bool bValidRow = (NULL != _pRowIterator);
        m_pDriverPoolingEnabled->Enable(bValidRow && m_pEnablePooling->IsChecked());
        m_pTimeoutLabel->Enable(bValidRow);
        m_pTimeout->Enable(bValidRow);

        if (!bValidRow)
        {   // positioned on an invalid row
            m_pDriver->SetText(OUString());
        }
        else
        {
            const DriverPooling *pDriverPos = static_cast<const DriverPooling*>(_pRowIterator);

            m_pDriver->SetText(pDriverPos->sName);
            m_pDriverPoolingEnabled->Check(pDriverPos->bEnabled);
            m_pTimeout->SetText(OUString::number(pDriverPos->nTimeoutSeconds));

            OnEnabledDisabled(m_pDriverPoolingEnabled);
        }

        return 0L;
    }

    //--------------------------------------------------------------------
    void ConnectionPoolOptionsPage::commitTimeoutField()
    {
        if (DriverPooling* pCurrentDriver = m_pDriverList->getCurrentRow())
        {
            pCurrentDriver->nTimeoutSeconds = static_cast<long>(m_pTimeout->GetValue());
            m_pDriverList->updateCurrentRow();
        }
    }

    //--------------------------------------------------------------------
    IMPL_LINK( ConnectionPoolOptionsPage, OnEnabledDisabled, const CheckBox*, _pCheckBox )
    {
        sal_Bool bGloballyEnabled = m_pEnablePooling->IsChecked();
        sal_Bool bLocalDriverChanged = m_pDriverPoolingEnabled == _pCheckBox;

        if (m_pEnablePooling == _pCheckBox)
        {
            m_pDriversLabel->Enable(bGloballyEnabled);
            m_pDriverList->Enable(bGloballyEnabled);
            m_pDriverLabel->Enable(bGloballyEnabled);
            m_pDriver->Enable(bGloballyEnabled);
            m_pDriverPoolingEnabled->Enable(bGloballyEnabled);
        }
        else
            OSL_ENSURE(bLocalDriverChanged, "ConnectionPoolOptionsPage::OnEnabledDisabled: where did this come from?");

        m_pTimeoutLabel->Enable(bGloballyEnabled && m_pDriverPoolingEnabled->IsChecked());
        m_pTimeout->Enable(bGloballyEnabled && m_pDriverPoolingEnabled->IsChecked());

        if (bLocalDriverChanged)
        {
            // update the list
            m_pDriverList->getCurrentRow()->bEnabled = m_pDriverPoolingEnabled->IsChecked();
            m_pDriverList->updateCurrentRow();
        }

        return 0L;
    }

} // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
