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

#include <sfx2/basedlgs.hxx>
#include <dstribut.hxx>
#include <svx/dialogs.hrc>
#include <svx/svddef.hxx>

/*************************************************************************
|*
|* Dialog
|*
\************************************************************************/

SvxDistributeDialog::SvxDistributeDialog(weld::Window* pParent,
    const SfxItemSet& rInAttrs, SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
    : SfxSingleTabDialogController(pParent, &rInAttrs, "cui/ui/distributiondialog.ui",
                                   "DistributionDialog")
{
    TabPageParent pPageParent(get_content_area(), this);
    mpPage = VclPtr<SvxDistributePage>::Create(pPageParent, rInAttrs, eHor, eVer);
    SetTabPage(mpPage);
}

SvxDistributeDialog::~SvxDistributeDialog()
{
}

/*************************************************************************
|*
|* Tabpage
|*
\************************************************************************/

SvxDistributePage::SvxDistributePage(TabPageParent pWindow,
    const SfxItemSet& rInAttrs, SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
    : SfxTabPage(pWindow, "cui/ui/distributionpage.ui", "DistributionPage",
        &rInAttrs)
    , m_eDistributeHor(eHor)
    , m_eDistributeVer(eVer)
    , m_xBtnHorNone(m_xBuilder->weld_radio_button("hornone"))
    , m_xBtnHorLeft(m_xBuilder->weld_radio_button("horleft"))
    , m_xBtnHorCenter(m_xBuilder->weld_radio_button("horcenter"))
    , m_xBtnHorDistance(m_xBuilder->weld_radio_button("hordistance"))
    , m_xBtnHorRight(m_xBuilder->weld_radio_button("horright"))
    , m_xBtnVerNone(m_xBuilder->weld_radio_button("vernone"))
    , m_xBtnVerTop(m_xBuilder->weld_radio_button("vertop"))
    , m_xBtnVerCenter(m_xBuilder->weld_radio_button("vercenter"))
    , m_xBtnVerDistance(m_xBuilder->weld_radio_button("verdistance"))
    , m_xBtnVerBottom(m_xBuilder->weld_radio_button("verbottom"))
{
}

SvxDistributePage::~SvxDistributePage()
{
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void SvxDistributePage::Reset(const SfxItemSet* )
{
    m_xBtnHorNone->set_active(false);
    m_xBtnHorLeft->set_active(false);
    m_xBtnHorCenter->set_active(false);
    m_xBtnHorDistance->set_active(false);
    m_xBtnHorRight->set_active(false);

    switch(m_eDistributeHor)
    {
        case SvxDistributeHorizontal::NONE : m_xBtnHorNone->set_active(true); break;
        case SvxDistributeHorizontal::Left : m_xBtnHorLeft->set_active(true); break;
        case SvxDistributeHorizontal::Center : m_xBtnHorCenter->set_active(true); break;
        case SvxDistributeHorizontal::Distance : m_xBtnHorDistance->set_active(true); break;
        case SvxDistributeHorizontal::Right    : m_xBtnHorRight->set_active(true);    break;
    }

    m_xBtnVerNone->set_active(false);
    m_xBtnVerTop->set_active(false);
    m_xBtnVerCenter->set_active(false);
    m_xBtnVerDistance->set_active(false);
    m_xBtnVerBottom->set_active(false);

    switch(m_eDistributeVer)
    {
        case SvxDistributeVertical::NONE : m_xBtnVerNone->set_active(true); break;
        case SvxDistributeVertical::Top : m_xBtnVerTop->set_active(true); break;
        case SvxDistributeVertical::Center : m_xBtnVerCenter->set_active(true); break;
        case SvxDistributeVertical::Distance : m_xBtnVerDistance->set_active(true); break;
        case SvxDistributeVertical::Bottom : m_xBtnVerBottom->set_active(true); break;
    }
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

bool SvxDistributePage::FillItemSet( SfxItemSet* )
{
    SvxDistributeHorizontal eDistributeHor(SvxDistributeHorizontal::NONE);
    SvxDistributeVertical eDistributeVer(SvxDistributeVertical::NONE);

    if(m_xBtnHorLeft->get_active())
        eDistributeHor = SvxDistributeHorizontal::Left;
    else if(m_xBtnHorCenter->get_active())
        eDistributeHor = SvxDistributeHorizontal::Center;
    else if(m_xBtnHorDistance->get_active())
        eDistributeHor = SvxDistributeHorizontal::Distance;
    else if(m_xBtnHorRight->get_active())
        eDistributeHor = SvxDistributeHorizontal::Right;

    if(m_xBtnVerTop->get_active())
        eDistributeVer = SvxDistributeVertical::Top;
    else if(m_xBtnVerCenter->get_active())
        eDistributeVer = SvxDistributeVertical::Center;
    else if(m_xBtnVerDistance->get_active())
        eDistributeVer = SvxDistributeVertical::Distance;
    else if(m_xBtnVerBottom->get_active())
        eDistributeVer = SvxDistributeVertical::Bottom;

    if(eDistributeHor != m_eDistributeHor || eDistributeVer != m_eDistributeVer)
    {
        m_eDistributeHor = eDistributeHor;
        m_eDistributeVer = eDistributeVer;
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
