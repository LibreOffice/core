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

#include <vcl/stdtext.hxx>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>
#include <bitmaps.hlst>
#include <strings.hrc>
#include <svdata.hxx>

void ShowServiceNotAvailableError(weld::Widget* pParent,
    std::u16string_view rServiceName, bool bError)
{
    OUString aText = VclResId(SV_STDTEXT_SERVICENOTAVAILABLE).replaceAll("%s", rServiceName);
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
        bError ? VclMessageType::Error : VclMessageType::Warning, VclButtonsType::Ok, aText));
    xBox->run();
}

static void ImplInitMsgBoxImageList()
{
    ImplSVData* pSVData = ImplGetSVData();
    std::vector<Image> &rImages = pSVData->mpWinData->maMsgBoxImgList;
    if (rImages.empty())
    {
        rImages.emplace_back(StockImage::Yes, SV_RESID_BITMAP_ERRORBOX);
        rImages.emplace_back(StockImage::Yes, SV_RESID_BITMAP_QUERYBOX);
        rImages.emplace_back(StockImage::Yes, SV_RESID_BITMAP_WARNINGBOX);
        rImages.emplace_back(StockImage::Yes, SV_RESID_BITMAP_INFOBOX);
    }
}

Image const & GetStandardInfoBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[3];
}

OUString GetStandardInfoBoxText()
{
    return VclResId(SV_MSGBOX_INFO);
}

Image const & GetStandardWarningBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[2];
}

OUString GetStandardWarningBoxText()
{
    return VclResId(SV_MSGBOX_WARNING);
}

Image const & GetStandardErrorBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[0];
}

OUString GetStandardErrorBoxText()
{
    return VclResId(SV_MSGBOX_ERROR);
}

Image const & GetStandardQueryBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[1];
}

OUString GetStandardQueryBoxText()
{
    return VclResId(SV_MSGBOX_QUERY);
}

OUString GetStandardText(StandardButtonType eButton)
{
    static const char* aResIdAry[static_cast<int>(StandardButtonType::Count)] =
    {
        // http://lists.freedesktop.org/archives/libreoffice/2013-January/044513.html
        // Under windows we don't want accelerators on ok/cancel but do on other
        // buttons
#ifdef _WIN32
        SV_BUTTONTEXT_OK_NOMNEMONIC,
        SV_BUTTONTEXT_CANCEL_NOMNEMONIC,
#else
        SV_BUTTONTEXT_OK,
        SV_BUTTONTEXT_CANCEL,
#endif
        SV_BUTTONTEXT_YES,
        SV_BUTTONTEXT_NO,
        SV_BUTTONTEXT_RETRY,
        SV_BUTTONTEXT_HELP,
        SV_BUTTONTEXT_CLOSE,
        SV_BUTTONTEXT_MORE,
        SV_BUTTONTEXT_IGNORE,
        SV_BUTTONTEXT_ABORT,
        SV_BUTTONTEXT_LESS,
        STR_WIZDLG_PREVIOUS,
        STR_WIZDLG_NEXT,
        STR_WIZDLG_FINISH,
    };

    return VclResId(aResIdAry[static_cast<sal_uInt16>(eButton)]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
