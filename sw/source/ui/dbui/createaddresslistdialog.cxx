/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <swtypes.hxx>
#include <createaddresslistdialog.hxx>
#include <customizeaddresslistdialog.hxx>
#include <mmconfigitem.hxx>
#include <comphelper/string.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/controldims.hrc>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <rtl/textenc.h>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <tools/urlobj.hxx>
#include <createaddresslistdialog.hrc>
#include <dbui.hrc>
#include <helpid.h>
#include <unomid.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using ::rtl::OUString;

class SwAddressControl_Impl : public Control
{
    ScrollBar                       m_aScrollBar;
    Window                          m_aWindow;

    ::std::vector<FixedText*>       m_aFixedTexts;
    ::std::vector<Edit*>            m_aEdits;

    SwCSVData*                      m_pData;
    Size                            m_aWinOutputSize;
    sal_Int32                       m_nLineHeight;
    sal_uInt32                      m_nCurrentDataSet;

    bool                            m_bNoDataSet;

    DECL_LINK(ScrollHdl_Impl, ScrollBar*);
    DECL_LINK(GotFocusHdl_Impl, Edit*);
    DECL_LINK(EditModifyHdl_Impl, Edit*);

    void                MakeVisible(const Rectangle& aRect);

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        Command( const CommandEvent& rCEvt );

    using Window::SetData;

public:
    SwAddressControl_Impl(Window* pParent, const ResId& rResId );
    ~SwAddressControl_Impl();

    void        SetData(SwCSVData& rDBData);

    void        SetCurrentDataSet(sal_uInt32 nSet);
    sal_uInt32  GetCurrentDataSet() const { return m_nCurrentDataSet;}
    void        SetCursorTo(sal_uInt32 nElement);
};

SwAddressControl_Impl::SwAddressControl_Impl(Window* pParent, const ResId& rResId ) :
    Control(pParent, rResId),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aScrollBar(this, ResId(SCR_1,*rResId.GetResMgr())),
    m_aWindow(this, ResId(WIN_DATA,*rResId.GetResMgr())),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pData(0),
    m_aWinOutputSize( m_aWindow.GetOutputSizePixel() ),
    m_nLineHeight(0),
    m_nCurrentDataSet(0),
    m_bNoDataSet(true)
{
    FreeResource();
    Link aScrollLink = LINK(this, SwAddressControl_Impl, ScrollHdl_Impl);
    m_aScrollBar.SetScrollHdl(aScrollLink);
    m_aScrollBar.SetEndScrollHdl(aScrollLink);
    m_aScrollBar.EnableDrag();

}

SwAddressControl_Impl::~SwAddressControl_Impl()
{
    ::std::vector<FixedText*>::iterator aTextIter;
    for(aTextIter = m_aFixedTexts.begin(); aTextIter != m_aFixedTexts.end(); ++aTextIter)
        delete *aTextIter;
    ::std::vector<Edit*>::iterator aEditIter;
    for(aEditIter = m_aEdits.begin(); aEditIter != m_aEdits.end(); ++aEditIter)
        delete *aEditIter;
}

