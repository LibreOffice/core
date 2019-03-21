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
#include <vcl/virdev.hxx>
#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <fstream>
#include <officecfg/Office/Common.hxx>
#include <strings.hrc>
#include <unotools/resmgr.hxx>
//#include <unotools/configmgr.hxx>
#include <dialmgr.hxx>


InfoDialog::InfoDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/infodialog.ui", "InfoDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pText(m_xBuilder->weld_text_view("lbText"))
    , m_pShowTip(m_xBuilder->weld_check_button("cbShowTip"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pPrev(m_xBuilder->weld_button("btnPrev"))
{
    m_pShowTip->connect_toggled( LINK(this, InfoDialog, OnShowTipToggled) );
    m_pNext->connect_clicked( LINK( this, InfoDialog, OnNextClick) );
    m_pPrev->connect_clicked( LINK( this, InfoDialog, OnPrevClick) );

    nCurrentTip=0;
    nNumberOfTips=RID_SVX_MAX_TIPOFTHEDAY;
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
/*    char* pId = nullptr;
    pId = static_cast<char>("RID_SVXSTR_TIPOFTHEDAY_" + OUString::number(nCurrentTip));
*/  OUString aText;
    switch(nCurrentTip) {
      case 0: aText = CuiResId( RID_SVXSTR_TIPOFTHEDAY_0 ); break;
      case 1: aText = CuiResId( RID_SVXSTR_TIPOFTHEDAY_1 ); break;
      case 2: aText = CuiResId( RID_SVXSTR_TIPOFTHEDAY_2 ); break;
    }
    m_pText->set_text(aText);

    m_pPrev->set_sensitive ( nCurrentTip>0 );
    m_pNext->set_sensitive ( nCurrentTip<nNumberOfTips-1 );

    // import the image
    OUString aPath = "$BRAND_BASE_DIR/" LIBO_SHARE_RESOURCE_FOLDER "/";
    rtl::Bootstrap::expandMacros( aPath );
    OUString aName = "tipoftheday_" + OUString::number(nCurrentTip)+".png";
    // use default image if none is available with the number
    if (!file_exists(aPath+aName)) aName = "tipoftheday.png";

    Graphic aGraphic;
    if ( GraphicFilter::LoadGraphic( aPath + aName, OUString(), aGraphic ) == ERRCODE_NONE )
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev;
        m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0,0),aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK(InfoDialog, OnShowTipToggled, weld::ToggleButton&, rButton, void)
{
    std::shared_ptr< comphelper::ConfigurationChanges > xChanges(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::TipOfTheDay::set(rButton.get_active(), xChanges);
    xChanges->commit();
}

IMPL_LINK(InfoDialog, OnNextClick, weld::Button&, rButton, void )
{
    if (nCurrentTip<nNumberOfTips) {
      nCurrentTip++;
      UpdateTip();
    }
}
IMPL_LINK(InfoDialog, OnPrevClick, weld::Button&, rButton, void )
{
    if (nCurrentTip>0) {
      nCurrentTip--;
      UpdateTip();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
