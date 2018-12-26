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

class SwAddressControl_Impl : public Control
{
    VclPtr<ScrollBar>                       m_pScrollBar;
    VclPtr<Window>                          m_pWindow;

    std::vector<VclPtr<FixedText> >       m_aFixedTexts;
    std::vector<VclPtr<Edit> >            m_aEdits;
    std::map<void*, sal_Int32>            m_aEditLines;

    SwCSVData*                      m_pData;
    Size                            m_aWinOutputSize;
    sal_Int32                       m_nLineHeight;
    sal_uInt32                      m_nCurrentDataSet;

    bool                            m_bNoDataSet;

    DECL_LINK(ScrollHdl_Impl, ScrollBar*, void);
    DECL_LINK(GotFocusHdl_Impl, Control&, void);
    DECL_LINK(EditModifyHdl_Impl, Edit&, void);

    void                MakeVisible(const tools::Rectangle& aRect);

    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual Size        GetOptimalSize() const override;

public:
    SwAddressControl_Impl(vcl::Window* pParent , WinBits nBits );
    virtual ~SwAddressControl_Impl() override;
    virtual void dispose() override;

    void        SetData(SwCSVData& rDBData);

    void        SetCurrentDataSet(sal_uInt32 nSet);
    sal_uInt32  GetCurrentDataSet() const { return m_nCurrentDataSet;}
    void        SetCursorTo(std::size_t nElement);
    virtual void Resize() override;
};

