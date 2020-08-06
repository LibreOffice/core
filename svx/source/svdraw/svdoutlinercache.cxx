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

#include <svdoutlinercache.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdetc.hxx>

SdrOutlinerCache::SdrOutlinerCache( SdrModel* pModel )
:   mpModel( pModel ),
    maModeOutline(),
    maModeText(),
    maActiveOutliners()
{
}

std::unique_ptr<SdrOutliner> SdrOutlinerCache::createOutliner( OutlinerMode nOutlinerMode )
{
    std::unique_ptr<SdrOutliner> pOutliner;

    if( (OutlinerMode::OutlineObject == nOutlinerMode) && !maModeOutline.empty() )
    {
        pOutliner = std::move(maModeOutline.back());
        maModeOutline.pop_back();
    }
    else if( (OutlinerMode::TextObject == nOutlinerMode) && !maModeText.empty() )
    {
        pOutliner = std::move(maModeText.back());
        maModeText.pop_back();
    }
    else
    {
        pOutliner = SdrMakeOutliner(nOutlinerMode, *mpModel);
        Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
        pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
        maActiveOutliners.insert(pOutliner.get());
    }

    return pOutliner;
}

SdrOutlinerCache::~SdrOutlinerCache()
{
}

void SdrOutlinerCache::disposeOutliner( std::unique_ptr<SdrOutliner> pOutliner )
{
    if( !pOutliner )
        return;

    OutlinerMode nOutlMode = pOutliner->GetOutlinerMode();

    if( OutlinerMode::OutlineObject == nOutlMode )
    {
        pOutliner->Clear();
        pOutliner->SetVertical( false );

        // Deregister on outliner, might be reused from outliner cache
        pOutliner->SetNotifyHdl( Link<EENotify&,void>() );
        maModeOutline.emplace_back(std::move(pOutliner));
    }
    else if( OutlinerMode::TextObject == nOutlMode )
    {
        pOutliner->Clear();
        pOutliner->SetVertical( false );

        // Deregister on outliner, might be reused from outliner cache
        pOutliner->SetNotifyHdl( Link<EENotify&,void>() );
        maModeText.emplace_back(std::move(pOutliner));
    }
    else
    {
        maActiveOutliners.erase(pOutliner.get());
    }
}

std::vector< SdrOutliner* > SdrOutlinerCache::GetActiveOutliners() const
{
    return std::vector< SdrOutliner* >(maActiveOutliners.begin(), maActiveOutliners.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
