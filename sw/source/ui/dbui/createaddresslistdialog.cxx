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

#include <sal/config.h>

#include <cstddef>

#include <osl/diagnose.h>
#include <swtypes.hxx>
#include "createaddresslistdialog.hxx"
#include "customizeaddresslistdialog.hxx"
#include <mmconfigitem.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <svtools/controldims.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <rtl/textenc.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <tools/urlobj.hxx>
#include <dbui.hrc>
#include <strings.hrc>
#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;

struct SwAddressFragment
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xOrigContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Entry> m_xEntry;

    SwAddressFragment(weld::Container* pGrid, int nLine)
        : m_xBuilder(Application::CreateBuilder(pGrid, "modules/swriter/ui/addressfragment.ui"))
        , m_xLabel(m_xBuilder->weld_label("label"))
        , m_xEntry(m_xBuilder->weld_entry("entry"))
    {
        m_xLabel->set_grid_left_attach(0);
        m_xLabel->set_grid_top_attach(nLine);

        m_xEntry->set_grid_left_attach(1);
        m_xEntry->set_grid_top_attach(nLine);
    }
};

class SwAddressControl_Impl
{
    std::map<weld::Entry*, sal_Int32> m_aEditLines;

    SwCSVData*                      m_pData;
    sal_uInt32                      m_nCurrentDataSet;

    bool                            m_bNoDataSet;

    std::unique_ptr<weld::ScrolledWindow> m_xScrollBar;
    std::unique_ptr<weld::Container> m_xWindow;
    std::vector<std::unique_ptr<SwAddressFragment>> m_aLines;

    DECL_LINK(GotFocusHdl_Impl, weld::Widget&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::Entry&, void);

    void                MakeVisible(const tools::Rectangle& aRect);

public:
    SwAddressControl_Impl(weld::Builder& rBuilder);

    void        SetData(SwCSVData& rDBData);

    void        SetCurrentDataSet(sal_uInt32 nSet);
    void        CurrentDataSetInvalidated() { m_nCurrentDataSet = std::numeric_limits<sal_uInt32>::max(); }
    sal_uInt32  GetCurrentDataSet() const { return m_nCurrentDataSet; }
    void        SetCursorTo(std::size_t nElement);
};

SwAddressControl_Impl::SwAddressControl_Impl(weld::Builder& rBuilder)
    : m_pData(nullptr)
    , m_nCurrentDataSet(0)
    , m_bNoDataSet(true)
    , m_xScrollBar(rBuilder.weld_scrolled_window("scrollwin"))
    , m_xWindow(rBuilder.weld_container("CONTAINER"))
{
}

void SwAddressControl_Impl::SetData(SwCSVData& rDBData)
{
    m_pData = &rDBData;
    //when the address data is updated then remove the controls an build again
    if (!m_aLines.empty())
    {
        m_aLines.clear();
        m_bNoDataSet = true;
    }

    Link<weld::Widget&,void> aFocusLink = LINK(this, SwAddressControl_Impl, GotFocusHdl_Impl);
    Link<weld::Entry&,void> aEditModifyLink = LINK(this, SwAddressControl_Impl, EditModifyHdl_Impl);
    sal_Int32 nLines = 0;
    for (const auto& rHeader : m_pData->aDBColumnHeaders)
    {
        m_aLines.emplace_back(new SwAddressFragment(m_xWindow.get(), nLines));

        // when we have one line, measure it to get the line height to use as
        // the basis for overall size request
        if (nLines == 0)
        {
            auto nLineHeight = m_xWindow->get_preferred_size().Height();
            m_xScrollBar->set_size_request(m_xScrollBar->get_approximate_digit_width() * 65,
                                           nLineHeight * 10);
        }

        weld::Label* pNewFT = m_aLines.back()->m_xLabel.get();
        weld::Entry* pNewED = m_aLines.back()->m_xEntry.get();
        //set nLines a position identifier - used in the ModifyHdl
        m_aEditLines[pNewED] = nLines;
        pNewED->connect_focus_in(aFocusLink);
        pNewED->connect_changed(aEditModifyLink);

        pNewFT->set_label(rHeader);

        nLines++;
    }
}