void SwAddressControl_Impl::SetData(SwCSVData& rDBData)
{
    m_pData = &rDBData;
    //when the address data is updated then remove the controls an build again
    if(m_aFixedTexts.size())
    {
        ::std::vector<FixedText*>::iterator aTextIter;
        for(aTextIter = m_aFixedTexts.begin(); aTextIter != m_aFixedTexts.end(); ++aTextIter)
            delete *aTextIter;
        ::std::vector<Edit*>::iterator aEditIter;
        for(aEditIter = m_aEdits.begin(); aEditIter != m_aEdits.end(); ++aEditIter)
            delete *aEditIter;
        m_aFixedTexts.clear();
        m_aEdits.clear();
        m_bNoDataSet = true;
    }
    //now create appropriate controls

    ::std::vector< OUString >::iterator  aHeaderIter;

    long nFTXPos = m_aWindow.LogicToPixel(Point(RSC_SP_CTRL_X, RSC_SP_CTRL_X), MAP_APPFONT).X();
    long nFTHeight = m_aWindow.LogicToPixel(Size(RSC_BS_CHARHEIGHT, RSC_BS_CHARHEIGHT), MAP_APPFONT).Height();
    long nFTWidth = 0;

    //determine the width of the FixedTexts
    for(aHeaderIter = m_pData->aDBColumnHeaders.begin();
                aHeaderIter != m_pData->aDBColumnHeaders.end();
                ++aHeaderIter)
    {
        sal_Int32 nTemp = m_aWindow.GetTextWidth(*aHeaderIter);
        if(nTemp > nFTWidth)
          nFTWidth = nTemp;
    }
    //add some pixels
    nFTWidth += 2;
    long nEDXPos = nFTWidth + nFTXPos +
            m_aWindow.LogicToPixel(Size(RSC_SP_CTRL_DESC_X, RSC_SP_CTRL_DESC_X), MAP_APPFONT).Width();
    long nEDHeight = m_aWindow.LogicToPixel(Size(RSC_CD_TEXTBOX_HEIGHT, RSC_CD_TEXTBOX_HEIGHT), MAP_APPFONT).Height();
    long nEDWidth = m_aWinOutputSize.Width() - nEDXPos - nFTXPos;
    m_nLineHeight = nEDHeight + m_aWindow.LogicToPixel(Size(RSC_SP_CTRL_GROUP_Y, RSC_SP_CTRL_GROUP_Y), MAP_APPFONT).Height();

    long nEDYPos = m_aWindow.LogicToPixel(Size(RSC_SP_CTRL_DESC_Y, RSC_SP_CTRL_DESC_Y), MAP_APPFONT).Height();
    long nFTYPos = nEDYPos + nEDHeight - nFTHeight;

    Link aFocusLink = LINK(this, SwAddressControl_Impl, GotFocusHdl_Impl);
    Link aEditModifyLink = LINK(this, SwAddressControl_Impl, EditModifyHdl_Impl);
    Edit* pLastEdit = 0;
    sal_Int32 nVisibleLines = 0;
    sal_uIntPtr nLines = 0;
    for(aHeaderIter = m_pData->aDBColumnHeaders.begin();
                aHeaderIter != m_pData->aDBColumnHeaders.end();
                ++aHeaderIter, nEDYPos += m_nLineHeight, nFTYPos += m_nLineHeight, nLines++)
    {
        FixedText* pNewFT = new FixedText(&m_aWindow, WB_RIGHT);
        Edit* pNewED = new Edit(&m_aWindow, WB_BORDER);
        //set nLines a position identifier - used in the ModifyHdl
        pNewED->SetData((void*)nLines);
        pNewED->SetGetFocusHdl(aFocusLink);
        pNewED->SetModifyHdl(aEditModifyLink);

        pNewFT->SetPosSizePixel(Point(nFTXPos, nFTYPos), Size(nFTWidth, nFTHeight));
        pNewED->SetPosSizePixel(Point(nEDXPos, nEDYPos), Size(nEDWidth, nEDHeight));
        if(nEDYPos + nEDHeight < m_aWinOutputSize.Height())
            ++nVisibleLines;

        pNewFT->SetText(*aHeaderIter);

        pNewFT->Show();
        pNewED->Show();
        m_aFixedTexts.push_back(pNewFT);
        m_aEdits.push_back(pNewED);
        pLastEdit = pNewED;
    }
    //scrollbar adjustment
    if(pLastEdit)
    {
        //the m_aWindow has to be at least as high as the ScrollBar and it must include the last Edit
        sal_Int32 nContentHeight = pLastEdit->GetPosPixel().Y() + nEDHeight +
                m_aWindow.LogicToPixel(Size(RSC_SP_CTRL_GROUP_Y, RSC_SP_CTRL_GROUP_Y), MAP_APPFONT).Height();
        if(nContentHeight < m_aScrollBar.GetSizePixel().Height())
        {
            nContentHeight = m_aScrollBar.GetSizePixel().Height();
            m_aScrollBar.Enable(sal_False);
        }
        else
        {
            m_aScrollBar.Enable(sal_True);
            m_aScrollBar.SetRange(Range(0, nLines));
            m_aScrollBar.SetThumbPos(0);
            m_aScrollBar.SetVisibleSize(nVisibleLines);
        }
        Size aWinOutputSize(m_aWinOutputSize);
        aWinOutputSize.Height() = nContentHeight;
        m_aWindow.SetOutputSizePixel(aWinOutputSize);

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
            ::std::vector<Edit*>::iterator aEditIter;
            sal_uInt32 nIndex = 0;
            for(aEditIter = m_aEdits.begin(); aEditIter != m_aEdits.end(); ++aEditIter, ++nIndex)
            {
                OSL_ENSURE(nIndex < m_pData->aDBData[m_nCurrentDataSet].size(),
                            "number of colums doesn't match number of Edits");
                (*aEditIter)->SetText(m_pData->aDBData[m_nCurrentDataSet][nIndex]);
            }
        }
    }
}

