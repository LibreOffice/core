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

#include <swtypes.hxx>
#include <mailmergehelper.hxx>
#include <svtools/stdctrl.hxx>
#include <mmconfigitem.hxx>
#include <docsh.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include "com/sun/star/mail/MailServiceProvider.hpp"
#include "com/sun/star/mail/XSmtpService.hpp"
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/passwd.hxx>

#include <dbui.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;


namespace SwMailMergeHelper
{

String  CallSaveAsDialog(String& rFilter)
{
    ErrCode nRet;
    String sFactory(OUString::createFromAscii(SwDocShell::Factory().GetShortName()));
    ::sfx2::FileDialogHelper aDialog( ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
                0,
                sFactory );

    String sRet;
    nRet = aDialog.Execute();
    if(ERRCODE_NONE == nRet)
    {
        uno::Reference < ui::dialogs::XFilePicker > xFP = aDialog.GetFilePicker();
        sRet = xFP->getFiles().getConstArray()[0];
        rFilter = aDialog.GetRealFilter();
    }
    return sRet;
}

/*
    simple address check: check for '@'
                            for at least one '.' after the '@'
                            and for at least to characters before and after the dot
*/
bool CheckMailAddress( const OUString& rMailAddress )
{
    String sAddress(rMailAddress);
    if (!(comphelper::string::getTokenCount(sAddress, '@') == 2))
        return false;
    sAddress = sAddress.GetToken(1, '@');
    if (comphelper::string::getTokenCount(sAddress, '.') < 2)
        return false;
    if(sAddress.GetToken( 0, '.').Len() < 2 || sAddress.GetToken( 1, '.').Len() < 2)
        return false;
    return true;
}

uno::Reference< mail::XSmtpService > ConnectToSmtpServer(
        SwMailMergeConfigItem& rConfigItem,
        uno::Reference< mail::XMailService >&  rxInMailService,
        const String& rInMailServerPassword,
        const String& rOutMailServerPassword,
        Window* pDialogParentWindow )
{
    uno::Reference< mail::XSmtpService > xSmtpServer;
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    try
    {
        uno::Reference< mail::XMailServiceProvider > xMailServiceProvider(
            mail::MailServiceProvider::create( xContext ) );
        xSmtpServer = uno::Reference< mail::XSmtpService > (
                        xMailServiceProvider->create(
                        mail::MailServiceType_SMTP
                        ), uno::UNO_QUERY);

        uno::Reference< mail::XConnectionListener> xConnectionListener(new SwConnectionListener());

        if(rConfigItem.IsAuthentication() && rConfigItem.IsSMTPAfterPOP())
        {
            uno::Reference< mail::XMailService > xInMailService =
                    xMailServiceProvider->create(
                    rConfigItem.IsInServerPOP() ?
                        mail::MailServiceType_POP3 : mail::MailServiceType_IMAP);
            //authenticate at the POP or IMAP server first
            String sPasswd = rConfigItem.GetInServerPassword();
            if(rInMailServerPassword.Len())
                sPasswd = rInMailServerPassword;
            uno::Reference<mail::XAuthenticator> xAuthenticator =
                new SwAuthenticator(
                    rConfigItem.GetInServerUserName(),
                    sPasswd,
                    pDialogParentWindow);

            xInMailService->addConnectionListener(xConnectionListener);
            //check connection
            uno::Reference< uno::XCurrentContext> xConnectionContext =
                    new SwConnectionContext(
                        rConfigItem.GetInServerName(),
                        rConfigItem.GetInServerPort(),
                        OUString("Insecure"));
            xInMailService->connect(xConnectionContext, xAuthenticator);
            rxInMailService = xInMailService;
        }
        uno::Reference< mail::XAuthenticator> xAuthenticator;
        if(rConfigItem.IsAuthentication() &&
                !rConfigItem.IsSMTPAfterPOP() &&
                !rConfigItem.GetMailUserName().isEmpty())
        {
            String sPasswd = rConfigItem.GetMailPassword();
            if(rOutMailServerPassword.Len())
                sPasswd = rOutMailServerPassword;
            xAuthenticator =
                new SwAuthenticator(rConfigItem.GetMailUserName(),
                        sPasswd,
                        pDialogParentWindow);
        }
        else
            xAuthenticator =  new SwAuthenticator();
        //just to check if the server exists
        xSmtpServer->getSupportedConnectionTypes();
        //check connection

        uno::Reference< uno::XCurrentContext> xConnectionContext =
                new SwConnectionContext(
                    rConfigItem.GetMailServer(),
                    rConfigItem.GetMailPort(),
                    rConfigItem.IsSecureConnection() ? OUString("Ssl") : OUString("Insecure") );
        xSmtpServer->connect(xConnectionContext, xAuthenticator);
        rxInMailService = uno::Reference< mail::XMailService >( xSmtpServer, uno::UNO_QUERY );
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL("exception caught");
    }
    return xSmtpServer;
}


} //namespace