SwAddressControl_Impl::SwAddressControl_Impl(vcl::Window* pParent, WinBits nBits ) :
    Control(pParent, nBits),
    m_pScrollBar(VclPtr<ScrollBar>::Create(this)),
    m_pWindow(VclPtr<vcl::Window>::Create(this, WB_DIALOGCONTROL)),
    m_pData(nullptr),
    m_nLineHeight(0),
    m_nCurrentDataSet(0),
    m_bNoDataSet(true)
{
    long nScrollBarWidth = m_pScrollBar->GetOutputSize().Width();
    Size aSize = GetOutputSizePixel();

    m_pWindow->SetSizePixel(Size(aSize.Width() - nScrollBarWidth, aSize.Height()));
    m_aWinOutputSize = m_pWindow->GetOutputSizePixel();
    m_pWindow->Show();
    m_pScrollBar->Show();

    Link<ScrollBar*,void> aScrollLink = LINK(this, SwAddressControl_Impl, ScrollHdl_Impl);
    m_pScrollBar->SetScrollHdl(aScrollLink);
    m_pScrollBar->SetEndScrollHdl(aScrollLink);
    m_pScrollBar->EnableDrag();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSwAddressControlImpl(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<SwAddressControl_Impl>::Create(pParent, WB_BORDER | WB_DIALOGCONTROL);
}

SwAddressControl_Impl::~SwAddressControl_Impl()
{
    disposeOnce();
}

void SwAddressControl_Impl::dispose()
{
    for(auto& rText : m_aFixedTexts)
        rText.disposeAndClear();
    m_aFixedTexts.clear();
    for(auto& rEdit : m_aEdits)
        rEdit.disposeAndClear();
    m_aEdits.clear();
    m_pScrollBar.disposeAndClear();
    m_pWindow.disposeAndClear();
    Control::dispose();
}

void SwAddressControl_Impl::SetData(SwCSVData& rDBData)
{
    m_pData = &rDBData;
    //when the address data is updated then remove the controls an build again
    if(!m_aFixedTexts.empty())
    {
        for(auto& rText : m_aFixedTexts)
            rText.disposeAndClear();
        m_aFixedTexts.clear();
        for(auto& rEdit : m_aEdits)
            rEdit.disposeAndClear();
        m_aEdits.clear();
        m_bNoDataSet = true;
    }
    //now create appropriate controls

    long nFTXPos = m_pWindow->LogicToPixel(Point(RSC_SP_CTRL_X, RSC_SP_CTRL_X), MapMode(MapUnit::MapAppFont)).X();
    long nFTHeight = m_pWindow->LogicToPixel(Size(RSC_BS_CHARHEIGHT, RSC_BS_CHARHEIGHT), MapMode(MapUnit::MapAppFont)).Height();
    long nFTWidth = 0;

    //determine the width of the FixedTexts
    for(const auto& rHeader : m_pData->aDBColumnHeaders)
    {
        sal_Int32 nTemp = m_pWindow->GetTextWidth(rHeader);
        if(nTemp > nFTWidth)
          nFTWidth = nTemp;
    }
    //add some pixels
    nFTWidth += 2;
    long nEDXPos = nFTWidth + nFTXPos +
            m_pWindow->LogicToPixel(Size(RSC_SP_CTRL_DESC_X, RSC_SP_CTRL_DESC_X), MapMode(MapUnit::MapAppFont)).Width();
    long nEDHeight = m_pWindow->LogicToPixel(Size(RSC_CD_TEXTBOX_HEIGHT, RSC_CD_TEXTBOX_HEIGHT), MapMode(MapUnit::MapAppFont)).Height();
    long nEDWidth = m_aWinOutputSize.Width() - nEDXPos - nFTXPos;
    m_nLineHeight = nEDHeight + m_pWindow->LogicToPixel(Size(RSC_SP_CTRL_GROUP_Y, RSC_SP_CTRL_GROUP_Y), MapMode(MapUnit::MapAppFont)).Height();

    long nEDYPos = m_pWindow->LogicToPixel(Size(RSC_SP_CTRL_DESC_Y, RSC_SP_CTRL_DESC_Y), MapMode(MapUnit::MapAppFont)).Height();
    long nFTYPos = nEDYPos + nEDHeight - nFTHeight;

    Link<Control&,void> aFocusLink = LINK(this, SwAddressControl_Impl, GotFocusHdl_Impl);
    Link<Edit&,void> aEditModifyLink = LINK(this, SwAddressControl_Impl, EditModifyHdl_Impl);
    Edit* pLastEdit = nullptr;
    sal_Int32 nVisibleLines = 0;
    sal_Int32 nLines = 0;
    for(const auto& rHeader : m_pData->aDBColumnHeaders)
    {
        VclPtr<FixedText> pNewFT = VclPtr<FixedText>::Create(m_pWindow, WB_RIGHT);
        VclPtr<Edit> pNewED = VclPtr<Edit>::Create(m_pWindow, WB_BORDER);
        //set nLines a position identifier - used in the ModifyHdl
        m_aEditLines[pNewED.get()] = nLines;
        pNewED->SetGetFocusHdl(aFocusLink);
        pNewED->SetModifyHdl(aEditModifyLink);

        pNewFT->SetPosSizePixel(Point(nFTXPos, nFTYPos), Size(nFTWidth, nFTHeight));
        pNewED->SetPosSizePixel(Point(nEDXPos, nEDYPos), Size(nEDWidth, nEDHeight));
        if(nEDYPos + nEDHeight < m_aWinOutputSize.Height())
            ++nVisibleLines;

        pNewFT->SetText(rHeader);

        pNewFT->Show();
        pNewED->Show();
        m_aFixedTexts.push_back(pNewFT);
        m_aEdits.push_back(pNewED);
        pLastEdit = pNewED;
        nEDYPos += m_nLineHeight;
        nFTYPos += m_nLineHeight;
        nLines++;
    }
    //scrollbar adjustment
    if(pLastEdit)
    {
        //the m_aWindow has to be at least as high as the ScrollBar and it must include the last Edit
        sal_Int32 nContentHeight = pLastEdit->GetPosPixel().Y() + nEDHeight +
                m_pWindow->LogicToPixel(Size(RSC_SP_CTRL_GROUP_Y, RSC_SP_CTRL_GROUP_Y), MapMode(MapUnit::MapAppFont)).Height();
        if(nContentHeight < m_pScrollBar->GetSizePixel().Height())
        {
            nContentHeight = m_pScrollBar->GetSizePixel().Height();
            // Reset the scrollbar's thumb to the top before it is disabled.
            m_pScrollBar->DoScroll(0);
            m_pScrollBar->SetThumbPos(0);
            m_pScrollBar->Enable(false);
        }
        else
        {
            m_pScrollBar->Enable();
            m_pScrollBar->SetRange(Range(0, nLines));
            m_pScrollBar->SetThumbPos(0);
            m_pScrollBar->SetVisibleSize(nVisibleLines);
            // Reset the scroll bar position (especially if items deleted)
            m_pScrollBar->DoScroll(m_pScrollBar->GetRangeMax());
            m_pScrollBar->DoScroll(0);
        }
        Size aWinOutputSize(m_aWinOutputSize);
        aWinOutputSize.setHeight( nContentHeight );
        m_pWindow->SetOutputSizePixel(aWinOutputSize);

    }
    // Even if no items in m_aEdits, the scrollbar will still exist;
    // we might as well disable it.
    if (m_aEdits.empty()) {
        m_pScrollBar->DoScroll(0);
        m_pScrollBar->SetThumbPos(0);
        m_pScrollBar->Enable(false);
    }
    Resize();
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
            for(auto& rEdit : m_aEdits)
            {
                OSL_ENSURE(nIndex < m_pData->aDBData[m_nCurrentDataSet].size(),
                            "number of columns doesn't match number of Edits");
                rEdit->SetText(m_pData->aDBData[m_nCurrentDataSet][nIndex]);
                ++nIndex;
            }
        }
    }
}

