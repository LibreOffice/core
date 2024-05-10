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

#include "prtsetup.hxx"
#include <svdata.hxx>
#include <strings.hrc>

#include <officecfg/Office/Common.hxx>

using namespace psp;

void RTSDialog::insertAllPPDValues(weld::ComboBox& rBox, const PPDParser* pParser, const PPDKey* pKey )
{
    if( ! pKey || ! pParser )
        return;

    const PPDValue* pValue = nullptr;
    OUString aOptionText;

    for (int i = 0; i < pKey->countValues(); ++i)
    {
        pValue = pKey->getValue( i );
        if (pValue->m_bCustomOption)
            continue;
        aOptionText = pParser->translateOption( pKey->getKey(), pValue->m_aOption) ;

        OUString sId(weld::toId(pValue));
        int nCurrentPos = rBox.find_id(sId);
        if( m_aJobData.m_aContext.checkConstraints( pKey, pValue ) )
        {
            if (nCurrentPos == -1)
                rBox.append(sId, aOptionText);
        }
        else
        {
            if (nCurrentPos != -1)
                rBox.remove(nCurrentPos);
        }
    }
    pValue = m_aJobData.m_aContext.getValue( pKey );
    if (pValue && !pValue->m_bCustomOption)
    {
        OUString sId(weld::toId(pValue));
        int nPos = rBox.find_id(sId);
        if (nPos != -1)
            rBox.set_active(nPos);
    }
}

/*
 * RTSDialog
 */