SwBoldFixedInfo::SwBoldFixedInfo(Window* pParent, const ResId& rResId) :
    FixedInfo(pParent, rResId)
{
    Font aFont = GetFont();
    aFont.SetWeight( WEIGHT_BOLD );
    SetFont( aFont );
}

SwBoldFixedInfo::~SwBoldFixedInfo()
{
}

struct  SwAddressPreview_Impl
{
    ::std::vector< OUString >    aAdresses;
    sal_uInt16                          nRows;
    sal_uInt16                          nColumns;
    sal_uInt16                          nSelectedAddress;
    bool                                bEnableScrollBar;

    SwAddressPreview_Impl() :
        nRows(1),
        nColumns(1),
        nSelectedAddress(0),
        bEnableScrollBar(false)
    {
    }
};

SwAddressPreview::SwAddressPreview(Window* pParent, const ResId& rResId)
    : Window( pParent, rResId )
    , aVScrollBar(this, WB_VSCROLL)
    , pImpl(new SwAddressPreview_Impl())
{
    aVScrollBar.SetScrollHdl(LINK(this, SwAddressPreview, ScrollHdl));
    positionScrollBar();
    Show();
}

SwAddressPreview::SwAddressPreview(Window* pParent, WinBits nStyle)
    : Window( pParent, nStyle )
    , aVScrollBar(this, WB_VSCROLL)
    , pImpl(new SwAddressPreview_Impl())
{
    aVScrollBar.SetScrollHdl(LINK(this, SwAddressPreview, ScrollHdl));
    positionScrollBar();
    Show();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwAddressPreview(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_DIALOGCONTROL;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SwAddressPreview(pParent, nWinStyle);
}

void SwAddressPreview::positionScrollBar()
{
    Size aSize(GetOutputSizePixel());
    Size aScrollSize(aVScrollBar.get_preferred_size().Width(), aSize.Height());
    aVScrollBar.SetSizePixel(aScrollSize);
    Point aSrollPos(aSize.Width() - aScrollSize.Width(), 0);
    aVScrollBar.SetPosPixel(aSrollPos);
}

void SwAddressPreview::Resize()
{
    Window::Resize();
    positionScrollBar();
}

IMPL_LINK_NOARG(SwAddressPreview, ScrollHdl)
{
    Invalidate();
    return 0;
}

void SwAddressPreview::AddAddress(const OUString& rAddress)
{
    pImpl->aAdresses.push_back(rAddress);
    UpdateScrollBar();
}

void SwAddressPreview::SetAddress(const OUString& rAddress)
{
    pImpl->aAdresses.clear();
    pImpl->aAdresses.push_back(rAddress);
    aVScrollBar.Show(sal_False);
    Invalidate();
}

sal_uInt16   SwAddressPreview::GetSelectedAddress()const
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAdresses.size(), "selection invalid");
    return pImpl->nSelectedAddress;
}

void SwAddressPreview::SelectAddress(sal_uInt16 nSelect)
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAdresses.size(), "selection invalid");
    pImpl->nSelectedAddress = nSelect;
    // now make it visible..
    sal_uInt16 nSelectRow = nSelect / pImpl->nColumns;
    sal_uInt16 nStartRow = (sal_uInt16)aVScrollBar.GetThumbPos();
    if( (nSelectRow < nStartRow) || (nSelectRow >= (nStartRow + pImpl->nRows) ))
        aVScrollBar.SetThumbPos( nSelectRow );
}