IMPL_LINK(SwAddressControl_Impl, ScrollHdl_Impl, ScrollBar*, pScroll, void)
{
    long nThumb = pScroll->GetThumbPos();
    m_pWindow->SetPosPixel(Point(0, - (m_nLineHeight * nThumb)));
}

IMPL_LINK(SwAddressControl_Impl, GotFocusHdl_Impl, Control&, rControl, void)
{
    Edit* pEdit = static_cast<Edit*>(&rControl);
    if(GetFocusFlags::Tab & pEdit->GetGetFocusFlags())
    {
        tools::Rectangle aRect(pEdit->GetPosPixel(), pEdit->GetSizePixel());
        MakeVisible(aRect);
    }
}

void SwAddressControl_Impl::MakeVisible(const tools::Rectangle & rRect)
{
    long nThumb = m_pScrollBar->GetThumbPos();
    //determine range of visible positions
    long nMinVisiblePos = - m_pWindow->GetPosPixel().Y();
    long nMaxVisiblePos = m_pScrollBar->GetSizePixel().Height() + nMinVisiblePos;
    if( rRect.TopLeft().Y() < nMinVisiblePos)
    {
        nThumb -= 1 + ((nMinVisiblePos - rRect.TopLeft().Y()) / m_nLineHeight);
    }
    else if(rRect.BottomLeft().Y() > nMaxVisiblePos)
    {
        nThumb += 1 + ((nMaxVisiblePos - rRect.BottomLeft().Y()) / m_nLineHeight);
    }
    if(nThumb != m_pScrollBar->GetThumbPos())
    {
        m_pScrollBar->SetThumbPos(nThumb);
        ScrollHdl_Impl(m_pScrollBar);
    }
}

// copy data changes into database
IMPL_LINK(SwAddressControl_Impl, EditModifyHdl_Impl, Edit&, rEdit, void)
{
    //get the data element number of the current set
    sal_Int32 nIndex = m_aEditLines[&rEdit];
    //get the index of the set
    OSL_ENSURE(m_pData->aDBData.size() > m_nCurrentDataSet, "wrong data set index" );
    if(m_pData->aDBData.size() > m_nCurrentDataSet)
    {
        m_pData->aDBData[m_nCurrentDataSet][nIndex] = rEdit.GetText();
    }
}

void SwAddressControl_Impl::SetCursorTo(std::size_t nElement)
{
    if(nElement < m_aEdits.size())
    {
        Edit* pEdit = m_aEdits[nElement].get();
        pEdit->GrabFocus();
        tools::Rectangle aRect(pEdit->GetPosPixel(), pEdit->GetSizePixel());
        MakeVisible(aRect);
    }

}

void SwAddressControl_Impl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case CommandEventId::Wheel:
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && CommandWheelMode::ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand( rCEvt, nullptr, m_pScrollBar );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
    }
}

bool SwAddressControl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if( pCEvt->GetCommand() == CommandEventId::Wheel )
        {
            Command(*pCEvt);
            return true;
        }
    }
    return Control::PreNotify(rNEvt);
}

Size SwAddressControl_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(250, 160), MapMode(MapUnit::MapAppFont));
}

