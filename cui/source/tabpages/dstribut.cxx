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
#include <dialmgr.hxx>

/*************************************************************************
|*
|* Dialog
|*
\************************************************************************/

SvxDistributeDialog::SvxDistributeDialog(vcl::Window* pParent,
    const SfxItemSet& rInAttrs, SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
    : SfxSingleTabDialog(pParent, rInAttrs, "DistributionDialog",
        "cui/ui/distributiondialog.ui")
    , mpPage(nullptr)
{
    mpPage = VclPtr<SvxDistributePage>::Create(get_content_area(), rInAttrs, eHor, eVer);
    SetTabPage(mpPage);
}

SvxDistributeDialog::~SvxDistributeDialog()
{
    disposeOnce();
}

void SvxDistributeDialog::dispose()
{
    mpPage.clear();
    SfxSingleTabDialog::dispose();
}

/*************************************************************************
|*
|* Tabpage
|*
\************************************************************************/

SvxDistributePage::SvxDistributePage(vcl::Window* pWindow,
    const SfxItemSet& rInAttrs, SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
    : SvxTabPage(pWindow, "DistributionPage", "cui/ui/distributionpage.ui",
        rInAttrs)
    , m_eDistributeHor(eHor)
    , m_eDistributeVer(eVer)
{
    get(m_pBtnHorNone, "hornone");
    get(m_pBtnHorLeft, "horleft");
    get(m_pBtnHorCenter, "horcenter");
    get(m_pBtnHorDistance, "hordistance");
    get(m_pBtnHorRight, "horright");

    get(m_pBtnVerNone, "vernone");
    get(m_pBtnVerTop, "vertop");
    get(m_pBtnVerCenter, "vercenter");
    get(m_pBtnVerDistance, "verdistance");
    get(m_pBtnVerBottom, "verbottom");
}

SvxDistributePage::~SvxDistributePage()
{
    disposeOnce();
}

void SvxDistributePage::dispose()
{
    m_pBtnHorNone.clear();
    m_pBtnHorLeft.clear();
    m_pBtnHorCenter.clear();
    m_pBtnHorDistance.clear();
    m_pBtnHorRight.clear();
    m_pBtnVerNone.clear();
    m_pBtnVerTop.clear();
    m_pBtnVerCenter.clear();
    m_pBtnVerDistance.clear();
    m_pBtnVerBottom.clear();
    SvxTabPage::dispose();
}


void SvxDistributePage::PointChanged(vcl::Window* /*pWindow*/, RectPoint /*eRP*/)
{
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void SvxDistributePage::Reset(const SfxItemSet* )
{
    m_pBtnHorNone->SetState(false);
    m_pBtnHorLeft->SetState(false);
    m_pBtnHorCenter->SetState(false);
    m_pBtnHorDistance->SetState(false);
    m_pBtnHorRight->SetState(false);

    switch(m_eDistributeHor)
    {
        case SvxDistributeHorizontal::NONE : m_pBtnHorNone->SetState(true); break;
        case SvxDistributeHorizontal::Left : m_pBtnHorLeft->SetState(true); break;
        case SvxDistributeHorizontal::Center : m_pBtnHorCenter->SetState(true); break;
        case SvxDistributeHorizontal::Distance : m_pBtnHorDistance->SetState(true); break;
        case SvxDistributeHorizontal::Right    : m_pBtnHorRight->SetState(true);    break;
    }

    m_pBtnVerNone->SetState(false);
    m_pBtnVerTop->SetState(false);
    m_pBtnVerCenter->SetState(false);
    m_pBtnVerDistance->SetState(false);
    m_pBtnVerBottom->SetState(false);

    switch(m_eDistributeVer)
    {
        case SvxDistributeVertical::NONE : m_pBtnVerNone->SetState(true); break;
        case SvxDistributeVertical::Top : m_pBtnVerTop->SetState(true); break;
        case SvxDistributeVertical::Center : m_pBtnVerCenter->SetState(true); break;
        case SvxDistributeVertical::Distance : m_pBtnVerDistance->SetState(true); break;
        case SvxDistributeVertical::Bottom : m_pBtnVerBottom->SetState(true); break;
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

    if(m_pBtnHorLeft->IsChecked())
        eDistributeHor = SvxDistributeHorizontal::Left;
    else if(m_pBtnHorCenter->IsChecked())
        eDistributeHor = SvxDistributeHorizontal::Center;
    else if(m_pBtnHorDistance->IsChecked())
        eDistributeHor = SvxDistributeHorizontal::Distance;
    else if(m_pBtnHorRight->IsChecked())
        eDistributeHor = SvxDistributeHorizontal::Right;

    if(m_pBtnVerTop->IsChecked())
        eDistributeVer = SvxDistributeVertical::Top;
    else if(m_pBtnVerCenter->IsChecked())
        eDistributeVer = SvxDistributeVertical::Center;
    else if(m_pBtnVerDistance->IsChecked())
        eDistributeVer = SvxDistributeVertical::Distance;
    else if(m_pBtnVerBottom->IsChecked())
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