IMPL_LINK(SwAddressControl_Impl, ScrollHdl_Impl, ScrollBar*, pScroll)
{
    long nThumb = pScroll->GetThumbPos();
    m_aWindow.SetPosPixel(Point(0, - (m_nLineHeight * nThumb)));

    return 0;
}

IMPL_LINK(SwAddressControl_Impl, GotFocusHdl_Impl, Edit*, pEdit)
{
    if(0 != (GETFOCUS_TAB & pEdit->GetGetFocusFlags()))
    {
        Rectangle aRect(pEdit->GetPosPixel(), pEdit->GetSizePixel());
        MakeVisible(aRect);
    }
    return 0;
}

void SwAddressControl_Impl::MakeVisible(const Rectangle & rRect)
{
    long nThumb = m_aScrollBar.GetThumbPos();
    //determine range of visible positions
    long nMinVisiblePos = - m_aWindow.GetPosPixel().Y();
    long nMaxVisiblePos = m_aScrollBar.GetSizePixel().Height() + nMinVisiblePos;
    if( rRect.TopLeft().Y() < nMinVisiblePos)
    {
        nThumb -= 1 + ((nMinVisiblePos - rRect.TopLeft().Y()) / m_nLineHeight);
    }
    else if(rRect.BottomLeft().Y() > nMaxVisiblePos)
    {
        nThumb += 1 + ((nMaxVisiblePos - rRect.BottomLeft().Y()) / m_nLineHeight);
    }
    if(nThumb != m_aScrollBar.GetThumbPos())
    {
        m_aScrollBar.SetThumbPos(nThumb);
        ScrollHdl_Impl(&m_aScrollBar);
    }
}

// copy data changes into database
IMPL_LINK(SwAddressControl_Impl, EditModifyHdl_Impl, Edit*, pEdit)
{
    //get the data element number of the current set
    sal_Int32 nIndex = (sal_Int32)(sal_IntPtr)pEdit->GetData();
    //get the index of the set
    OSL_ENSURE(m_pData->aDBData.size() > m_nCurrentDataSet, "wrong data set index" );
    if(m_pData->aDBData.size() > m_nCurrentDataSet)
    {
        m_pData->aDBData[m_nCurrentDataSet][nIndex] = pEdit->GetText();
    }
    return 0;
}

void SwAddressControl_Impl::SetCursorTo(sal_uInt32 nElement)
{
    if(nElement < m_aEdits.size())
    {
        Edit* pEdit = m_aEdits[nElement];
        pEdit->GrabFocus();
        Rectangle aRect(pEdit->GetPosPixel(), pEdit->GetSizePixel());
        MakeVisible(aRect);
    }

}

void SwAddressControl_Impl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && COMMAND_WHEEL_ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand( rCEvt, 0, &m_aScrollBar );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
    }
}

long SwAddressControl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == EVENT_COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        sal_uInt16 nCmd = pCEvt->GetCommand();
        if( COMMAND_WHEEL == nCmd )
        {
            Command(*pCEvt);
            return 1;
        }
    }
    return Control::PreNotify(rNEvt);
}