void SwAddressControl_Impl::Resize()
{
    Window::Resize();
    m_pScrollBar->SetSizePixel(Size(m_pScrollBar->GetOutputSizePixel().Width(), GetOutputSizePixel().Height()));

    if(m_nLineHeight)
        m_pScrollBar->SetVisibleSize(m_pScrollBar->GetOutputSize().Height() / m_nLineHeight);
    m_pScrollBar->DoScroll(0);

    long nScrollBarWidth = m_pScrollBar->GetOutputSize().Width();
    Size aSize = GetOutputSizePixel();

    m_pWindow->SetSizePixel(Size(aSize.Width() - nScrollBarWidth, m_pWindow->GetOutputSizePixel().Height()));
    m_pScrollBar->SetPosPixel(Point(aSize.Width() - nScrollBarWidth, 0));

    if(!m_aEdits.empty())
    {
        long nNewEditSize = aSize.Width() - (*m_aEdits.begin())->GetPosPixel().X() - nScrollBarWidth - 6;

        for(auto& rEdit : m_aEdits)
        {
            rEdit->SetSizePixel(Size(nNewEditSize, rEdit->GetSizePixel().Height()));
        }
    }

}

SwCreateAddressListDialog::SwCreateAddressListDialog(
        vcl::Window* pParent, const OUString& rURL, SwMailMergeConfigItem const & rConfig) :
    SfxModalDialog(pParent, "CreateAddressList", "modules/swriter/ui/createaddresslist.ui"),
    m_sAddressListFilterName( SwResId(    ST_FILTERNAME)),
    m_sURL(rURL),
    m_pCSVData( new SwCSVData )
{
    get(m_pNewPB, "NEW");
    get(m_pDeletePB, "DELETE");
    get(m_pFindPB, "FIND");
    get(m_pCustomizePB, "CUSTOMIZE");
    get(m_pStartPB, "START");
    get(m_pPrevPB, "PREV");
    get(m_pSetNoNF, "SETNO-nospin");
    m_pSetNoNF->SetFirst(1);
    m_pSetNoNF->SetMin(1);
    get(m_pNextPB, "NEXT");
    get(m_pEndPB, "END");
    get(m_pOK, "ok");
    get(m_pAddressControl, "CONTAINER");

    m_pNewPB->SetClickHdl(LINK(this, SwCreateAddressListDialog, NewHdl_Impl));
    m_pDeletePB->SetClickHdl(LINK(this, SwCreateAddressListDialog, DeleteHdl_Impl));
    m_pFindPB->SetClickHdl(LINK(this, SwCreateAddressListDialog, FindHdl_Impl));
    m_pCustomizePB->SetClickHdl(LINK(this, SwCreateAddressListDialog, CustomizeHdl_Impl));
    m_pOK->SetClickHdl(LINK(this, SwCreateAddressListDialog, OkHdl_Impl));

    Link<Button*,void> aLk = LINK(this, SwCreateAddressListDialog, DBCursorHdl_Impl);
    m_pStartPB->SetClickHdl(aLk);
    m_pPrevPB->SetClickHdl(aLk);
    m_pSetNoNF->SetModifyHdl(LINK(this, SwCreateAddressListDialog, DBNumCursorHdl_Impl));
    m_pNextPB->SetClickHdl(aLk);
    m_pEndPB->SetClickHdl(aLk);

    if(!m_sURL.isEmpty())
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
    m_pAddressControl->SetData(*m_pCSVData);
    m_pAddressControl->SetCurrentDataSet(0);
    m_pSetNoNF->SetMax(m_pCSVData->aDBData.size());
    UpdateButtons();
}

SwCreateAddressListDialog::~SwCreateAddressListDialog()
{
    disposeOnce();
}