void SwAddressPreview::Clear()
{
    pImpl->aAdresses.clear();
    pImpl->nSelectedAddress = 0;
    UpdateScrollBar();
}

void SwAddressPreview::ReplaceSelectedAddress(const OUString& rNew)
{
    pImpl->aAdresses[pImpl->nSelectedAddress] = rNew;
    Invalidate();
}

void SwAddressPreview::RemoveSelectedAddress()
{
    pImpl->aAdresses.erase(pImpl->aAdresses.begin() + pImpl->nSelectedAddress);
    if(pImpl->nSelectedAddress)
        --pImpl->nSelectedAddress;
    UpdateScrollBar();
    Invalidate();
}

void SwAddressPreview::SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns)
{
    pImpl->nRows = nRows;
    pImpl->nColumns = nColumns;
    UpdateScrollBar();
}

void SwAddressPreview::EnableScrollBar(bool bEnable)
{
    pImpl->bEnableScrollBar = bEnable;
}

void SwAddressPreview::UpdateScrollBar()
{
    if(pImpl->nColumns)
    {
        aVScrollBar.SetVisibleSize(pImpl->nRows);
        sal_uInt16 nResultingRows = (sal_uInt16)(pImpl->aAdresses.size() + pImpl->nColumns - 1) / pImpl->nColumns;
        ++nResultingRows;
        aVScrollBar.Show(pImpl->bEnableScrollBar && nResultingRows > pImpl->nRows);
        aVScrollBar.SetRange(Range(0, nResultingRows));
        if(aVScrollBar.GetThumbPos() > nResultingRows)
            aVScrollBar.SetThumbPos(nResultingRows);
    }
}

void SwAddressPreview::Paint(const Rectangle&)
{
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    SetFillColor(rSettings.GetWindowColor());
    SetLineColor( Color(COL_TRANSPARENT) );
    DrawRect( Rectangle(Point(0, 0), GetOutputSizePixel()) );
    Color aPaintColor(IsEnabled() ? rSettings.GetWindowTextColor() : rSettings.GetDisableColor());
    SetLineColor(aPaintColor);
    Font aFont(GetFont());
    aFont.SetColor(aPaintColor);
    SetFont(aFont);

    Size aSize = GetOutputSizePixel();
    sal_uInt16 nStartRow = 0;
    if(aVScrollBar.IsVisible())
    {
        aSize.Width() -= aVScrollBar.GetSizePixel().Width();
        nStartRow = (sal_uInt16)aVScrollBar.GetThumbPos();
    }
    Size aPartSize( aSize.Width()/pImpl->nColumns, aSize.Height()/pImpl->nRows );
    aPartSize.Width() -= 2;
    aPartSize.Height() -= 2;

    sal_uInt16 nAddress = nStartRow * pImpl->nColumns;
    const sal_uInt16 nNumAddresses = static_cast< sal_uInt16 >(pImpl->aAdresses.size());
    for(sal_uInt16 nRow = 0; nRow < pImpl->nRows ; ++nRow)
    {
        for(sal_uInt16 nCol = 0; nCol < pImpl->nColumns; ++nCol)
        {
            if(nAddress >= nNumAddresses)
                break;
            Point aPos(nCol * aPartSize.Width(), (nRow) * aPartSize.Height());
            aPos.Move(1,1);
            bool bIsSelected = nAddress == pImpl->nSelectedAddress;
            if((pImpl->nColumns * pImpl->nRows) == 1)
                bIsSelected = false;
            OUString adr(pImpl->aAdresses[nAddress]);
            DrawText_Impl(adr,aPos,aPartSize,bIsSelected);
            ++nAddress;
        }
    }
    SetClipRegion();
}