void SwAddressControl_Impl::SetCurrentDataSet(sal_uInt32 nSet)
{
    if(m_bNoDataSet || m_nCurrentDataSet != nSet)
    {
        m_bNoDataSet = false;
        m_nCurrentDataSet = nSet;
        OSL_ENSURE(m_pData->aDBData.size() > m_nCurrentDataSet, "wrong data set index");
        if(m_pData->aDBData.size() > m_nCurrentDataSet)
        {
            sal_uInt32 nIndex = 0;
            for(auto& rLine : m_aLines)
            {
                OSL_ENSURE(nIndex < m_pData->aDBData[m_nCurrentDataSet].size(),
                            "number of columns doesn't match number of Edits");
                rLine->m_xEntry->set_text(m_pData->aDBData[m_nCurrentDataSet][nIndex]);
                ++nIndex;
            }
        }
    }
}

IMPL_LINK(SwAddressControl_Impl, GotFocusHdl_Impl, weld::Widget&, rEdit, void)
{
    int x, y, width, height;
    rEdit.get_extents_relative_to(*m_xWindow, x, y, width, height);
    // the container has a border of 3 in the .ui
    tools::Rectangle aRect(Point(x - 3, y - 3), Size(width + 6, height + 6));
    MakeVisible(aRect);
}

void SwAddressControl_Impl::MakeVisible(const tools::Rectangle & rRect)
{
    //determine range of visible positions
    auto nMinVisiblePos = m_xScrollBar->vadjustment_get_value();
    auto nMaxVisiblePos = nMinVisiblePos + m_xScrollBar->vadjustment_get_page_size();
    if (rRect.Top() < nMinVisiblePos || rRect.Bottom() > nMaxVisiblePos)
        m_xScrollBar->vadjustment_set_value(rRect.Top());
}

// copy data changes into database
IMPL_LINK(SwAddressControl_Impl, EditModifyHdl_Impl, weld::Entry&, rEdit, void)
{
    //get the data element number of the current set
    sal_Int32 nIndex = m_aEditLines[&rEdit];
    //get the index of the set
    OSL_ENSURE(m_pData->aDBData.size() > m_nCurrentDataSet, "wrong data set index" );
    if (m_pData->aDBData.size() > m_nCurrentDataSet)
    {
        m_pData->aDBData[m_nCurrentDataSet][nIndex] = rEdit.get_text();
    }
}

void SwAddressControl_Impl::SetCursorTo(std::size_t nElement)
{
    if (nElement < m_aLines.size())
    {
        weld::Entry* pEdit = m_aLines[nElement]->m_xEntry.get();
        pEdit->grab_focus();
        GotFocusHdl_Impl(*pEdit);
    }

}