void SwCreateAddressListDialog::dispose()
{
    m_pCSVData.reset();
    m_pAddressControl.clear();
    m_pNewPB.clear();
    m_pDeletePB.clear();
    m_pFindPB.clear();
    m_pCustomizePB.clear();
    m_pStartPB.clear();
    m_pPrevPB.clear();
    m_pSetNoNF.clear();
    m_pNextPB.clear();
    m_pEndPB.clear();
    m_pOK.clear();
    m_xFindDlg.reset();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, NewHdl_Impl, Button*, void)
{
    sal_uInt32 nCurrent = m_pAddressControl->GetCurrentDataSet();
    std::vector<OUString> aNewData;
    aNewData.insert(aNewData.begin(), m_pCSVData->aDBColumnHeaders.size(), OUString());
    m_pCSVData->aDBData.insert(m_pCSVData->aDBData.begin() + ++nCurrent, aNewData);
    m_pSetNoNF->SetMax(m_pCSVData->aDBData.size());
    //the NumericField start at 1
    m_pSetNoNF->SetValue(nCurrent + 1);
    //the address control starts at 0
    m_pAddressControl->SetCurrentDataSet(nCurrent);
    UpdateButtons();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DeleteHdl_Impl, Button*, void)
{
    sal_uInt32 nCurrent = m_pAddressControl->GetCurrentDataSet();
    if(m_pCSVData->aDBData.size() > 1)
    {
        m_pCSVData->aDBData.erase(m_pCSVData->aDBData.begin() + nCurrent);
        if(nCurrent)
            --nCurrent;
    }
    else
    {
        // if only one set is available then clear the data
        m_pCSVData->aDBData[0].assign(m_pCSVData->aDBData[0].size(), OUString());
        m_pDeletePB->Enable(false);
    }
    m_pAddressControl->SetCurrentDataSet(nCurrent);
    m_pSetNoNF->SetMax(m_pCSVData->aDBData.size());
    UpdateButtons();
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, FindHdl_Impl, Button*, void)
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
        m_xFindDlg->show(!m_xFindDlg->get_visible());
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, CustomizeHdl_Impl, Button*, void)
{
    SwCustomizeAddressListDialog aDlg(GetFrameWeld(), *m_pCSVData);
    if (aDlg.run() == RET_OK)
    {
        m_pCSVData = aDlg.ReleaseNewData();
        m_pAddressControl->SetData(*m_pCSVData);
        m_pAddressControl->SetCurrentDataSet(m_pAddressControl->GetCurrentDataSet());
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

IMPL_LINK_NOARG(SwCreateAddressListDialog, OkHdl_Impl, Button*, void)
{
    if(m_sURL.isEmpty())
    {
        sfx2::FileDialogHelper aDlgHelper(TemplateDescription::FILESAVE_SIMPLE,
                                          FileDialogFlags::NONE, GetFrameWeld());
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
        EndDialog(RET_OK);
    }
}

IMPL_LINK(SwCreateAddressListDialog, DBCursorHdl_Impl, Button*, pButton, void)
{
    sal_uInt32 nValue = static_cast< sal_uInt32 >(m_pSetNoNF->GetValue());

    if(pButton == m_pStartPB)
        nValue = 1;
    else if(pButton == m_pPrevPB)
    {
        if(nValue > 1)
            --nValue;
    }
    else if(pButton == m_pNextPB)
    {
        if(nValue < static_cast<sal_uInt32>(m_pSetNoNF->GetMax()))
            ++nValue;
    }
    else //m_aEndPB
        nValue = static_cast< sal_uInt32 >(m_pSetNoNF->GetMax());
    if(nValue != m_pSetNoNF->GetValue())
    {
        m_pSetNoNF->SetValue(nValue);
        DBNumCursorHdl_Impl(*m_pSetNoNF);
    }
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DBNumCursorHdl_Impl, Edit&, void)
{
    m_pAddressControl->SetCurrentDataSet( static_cast< sal_uInt32 >(m_pSetNoNF->GetValue() - 1) );
    UpdateButtons();
}

void SwCreateAddressListDialog::UpdateButtons()
{
    sal_uInt32 nCurrent = static_cast< sal_uInt32 >(m_pSetNoNF->GetValue() );
    sal_uInt32 nSize = static_cast<sal_uInt32>(m_pCSVData->aDBData.size());
    m_pStartPB->Enable(nCurrent != 1);
    m_pPrevPB->Enable(nCurrent != 1);
    m_pNextPB->Enable(nCurrent != nSize);
    m_pEndPB->Enable(nCurrent != nSize);
    m_pDeletePB->Enable(nSize > 0);
}

void SwCreateAddressListDialog::Find(const OUString& rSearch, sal_Int32 nColumn)
{
    const OUString sSearch = rSearch.toAsciiLowerCase();
    sal_uInt32 nCurrent = m_pAddressControl->GetCurrentDataSet();
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
        m_pAddressControl->SetCurrentDataSet(nPos);
        m_pSetNoNF->SetValue( nPos + 1 );
        UpdateButtons();
        m_pAddressControl->SetCursorTo(nElement);
    }
}

SwFindEntryDialog::SwFindEntryDialog(SwCreateAddressListDialog* pParent)
    : GenericDialogController(pParent->GetFrameWeld(), "modules/swriter/ui/findentrydialog.ui", "FindEntryDialog")
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
    m_xDialog->show(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