RTSDialog::RTSDialog(const PrinterInfo& rJobData, weld::Window* pParent)
    : GenericDialogController(pParent, u"vcl/ui/printerpropertiesdialog.ui"_ustr, u"PrinterPropertiesDialog"_ustr)
    , m_aJobData(rJobData)
    , m_bDataModified(false)
    , m_xTabControl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xCancelButton(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xPaperPage(new RTSPaperPage(m_xTabControl->get_page(u"paper"_ustr), this))
    , m_xDevicePage(new RTSDevicePage(m_xTabControl->get_page(u"device"_ustr), this))
{
    OUString aTitle(m_xDialog->get_title());
    m_xDialog->set_title(aTitle.replaceAll("%s", m_aJobData.m_aPrinterName));

    m_xTabControl->connect_enter_page( LINK( this, RTSDialog, ActivatePage ) );
    m_xOKButton->connect_clicked( LINK( this, RTSDialog, ClickButton ) );
    m_xCancelButton->connect_clicked( LINK( this, RTSDialog, ClickButton ) );
    ActivatePage(m_xTabControl->get_current_page_ident());
}

RTSDialog::~RTSDialog()
{
}

IMPL_LINK(RTSDialog, ActivatePage, const OUString&, rPage, void)
{
    if (rPage == "paper")
        m_xPaperPage->update();
}

IMPL_LINK( RTSDialog, ClickButton, weld::Button&, rButton, void )
{
    if (&rButton == m_xOKButton.get())
    {
        // refresh the changed values
        if (m_xPaperPage)
        {
            // orientation
            m_aJobData.m_eOrientation = m_xPaperPage->getOrientation() == 0 ?
                orientation::Portrait : orientation::Landscape;
            // assume use of paper size from printer setup if the user
            // got here via File > Printer Settings ...
            if ( m_aJobData.meSetupMode == PrinterSetupMode::DocumentGlobal )
               m_aJobData.m_bPapersizeFromSetup = true;
        }
        if( m_xDevicePage )
        {
            m_aJobData.m_nColorDepth    = m_xDevicePage->getDepth();
            m_aJobData.m_nColorDevice   = m_xDevicePage->getColorDevice();
        }
        m_xDialog->response(RET_OK);
    }
    else if (&rButton == m_xCancelButton.get())
        m_xDialog->response(RET_CANCEL);
}

/*
 * RTSPaperPage
 */

RTSPaperPage::RTSPaperPage(weld::Widget* pPage, RTSDialog* pDialog)
    : m_xBuilder(Application::CreateBuilder(pPage, u"vcl/ui/printerpaperpage.ui"_ustr))
    , m_pParent(pDialog)
    , m_xContainer(m_xBuilder->weld_widget(u"PrinterPaperPage"_ustr))
    , m_xCbFromSetup(m_xBuilder->weld_check_button(u"papersizefromsetup"_ustr))
    , m_xPaperText(m_xBuilder->weld_label(u"paperft"_ustr))
    , m_xPaperBox(m_xBuilder->weld_combo_box(u"paperlb"_ustr))
    , m_xOrientText(m_xBuilder->weld_label(u"orientft"_ustr))
    , m_xOrientBox(m_xBuilder->weld_combo_box(u"orientlb"_ustr))
    , m_xDuplexText(m_xBuilder->weld_label(u"duplexft"_ustr))
    , m_xDuplexBox(m_xBuilder->weld_combo_box(u"duplexlb"_ustr))
    , m_xSlotText(m_xBuilder->weld_label(u"slotft"_ustr))
    , m_xSlotBox(m_xBuilder->weld_combo_box(u"slotlb"_ustr))
{
    //PrinterPaperPage
    m_xPaperBox->connect_changed( LINK( this, RTSPaperPage, SelectHdl ) );
    m_xOrientBox->connect_changed( LINK( this, RTSPaperPage, SelectHdl ) );
    m_xDuplexBox->connect_changed( LINK( this, RTSPaperPage, SelectHdl ) );
    m_xSlotBox->connect_changed( LINK( this, RTSPaperPage, SelectHdl ) );
    m_xCbFromSetup->connect_toggled( LINK( this, RTSPaperPage, CheckBoxHdl ) );

    update();
}

RTSPaperPage::~RTSPaperPage()
{
}

void RTSPaperPage::update()
{
    const PPDKey* pKey      = nullptr;

    // orientation
    m_xOrientBox->set_active(m_pParent->m_aJobData.m_eOrientation == orientation::Portrait ? 0 : 1);

    // duplex
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( u"Duplex"_ustr )) )
    {
        m_pParent->insertAllPPDValues( *m_xDuplexBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_xDuplexText->set_sensitive( false );
        m_xDuplexBox->set_sensitive( false );
    }

    // paper
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( u"PageSize"_ustr )) )
    {
        m_pParent->insertAllPPDValues( *m_xPaperBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_xPaperText->set_sensitive( false );
        m_xPaperBox->set_sensitive( false );
    }

    // input slots
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( u"InputSlot"_ustr )) )
    {
        m_pParent->insertAllPPDValues( *m_xSlotBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_xSlotText->set_sensitive( false );
        m_xSlotBox->set_sensitive( false );
    }

    if ( m_pParent->m_aJobData.meSetupMode != PrinterSetupMode::SingleJob )
        return;

    m_xCbFromSetup->show();

    if ( m_pParent->m_aJobData.m_bPapersizeFromSetup )
        m_xCbFromSetup->set_active(m_pParent->m_aJobData.m_bPapersizeFromSetup);
    // disable those, unless user wants to use papersize from printer prefs
    // as they have no influence on what's going to be printed anyway
    else
    {
        m_xPaperText->set_sensitive( false );
        m_xPaperBox->set_sensitive( false );
        m_xOrientText->set_sensitive( false );
        m_xOrientBox->set_sensitive( false );
    }
}

IMPL_LINK( RTSPaperPage, SelectHdl, weld::ComboBox&, rBox, void )
{
    const PPDKey* pKey = nullptr;
    if( &rBox == m_xPaperBox.get() )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( u"PageSize"_ustr );
    }
    else if( &rBox == m_xDuplexBox.get() )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( u"Duplex"_ustr );
    }
    else if( &rBox == m_xSlotBox.get() )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( u"InputSlot"_ustr );
    }
    else if( &rBox == m_xOrientBox.get() )
    {
        m_pParent->m_aJobData.m_eOrientation = m_xOrientBox->get_active() == 0 ? orientation::Portrait : orientation::Landscape;
    }
    if( pKey )
    {
        PPDValue* pValue = weld::fromId<PPDValue*>(rBox.get_active_id());
        m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
        update();
    }

    m_pParent->SetDataModified( true );
}

