/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <tools/ref.hxx>
#include <tools/debug.hxx>

#include "strings.hrc"
#include "dlgctrls.hxx"
#include "sdresid.hxx"
#include "fadedef.h"
#include "sdpage.hxx"

using namespace ::sd;
using namespace ::rtl;

struct FadeEffectLBImpl
{
    std::vector< TransitionPresetPtr > maPresets;
};

FadeEffectLB::FadeEffectLB( Window* pParent, SdResId Id )
:   ListBox( pParent, Id ),
    mpImpl( new FadeEffectLBImpl )
{
}

FadeEffectLB::~FadeEffectLB()
{
    delete mpImpl;
}

void FadeEffectLB::Fill()
{
    TransitionPresetPtr pPreset;

    InsertEntry( String( SdResId( STR_EFFECT_NONE ) ) );
    mpImpl->maPresets.push_back( pPreset );

    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIter;
    for( aIter = rPresetList.begin(); aIter != rPresetList.end(); aIter++ )
    {
        pPreset = (*aIter);
        const OUString aUIName( pPreset->getUIName() );
        if( aUIName.getLength() )
        {
            InsertEntry( aUIName );
            mpImpl->maPresets.push_back( pPreset );
        }
    }

    SelectEntryPos(0);
}

// -----------------------------------------------------------------------------

void FadeEffectLB::applySelected( SdPage* pSlide ) const
{
    const sal_uInt16 nPos = GetSelectEntryPos();

    if( pSlide && (nPos < mpImpl->maPresets.size() ) )
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
            pSlide->setTransitionDirection( sal_True );
            pSlide->setTransitionFadeColor( 0 );
        }
    }
}
