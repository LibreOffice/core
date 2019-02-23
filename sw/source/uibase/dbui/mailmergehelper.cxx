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
#include <mmconfigitem.hxx>
#include <docsh.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/mail/MailServiceProvider.hpp>
#include <com/sun/star/mail/XSmtpService.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <sfx2/passwd.hxx>

#include <dbui.hrc>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

namespace SwMailMergeHelper
{

OUString CallSaveAsDialog(weld::Window* pParent, OUString& rFilter)
{
    ::sfx2::FileDialogHelper aDialog( ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
                FileDialogFlags::NONE,
                SwDocShell::Factory().GetFactoryName(), SfxFilterFlags::NONE, SfxFilterFlags::NONE, pParent);

    if (aDialog.Execute()!=ERRCODE_NONE)
    {
        return OUString();
    }

    rFilter = aDialog.GetRealFilter();
    uno::Reference < ui::dialogs::XFilePicker3 > xFP = aDialog.GetFilePicker();
    return xFP->getSelectedFiles().getConstArray()[0];
}

/*
    simple address check: check for '@'
                            for at least one '.' after the '@'
                            and for at least two characters before and after the dot
*/
bool CheckMailAddress( const OUString& rMailAddress )
{
    const sal_Int32 nPosAt = rMailAddress.indexOf('@');
    if (nPosAt<0 || rMailAddress.lastIndexOf('@')!=nPosAt)
        return false;
    const sal_Int32 nPosDot = rMailAddress.indexOf('.', nPosAt);
    return !(nPosDot<0 || nPosDot-nPosAt<3 || rMailAddress.getLength()-nPosDot<3);
}

uno::Reference< mail::XSmtpService > ConnectToSmtpServer(
        SwMailMergeConfigItem const & rConfigItem,
        uno::Reference< mail::XMailService >&  rxInMailService,
        const OUString& rInMailServerPassword,
        const OUString& rOutMailServerPassword,
        weld::Window* pDialogParentWindow )
{
    uno::Reference< mail::XSmtpService > xSmtpServer;
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    try
    {
        uno::Reference< mail::XMailServiceProvider > xMailServiceProvider(
            mail::MailServiceProvider::create( xContext ) );
        xSmtpServer.set(xMailServiceProvider->create(mail::MailServiceType_SMTP), uno::UNO_QUERY);

        uno::Reference< mail::XConnectionListener> xConnectionListener(new SwConnectionListener());

        if(rConfigItem.IsAuthentication() && rConfigItem.IsSMTPAfterPOP())
        {
            uno::Reference< mail::XMailService > xInMailService =
                    xMailServiceProvider->create(
                    rConfigItem.IsInServerPOP() ?
                        mail::MailServiceType_POP3 : mail::MailServiceType_IMAP);
            //authenticate at the POP or IMAP server first
            OUString sPasswd = rConfigItem.GetInServerPassword();
            if(!rInMailServerPassword.isEmpty())
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
                        "Insecure");
            xInMailService->connect(xConnectionContext, xAuthenticator);
            rxInMailService = xInMailService;
        }
        uno::Reference< mail::XAuthenticator> xAuthenticator;
        if(rConfigItem.IsAuthentication() &&
                !rConfigItem.IsSMTPAfterPOP() &&
                !rConfigItem.GetMailUserName().isEmpty())
        {
            OUString sPasswd = rConfigItem.GetMailPassword();
            if(!rOutMailServerPassword.isEmpty())
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
        rxInMailService.set( xSmtpServer, uno::UNO_QUERY );
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL("exception caught");
    }
    return xSmtpServer;
}

} //namespace