IMPL_LINK_NOARG(RTSPaperPage, CheckBoxHdl, weld::Toggleable&, void)
{
    bool bFromSetup = m_xCbFromSetup->get_active();
    m_pParent->m_aJobData.m_bPapersizeFromSetup = bFromSetup;
    m_xPaperText->set_sensitive(bFromSetup);
    m_xPaperBox->set_sensitive(bFromSetup);
    m_xOrientText->set_sensitive(bFromSetup);
    m_xOrientBox->set_sensitive(bFromSetup);
    m_pParent->SetDataModified(true);
}

/*
 * RTSDevicePage
 */
RTSDevicePage::RTSDevicePage(weld::Widget* pPage, RTSDialog* pParent)
    : m_xBuilder(Application::CreateBuilder(pPage, u"vcl/ui/printerdevicepage.ui"_ustr))
    , m_pCustomValue(nullptr)
    , m_pParent(pParent)
    , m_xContainer(m_xBuilder->weld_widget(u"PrinterDevicePage"_ustr))
    , m_xPPDKeyBox(m_xBuilder->weld_tree_view(u"options"_ustr))
    , m_xPPDValueBox(m_xBuilder->weld_tree_view(u"values"_ustr))
    , m_xCustomEdit(m_xBuilder->weld_entry(u"custom"_ustr))
    , m_xSpaceBox(m_xBuilder->weld_combo_box(u"colorspace"_ustr))
    , m_xDepthBox(m_xBuilder->weld_combo_box(u"colordepth"_ustr))
    , m_aReselectCustomIdle("RTSDevicePage m_aReselectCustomIdle")
{
    m_aReselectCustomIdle.SetInvokeHandler(LINK(this, RTSDevicePage, ImplHandleReselectHdl));

    m_xPPDKeyBox->set_size_request(m_xPPDKeyBox->get_approximate_digit_width() * 32,
                                   m_xPPDKeyBox->get_height_rows(12));

    m_xCustomEdit->connect_changed(LINK(this, RTSDevicePage, ModifyHdl));

    m_xPPDKeyBox->connect_changed( LINK( this, RTSDevicePage, SelectHdl ) );
    m_xPPDValueBox->connect_changed( LINK( this, RTSDevicePage, SelectHdl ) );

    m_xSpaceBox->connect_changed(LINK(this, RTSDevicePage, ComboChangedHdl));
    m_xDepthBox->connect_changed(LINK(this, RTSDevicePage, ComboChangedHdl));

    switch( m_pParent->m_aJobData.m_nColorDevice )
    {
        case 0:
            m_xSpaceBox->set_active(0);
            break;
        case 1:
            m_xSpaceBox->set_active(1);
            break;
        case -1:
            m_xSpaceBox->set_active(2);
            break;
    }

    if (m_pParent->m_aJobData.m_nColorDepth == 8)
        m_xDepthBox->set_active(0);
    else if (m_pParent->m_aJobData.m_nColorDepth == 24)
        m_xDepthBox->set_active(1);

    // fill ppd boxes
    if( !m_pParent->m_aJobData.m_pParser )
        return;

    for( int i = 0; i < m_pParent->m_aJobData.m_pParser->getKeys(); i++ )
    {
        const PPDKey* pKey = m_pParent->m_aJobData.m_pParser->getKey( i );

        // skip options already shown somewhere else
        // also skip options from the "InstallableOptions" PPD group
        // Options in that group define hardware features that are not
        // job-specific and should better be handled in the system-wide
        // printer configuration. Keyword is defined in PPD specification
        // (version 4.3), section 5.4.
        if( pKey->isUIKey()                   &&
            pKey->getKey() != "PageSize"      &&
            pKey->getKey() != "InputSlot"     &&
            pKey->getKey() != "PageRegion"    &&
            pKey->getKey() != "Duplex"        &&
            pKey->getGroup() != "InstallableOptions")
        {
            OUString aEntry( m_pParent->m_aJobData.m_pParser->translateKey( pKey->getKey() ) );
            m_xPPDKeyBox->append(weld::toId(pKey), aEntry);
        }
    }
}

RTSDevicePage::~RTSDevicePage()
{
}