void  SwAddressPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    Window::MouseButtonDown(rMEvt);
    if(rMEvt.IsLeft() && ( pImpl->nRows || pImpl->nColumns))
    {
        //determine the selected address
        const Point& rMousePos = rMEvt.GetPosPixel();
        Size aSize(GetOutputSizePixel());
        Size aPartSize( aSize.Width()/pImpl->nColumns, aSize.Height()/pImpl->nRows );
        sal_uInt32 nRow = rMousePos.Y() / aPartSize.Height() ;
        if(aVScrollBar.IsVisible())
        {
            nRow += (sal_uInt16)aVScrollBar.GetThumbPos();
        }
        sal_uInt32 nCol = rMousePos.X() / aPartSize.Width();
        sal_uInt32 nSelect = nRow * pImpl->nColumns + nCol;

        if( nSelect < pImpl->aAdresses.size() &&
                pImpl->nSelectedAddress != (sal_uInt16)nSelect)
        {
            pImpl->nSelectedAddress = (sal_uInt16)nSelect;
            m_aSelectHdl.Call(this);
        }
        Invalidate();
    }
}

void  SwAddressPreview::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    if(pImpl->nRows || pImpl->nColumns)
    {
        sal_uInt32 nSelectedRow =    (pImpl->nSelectedAddress + 1)/ pImpl->nColumns;
        sal_uInt32 nSelectedColumn = pImpl->nSelectedAddress % nSelectedRow;
        switch(nKey)
        {
            case KEY_UP:
                if(nSelectedRow)
                    --nSelectedRow;
            break;
            case KEY_DOWN:
                if(pImpl->aAdresses.size() > sal_uInt32(pImpl->nSelectedAddress + pImpl->nColumns))
                    ++nSelectedRow;
            break;
            case KEY_LEFT:
                if(nSelectedColumn)
                    --nSelectedColumn;
            break;
            case KEY_RIGHT:
                if(nSelectedColumn < sal_uInt32(pImpl->nColumns - 1) &&
                       pImpl->aAdresses.size() - 1 > pImpl->nSelectedAddress )
                    ++nSelectedColumn;
            break;
        }
        sal_uInt32 nSelect = nSelectedRow * pImpl->nColumns + nSelectedColumn;
        if( nSelect < pImpl->aAdresses.size() &&
                pImpl->nSelectedAddress != (sal_uInt16)nSelect)
        {
            pImpl->nSelectedAddress = (sal_uInt16)nSelect;
            m_aSelectHdl.Call(this);
            Invalidate();
        }
    }
    else
        Window::KeyInput(rKEvt);
}

void SwAddressPreview::StateChanged( StateChangedType nStateChange )
{
    if(nStateChange == STATE_CHANGE_ENABLE)
        Invalidate();
    Window::StateChanged(nStateChange);
}

void SwAddressPreview::DrawText_Impl(
        const OUString& rAddress, const Point& rTopLeft, const Size& rSize, bool bIsSelected)
{
    SetClipRegion( Region( Rectangle(rTopLeft, rSize)) );
    if(bIsSelected)
    {
        //selection rectangle
        SetFillColor(Color(COL_TRANSPARENT));
        DrawRect(Rectangle(rTopLeft, rSize));
    }
    sal_Int32 nHeight = GetTextHeight();
    String sAddress(rAddress);
    sal_uInt16 nTokens = comphelper::string::getTokenCount(sAddress, '\n');
    Point aStart = rTopLeft;
    //put it away from the border
    aStart.Move( 2, 2);
    for(sal_uInt16 nToken = 0; nToken < nTokens; nToken++)
    {
        DrawText( aStart, sAddress.GetToken(nToken, '\n') );
        aStart.Y() += nHeight;
    }
}