struct  SwAddressPreview_Impl
{
    std::vector< OUString >      aAddresses;
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

SwAddressPreview::SwAddressPreview(vcl::Window* pParent, WinBits nStyle)
    : Window( pParent, nStyle )
    , aVScrollBar(VclPtr<ScrollBar>::Create(this, WB_VSCROLL))
    , pImpl(new SwAddressPreview_Impl())
{
    aVScrollBar->SetScrollHdl(LINK(this, SwAddressPreview, ScrollHdl));
    positionScrollBar();
    Show();
}

SwAddressPreview::~SwAddressPreview()
{
    disposeOnce();
}

void SwAddressPreview::dispose()
{
    aVScrollBar.disposeAndClear();
    vcl::Window::dispose();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(SwAddressPreview, WB_TABSTOP)

void SwAddressPreview::positionScrollBar()
{
    Size aSize(GetOutputSizePixel());
    Size aScrollSize(aVScrollBar->get_preferred_size().Width(), aSize.Height());
    aVScrollBar->SetSizePixel(aScrollSize);
    Point aSrollPos(aSize.Width() - aScrollSize.Width(), 0);
    aVScrollBar->SetPosPixel(aSrollPos);
}

void SwAddressPreview::Resize()
{
    Window::Resize();
    positionScrollBar();
}

IMPL_LINK_NOARG(SwAddressPreview, ScrollHdl, ScrollBar*, void)
{
    Invalidate();
}

void SwAddressPreview::AddAddress(const OUString& rAddress)
{
    pImpl->aAddresses.push_back(rAddress);
    UpdateScrollBar();
}

void SwAddressPreview::SetAddress(const OUString& rAddress)
{
    pImpl->aAddresses.clear();
    pImpl->aAddresses.push_back(rAddress);
    aVScrollBar->Show(false);
    Invalidate();
}

sal_uInt16   SwAddressPreview::GetSelectedAddress()const
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAddresses.size(), "selection invalid");
    return pImpl->nSelectedAddress;
}

void SwAddressPreview::SelectAddress(sal_uInt16 nSelect)
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAddresses.size(), "selection invalid");
    pImpl->nSelectedAddress = nSelect;
    // now make it visible..
    sal_uInt16 nSelectRow = nSelect / pImpl->nColumns;
    sal_uInt16 nStartRow = static_cast<sal_uInt16>(aVScrollBar->GetThumbPos());
    if( (nSelectRow < nStartRow) || (nSelectRow >= (nStartRow + pImpl->nRows) ))
        aVScrollBar->SetThumbPos( nSelectRow );
}

void SwAddressPreview::Clear()
{
    pImpl->aAddresses.clear();
    pImpl->nSelectedAddress = 0;
    UpdateScrollBar();
}

void SwAddressPreview::SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns)
{
    pImpl->nRows = nRows;
    pImpl->nColumns = nColumns;
    UpdateScrollBar();
}

void SwAddressPreview::UpdateScrollBar()
{
    if(pImpl->nColumns)
    {
        aVScrollBar->SetVisibleSize(pImpl->nRows);
        sal_uInt16 nResultingRows = static_cast<sal_uInt16>(pImpl->aAddresses.size() + pImpl->nColumns - 1) / pImpl->nColumns;
        ++nResultingRows;
        aVScrollBar->Show(pImpl->bEnableScrollBar && nResultingRows > pImpl->nRows);
        aVScrollBar->SetRange(Range(0, nResultingRows));
        if(aVScrollBar->GetThumbPos() > nResultingRows)
            aVScrollBar->SetThumbPos(nResultingRows);
    }
}

void SwAddressPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetFillColor(rSettings.GetWindowColor());
    rRenderContext.SetLineColor(COL_TRANSPARENT);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), GetOutputSizePixel()));
    Color aPaintColor(IsEnabled() ? rSettings.GetWindowTextColor() : rSettings.GetDisableColor());
    rRenderContext.SetLineColor(aPaintColor);
    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetColor(aPaintColor);
    rRenderContext.SetFont(aFont);

    Size aSize(GetOutputSizePixel());
    sal_uInt16 nStartRow = 0;
    if(aVScrollBar->IsVisible())
    {
        aSize.AdjustWidth( -(aVScrollBar->GetSizePixel().Width()) );
        nStartRow = static_cast<sal_uInt16>(aVScrollBar->GetThumbPos());
    }
    Size aPartSize(aSize.Width() / pImpl->nColumns,
                   aSize.Height() / pImpl->nRows);
    aPartSize.AdjustWidth( -2 );
    aPartSize.AdjustHeight( -2 );

    sal_uInt16 nAddress = nStartRow * pImpl->nColumns;
    const sal_uInt16 nNumAddresses = static_cast<sal_uInt16>(pImpl->aAddresses.size());
    for (sal_uInt16 nRow = 0; nRow < pImpl->nRows ; ++nRow)
    {
        for (sal_uInt16 nCol = 0; nCol < pImpl->nColumns; ++nCol)
        {
            if (nAddress >= nNumAddresses)
                break;
            Point aPos(nCol * aPartSize.Width(),
                       nRow * aPartSize.Height());
            aPos.Move(1, 1);
            bool bIsSelected = nAddress == pImpl->nSelectedAddress;
            if ((pImpl->nColumns * pImpl->nRows) == 1)
                bIsSelected = false;
            OUString adr(pImpl->aAddresses[nAddress]);
            DrawText_Impl(rRenderContext, adr, aPos, aPartSize, bIsSelected);
            ++nAddress;
        }
    }
    rRenderContext.SetClipRegion();
}

void SwAddressPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    Window::MouseButtonDown(rMEvt);
    if (rMEvt.IsLeft() && pImpl->nRows && pImpl->nColumns)
    {
        //determine the selected address
        const Point& rMousePos = rMEvt.GetPosPixel();
        Size aSize(GetOutputSizePixel());
        Size aPartSize( aSize.Width()/pImpl->nColumns, aSize.Height()/pImpl->nRows );
        sal_uInt32 nRow = rMousePos.Y() / aPartSize.Height() ;
        if(aVScrollBar->IsVisible())
        {
            nRow += static_cast<sal_uInt16>(aVScrollBar->GetThumbPos());
        }
        sal_uInt32 nCol = rMousePos.X() / aPartSize.Width();
        sal_uInt32 nSelect = nRow * pImpl->nColumns + nCol;

        if( nSelect < pImpl->aAddresses.size() &&
                pImpl->nSelectedAddress != static_cast<sal_uInt16>(nSelect))
        {
            pImpl->nSelectedAddress = static_cast<sal_uInt16>(nSelect);
            m_aSelectHdl.Call(nullptr);
        }
        Invalidate();
    }
}

void  SwAddressPreview::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    bool bHandled = false;
    if (pImpl->nRows && pImpl->nColumns)
    {
        sal_uInt32 nSelectedRow = pImpl->nSelectedAddress / pImpl->nColumns;
        sal_uInt32 nSelectedColumn = pImpl->nSelectedAddress - (nSelectedRow * pImpl->nColumns);
        switch(nKey)
        {
            case KEY_UP:
                if(nSelectedRow)
                    --nSelectedRow;
                bHandled = true;
            break;
            case KEY_DOWN:
                if(pImpl->aAddresses.size() > sal_uInt32(pImpl->nSelectedAddress + pImpl->nColumns))
                    ++nSelectedRow;
                bHandled = true;
            break;
            case KEY_LEFT:
                if(nSelectedColumn)
                    --nSelectedColumn;
                bHandled = true;
            break;
            case KEY_RIGHT:
                if(nSelectedColumn < sal_uInt32(pImpl->nColumns - 1) &&
                       pImpl->aAddresses.size() - 1 > pImpl->nSelectedAddress )
                    ++nSelectedColumn;
                bHandled = true;
            break;
        }
        sal_uInt32 nSelect = nSelectedRow * pImpl->nColumns + nSelectedColumn;
        if( nSelect < pImpl->aAddresses.size() &&
                pImpl->nSelectedAddress != static_cast<sal_uInt16>(nSelect))
        {
            pImpl->nSelectedAddress = static_cast<sal_uInt16>(nSelect);
            m_aSelectHdl.Call(nullptr);
            Invalidate();
        }
    }
    if (!bHandled)
        Window::KeyInput(rKEvt);
}

void SwAddressPreview::StateChanged( StateChangedType nStateChange )
{
    if (nStateChange == StateChangedType::Enable)
        Invalidate();
    Window::StateChanged(nStateChange);
}