SwCreateAddressListDialog::SwCreateAddressListDialog(
        weld::Window* pParent, const OUString& rURL, SwMailMergeConfigItem const & rConfig)
    : SfxDialogController(pParent, "modules/swriter/ui/createaddresslist.ui", "CreateAddressList")
    , m_sAddressListFilterName(SwResId(ST_FILTERNAME))
    , m_sURL(rURL)
    , m_pCSVData(new SwCSVData)
    , m_xAddressControl(new SwAddressControl_Impl(*m_xBuilder))
    , m_xNewPB(m_xBuilder->weld_button("NEW"))
    , m_xDeletePB(m_xBuilder->weld_button("DELETE"))
    , m_xFindPB(m_xBuilder->weld_button("FIND"))
    , m_xCustomizePB(m_xBuilder->weld_button("CUSTOMIZE"))
    , m_xStartPB(m_xBuilder->weld_button("START"))
    , m_xPrevPB(m_xBuilder->weld_button("PREV"))
    , m_xSetNoED(m_xBuilder->weld_entry("SETNO-nospin"))
    , m_xSetNoNF(m_xBuilder->weld_spin_button("SETNO"))
    , m_xNextPB(m_xBuilder->weld_button("NEXT"))
    , m_xEndPB(m_xBuilder->weld_button("END"))
    , m_xOK(m_xBuilder->weld_button("ok"))
{
    m_xSetNoNF->set_min(1);

    m_xNewPB->connect_clicked(LINK(this, SwCreateAddressListDialog, NewHdl_Impl));
    m_xDeletePB->connect_clicked(LINK(this, SwCreateAddressListDialog, DeleteHdl_Impl));
    m_xFindPB->connect_clicked(LINK(this, SwCreateAddressListDialog, FindHdl_Impl));
    m_xCustomizePB->connect_clicked(LINK(this, SwCreateAddressListDialog, CustomizeHdl_Impl));
    m_xOK->connect_clicked(LINK(this, SwCreateAddressListDialog, OkHdl_Impl));

    Link<weld::Button&,void> aLk = LINK(this, SwCreateAddressListDialog, DBCursorHdl_Impl);
    m_xStartPB->connect_clicked(aLk);
    m_xPrevPB->connect_clicked(aLk);
    m_xSetNoED->connect_changed(LINK(this, SwCreateAddressListDialog, DBNumCursorHdl_Impl));
    m_xSetNoED->connect_focus_out(LINK(this, SwCreateAddressListDialog, RefreshNum_Impl));
    m_xNextPB->connect_clicked(aLk);
    m_xEndPB->connect_clicked(aLk);

    if (!m_sURL.isEmpty())
    {
        //file exists, has to be loaded here
        SfxMedium aMedium( m_sURL, StreamMode::READ );
        SvStream* pStream = aMedium.GetInStream();
        if(pStream)
        {
            pStream->SetLineDelimiter( LINEEND_LF );
            pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);

            OUString sLine;
            bool bRead = pStream->ReadByteStringLine( sLine, RTL_TEXTENCODING_UTF8 );

            if(bRead && !sLine.isEmpty())
            {
                sal_Int32 nIndex = 0;
                do
                {
                    const OUString sHeader = sLine.getToken( 0, '\t', nIndex );
                    OSL_ENSURE(sHeader.getLength() > 2 &&
                            sHeader.startsWith("\"") && sHeader.endsWith("\""),
                            "Wrong format of header");
                    if(sHeader.getLength() > 2)
                    {
                        m_pCSVData->aDBColumnHeaders.push_back( sHeader.copy(1, sHeader.getLength() -2));
                    }
                }
                while (nIndex > 0);
            }
            while(pStream->ReadByteStringLine( sLine, RTL_TEXTENCODING_UTF8 ))
            {
                std::vector<OUString> aNewData;
                //analyze data line
                sal_Int32 nIndex = { sLine.isEmpty() ? -1 : 0 };
                while (nIndex >= 0)
                {
                    const OUString sData = sLine.getToken( 0, '\t', nIndex );
                    OSL_ENSURE( sData.startsWith("\"") && sData.endsWith("\""),
                            "Wrong format of line");
                    if(sData.getLength() >= 2)
                        aNewData.push_back(sData.copy(1, sData.getLength() - 2));
                    else
                        aNewData.push_back(sData);
                }
                m_pCSVData->aDBData.push_back( aNewData );
            }
        }
    }
    else
    {
        //database has to be created
        const std::vector<std::pair<OUString, int>>& rAddressHeader = rConfig.GetDefaultAddressHeaders();
        const sal_uInt32 nCount = rAddressHeader.size();
        for(sal_uInt32 nHeader = 0; nHeader < nCount; ++nHeader)
            m_pCSVData->aDBColumnHeaders.push_back(rAddressHeader[nHeader].first);
        std::vector<OUString> aNewData;
        aNewData.insert(aNewData.begin(), nCount, OUString());
        m_pCSVData->aDBData.push_back(aNewData);
    }
    //now fill the address control
    m_xAddressControl->SetData(*m_pCSVData);
    m_xAddressControl->SetCurrentDataSet(0);
    m_xSetNoNF->set_max(m_pCSVData->aDBData.size());

    m_xSetNoNF->set_value(1);
    RefreshNum_Impl(*m_xSetNoED);

    UpdateButtons();
}

SwCreateAddressListDialog::~SwCreateAddressListDialog()
{
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, NewHdl_Impl, weld::Button&, void)
{
    sal_uInt32 nCurrent = m_xAddressControl->GetCurrentDataSet();
    std::vector<OUString> aNewData;
    aNewData.insert(aNewData.begin(), m_pCSVData->aDBColumnHeaders.size(), OUString());
    m_pCSVData->aDBData.insert(m_pCSVData->aDBData.begin() + ++nCurrent, aNewData);
    m_xSetNoNF->set_max(m_pCSVData->aDBData.size());
    //the NumericField start at 1
    m_xSetNoNF->set_value(nCurrent + 1);
    RefreshNum_Impl(*m_xSetNoED);
    //the address control starts at 0
    m_xAddressControl->SetCurrentDataSet(nCurrent);
    UpdateButtons();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DeleteHdl_Impl, weld::Button&, void)
{
    sal_uInt32 nCurrent = m_xAddressControl->GetCurrentDataSet();
    if (m_pCSVData->aDBData.size() > 1)
    {
        m_pCSVData->aDBData.erase(m_pCSVData->aDBData.begin() + nCurrent);
        if (nCurrent)
            --nCurrent;
    }
    else
    {
        // if only one set is available then clear the data
        m_pCSVData->aDBData[0].assign(m_pCSVData->aDBData[0].size(), OUString());
        m_xDeletePB->set_sensitive(false);
    }
    m_xAddressControl->CurrentDataSetInvalidated();
    m_xAddressControl->SetCurrentDataSet(nCurrent);
    m_xSetNoNF->set_max(m_pCSVData->aDBData.size());
    UpdateButtons();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, FindHdl_Impl, weld::Button&, void)
{
    if (!m_xFindDlg)
    {
        m_xFindDlg.reset(new SwFindEntryDialog(this));
        weld::ComboBox& rColumnBox = m_xFindDlg->GetFieldsListBox();
        for(const auto& rHeader : m_pCSVData->aDBColumnHeaders)
            rColumnBox.append_text(rHeader);
        rColumnBox.set_active(0);
        m_xFindDlg->show();
    }
    else
        m_xFindDlg->set_visible(!m_xFindDlg->get_visible());
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, CustomizeHdl_Impl, weld::Button&, void)
{
    SwCustomizeAddressListDialog aDlg(m_xDialog.get(), *m_pCSVData);
    if (aDlg.run() == RET_OK)
    {
        m_pCSVData = aDlg.ReleaseNewData();
        m_xAddressControl->SetData(*m_pCSVData);
        m_xAddressControl->SetCurrentDataSet(m_xAddressControl->GetCurrentDataSet());
    }

    //update find dialog
    if (m_xFindDlg)
    {
        weld::ComboBox& rColumnBox = m_xFindDlg->GetFieldsListBox();
        rColumnBox.clear();
        for(const auto& rHeader : m_pCSVData->aDBColumnHeaders)
            rColumnBox.append_text(rHeader);
    }
}