SwCreateAddressListDialog::SwCreateAddressListDialog(
        Window* pParent, const String& rURL, SwMailMergeConfigItem& rConfig) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_CREATEADDRESSLIST)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aAddressInformation( this, SW_RES(  FI_ADDRESSINFORMATION)),
    m_pAddressControl(new SwAddressControl_Impl(this, SW_RES(CT_ADDRESS))),
    m_aNewPB( this, SW_RES(               PB_NEW)),
    m_aDeletePB( this, SW_RES(            PB_DELETE)),
    m_aFindPB( this, SW_RES(              PB_FIND)),
    m_aCustomizePB( this, SW_RES(         PB_CUSTOMIZE)),

    m_aViewEntriesFI( this, SW_RES(       FI_VIEWENTRIES)),
    m_aStartPB( this, SW_RES(             PB_START)),
    m_aPrevPB( this, SW_RES(              PB_PREV)),
    m_aSetNoNF( this, SW_RES(             NF_SETNO)),
    m_aNextPB( this, SW_RES(              PB_NEXT  )),
    m_aEndPB( this, SW_RES(               PB_END)),

    m_aSeparatorFL( this, SW_RES(         FL_SEPARATOR)),

    m_aOK( this, SW_RES(                  PB_OK)),
    m_aCancel( this, SW_RES(              PB_CANCEL)),
    m_aHelp( this, SW_RES(                PB_HELP)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sAddressListFilterName( SW_RES(    ST_FILTERNAME)),
    m_sURL(rURL),
    m_pCSVData( new SwCSVData ),
    m_pFindDlg(0)
{
    FreeResource();
    m_aNewPB.SetClickHdl(LINK(this, SwCreateAddressListDialog, NewHdl_Impl));
    m_aDeletePB.SetClickHdl(LINK(this, SwCreateAddressListDialog, DeleteHdl_Impl));
    m_aFindPB.SetClickHdl(LINK(this, SwCreateAddressListDialog, FindHdl_Impl));
    m_aCustomizePB.SetClickHdl(LINK(this, SwCreateAddressListDialog, CustomizeHdl_Impl));
    m_aOK.SetClickHdl(LINK(this, SwCreateAddressListDialog, OkHdl_Impl));

    Link aLk = LINK(this, SwCreateAddressListDialog, DBCursorHdl_Impl);
    m_aStartPB.SetClickHdl(aLk);
    m_aPrevPB.SetClickHdl(aLk);
    m_aSetNoNF.SetModifyHdl(LINK(this, SwCreateAddressListDialog, DBNumCursorHdl_Impl));
    m_aNextPB.SetClickHdl(aLk);
    m_aEndPB.SetClickHdl(aLk);

    if(m_sURL.Len())
    {
        //file exists, has to be loaded here
        SfxMedium aMedium( m_sURL, STREAM_READ );
        SvStream* pStream = aMedium.GetInStream();
        if(pStream)
        {
            pStream->SetLineDelimiter( LINEEND_LF );
            pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);

            OUString sSemi(';');
            OUString sQuote('"');
            String sTempMiddle(sQuote);
            sTempMiddle += sal_Unicode(9);
            OUString sMiddle(sTempMiddle);

            String sLine;
            sal_Bool bRead = pStream->ReadByteStringLine( sLine, RTL_TEXTENCODING_UTF8 );

            if(bRead)
            {
                //header line
                xub_StrLen nHeaders = comphelper::string::getTokenCount(sLine, '\t');
                xub_StrLen nIndex = 0;
                for( xub_StrLen nToken = 0; nToken < nHeaders; ++nToken)
                {
                    String sHeader = sLine.GetToken( 0, '\t', nIndex );
                    OSL_ENSURE(sHeader.Len() > 2 &&
                            sHeader.GetChar(0) == '\"' && sHeader.GetChar(sHeader.Len() - 1) == '\"',
                            "Wrong format of header");
                    if(sHeader.Len() > 2)
                    {
                        m_pCSVData->aDBColumnHeaders.push_back( sHeader.Copy(1, sHeader.Len() -2));
                    }
                }
            }
            while(pStream->ReadByteStringLine( sLine, RTL_TEXTENCODING_UTF8 ))
            {
                ::std::vector<OUString> aNewData;
                //analyze data line
                xub_StrLen nDataCount = comphelper::string::getTokenCount(sLine, '\t');
                xub_StrLen nIndex = 0;
                for( xub_StrLen nToken = 0; nToken < nDataCount; ++nToken)
                {
                    String sData = sLine.GetToken( 0, '\t', nIndex );
                    OSL_ENSURE(sData.Len() >= 2 &&
                                sData.GetChar(0) == '\"' && sData.GetChar(sData.Len() - 1) == '\"',
                            "Wrong format of line");
                    if(sData.Len() >= 2)
                        aNewData.push_back(sData.Copy(1, sData.Len() - 2));
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
        const ResStringArray& rAddressHeader = rConfig.GetDefaultAddressHeaders();
        sal_uInt32 nCount = rAddressHeader.Count();
        for(sal_uInt16 nHeader = 0; nHeader < nCount; ++nHeader)
            m_pCSVData->aDBColumnHeaders.push_back( rAddressHeader.GetString(nHeader));
        ::std::vector<OUString> aNewData;
        String sTemp;
        aNewData.insert(aNewData.begin(), nCount, sTemp);
        m_pCSVData->aDBData.push_back(aNewData);
    }
    //now fill the address control
    m_pAddressControl->SetData(*m_pCSVData);
    m_pAddressControl->SetCurrentDataSet(0);
    m_aSetNoNF.SetMax(m_pCSVData->aDBData.size());
    UpdateButtons();
}

SwCreateAddressListDialog::~SwCreateAddressListDialog()
{
    delete m_pAddressControl;
    delete m_pCSVData;
    delete m_pFindDlg;
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, NewHdl_Impl)
{
    sal_uInt32 nCurrent = m_pAddressControl->GetCurrentDataSet();
    ::std::vector<OUString> aNewData;
    String sTemp;
    aNewData.insert(aNewData.begin(), m_pCSVData->aDBColumnHeaders.size(), sTemp);
    m_pCSVData->aDBData.insert(m_pCSVData->aDBData.begin() + ++nCurrent, aNewData);
    m_aSetNoNF.SetMax(m_pCSVData->aDBData.size());
    //the NumericField start at 1
    m_aSetNoNF.SetValue(nCurrent + 1);
    //the address control starts at 0
    m_pAddressControl->SetCurrentDataSet(nCurrent);
    UpdateButtons();
    return 0;
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DeleteHdl_Impl)
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
        String sTemp;
        m_pCSVData->aDBData[0].assign(m_pCSVData->aDBData[0].size(), sTemp);
        m_aDeletePB.Enable(sal_False);
    }
    m_pAddressControl->SetCurrentDataSet(nCurrent);
    m_aSetNoNF.SetMax(m_pCSVData->aDBData.size());
    UpdateButtons();
    return 0;
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, FindHdl_Impl)
{
    if(!m_pFindDlg)
    {
        m_pFindDlg = new SwFindEntryDialog(this);
        ListBox& rColumnBox = m_pFindDlg->GetFieldsListBox();
        ::std::vector< OUString >::iterator  aHeaderIter;
        for(aHeaderIter = m_pCSVData->aDBColumnHeaders.begin();
                    aHeaderIter != m_pCSVData->aDBColumnHeaders.end();
                    ++aHeaderIter)
            rColumnBox.InsertEntry(*aHeaderIter);
        rColumnBox.SelectEntryPos( 0 );
    }
    else
        m_pFindDlg->Show(!m_pFindDlg->IsVisible());
    return 0;
}

IMPL_LINK(SwCreateAddressListDialog, CustomizeHdl_Impl, PushButton*, pButton)
{
    SwCustomizeAddressListDialog* pDlg = new SwCustomizeAddressListDialog(pButton, *m_pCSVData);
    if(RET_OK == pDlg->Execute())
    {
        delete m_pCSVData;
        m_pCSVData = pDlg->GetNewData();
        m_pAddressControl->SetData(*m_pCSVData);
        m_pAddressControl->SetCurrentDataSet(m_pAddressControl->GetCurrentDataSet());
    }
    delete pDlg;

    //update find dialog
    if(m_pFindDlg)
    {
        ListBox& rColumnBox = m_pFindDlg->GetFieldsListBox();
        rColumnBox.Clear();
        ::std::vector< OUString >::iterator  aHeaderIter;
        for(aHeaderIter = m_pCSVData->aDBColumnHeaders.begin();
                    aHeaderIter != m_pCSVData->aDBColumnHeaders.end();
                    ++aHeaderIter)
            rColumnBox.InsertEntry(*aHeaderIter);
    }
    return 0;
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, OkHdl_Impl)
{
    if(!m_sURL.Len())
    {
        sfx2::FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_SIMPLE, 0 );
        uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

        String sPath( SvtPathOptions().SubstituteVariable(
                    rtl::OUString("$(userurl)/database") ));
        aDlgHelper.SetDisplayDirectory( sPath );
        uno::Reference< XFilterManager > xFltMgr(xFP, uno::UNO_QUERY);
        ::rtl::OUString sCSV(C2U("*.csv"));
        xFltMgr->appendFilter( m_sAddressListFilterName, sCSV );
        xFltMgr->setCurrentFilter( m_sAddressListFilterName ) ;

        if( ERRCODE_NONE == aDlgHelper.Execute() )
        {
            m_sURL = xFP->getFiles().getConstArray()[0];
            INetURLObject aResult( m_sURL );
            aResult.setExtension(rtl::OUString("csv"));
            m_sURL = aResult.GetMainURL(INetURLObject::NO_DECODE);
        }
    }
    if(m_sURL.Len())
    {
        SfxMedium aMedium( m_sURL, STREAM_READWRITE|STREAM_TRUNC );
        SvStream* pStream = aMedium.GetOutStream();
        pStream->SetLineDelimiter( LINEEND_LF );
        pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);

        OUString sSemi(';');
        OUString sQuote('"');
        String sTempMiddle(sQuote);
        sTempMiddle += sal_Unicode(9);
        OUString sMiddle(sTempMiddle);
        sMiddle += sQuote;

        //create a string for the header line
        OUString sLine(sQuote);
        ::std::vector< OUString >::iterator  aHeaderIter;
        for(aHeaderIter = m_pCSVData->aDBColumnHeaders.begin();
                    aHeaderIter != m_pCSVData->aDBColumnHeaders.end();
                    ++aHeaderIter)
        {
            sLine += *aHeaderIter;
            sLine += sMiddle;
        }
        //remove tab and quote
        sLine = sLine.copy( 0, sLine.getLength() - 2 );
        pStream->WriteByteStringLine( sLine, RTL_TEXTENCODING_UTF8 );

        ::std::vector< ::std::vector< OUString > >::iterator aDataIter;
        for( aDataIter = m_pCSVData->aDBData.begin(); aDataIter != m_pCSVData->aDBData.end(); ++aDataIter)
        {
            sLine = sQuote;
            ::std::vector< OUString >::iterator  aColumnIter;
            for(aColumnIter = aDataIter->begin(); aColumnIter != aDataIter->end(); ++aColumnIter)
            {
                sLine += *aColumnIter;
                sLine += sMiddle;
            }
            //remove tab and quote
            sLine = sLine.copy( 0, sLine.getLength() - 2 );
            pStream->WriteByteStringLine( sLine, RTL_TEXTENCODING_UTF8 );
        }
        aMedium.Commit();
        EndDialog(RET_OK);
    }

    return 0;
}