void SwAddressPreview::DrawText_Impl(vcl::RenderContext& rRenderContext, const OUString& rAddress,
                                     const Point& rTopLeft, const Size& rSize, bool bIsSelected)
{
    rRenderContext.SetClipRegion(vcl::Region(tools::Rectangle(rTopLeft, rSize)));
    if (bIsSelected)
    {
        //selection rectangle
        rRenderContext.SetFillColor(COL_TRANSPARENT);
        rRenderContext.DrawRect(tools::Rectangle(rTopLeft, rSize));
    }
    sal_Int32 nHeight = GetTextHeight();
    Point aStart = rTopLeft;
    //put it away from the border
    aStart.Move(2, 2);
    sal_Int32 nPos = 0;
    do
    {
        rRenderContext.DrawText(aStart, rAddress.getToken(0, '\n', nPos));
        aStart.AdjustY(nHeight );
    }
    while (nPos >= 0);
}

OUString SwAddressPreview::FillData(
        const OUString& rAddress,
        SwMailMergeConfigItem const & rConfigItem,
        const Sequence< OUString>* pAssignments)
{
    //find the column names in the address string (with name assignment!) and
    //exchange the placeholder (like <Firstname>) with the database content
    //unassigned columns are expanded to <not assigned>
    Reference< XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), UNO_QUERY);
    Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    Sequence< OUString> aAssignment = pAssignments ?
                    *pAssignments :
                    rConfigItem.GetColumnAssignment(
                                                rConfigItem.GetCurrentDBData() );
    const OUString* pAssignment = aAssignment.getConstArray();
    const std::vector<std::pair<OUString, int>>& rDefHeaders = rConfigItem.GetDefaultAddressHeaders();
    OUString sNotAssigned = "<" + SwResId(STR_NOTASSIGNED) + ">";

    bool bIncludeCountry = rConfigItem.IsIncludeCountry();
    const OUString rExcludeCountry = rConfigItem.GetExcludeCountry();
    bool bSpecialReplacementForCountry = (!bIncludeCountry || !rExcludeCountry.isEmpty());
    OUString sCountryColumn;
    if( bSpecialReplacementForCountry )
    {
        sCountryColumn = rDefHeaders[MM_PART_COUNTRY].first;
        Sequence< OUString> aSpecialAssignment =
                        rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
        if(aSpecialAssignment.getLength() > MM_PART_COUNTRY && aSpecialAssignment[MM_PART_COUNTRY].getLength())
            sCountryColumn = aSpecialAssignment[MM_PART_COUNTRY];
    }

    SwAddressIterator aIter(rAddress);
    OUStringBuffer sAddress;
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            //get the default column name

            //find the appropriate assignment
            OUString sConvertedColumn = aItem.sText;
            for(sal_uInt32 nColumn = 0;
                    nColumn < rDefHeaders.size() && nColumn < sal_uInt32(aAssignment.getLength());
                                                                                ++nColumn)
            {
                if (rDefHeaders[nColumn].first == aItem.sText &&
                    !pAssignment[nColumn].isEmpty())
                {
                    sConvertedColumn = pAssignment[nColumn];
                    break;
                }
            }
            if(!sConvertedColumn.isEmpty() &&
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
                                aItem.sText.clear();
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
        sAddress.append(aItem.sText);
    }
    return sAddress.makeStringAndClear();
}

AddressPreview::AddressPreview(std::unique_ptr<weld::ScrolledWindow> xWindow)
    : pImpl(new SwAddressPreview_Impl())
    , m_xVScrollBar(std::move(xWindow))
{
    m_xVScrollBar->set_user_managed_scrolling();
    m_xVScrollBar->connect_vadjustment_changed(LINK(this, AddressPreview, ScrollHdl));
}

AddressPreview::~AddressPreview()
{
}

IMPL_LINK_NOARG(AddressPreview, ScrollHdl, weld::ScrolledWindow&, void)
{
    Invalidate();
}

void AddressPreview::AddAddress(const OUString& rAddress)
{
    pImpl->aAddresses.push_back(rAddress);
    UpdateScrollBar();
}

