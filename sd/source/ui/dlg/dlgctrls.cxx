/*************************************************************************
 *
 *  $RCSfile: dlgctrls.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:01:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

    if( pSlide && (nPos >= 0) && (nPos < mpImpl->maPresets.size() ) )
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
