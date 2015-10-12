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
#include <vcl/builderfactory.hxx>
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

        Link<const DriverPooling*,void>               m_aRowChangeHandler;

    public:
        explicit DriverListControl(vcl::Window* _pParent);

        virtual void Init() override;
                void Update(const DriverPoolingSettings& _rSettings);
        virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const override;

        // the handler will be called with a DriverPoolingSettings::const_iterator as parameter,
        // or NULL if no valid current row exists
        void SetRowChangeHandler(const Link<const DriverPooling*,void>& _rHdl) { m_aRowChangeHandler = _rHdl; }

        DriverPooling* getCurrentRow();
        void                                    updateCurrentRow();

        const DriverPoolingSettings& getSettings() const { return m_aSettings; }

        void        saveValue()             { m_aSavedSettings = m_aSettings; }
        bool    isModified() const;

    protected:
        virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) override;
        virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) override;

        virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const override;

        virtual bool SeekRow( long nRow ) override;
        virtual bool SaveModified() override;

        virtual bool IsTabAllowed(bool _bForward) const override;

        virtual void StateChanged( StateChangedType nStateChange ) override;

        virtual void CursorMoved() override;

    protected:
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;


    private:
        OUString implGetCellText(DriverPoolingSettings::const_iterator _rPos, sal_uInt16 _nColId) const;
    };


    DriverListControl::DriverListControl(vcl::Window* _pParent)
        :EditBrowseBox(_pParent, EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT, WB_BORDER,
                       BrowserMode::AUTO_VSCROLL | BrowserMode::AUTO_HSCROLL | BrowserMode::HIDECURSOR | BrowserMode::AUTOSIZE_LASTCOL | BrowserMode::KEEPHIGHLIGHT)
        ,m_aSeekRow(m_aSettings.end())
        ,m_sYes(CUI_RES(RID_SVXSTR_YES))
        ,m_sNo(CUI_RES(RID_SVXSTR_NO))
    {
        SetStyle((GetStyle() & ~WB_HSCROLL) | WB_AUTOHSCROLL);

        SetUniqueId(UID_OFA_CONNPOOL_DRIVERLIST_BACK);
    }

    VCL_BUILDER_FACTORY(DriverListControl)

    bool DriverListControl::IsTabAllowed(bool /*_bForward*/) const
    {
        // no travelling within the fields via RETURN and TAB
        return false;
    }


    bool DriverListControl::isModified() const
    {
        if (m_aSettings.size() != m_aSavedSettings.size())
            return true;

        DriverPoolingSettings::const_iterator aCurrent = m_aSettings.begin();
        DriverPoolingSettings::const_iterator aCurrentEnd = m_aSettings.end();
        DriverPoolingSettings::const_iterator aSaved = m_aSavedSettings.begin();
        for (;aCurrent != aCurrentEnd; ++aCurrent, ++aSaved)
        {
            if (*aCurrent != *aSaved)
                return true;
        }

        return false;
    }


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

    DriverPooling* DriverListControl::getCurrentRow()
    {
        OSL_ENSURE( ( GetCurRow() < m_aSettings.size() ) && ( GetCurRow() >= 0 ),
            "DriverListControl::getCurrentRow: invalid current row!");

        if ( ( GetCurRow() >= 0 ) && ( GetCurRow() < m_aSettings.size() ) )
            return &(*(m_aSettings.begin() + GetCurRow()));

        return NULL;
    }


    void DriverListControl::updateCurrentRow()
    {
        Window::Invalidate( GetRowRectPixel( GetCurRow() ), InvalidateFlags::Update );
    }


    void DriverListControl::Update(const DriverPoolingSettings& _rSettings)
    {
        m_aSettings = _rSettings;

        SetUpdateMode(false);
        RowRemoved(0, GetRowCount());
        RowInserted(0, m_aSettings.size());
        SetUpdateMode(true);

        ActivateCell(1, 0);
    }


    sal_uInt32 DriverListControl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
    {
        return GetDataWindow().GetTextWidth(GetCellText(nRow, nColId));
    }


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


    void DriverListControl::StateChanged( StateChangedType nStateChange )
    {
        if (StateChangedType::Enable == nStateChange)
            Window::Invalidate(InvalidateFlags::Update);
        EditBrowseBox::StateChanged( nStateChange );
    }


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


    void DriverListControl::InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol )
    {
        rController->GetWindow().SetText(GetCellText(nRow, nCol));
    }


    ::svt::CellController* DriverListControl::GetController( long /*nRow*/, sal_uInt16 /*nCol*/ )
    {
        return NULL;
    }


    bool DriverListControl::SaveModified()
    {
        return true;
    }


    bool DriverListControl::SeekRow( long _nRow )
    {
        EditBrowseBox::SeekRow(_nRow);

        if (_nRow < m_aSettings.size())
            m_aSeekRow = m_aSettings.begin() + _nRow;
        else
            m_aSeekRow = m_aSettings.end();

        return m_aSeekRow != m_aSettings.end();
    }


    void DriverListControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const
    {
        OSL_ENSURE(m_aSeekRow != m_aSettings.end(), "DriverListControl::PaintCell: invalid row!");

        if (m_aSeekRow != m_aSettings.end())
        {
            rDev.SetClipRegion(vcl::Region(rRect));

            DrawTextFlags nStyle = DrawTextFlags::Clip;
            if (!IsEnabled())
                nStyle |= DrawTextFlags::Disable;
            switch (nColId)
            {
                case 1: nStyle |= DrawTextFlags::Left; break;
                case 2:
                case 3: nStyle |= DrawTextFlags::Center; break;
            }

            rDev.DrawText(rRect, implGetCellText(m_aSeekRow, nColId), nStyle);

            rDev.SetClipRegion();
        }
    }

    ConnectionPoolOptionsPage::ConnectionPoolOptionsPage(vcl::Window* _pParent, const SfxItemSet& _rAttrSet)
        : SfxTabPage(_pParent, "ConnPoolPage", "cui/ui/connpooloptions.ui", &_rAttrSet)
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

    ConnectionPoolOptionsPage::~ConnectionPoolOptionsPage()
    {
        disposeOnce();
    }

    void ConnectionPoolOptionsPage::dispose()
    {
        m_pEnablePooling.clear();
        m_pDriversLabel.clear();
        m_pDriverList.clear();
        m_pDriverLabel.clear();
        m_pDriver.clear();
        m_pDriverPoolingEnabled.clear();
        m_pTimeoutLabel.clear();
        m_pTimeout.clear();
        SfxTabPage::dispose();
    }

    VclPtr<SfxTabPage> ConnectionPoolOptionsPage::Create(vcl::Window* _pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<ConnectionPoolOptionsPage>::Create(_pParent, *_rAttrSet);
    }

    void ConnectionPoolOptionsPage::implInitControls(const SfxItemSet& _rSet, bool /*_bFromReset*/)
    {
        // the enabled flag
        SFX_ITEMSET_GET( _rSet, pEnabled, SfxBoolItem, SID_SB_POOLING_ENABLED, true );
        OSL_ENSURE(pEnabled, "ConnectionPoolOptionsPage::implInitControls: missing the Enabled item!");
        m_pEnablePooling->Check(pEnabled == nullptr || pEnabled->GetValue());

        m_pEnablePooling->SaveValue();

        // the settings for the single drivers
        SFX_ITEMSET_GET( _rSet, pDriverSettings, DriverPoolingSettingsItem, SID_SB_DRIVER_TIMEOUTS, true );
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


    bool ConnectionPoolOptionsPage::Notify( NotifyEvent& _rNEvt )
    {
        if (MouseNotifyEvent::LOSEFOCUS == _rNEvt.GetType())
            if (m_pTimeout->IsWindowOrChild(_rNEvt.GetWindow()))
                commitTimeoutField();

        return SfxTabPage::Notify(_rNEvt);
    }


    bool ConnectionPoolOptionsPage::FillItemSet(SfxItemSet* _rSet)
    {
        commitTimeoutField();

        bool bModified = false;
        // the enabled flag
        if (m_pEnablePooling->IsValueChangedFromSaved())
        {
            _rSet->Put(SfxBoolItem(SID_SB_POOLING_ENABLED, m_pEnablePooling->IsChecked()), SID_SB_POOLING_ENABLED);
            bModified = true;
        }

        // the settings for the single drivers
        if (m_pDriverList->isModified())
        {
            _rSet->Put(DriverPoolingSettingsItem(SID_SB_DRIVER_TIMEOUTS, m_pDriverList->getSettings()), SID_SB_DRIVER_TIMEOUTS);
            bModified = true;
        }

        return bModified;
    }


    void ConnectionPoolOptionsPage::ActivatePage( const SfxItemSet& _rSet)
    {
        SfxTabPage::ActivatePage(_rSet);
        implInitControls(_rSet, false);
    }


    void ConnectionPoolOptionsPage::Reset(const SfxItemSet* _rSet)
    {
        implInitControls(*_rSet, true);
    }


    IMPL_LINK_TYPED( ConnectionPoolOptionsPage, OnDriverRowChanged, const DriverPooling*, pDriverPos, void )
    {
        bool bValidRow = (NULL != pDriverPos);
        m_pDriverPoolingEnabled->Enable(bValidRow && m_pEnablePooling->IsChecked());
        m_pTimeoutLabel->Enable(bValidRow);
        m_pTimeout->Enable(bValidRow);

        if (!bValidRow)
        {   // positioned on an invalid row
            m_pDriver->SetText(OUString());
        }
        else
        {
            m_pDriver->SetText(pDriverPos->sName);
            m_pDriverPoolingEnabled->Check(pDriverPos->bEnabled);
            m_pTimeout->SetText(OUString::number(pDriverPos->nTimeoutSeconds));

            OnEnabledDisabled(m_pDriverPoolingEnabled);
        }
    }


    void ConnectionPoolOptionsPage::commitTimeoutField()
    {
        if (DriverPooling* pCurrentDriver = m_pDriverList->getCurrentRow())
        {
            pCurrentDriver->nTimeoutSeconds = static_cast<long>(m_pTimeout->GetValue());
            m_pDriverList->updateCurrentRow();
        }
    }


    IMPL_LINK_TYPED( ConnectionPoolOptionsPage, OnEnabledDisabled, Button*, _pCheckBox, void )
    {
        bool bGloballyEnabled = m_pEnablePooling->IsChecked();
        bool bLocalDriverChanged = m_pDriverPoolingEnabled == _pCheckBox;

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
    }

} // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