void AddressPreview::SetAddress(const OUString& rAddress)
{
    pImpl->aAddresses.clear();
    pImpl->aAddresses.push_back(rAddress);
    m_xVScrollBar->set_vpolicy(VclPolicyType::NEVER);
    Invalidate();
}

sal_uInt16 AddressPreview::GetSelectedAddress()const
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAddresses.size(), "selection invalid");
    return pImpl->nSelectedAddress;
}

void AddressPreview::SelectAddress(sal_uInt16 nSelect)
{
    OSL_ENSURE(pImpl->nSelectedAddress < pImpl->aAddresses.size(), "selection invalid");
    pImpl->nSelectedAddress = nSelect;
    // now make it visible..
    sal_uInt16 nSelectRow = nSelect / pImpl->nColumns;
    sal_uInt16 nStartRow = m_xVScrollBar->vadjustment_get_value();
    if( (nSelectRow < nStartRow) || (nSelectRow >= (nStartRow + pImpl->nRows) ))
        m_xVScrollBar->vadjustment_set_value(nSelectRow);
}

void AddressPreview::ReplaceSelectedAddress(const OUString& rNew)
{
    pImpl->aAddresses[pImpl->nSelectedAddress] = rNew;
    Invalidate();
}

void AddressPreview::RemoveSelectedAddress()
{
    pImpl->aAddresses.erase(pImpl->aAddresses.begin() + pImpl->nSelectedAddress);
    if(pImpl->nSelectedAddress)
        --pImpl->nSelectedAddress;
    UpdateScrollBar();
    Invalidate();
}

void AddressPreview::SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns)
{
    pImpl->nRows = nRows;
    pImpl->nColumns = nColumns;
    UpdateScrollBar();
}

void AddressPreview::EnableScrollBar()
{
    pImpl->bEnableScrollBar = true;
}

void AddressPreview::UpdateScrollBar()
{
    if (pImpl->nColumns)
    {
        sal_uInt16 nResultingRows = static_cast<sal_uInt16>(pImpl->aAddresses.size() + pImpl->nColumns - 1) / pImpl->nColumns;
        ++nResultingRows;
        auto nValue = m_xVScrollBar->vadjustment_get_value();
        if (nValue > nResultingRows)
            nValue = nResultingRows;
        m_xVScrollBar->set_vpolicy(pImpl->bEnableScrollBar && nResultingRows > pImpl->nRows ? VclPolicyType::ALWAYS : VclPolicyType::NEVER);
        m_xVScrollBar->vadjustment_configure(nValue, 0, nResultingRows, 1, 10, pImpl->nRows);
    }
}

void AddressPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetFillColor(rSettings.GetWindowColor());
    rRenderContext.SetLineColor(COL_TRANSPARENT);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), GetOutputSizePixel()));
    Color aPaintColor(IsEnabled() ? rSettings.GetWindowTextColor() : rSettings.GetDisableColor());
    rRenderContext.SetLineColor(aPaintColor);
    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetColor(aPaintColor);
    rRenderContext.SetFont(aFont);

    Size aSize(GetOutputSizePixel());
    sal_uInt16 nStartRow = 0;
    if (m_xVScrollBar->get_vpolicy() != VclPolicyType::NEVER)
    {
        aSize.AdjustWidth(-m_xVScrollBar->get_vscroll_width());
        nStartRow = m_xVScrollBar->vadjustment_get_value();
    }
    Size aPartSize(aSize.Width() / pImpl->nColumns,
                   aSize.Height() / pImpl->nRows);
    aPartSize.AdjustWidth( -2 );
    aPartSize.AdjustHeight( -2 );

    sal_uInt16 nAddress = nStartRow * pImpl->nColumns;
    const sal_uInt16 nNumAddresses = static_cast<sal_uInt16>(pImpl->aAddresses.size());
    for (sal_uInt16 nRow = 0; nRow < pImpl->nRows ; ++nRow)
    {
        for (sal_uInt16 nCol = 0; nCol < pImpl->nColumns; ++nCol)
        {
            if (nAddress >= nNumAddresses)
                break;
            Point aPos(nCol * aPartSize.Width(),
                       nRow * aPartSize.Height());
            aPos.Move(1, 1);
            bool bIsSelected = nAddress == pImpl->nSelectedAddress;
            if ((pImpl->nColumns * pImpl->nRows) == 1)
                bIsSelected = false;
            OUString adr(pImpl->aAddresses[nAddress]);
            DrawText_Impl(rRenderContext, adr, aPos, aPartSize, bIsSelected);
            ++nAddress;
        }
    }
    rRenderContext.SetClipRegion();
}

