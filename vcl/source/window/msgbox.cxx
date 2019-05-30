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

#include <strings.hrc>
#include <bitmaps.hlst>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

#include <vcl/textrectinfo.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>

static void ImplInitMsgBoxImageList()
{
    ImplSVData* pSVData = ImplGetSVData();
    std::vector<Image>& rImages = pSVData->mpWinData->maMsgBoxImgList;
    if (rImages.empty())
    {
        rImages.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_ERRORBOX));
        rImages.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_QUERYBOX));
        rImages.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_WARNINGBOX));
        rImages.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_INFOBOX));
    }
}

Image const& GetStandardInfoBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[3];
}

OUString GetStandardInfoBoxText() { return VclResId(SV_MSGBOX_INFO); }

Image const& GetStandardWarningBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[2];
}

OUString GetStandardWarningBoxText() { return VclResId(SV_MSGBOX_WARNING); }

Image const& GetStandardErrorBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[0];
}

OUString GetStandardErrorBoxText() { return VclResId(SV_MSGBOX_ERROR); }

Image const& GetStandardQueryBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->mpWinData->maMsgBoxImgList[1];
}

OUString GetStandardQueryBoxText() { return VclResId(SV_MSGBOX_QUERY); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
