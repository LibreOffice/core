/*************************************************************************
 *
 *  $RCSfile: dlgctrls.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-30 15:45:02 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#include <tools/ref.hxx>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "strings.hrc"
#include "dlgctrls.hxx"
#include "sdresid.hxx"
#include "fadedef.h"

using namespace ::com::sun::star;

// -----------------------
// - FadeEffectPair list -
// -----------------------

struct FadeEffectPair
{
    presentation::FadeEffect    meFE;
    USHORT                      mnResId;
};

// -----------------------------------------------------------------------------

static FadeEffectPair aEffects[] =
{
    { presentation::FadeEffect_NONE, STR_EFFECT_NONE },

    { presentation::FadeEffect_FADE_FROM_LEFT, STR_EFFECT_FADE_FROM_L },
    { presentation::FadeEffect_FADE_FROM_UPPERLEFT, STR_EFFECT_FADE_FROM_UL },
    { presentation::FadeEffect_FADE_FROM_TOP, STR_EFFECT_FADE_FROM_T },
    { presentation::FadeEffect_FADE_FROM_UPPERRIGHT, STR_EFFECT_FADE_FROM_UR },
    { presentation::FadeEffect_FADE_FROM_RIGHT, STR_EFFECT_FADE_FROM_R },
    { presentation::FadeEffect_FADE_FROM_LOWERRIGHT, STR_EFFECT_FADE_FROM_LR },
    { presentation::FadeEffect_FADE_FROM_BOTTOM, STR_EFFECT_FADE_FROM_B },
    { presentation::FadeEffect_FADE_FROM_LOWERLEFT, STR_EFFECT_FADE_FROM_LL },

    { presentation::FadeEffect_MOVE_FROM_LEFT, STR_EFFECT_DISCARD_FROM_L },
    { presentation::FadeEffect_MOVE_FROM_UPPERLEFT, STR_EFFECT_DISCARD_FROM_UL },
    { presentation::FadeEffect_MOVE_FROM_TOP, STR_EFFECT_DISCARD_FROM_T },
    { presentation::FadeEffect_MOVE_FROM_UPPERRIGHT, STR_EFFECT_DISCARD_FROM_UR },
    { presentation::FadeEffect_MOVE_FROM_RIGHT, STR_EFFECT_DISCARD_FROM_R },
    { presentation::FadeEffect_MOVE_FROM_LOWERRIGHT, STR_EFFECT_DISCARD_FROM_LR },
    { presentation::FadeEffect_MOVE_FROM_BOTTOM, STR_EFFECT_DISCARD_FROM_B },
    { presentation::FadeEffect_MOVE_FROM_LOWERLEFT, STR_EFFECT_DISCARD_FROM_LL },

    { presentation::FadeEffect_UNCOVER_TO_LEFT, STR_EFFECT_MOVE_TO_L },
    { presentation::FadeEffect_UNCOVER_TO_UPPERLEFT, STR_EFFECT_MOVE_TO_UL },
    { presentation::FadeEffect_UNCOVER_TO_TOP, STR_EFFECT_MOVE_TO_T },
    { presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT, STR_EFFECT_MOVE_TO_UR },
    { presentation::FadeEffect_UNCOVER_TO_RIGHT, STR_EFFECT_MOVE_TO_R },
    { presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT, STR_EFFECT_MOVE_TO_LR },
    { presentation::FadeEffect_UNCOVER_TO_BOTTOM, STR_EFFECT_MOVE_TO_B },
    { presentation::FadeEffect_UNCOVER_TO_LOWERLEFT, STR_EFFECT_MOVE_TO_LL },

    { presentation::FadeEffect_ROLL_FROM_LEFT, STR_EFFECT_ROLL_FROM_L },
    { presentation::FadeEffect_ROLL_FROM_TOP, STR_EFFECT_ROLL_FROM_T },
    { presentation::FadeEffect_ROLL_FROM_RIGHT, STR_EFFECT_ROLL_FROM_R },
    { presentation::FadeEffect_ROLL_FROM_BOTTOM, STR_EFFECT_ROLL_FROM_B },

    { presentation::FadeEffect_STRETCH_FROM_LEFT, STR_EFFECT_STRETCH_FROM_L },
    { presentation::FadeEffect_STRETCH_FROM_TOP, STR_EFFECT_STRETCH_FROM_T },
    { presentation::FadeEffect_STRETCH_FROM_RIGHT, STR_EFFECT_STRETCH_FROM_R },
    { presentation::FadeEffect_STRETCH_FROM_BOTTOM, STR_EFFECT_STRETCH_FROM_B },

    { presentation::FadeEffect_WAVYLINE_FROM_LEFT, STR_EFFECT_WAVYLINE_FROM_L },
    { presentation::FadeEffect_WAVYLINE_FROM_TOP, STR_EFFECT_WAVYLINE_FROM_T },
    { presentation::FadeEffect_WAVYLINE_FROM_RIGHT, STR_EFFECT_WAVYLINE_FROM_R },
    { presentation::FadeEffect_WAVYLINE_FROM_BOTTOM, STR_EFFECT_WAVYLINE_FROM_B },

    { presentation::FadeEffect_FADE_TO_CENTER, STR_EFFECT_FADE_TO_CENTER },
    { presentation::FadeEffect_FADE_FROM_CENTER, STR_EFFECT_FADE_FROM_CENTER },

    { presentation::FadeEffect_HORIZONTAL_STRIPES, STR_EFFECT_HORIZONTAL_STRIPES },
    { presentation::FadeEffect_VERTICAL_STRIPES, STR_EFFECT_VERTICAL_STRIPES },

    { presentation::FadeEffect_HORIZONTAL_LINES, STR_EFFECT_HORIZONTAL_LINES },
    { presentation::FadeEffect_VERTICAL_LINES, STR_EFFECT_VERTICAL_LINES },

    { presentation::FadeEffect_HORIZONTAL_CHECKERBOARD, STR_EFFECT_HORIZONTAL_CHECKERBOARD },
    { presentation::FadeEffect_VERTICAL_CHECKERBOARD, STR_EFFECT_VERTICAL_CHECKERBOARD },

    { presentation::FadeEffect_CLOCKWISE, STR_EFFECT_CLOCKWISE },
    { presentation::FadeEffect_COUNTERCLOCKWISE, STR_EFFECT_COUNTERCLOCKWISE },

    { presentation::FadeEffect_OPEN_HORIZONTAL, STR_EFFECT_OPEN_HORIZONTAL },
    { presentation::FadeEffect_OPEN_VERTICAL, STR_EFFECT_OPEN_VERTICAL },

    { presentation::FadeEffect_CLOSE_HORIZONTAL, STR_EFFECT_CLOSE_HORIZONTAL },
    { presentation::FadeEffect_CLOSE_VERTICAL, STR_EFFECT_CLOSE_VERTICAL },

    { presentation::FadeEffect_SPIRALIN_LEFT, STR_EFFECT_SPIRALIN_L },
    { presentation::FadeEffect_SPIRALIN_RIGHT, STR_EFFECT_SPIRALIN_R },

    { presentation::FadeEffect_SPIRALOUT_LEFT, STR_EFFECT_SPIRALOUT_L },
    { presentation::FadeEffect_SPIRALOUT_RIGHT, STR_EFFECT_SPIRALOUT_R },

    { presentation::FadeEffect_DISSOLVE, STR_EFFECT_DISSOLVE },

    { presentation::FadeEffect_RANDOM, STR_EFFECT_RANDOM }
};

/*************************************************************************
|*
|*  FadeEffectLB: Fuellt die Listbox mit Strings
|*
\************************************************************************/

void FadeEffectLB::Fill()
{
    for( long i = 0, nCount = sizeof( aEffects ) / sizeof( FadeEffectPair ); i < nCount; i++ )
        InsertEntry( SdResId( aEffects[ i ].mnResId ) );
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

presentation::FadeEffect FadeEffectLB::GetSelectedEffect() const
{
    presentation::FadeEffect    eFE;
    const USHORT                nPos = GetSelectEntryPos();

    if( ( LISTBOX_ENTRY_NOTFOUND != nPos ) && ( nPos < ( sizeof( aEffects ) / sizeof( FadeEffectPair ) ) ) )
        eFE = aEffects[ nPos ].meFE;
    else
        eFE = presentation::FadeEffect_FADE_FROM_LEFT;

    return eFE;
}