bool AddressPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (rMEvt.IsLeft() && pImpl->nRows && pImpl->nColumns)
    {
        //determine the selected address
        const Point& rMousePos = rMEvt.GetPosPixel();
        Size aSize(GetOutputSizePixel());
        Size aPartSize( aSize.Width()/pImpl->nColumns, aSize.Height()/pImpl->nRows );
        sal_uInt32 nRow = rMousePos.Y() / aPartSize.Height() ;
        if (m_xVScrollBar->get_vpolicy() != VclPolicyType::NEVER)
        {
            nRow += m_xVScrollBar->vadjustment_get_value();
        }
        sal_uInt32 nCol = rMousePos.X() / aPartSize.Width();
        sal_uInt32 nSelect = nRow * pImpl->nColumns + nCol;

        if( nSelect < pImpl->aAddresses.size() &&
                pImpl->nSelectedAddress != static_cast<sal_uInt16>(nSelect))
        {
            pImpl->nSelectedAddress = static_cast<sal_uInt16>(nSelect);
        }
        Invalidate();
    }
    return true;
}

bool AddressPreview::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    bool bHandled = false;
    if (pImpl->nRows && pImpl->nColumns)
    {
        sal_uInt32 nSelectedRow = pImpl->nSelectedAddress / pImpl->nColumns;
        sal_uInt32 nSelectedColumn = pImpl->nSelectedAddress - (nSelectedRow * pImpl->nColumns);
        switch(nKey)
        {
            case KEY_UP:
                if(nSelectedRow)
                    --nSelectedRow;
                bHandled = true;
            break;
            case KEY_DOWN:
                if(pImpl->aAddresses.size() > sal_uInt32(pImpl->nSelectedAddress + pImpl->nColumns))
                    ++nSelectedRow;
                bHandled = true;
            break;
            case KEY_LEFT:
                if(nSelectedColumn)
                    --nSelectedColumn;
                bHandled = true;
            break;
            case KEY_RIGHT:
                if(nSelectedColumn < sal_uInt32(pImpl->nColumns - 1) &&
                       pImpl->aAddresses.size() - 1 > pImpl->nSelectedAddress )
                    ++nSelectedColumn;
                bHandled = true;
            break;
        }
        sal_uInt32 nSelect = nSelectedRow * pImpl->nColumns + nSelectedColumn;
        if( nSelect < pImpl->aAddresses.size() &&
                pImpl->nSelectedAddress != static_cast<sal_uInt16>(nSelect))
        {
            pImpl->nSelectedAddress = static_cast<sal_uInt16>(nSelect);
            Invalidate();
        }
    }
    return bHandled;
}

void AddressPreview::DrawText_Impl(vcl::RenderContext& rRenderContext, const OUString& rAddress,
                                     const Point& rTopLeft, const Size& rSize, bool bIsSelected)
{
    rRenderContext.SetClipRegion(vcl::Region(tools::Rectangle(rTopLeft, rSize)));
    if (bIsSelected)
    {
        //selection rectangle
        rRenderContext.SetFillColor(COL_TRANSPARENT);
        rRenderContext.DrawRect(tools::Rectangle(rTopLeft, rSize));
    }
    sal_Int32 nHeight = GetTextHeight();
    Point aStart = rTopLeft;
    //put it away from the border
    aStart.Move(2, 2);
    sal_Int32 nPos = 0;
    do
    {
        rRenderContext.DrawText(aStart, rAddress.getToken(0, '\n', nPos));
        aStart.AdjustY(nHeight );
    }
    while (nPos >= 0);
}