IMPL_LINK(SwCreateAddressListDialog, DBCursorHdl_Impl, PushButton*, pButton)
{
    sal_uInt32 nValue = static_cast< sal_uInt32 >(m_aSetNoNF.GetValue());

    if(pButton == &m_aStartPB)
        nValue = 1;
    else if(pButton == &m_aPrevPB)
    {
        if(nValue > 1)
            --nValue;
    }
    else if(pButton == &m_aNextPB)
    {
        if(nValue < (sal_uInt32)m_aSetNoNF.GetMax())
            ++nValue;
    }
    else //m_aEndPB
        nValue = static_cast< sal_uInt32 >(m_aSetNoNF.GetMax());
    if(nValue != m_aSetNoNF.GetValue())
    {
        m_aSetNoNF.SetValue(nValue);
        DBNumCursorHdl_Impl(&m_aSetNoNF);
    }
    return 0;
}

IMPL_LINK_NOARG(SwCreateAddressListDialog, DBNumCursorHdl_Impl)
{
    m_pAddressControl->SetCurrentDataSet( static_cast< sal_uInt32 >(m_aSetNoNF.GetValue() - 1) );
    UpdateButtons();
    return 0;
}

void SwCreateAddressListDialog::UpdateButtons()
{
    sal_uInt32 nCurrent = static_cast< sal_uInt32 >(m_aSetNoNF.GetValue() );
    sal_uInt32 nSize = (sal_uInt32 )m_pCSVData->aDBData.size();
    m_aStartPB.Enable(nCurrent != 1);
    m_aPrevPB.Enable(nCurrent != 1);
    m_aNextPB.Enable(nCurrent != nSize);
    m_aEndPB.Enable(nCurrent != nSize);
    m_aDeletePB.Enable(nSize > 0);
}

