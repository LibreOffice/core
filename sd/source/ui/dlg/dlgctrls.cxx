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

#include "strings.hrc"
#include "dlgctrls.hxx"
#include "sdresid.hxx"
#include "fadedef.h"
#include "sdpage.hxx"

using namespace ::sd;

struct FadeEffectLBImpl
{
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
    TransitionPresetPtr pPreset;

    InsertEntry( SD_RESSTR( STR_EFFECT_NONE ) );
    mpImpl->maPresets.push_back( pPreset );

    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIter;
    for( aIter = rPresetList.begin(); aIter != rPresetList.end(); ++aIter )
    {
        pPreset = (*aIter);
        const OUString aUIName( pPreset->getUIName() );
        if( !aUIName.isEmpty() )
        {
            InsertEntry( aUIName );
            mpImpl->maPresets.push_back( pPreset );
        }
    }

    SelectEntryPos(0);
}

VCL_BUILDER_DECL_FACTORY(FadeEffectLB)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    rRet = VclPtr<FadeEffectLB>::Create(pParent, nBits);
}

void FadeEffectLB::applySelected( SdPage* pSlide ) const
{
    const sal_Int32 nPos = GetSelectEntryPos();

    if( pSlide && (static_cast<size_t>(nPos) < mpImpl->maPresets.size() ) )
    {
        TransitionPresetPtr pPreset( mpImpl->maPresets[nPos] );

        if( pPreset.get() )
        {
            pPreset->apply( pSlide );
        }
        else
        {
            pSlide->setTransitionType( 0 );
            pSlide->setTransitionSubtype( 0 );
            pSlide->setTransitionDirection( true );
            pSlide->setTransitionFadeColor( 0 );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