SwMergeAddressItem   SwAddressIterator::Next()
{
    //currently the string may either start with a '<' then it's a column
    //otherwise it's simple text maybe containing a return
    SwMergeAddressItem   aRet;
    if(!sAddress.isEmpty())
    {
        if(sAddress[0] == '<')
        {
            aRet.bIsColumn = true;
            sal_Int32 nClose = sAddress.indexOf('>');
            OSL_ENSURE(nClose != -1, "closing '>' not found");
            if( nClose != -1 )
            {
                aRet.sText = sAddress.copy(1, nClose - 1);
                sAddress = sAddress.copy(nClose + 1);
            }
            else
            {
                aRet.sText = sAddress.copy(1, 1);
                sAddress = sAddress.copy(1);
            }
        }
        else
        {
            sal_Int32 nOpen = sAddress.indexOf('<');
            sal_Int32 nReturn = sAddress.indexOf('\n');
            if(nReturn == 0)
            {
                aRet.bIsReturn = true;
                aRet.sText = "\n";
                sAddress = sAddress.copy(1);
            }
            else if(-1 == nOpen && -1 == nReturn)
            {
                aRet.sText = sAddress;
                sAddress.clear();
            }
            else
            {
                if (nOpen == -1)
                    nOpen = sAddress.getLength();
                if (nReturn == -1)
                    nReturn = sAddress.getLength();
                sal_Int32 nTarget = std::min(nOpen, nReturn);
                aRet.sText = sAddress.copy(0, nTarget);
                sAddress = sAddress.copy(nTarget);
            }
        }
    }
    return aRet;

}

SwAuthenticator::~SwAuthenticator()
{
}

OUString SwAuthenticator::getUserName( )
{
    return m_aUserName;
}

