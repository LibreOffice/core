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

#include <vcl/builderfactory.hxx>

#include <map>
#include <set>

#include "strings.hrc"
#include "dlgctrls.hxx"
#include "sdresid.hxx"
#include "fadedef.h"
#include "sdpage.hxx"

using namespace ::sd;

struct FadeEffectLBImpl
{
    // The set id of each entry
    std::vector< OUString > maSet;

    // How many variants each transition set has
    std::map< OUString, int > maNumVariants;

    std::vector< TransitionPresetPtr > maPresets;
};

FadeEffectLB::FadeEffectLB(vcl::Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
    , mpImpl(new FadeEffectLBImpl)
{
}

FadeEffectLB::~FadeEffectLB()
{
    disposeOnce();
}

void FadeEffectLB::dispose()
{
    delete mpImpl;
    ListBox::dispose();
}

void FadeEffectLB::Fill()
{
    InsertEntry( SD_RESSTR( STR_EFFECT_NONE ) );
    mpImpl->maPresets.push_back( TransitionPresetPtr() );
    mpImpl->maSet.push_back( "" );

    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();

    for( auto aIter = rPresetList.begin(); aIter != rPresetList.end(); ++aIter )
    {
        TransitionPresetPtr pPreset = *aIter;
        const OUString sLabel( pPreset->getSetLabel() );
        if( !sLabel.isEmpty() )
        {
            if( mpImpl->maNumVariants.find( pPreset->getSetId() ) == mpImpl->maNumVariants.end() )
            {
                InsertEntry( sLabel );
                mpImpl->maSet.push_back( pPreset->getSetId() );
                mpImpl->maNumVariants[pPreset->getSetId()] = 1;
            }
            else
            {
                mpImpl->maNumVariants[pPreset->getSetId()]++;
            }
            mpImpl->maPresets.push_back( pPreset );
        }
    }

    assert( static_cast<size_t>( GetEntryCount() ) == mpImpl->maSet.size() );
    assert( mpImpl->maPresets.size() == 1 + TransitionPreset::getTransitionPresetList().size() );

    SelectEntryPos(0);
}

void FadeEffectLB::FillVariantLB(ListBox& rVariantLB)
{
    rVariantLB.Clear();
    for( auto aIter = mpImpl->maPresets.begin(); aIter != mpImpl->maPresets.end(); ++aIter )
    {
        TransitionPresetPtr pPreset = *aIter;
        if( !pPreset )
            continue;
        const OUString sLabel( pPreset->getSetLabel() );
        if( !sLabel.isEmpty() && mpImpl->maSet[GetSelectEntryPos()].equals( pPreset->getSetId() ) )
        {
            rVariantLB.InsertEntry( pPreset->getVariantLabel() );
        }
    }
    if( rVariantLB.GetEntryCount() > 0 )
        rVariantLB.SelectEntryPos( 0 );
}

VCL_BUILDER_DECL_FACTORY(FadeEffectLB)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    rRet = VclPtr<FadeEffectLB>::Create(pParent, nBits);
}

void FadeEffectLB::applySelected( SdPage* pSlide, ListBox& rVariantLB ) const
{
    if( !pSlide )
        return;

    if( GetSelectEntryPos() == 0 )
    {
        pSlide->setTransitionType( 0 );
        pSlide->setTransitionSubtype( 0 );
        pSlide->setTransitionDirection( true );
        pSlide->setTransitionFadeColor( 0 );
        return;
    }

    int nMatch = 0;
    for( auto aIter = mpImpl->maPresets.begin(); aIter != mpImpl->maPresets.end(); ++aIter )
    {
        TransitionPresetPtr pPreset = *aIter;
        if( !pPreset )
            continue;
        const OUString sLabel( pPreset->getSetLabel() );
        if( !sLabel.isEmpty() && mpImpl->maSet[GetSelectEntryPos()].equals( pPreset->getSetId() ) )
        {
            if( nMatch == rVariantLB.GetSelectEntryPos() )
            {
                pPreset->apply( pSlide );
                break;
            }
            nMatch++;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