String SwAddressPreview::FillData(
        const OUString& rAddress,
        SwMailMergeConfigItem& rConfigItem,
        const Sequence< OUString>* pAssignments)
{
    //find the column names in the address string (with name assignment!) and
    //exchange the placeholder (like <Firstname>) with the database content
    //unassigned columns are expanded to <not assigned>
    Reference< XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), UNO_QUERY);
    Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
    Sequence< OUString> aAssignment = pAssignments ?
                    *pAssignments :
                    rConfigItem.GetColumnAssignment(
                                                rConfigItem.GetCurrentDBData() );
    const OUString* pAssignment = aAssignment.getConstArray();
    const ResStringArray& rDefHeaders = rConfigItem.GetDefaultAddressHeaders();
    String sAddress(rAddress);
    String sNotAssigned(SW_RES(STR_NOTASSIGNED));
    sNotAssigned.Insert('<', 0);
    sNotAssigned += '>';

    sal_Bool bIncludeCountry = rConfigItem.IsIncludeCountry();
    const OUString rExcludeCountry = rConfigItem.GetExcludeCountry();
    bool bSpecialReplacementForCountry = (!bIncludeCountry || !rExcludeCountry.isEmpty());
    String sCountryColumn;
    if( bSpecialReplacementForCountry )
    {
        sCountryColumn = rDefHeaders.GetString(MM_PART_COUNTRY);
        Sequence< OUString> aSpecialAssignment =
                        rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
        if(aSpecialAssignment.getLength() > MM_PART_COUNTRY && aSpecialAssignment[MM_PART_COUNTRY].getLength())
            sCountryColumn = aSpecialAssignment[MM_PART_COUNTRY];
    }

    SwAddressIterator aIter(sAddress);
    sAddress.Erase();
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            //get the default column name

            //find the appropriate assignment
            String sConvertedColumn = aItem.sText;
            for(sal_uInt16 nColumn = 0;
                    nColumn < rDefHeaders.Count() && nColumn < aAssignment.getLength();
                                                                                ++nColumn)
            {
                if (rDefHeaders.GetString(nColumn).equals(aItem.sText) &&
                    !pAssignment[nColumn].isEmpty())
                {
                    sConvertedColumn = pAssignment[nColumn];
                    break;
                }
            }
            if(sConvertedColumn.Len() &&
                    xColAccess.is() &&
                    xColAccess->hasByName(sConvertedColumn))
            {
                //get the content and exchange it in the address string
                Any aCol = xColAccess->getByName(sConvertedColumn);
                Reference< XColumn > xColumn;
                aCol >>= xColumn;
                if(xColumn.is())
                {
                    try
                    {
                        OUString sReplace = xColumn->getString();

                        if( bSpecialReplacementForCountry && sCountryColumn == sConvertedColumn )
                        {
                            if( !rExcludeCountry.isEmpty() && sReplace != rExcludeCountry )
                                aItem.sText = sReplace;
                            else
                                aItem.sText.Erase();
                        }
                        else
                        {
                            aItem.sText = sReplace;
                        }
                    }
                    catch (const sdbc::SQLException&)
                    {
                        OSL_FAIL("SQLException caught");
                    }
                }
            }
            else
            {
                aItem.sText = sNotAssigned;
            }

        }
        sAddress += aItem.sText;
    }
    return sAddress;
}

SwMergeAddressItem   SwAddressIterator::Next()
{
    //currently the string may either start with a '<' then it's a column
    //otherwise it's simple text maybe containing a return
    SwMergeAddressItem   aRet;
    if(sAddress.Len())
    {
        if(sAddress.GetChar(0) == '<')
        {
            aRet.bIsColumn = true;
            xub_StrLen nClose = sAddress.Search('>');
            OSL_ENSURE(nClose != STRING_NOTFOUND, "closing '>' not found");
            if( nClose != STRING_NOTFOUND )
            {
                aRet.sText = sAddress.Copy(1, nClose - 1);
                sAddress.Erase(0, nClose + 1);
            }
            else
            {
                aRet.sText = sAddress.Copy(1, 1);
                sAddress.Erase(0, 1);
            }
        }
        else
        {
            xub_StrLen nOpen = sAddress.Search('<');
            xub_StrLen nReturn = sAddress.Search('\n');
            if(nReturn == 0)
            {
                aRet.bIsReturn = true;
                aRet.sText = '\n';
                sAddress.Erase(0, 1);
            }
            else if(STRING_NOTFOUND == nOpen && STRING_NOTFOUND == nReturn)
            {
                nOpen = sAddress.Len();
                aRet.sText = sAddress;
                sAddress.Erase();
            }
            else
            {
                xub_StrLen nTarget = ::std::min(nOpen, nReturn);
                aRet.sText = sAddress.Copy(0, nTarget);
                sAddress.Erase(0, nTarget);
            }
        }
    }
    return aRet;

}