void SwCreateAddressListDialog::Find(const String& rSearch, sal_Int32 nColumn)
{
    OUString sSearch = rSearch;
    sSearch.toAsciiLowerCase();
    sal_uInt32 nCurrent = m_pAddressControl->GetCurrentDataSet();
    //search forward
    bool bFound = false;
    sal_uInt32 nStart = nCurrent + 1;
    sal_uInt32 nEnd = m_pCSVData->aDBData.size();
    sal_uInt32 nElement = 0;
    sal_uInt32 nPos = 0;
    for(short nTemp = 0; nTemp < 2 && !bFound; nTemp++)
    {
        for(nPos = nStart; nPos < nEnd; ++nPos)
        {
            ::std::vector< OUString> aData = m_pCSVData->aDBData[nPos];
            if(nColumn >=0)
                bFound = -1 != aData[(sal_uInt32)nColumn].toAsciiLowerCase().indexOf(sSearch);
            else
            {
                for( nElement = 0; nElement < aData.size(); ++nElement)
                {
                    bFound = -1 != aData[nElement].toAsciiLowerCase().indexOf(sSearch);
                    if(bFound)
                    {
                        nColumn = nElement;
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
        m_aSetNoNF.SetValue( nPos + 1 );
        UpdateButtons();
        m_pAddressControl->SetCursorTo(nElement);
    }
}

SwFindEntryDialog::SwFindEntryDialog(SwCreateAddressListDialog* pParent) :
    ModelessDialog(pParent, SW_RES(DLG_MM_FIND_ENTRY)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aFindFT( this, SW_RES(      FT_FIND      )),
    m_aFindED( this, SW_RES(      ED_FIND      )),
    m_aFindOnlyCB( this, SW_RES(  CB_FINDONLY )),
    m_aFindOnlyLB( this, SW_RES(  LB_FINDONLY  )),
    m_aFindPB( this, SW_RES(      PB_FIND)),
    m_aCancel( this, SW_RES(      PB_CANCEL)),
    m_aHelp( this, SW_RES(        PB_HELP)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
   m_pParent(pParent)
{
    FreeResource();
    m_aFindPB.SetClickHdl(LINK(this, SwFindEntryDialog, FindHdl_Impl));
    m_aFindED.SetModifyHdl(LINK(this, SwFindEntryDialog, FindEnableHdl_Impl));
    m_aCancel.SetClickHdl(LINK(this, SwFindEntryDialog, CloseHdl_Impl));
}

SwFindEntryDialog::~SwFindEntryDialog()
{
}

IMPL_LINK_NOARG(SwFindEntryDialog, FindHdl_Impl)
{
    sal_Int32 nColumn = -1;
    if(m_aFindOnlyCB.IsChecked())
        nColumn = m_aFindOnlyLB.GetSelectEntryPos();
    if(nColumn != LISTBOX_ENTRY_NOTFOUND)
        m_pParent->Find(m_aFindED.GetText(), nColumn);
    return 0;
}

IMPL_LINK_NOARG(SwFindEntryDialog, FindEnableHdl_Impl)
{
    m_aFindPB.Enable(m_aFindED.GetText().Len() > 0);
    return 0;
}

IMPL_LINK_NOARG(SwFindEntryDialog, CloseHdl_Impl)
{
    Show(sal_False);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