namespace
{

void lcl_WriteValues(const std::vector<OUString> *pFields, SvStream* pStream)
{
    OUStringBuffer sLine;
    const std::vector< OUString >::const_iterator aBegin = pFields->begin();
    const std::vector< OUString >::const_iterator aEnd = pFields->end();
    for(std::vector< OUString >::const_iterator aIter = aBegin; aIter != aEnd; ++aIter)
    {
        if (aIter==aBegin)
        {
            sLine.append("\"").append(*aIter).append("\"");
        }
        else
        {
            sLine.append("\t\"").append(*aIter).append("\"");
        }
    }
    pStream->WriteByteStringLine( sLine.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

}

IMPL_LINK_NOARG(SwCreateAddressListDialog, OkHdl_Impl, weld::Button&, void)
{
    if(m_sURL.isEmpty())
    {
        sfx2::FileDialogHelper aDlgHelper(TemplateDescription::FILESAVE_SIMPLE,
                                          FileDialogFlags::NONE, m_xDialog.get());
        uno::Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

        const OUString sPath( SvtPathOptions().SubstituteVariable("$(userurl)/database") );
        aDlgHelper.SetDisplayDirectory( sPath );
        uno::Reference< XFilterManager > xFltMgr(xFP, uno::UNO_QUERY);
        xFltMgr->appendFilter( m_sAddressListFilterName, "*.csv" );
        xFltMgr->setCurrentFilter( m_sAddressListFilterName ) ;

        if( ERRCODE_NONE == aDlgHelper.Execute() )
        {
            m_sURL = xFP->getSelectedFiles().getConstArray()[0];
            INetURLObject aResult( m_sURL );
            aResult.setExtension("csv");
            m_sURL = aResult.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        }
    }
    if(!m_sURL.isEmpty())
    {
        SfxMedium aMedium( m_sURL, StreamMode::READWRITE|StreamMode::TRUNC );
        SvStream* pStream = aMedium.GetOutStream();
        pStream->SetLineDelimiter( LINEEND_LF );
        pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);

        lcl_WriteValues(&(m_pCSVData->aDBColumnHeaders), pStream);

        for(const auto& rData : m_pCSVData->aDBData)
        {
            lcl_WriteValues(&rData, pStream);
        }
        aMedium.Commit();
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK(SwCreateAddressListDialog, DBCursorHdl_Impl, weld::Button&, rButton, void)
{
    int nValue = m_xSetNoNF->get_value();

    if (&rButton == m_xStartPB.get())
        nValue = 1;
    else if (&rButton == m_xPrevPB.get())
    {
        if (nValue > 1)
            --nValue;
    }
    else if (&rButton == m_xNextPB.get())
    {
        if (nValue < m_xSetNoNF->get_max())
            ++nValue;
    }
    else //m_aEndPB
        nValue = m_xSetNoNF->get_max();
    if (nValue != m_xSetNoNF->get_value())
    {
        m_xSetNoNF->set_value(nValue);
        RefreshNum_Impl(*m_xSetNoED);
        DBNumCursor();
    }
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DBNumCursorHdl_Impl, weld::Entry&, void)
{
    m_xSetNoNF->set_text(m_xSetNoED->get_text());
    DBNumCursor();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, RefreshNum_Impl, weld::Widget&, void)
{
    m_xSetNoED->set_text(OUString::number(m_xSetNoNF->get_value()));
}

void SwCreateAddressListDialog::DBNumCursor()
{
    m_xAddressControl->SetCurrentDataSet(m_xSetNoNF->get_value() - 1);
    UpdateButtons();
}

void SwCreateAddressListDialog::UpdateButtons()
{
    sal_uInt32 nCurrent = static_cast< sal_uInt32 >(m_xSetNoNF->get_value() );
    sal_uInt32 nSize = static_cast<sal_uInt32>(m_pCSVData->aDBData.size());
    m_xStartPB->set_sensitive(nCurrent != 1);
    m_xPrevPB->set_sensitive(nCurrent != 1);
    m_xNextPB->set_sensitive(nCurrent != nSize);
    m_xEndPB->set_sensitive(nCurrent != nSize);
    m_xDeletePB->set_sensitive(nSize > 0);
}

void SwCreateAddressListDialog::Find(const OUString& rSearch, sal_Int32 nColumn)
{
    const OUString sSearch = rSearch.toAsciiLowerCase();
    sal_uInt32 nCurrent = m_xAddressControl->GetCurrentDataSet();
    //search forward
    bool bFound = false;
    sal_uInt32 nStart = nCurrent + 1;
    sal_uInt32 nEnd = m_pCSVData->aDBData.size();
    std::size_t nElement = 0;
    sal_uInt32 nPos = 0;
    for(short nTemp = 0; nTemp < 2 && !bFound; nTemp++)
    {
        for(nPos = nStart; nPos < nEnd; ++nPos)
        {
            std::vector< OUString> const & aData = m_pCSVData->aDBData[nPos];
            if(nColumn >=0)
                bFound = -1 != aData[static_cast<sal_uInt32>(nColumn)].toAsciiLowerCase().indexOf(sSearch);
            else
            {
                for( nElement = 0; nElement < aData.size(); ++nElement)
                {
                    bFound = -1 != aData[nElement].toAsciiLowerCase().indexOf(sSearch);
                    if(bFound)
                    {
                        nColumn = nElement; //TODO: std::size_t -> sal_Int32!
                        break;
                    }
                }
            }
            if(bFound)
                break;
        }
        nStart = 0;
        nEnd = nCurrent + 1;
    }
    if(bFound)
    {
        m_xAddressControl->SetCurrentDataSet(nPos);
        m_xSetNoNF->set_value( nPos + 1 );
        RefreshNum_Impl(*m_xSetNoED);
        UpdateButtons();
        m_xAddressControl->SetCursorTo(nElement);
    }
}

SwFindEntryDialog::SwFindEntryDialog(SwCreateAddressListDialog* pParent)
    : GenericDialogController(pParent->getDialog(), "modules/swriter/ui/findentrydialog.ui", "FindEntryDialog")
    , m_pParent(pParent)
    , m_xFindED(m_xBuilder->weld_entry("entry"))
    , m_xFindOnlyCB(m_xBuilder->weld_check_button("findin"))
    , m_xFindOnlyLB(m_xBuilder->weld_combo_box("area"))
    , m_xFindPB(m_xBuilder->weld_button("find"))
    , m_xCancel(m_xBuilder->weld_button("cancel"))
{
    m_xFindPB->connect_clicked(LINK(this, SwFindEntryDialog, FindHdl_Impl));
    m_xFindED->connect_changed(LINK(this, SwFindEntryDialog, FindEnableHdl_Impl));
    m_xCancel->connect_clicked(LINK(this, SwFindEntryDialog, CloseHdl_Impl));
}

SwFindEntryDialog::~SwFindEntryDialog()
{
}

IMPL_LINK_NOARG(SwFindEntryDialog, FindHdl_Impl, weld::Button&, void)
{
    sal_Int32 nColumn = -1;
    if (m_xFindOnlyCB->get_active())
        nColumn = m_xFindOnlyLB->get_active();
    m_pParent->Find(m_xFindED->get_text(), nColumn);
}

IMPL_LINK_NOARG(SwFindEntryDialog, FindEnableHdl_Impl, weld::Entry&, void)
{
    m_xFindPB->set_sensitive(!m_xFindED->get_text().isEmpty());
}

IMPL_LINK_NOARG(SwFindEntryDialog, CloseHdl_Impl, weld::Button&, void)
{
    m_xDialog->hide();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