SwAuthenticator::~SwAuthenticator()
{
}

OUString SwAuthenticator::getUserName( ) throw (RuntimeException)
{
    return m_aUserName;
}

OUString SwAuthenticator::getPassword(  ) throw (RuntimeException)
{
    if(!m_aUserName.isEmpty() && m_aPassword.isEmpty() && m_pParentWindow)
    {
       SfxPasswordDialog* pPasswdDlg =
                new SfxPasswordDialog( m_pParentWindow );
       pPasswdDlg->SetMinLen( 0 );
       if(RET_OK == pPasswdDlg->Execute())
            m_aPassword = pPasswdDlg->GetPassword();
    }
    return m_aPassword;
}

SwConnectionContext::SwConnectionContext(
        const OUString& rMailServer, sal_Int16 nPort,
        const OUString& rConnectionType) :
    m_sMailServer(rMailServer),
    m_nPort(nPort),
    m_sConnectionType(rConnectionType)
{
}

SwConnectionContext::~SwConnectionContext()
{
}

uno::Any SwConnectionContext::getValueByName( const OUString& rName )
                                                throw (uno::RuntimeException)
{
    uno::Any aRet;
    if( !rName.compareToAscii( "ServerName" ))
        aRet <<= m_sMailServer;
    else if( !rName.compareToAscii( "Port" ))
        aRet <<= (sal_Int32) m_nPort;
    else if( !rName.compareToAscii( "ConnectionType" ))
        aRet <<= m_sConnectionType;
    return aRet;
}

SwConnectionListener::~SwConnectionListener()
{
}

void SwConnectionListener::connected(const lang::EventObject& /*aEvent*/)
    throw (uno::RuntimeException)
{
}

void SwConnectionListener::disconnected(const lang::EventObject& /*aEvent*/)
    throw (uno::RuntimeException)
{
}

void SwConnectionListener::disposing(const lang::EventObject& /*aEvent*/)
    throw(uno::RuntimeException)
{
}

SwMailTransferable::SwMailTransferable(const OUString& rBody, const OUString& rMimeType) :
    cppu::WeakComponentImplHelper2< datatransfer::XTransferable, beans::XPropertySet >(m_aMutex),
    m_aMimeType( rMimeType ),
    m_sBody( rBody ),
    m_bIsBody( true )
{
}

SwMailTransferable::SwMailTransferable(const OUString& rURL,
                const OUString& rName, const OUString& rMimeType) :
    cppu::WeakComponentImplHelper2< datatransfer::XTransferable, beans::XPropertySet >(m_aMutex),
    m_aMimeType( rMimeType ),
    m_aURL(rURL),
    m_aName( rName ),
    m_bIsBody( false )
{
}

SwMailTransferable::~SwMailTransferable()
{
}

uno::Any SwMailTransferable::getTransferData( const datatransfer::DataFlavor& /*aFlavor*/ )
                            throw (datatransfer::UnsupportedFlavorException,
                            io::IOException, uno::RuntimeException)
{
    uno::Any aRet;
    if( m_bIsBody )
        aRet <<= OUString(m_sBody);
    else
    {
        Sequence<sal_Int8> aData;
        SfxMedium aMedium( m_aURL, STREAM_STD_READ );
        SvStream* pStream = aMedium.GetInStream();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE && pStream)
        {
            pStream->Seek(STREAM_SEEK_TO_END);
            aData.realloc(pStream->Tell());
            pStream->Seek(0);
            sal_Int8 * pData = aData.getArray();
            pStream->Read( pData, aData.getLength() );
        }
        aRet <<= aData;
    }
    return aRet;
}

uno::Sequence< datatransfer::DataFlavor > SwMailTransferable::getTransferDataFlavors(  )
                            throw (uno::RuntimeException)
{
    uno::Sequence< datatransfer::DataFlavor > aRet(1);
    aRet[0].MimeType = m_aMimeType;
    if( m_bIsBody )
    {
        aRet[0].DataType = getCppuType((OUString*)0);
    }
    else
    {
        aRet[0].HumanPresentableName = m_aName;
        aRet[0].DataType = getCppuType((uno::Sequence<sal_Int8>*)0);
    }
    return aRet;
}