sal_uLong RTSDevicePage::getDepth() const
{
    sal_uInt16 nSelectPos = m_xDepthBox->get_active();
    if (nSelectPos == 0)
        return 8;
    else
        return 24;
}

sal_uLong RTSDevicePage::getColorDevice() const
{
    sal_uInt16 nSelectPos = m_xSpaceBox->get_active();
    switch (nSelectPos)
    {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return -1;
    }
    return 0;
}

IMPL_LINK(RTSDevicePage, ModifyHdl, weld::Entry&, rEdit, void)
{
    if (m_pCustomValue)
    {
        // tdf#123734 Custom PPD option values are a CUPS extension to PPDs and the user-set value
        // needs to be prefixed with "Custom." in order to be processed properly
        m_pCustomValue->m_aCustomOption = "Custom." + rEdit.get_text();
        m_pCustomValue->m_bCustomOptionSetViaApp = true;
    }
}

IMPL_LINK( RTSDevicePage, SelectHdl, weld::TreeView&, rBox, void )
{
    if (&rBox == m_xPPDKeyBox.get())
    {
        const PPDKey* pKey = weld::fromId<PPDKey*>(m_xPPDKeyBox->get_selected_id());
        FillValueBox( pKey );
    }
    else if (&rBox == m_xPPDValueBox.get())
    {
        const PPDKey* pKey = weld::fromId<PPDKey*>(m_xPPDKeyBox->get_selected_id());
        const PPDValue* pValue = weld::fromId<PPDValue*>(m_xPPDValueBox->get_selected_id());
        if (pKey && pValue)
        {
            m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
            ValueBoxChanged(pKey);
        }
    }
    m_pParent->SetDataModified( true );
}

IMPL_LINK_NOARG( RTSDevicePage, ComboChangedHdl, weld::ComboBox&, void )
{
    m_pParent->SetDataModified( true );
}

void RTSDevicePage::FillValueBox( const PPDKey* pKey )
{
    m_xPPDValueBox->clear();
    m_xCustomEdit->hide();

    if( ! pKey )
        return;

    const PPDValue* pValue = nullptr;
    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        if( m_pParent->m_aJobData.m_aContext.checkConstraints( pKey, pValue ) &&
            m_pParent->m_aJobData.m_pParser )
        {
            OUString aEntry;
            if (pValue->m_bCustomOption)
                aEntry = VclResId(SV_PRINT_CUSTOM_TXT);
            else
                aEntry = m_pParent->m_aJobData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption);
            m_xPPDValueBox->append(weld::toId(pValue), aEntry);
        }
    }
    pValue = m_pParent->m_aJobData.m_aContext.getValue( pKey );
    m_xPPDValueBox->select_id(weld::toId(pValue));

    ValueBoxChanged(pKey);
}

IMPL_LINK_NOARG(RTSDevicePage, ImplHandleReselectHdl, Timer*, void)
{
    //in case selected entry is now not visible select it again to scroll it into view
    m_xPPDValueBox->select(m_xPPDValueBox->get_selected_index());
}

void RTSDevicePage::ValueBoxChanged( const PPDKey* pKey )
{
    const PPDValue* pValue = m_pParent->m_aJobData.m_aContext.getValue(pKey);
    if (pValue->m_bCustomOption)
    {
        m_pCustomValue = pValue;
        m_pParent->m_aJobData.m_aContext.setValue(pKey, pValue);
        // don't show the "Custom." prefix in the UI, s.a. comment in ModifyHdl
        m_xCustomEdit->set_text(m_pCustomValue->m_aCustomOption.replaceFirst("Custom.", ""));
        m_xCustomEdit->show();
        m_aReselectCustomIdle.Start();
    }
    else
        m_xCustomEdit->hide();
}

int SetupPrinterDriver(weld::Window* pParent, ::psp::PrinterInfo& rJobData)
{
    int nRet = 0;
    RTSDialog aDialog(rJobData, pParent);

    // return 0 if cancel was pressed or if the data
    // weren't modified, 1 otherwise
    if (aDialog.run() != RET_CANCEL)
    {
        rJobData = aDialog.getSetup();
        nRet = aDialog.GetDataModified() ? 1 : 0;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
