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

#include <sal/log.hxx>

#include <infodlg.hxx>

#include <vcl/graphicfilter.hxx>
#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <fstream>

InfoDialog::InfoDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/infodialog.ui", "InfoDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pText(m_xBuilder->weld_text_view("lbText"))
    , m_pDontShowAgain(m_xBuilder->weld_check_button("cbShow"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pPrev(m_xBuilder->weld_button("btnPrev"))
{
    nCurrentTip=0;
    UpdateTip();
}

inline bool file_exists(const OUString& fileName)
{
    FILE  *f= fopen(OUStringToOString(fileName,RTL_TEXTENCODING_ASCII_US).getStr(), "r");
    if (f != nullptr)
    {
        fclose(f);
        return true;
    }
    return false;
}

void InfoDialog::UpdateTip()
{
    OUString aText;
    aText = "Editable PDFs? Yes, you can create Hybrid PDFs with LibreOffice "
            " "
            "https://wiki.documentfoundation.org/Documentation/HowTo/CreateAHybridPDF";
    m_pText->set_text(aText);

    m_pPrev->set_sensitive ( nCurrentTip>0 );

    // import the image
    OUString aPath = "$BRAND_BASE_DIR/" LIBO_SHARE_RESOURCE_FOLDER "/";
    rtl::Bootstrap::expandMacros( aPath );
    OUString aName = "tipoftheday_" + OUString::number(nCurrentTip)+".png";
    // use default image if none is available with the number
    if (!file_exists(aPath+aName)) aName = "tipoftheday.png";

    Graphic aGraphic;
    if ( GraphicFilter::LoadGraphic( aPath + aName, OUString(), aGraphic ) == ERRCODE_NONE )
    {
//        m_pImage->SetImage(Image(aGraphic.GetBitmapEx()));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