sal_Bool SwMailTransferable::isDataFlavorSupported(
            const datatransfer::DataFlavor& aFlavor )
                            throw (uno::RuntimeException)
{
    return (aFlavor.MimeType == OUString(m_aMimeType));
}

uno::Reference< beans::XPropertySetInfo > SwMailTransferable::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void SwMailTransferable::setPropertyValue( const OUString& , const uno::Any& )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
}

uno::Any SwMailTransferable::getPropertyValue( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName == "URL" )
        aRet <<= m_aURL;
    return aRet;
}

void SwMailTransferable::addPropertyChangeListener(
    const OUString&, const uno::Reference< beans::XPropertyChangeListener >&  )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SwMailTransferable::removePropertyChangeListener(
    const OUString&,
    const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SwMailTransferable::addVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SwMailTransferable::removeVetoableChangeListener(
    const OUString& ,
    const uno::Reference< beans::XVetoableChangeListener >&  )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

SwMailMessage::SwMailMessage() :
        cppu::WeakComponentImplHelper1< mail::XMailMessage>(m_aMutex)
{
}

SwMailMessage::~SwMailMessage()
{
}

OUString SwMailMessage::getSenderName() throw (uno::RuntimeException)
{
    return m_sSenderName;
}

OUString SwMailMessage::getSenderAddress() throw (uno::RuntimeException)
{
    return m_sSenderAddress;
}

OUString SwMailMessage::getReplyToAddress() throw (uno::RuntimeException)
{
    return m_sReplyToAddress;
}

void SwMailMessage::setReplyToAddress( const OUString& _replytoaddress ) throw (uno::RuntimeException)
{
    m_sReplyToAddress = _replytoaddress;
}

OUString SwMailMessage::getSubject() throw (uno::RuntimeException)
{
    return m_sSubject;
}

void SwMailMessage::setSubject( const OUString& _subject ) throw (uno::RuntimeException)
{
    m_sSubject = _subject;
}

uno::Reference< datatransfer::XTransferable > SwMailMessage::getBody() throw (uno::RuntimeException)
{
    return m_xBody;
}

void SwMailMessage::setBody(
        const uno::Reference< datatransfer::XTransferable >& rBody )
                                                throw (uno::RuntimeException)
{
    m_xBody = rBody;
}

void  SwMailMessage::addRecipient( const OUString& rRecipientAddress )
        throw (uno::RuntimeException)
{
    m_aRecipients.realloc(m_aRecipients.getLength() + 1);
    m_aRecipients[m_aRecipients.getLength() - 1] = rRecipientAddress;
}

void  SwMailMessage::addCcRecipient( const OUString& rRecipientAddress )
        throw (uno::RuntimeException)
{
    m_aCcRecipients.realloc(m_aCcRecipients.getLength() + 1);
    m_aCcRecipients[m_aCcRecipients.getLength() - 1] = rRecipientAddress;

}

void  SwMailMessage::addBccRecipient( const OUString& rRecipientAddress ) throw (uno::RuntimeException)
{
    m_aBccRecipients.realloc(m_aBccRecipients.getLength() + 1);
    m_aBccRecipients[m_aBccRecipients.getLength() - 1] = rRecipientAddress;
}

uno::Sequence< OUString > SwMailMessage::getRecipients(  ) throw (uno::RuntimeException)
{
    return m_aRecipients;
}

uno::Sequence< OUString > SwMailMessage::getCcRecipients(  ) throw (uno::RuntimeException)
{
    return m_aCcRecipients;
}

uno::Sequence< OUString > SwMailMessage::getBccRecipients(  ) throw (uno::RuntimeException)
{
    return m_aBccRecipients;
}

void SwMailMessage::addAttachment( const mail::MailAttachment& rMailAttachment )
            throw (uno::RuntimeException)
{
    m_aAttachments.realloc(m_aAttachments.getLength() + 1);
    m_aAttachments[m_aAttachments.getLength() - 1] = rMailAttachment;
}

uno::Sequence< mail::MailAttachment > SwMailMessage::getAttachments(  )
                                            throw (uno::RuntimeException)
{
    return m_aAttachments;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