OUString SwAuthenticator::getPassword(  )
{
    if(!m_aUserName.isEmpty() && m_aPassword.isEmpty() && m_pParentWindow)
    {
       SfxPasswordDialog aPasswdDlg(m_pParentWindow);
       aPasswdDlg.SetMinLen(0);
       if (RET_OK == aPasswdDlg.run())
            m_aPassword = aPasswdDlg.GetPassword();
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
{
    uno::Any aRet;
    if( rName == "ServerName" )
        aRet <<= m_sMailServer;
    else if( rName == "Port" )
        aRet <<= static_cast<sal_Int32>(m_nPort);
    else if( rName == "ConnectionType" )
        aRet <<= m_sConnectionType;
    return aRet;
}

SwConnectionListener::~SwConnectionListener()
{
}

void SwConnectionListener::connected(const lang::EventObject& /*aEvent*/)
{
}

void SwConnectionListener::disconnected(const lang::EventObject& /*aEvent*/)
{
}

void SwConnectionListener::disposing(const lang::EventObject& /*aEvent*/)
{
}

SwMailTransferable::SwMailTransferable(const OUString& rBody, const OUString& rMimeType) :
    cppu::WeakComponentImplHelper< datatransfer::XTransferable, beans::XPropertySet >(m_aMutex),
    m_aMimeType( rMimeType ),
    m_sBody( rBody ),
    m_bIsBody( true )
{
}

SwMailTransferable::SwMailTransferable(const OUString& rURL,
                const OUString& rName, const OUString& rMimeType) :
    cppu::WeakComponentImplHelper< datatransfer::XTransferable, beans::XPropertySet >(m_aMutex),
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
{
    uno::Any aRet;
    if( m_bIsBody )
        aRet <<= m_sBody;
    else
    {
        Sequence<sal_Int8> aData;
        SfxMedium aMedium( m_aURL, StreamMode::STD_READ );
        SvStream* pStream = aMedium.GetInStream();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE && pStream)
        {
            aData.realloc(pStream->TellEnd());
            pStream->Seek(0);
            sal_Int8 * pData = aData.getArray();
            pStream->ReadBytes( pData, aData.getLength() );
        }
        aRet <<= aData;
    }
    return aRet;
}

uno::Sequence< datatransfer::DataFlavor > SwMailTransferable::getTransferDataFlavors(  )
{
    uno::Sequence< datatransfer::DataFlavor > aRet(1);
    aRet[0].MimeType = m_aMimeType;
    if( m_bIsBody )
    {
        aRet[0].DataType = cppu::UnoType<OUString>::get();
    }
    else
    {
        aRet[0].HumanPresentableName = m_aName;
        aRet[0].DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
    }
    return aRet;
}

sal_Bool SwMailTransferable::isDataFlavorSupported(
            const datatransfer::DataFlavor& aFlavor )
{
    return (aFlavor.MimeType == m_aMimeType);
}

uno::Reference< beans::XPropertySetInfo > SwMailTransferable::getPropertySetInfo(  )
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void SwMailTransferable::setPropertyValue( const OUString& , const uno::Any& )
{
}

uno::Any SwMailTransferable::getPropertyValue( const OUString& rPropertyName )
{
    uno::Any aRet;
    if ( rPropertyName == "URL" )
        aRet <<= m_aURL;
    return aRet;
}

void SwMailTransferable::addPropertyChangeListener(
    const OUString&, const uno::Reference< beans::XPropertyChangeListener >&  )
{
}

void SwMailTransferable::removePropertyChangeListener(
    const OUString&,
    const uno::Reference< beans::XPropertyChangeListener >& )
{
}

void SwMailTransferable::addVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
{
}

void SwMailTransferable::removeVetoableChangeListener(
    const OUString& ,
    const uno::Reference< beans::XVetoableChangeListener >&  )
{
}

SwMailMessage::SwMailMessage() :
        cppu::WeakComponentImplHelper< mail::XMailMessage>(m_aMutex)
{
}

SwMailMessage::~SwMailMessage()
{
}

OUString SwMailMessage::getSenderName()
{
    return m_sSenderName;
}

OUString SwMailMessage::getSenderAddress()
{
    return m_sSenderAddress;
}

OUString SwMailMessage::getReplyToAddress()
{
    return m_sReplyToAddress;
}

void SwMailMessage::setReplyToAddress( const OUString& _replytoaddress )
{
    m_sReplyToAddress = _replytoaddress;
}

OUString SwMailMessage::getSubject()
{
    return m_sSubject;
}

void SwMailMessage::setSubject( const OUString& _subject )
{
    m_sSubject = _subject;
}

uno::Reference< datatransfer::XTransferable > SwMailMessage::getBody()
{
    return m_xBody;
}

void SwMailMessage::setBody(
        const uno::Reference< datatransfer::XTransferable >& rBody )
{
    m_xBody = rBody;
}

void  SwMailMessage::addRecipient( const OUString& rRecipientAddress )
{
    m_aRecipients.realloc(m_aRecipients.getLength() + 1);
    m_aRecipients[m_aRecipients.getLength() - 1] = rRecipientAddress;
}

void  SwMailMessage::addCcRecipient( const OUString& rRecipientAddress )
{
    m_aCcRecipients.realloc(m_aCcRecipients.getLength() + 1);
    m_aCcRecipients[m_aCcRecipients.getLength() - 1] = rRecipientAddress;

}

void  SwMailMessage::addBccRecipient( const OUString& rRecipientAddress )
{
    m_aBccRecipients.realloc(m_aBccRecipients.getLength() + 1);
    m_aBccRecipients[m_aBccRecipients.getLength() - 1] = rRecipientAddress;
}

uno::Sequence< OUString > SwMailMessage::getRecipients(  )
{
    return m_aRecipients;
}

uno::Sequence< OUString > SwMailMessage::getCcRecipients(  )
{
    return m_aCcRecipients;
}

uno::Sequence< OUString > SwMailMessage::getBccRecipients(  )
{
    return m_aBccRecipients;
}

void SwMailMessage::addAttachment( const mail::MailAttachment& rMailAttachment )
{
    m_aAttachments.realloc(m_aAttachments.getLength() + 1);
    m_aAttachments[m_aAttachments.getLength() - 1] = rMailAttachment;
}

uno::Sequence< mail::MailAttachment > SwMailMessage::getAttachments(  )
{
    return m_aAttachments;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
