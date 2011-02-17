/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/*
void FadeEffectLB::SelectEffect( presentation::FadeEffect eFE )
{
    sal_Bool bFound = sal_False;

    for( long i = 0, nCount = sizeof( aEffects ) / sizeof( FadeEffectPair ); ( i < nCount ) && !bFound; i++ )
    {
        if( aEffects[ i ].meFE == eFE )
        {
            SelectEntryPos( (sal_uInt16) i );
            bFound = sal_True;
        }
    }
}
*/

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
