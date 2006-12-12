/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgctrls.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:02:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <tools/ref.hxx>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

FadeEffectLB::FadeEffectLB( Window* pParent, WinBits aWB )
:   ListBox( pParent, aWB ),
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
    BOOL bFound = FALSE;

    for( long i = 0, nCount = sizeof( aEffects ) / sizeof( FadeEffectPair ); ( i < nCount ) && !bFound; i++ )
    {
        if( aEffects[ i ].meFE == eFE )
        {
            SelectEntryPos( (USHORT) i );
            bFound = TRUE;
        }
    }
}
*/

// -----------------------------------------------------------------------------

void FadeEffectLB::applySelected( SdPage* pSlide ) const
{
    const USHORT nPos = GetSelectEntryPos();

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
