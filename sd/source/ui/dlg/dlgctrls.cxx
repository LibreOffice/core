/*************************************************************************
 *
 *  $RCSfile: dlgctrls.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:32 $
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

/*************************************************************************
|*
|*  FadeEffectLB: Fuellt die Listbox mit Strings
|*
\************************************************************************/

void FadeEffectLB::Fill()
{
    for( USHORT nID = presentation::FadeEffect_NONE;
         nID < FADE_EFFECT_COUNT;
         nID++ )
    {
        USHORT nRId = GetSdResId( (presentation::FadeEffect)nID );
        if( nRId > 0 )
            InsertEntry( String( SdResId( nRId ) ) );
    }
}

/*************************************************************************
|*
|*  FadeEffectLB: Gibt zum Effekt den entspr. String zurueck
|*
\************************************************************************/

USHORT FadeEffectLB::GetSdResId( presentation::FadeEffect eFE )
{
    switch( eFE )
    {
        case presentation::FadeEffect_NONE:                     return STR_EFFECT_NONE;
        case presentation::FadeEffect_FADE_FROM_LEFT:       return STR_EFFECT_FADE_FROM_LEFT;
        case presentation::FadeEffect_FADE_FROM_TOP:            return STR_EFFECT_FADE_FROM_TOP;
        case presentation::FadeEffect_FADE_FROM_RIGHT:      return STR_EFFECT_FADE_FROM_RIGHT;
        case presentation::FadeEffect_FADE_FROM_BOTTOM:     return STR_EFFECT_FADE_FROM_BOTTOM;
        case presentation::FadeEffect_FADE_TO_CENTER:       return STR_EFFECT_FADE_TO_CENTER;
        case presentation::FadeEffect_FADE_FROM_CENTER:         return STR_EFFECT_FADE_FROM_CENTER;
        case presentation::FadeEffect_MOVE_FROM_LEFT:       return STR_EFFECT_DISCARD_FROM_LEFT;
        case presentation::FadeEffect_MOVE_FROM_TOP:            return STR_EFFECT_DISCARD_FROM_TOP;
        case presentation::FadeEffect_MOVE_FROM_RIGHT:      return STR_EFFECT_DISCARD_FROM_RIGHT;
        case presentation::FadeEffect_MOVE_FROM_BOTTOM:     return STR_EFFECT_DISCARD_FROM_BOTTOM;
        case presentation::FadeEffect_ROLL_FROM_LEFT:       return STR_EFFECT_ROLL_FROM_LEFT;
        case presentation::FadeEffect_ROLL_FROM_TOP:         return STR_EFFECT_ROLL_FROM_TOP;
        case presentation::FadeEffect_ROLL_FROM_RIGHT:       return STR_EFFECT_ROLL_FROM_RIGHT;
        case presentation::FadeEffect_ROLL_FROM_BOTTOM:     return STR_EFFECT_ROLL_FROM_BOTTOM;

        case presentation::FadeEffect_VERTICAL_STRIPES:         return STR_EFFECT_VERTICAL_STRIPES;
        case presentation::FadeEffect_HORIZONTAL_STRIPES:   return STR_EFFECT_HORIZONTAL_STRIPES;
        case presentation::FadeEffect_CLOCKWISE:                return STR_EFFECT_CLOCKWISE;
        case presentation::FadeEffect_COUNTERCLOCKWISE:         return STR_EFFECT_COUNTERCLOCKWISE;
        case presentation::FadeEffect_FADE_FROM_UPPERLEFT:  return STR_EFFECT_FADE_FROM_UPPERLEFT;
        case presentation::FadeEffect_FADE_FROM_UPPERRIGHT:     return STR_EFFECT_FADE_FROM_UPPERRIGHT;
        case presentation::FadeEffect_FADE_FROM_LOWERLEFT:  return STR_EFFECT_FADE_FROM_LOWERLEFT;
        case presentation::FadeEffect_FADE_FROM_LOWERRIGHT:     return STR_EFFECT_FADE_FROM_LOWERRIGHT;
        case presentation::FadeEffect_CLOSE_VERTICAL:       return STR_EFFECT_CLOSE_VERTICAL;
        case presentation::FadeEffect_CLOSE_HORIZONTAL:         return STR_EFFECT_CLOSE_HORIZONTAL;
        case presentation::FadeEffect_OPEN_VERTICAL:            return STR_EFFECT_OPEN_VERTICAL;
        case presentation::FadeEffect_OPEN_HORIZONTAL:      return STR_EFFECT_OPEN_HORIZONTAL;

        case presentation::FadeEffect_SPIRALIN_LEFT:            return STR_EFFECT_SPIRALIN_LEFT;
        case presentation::FadeEffect_SPIRALIN_RIGHT:       return STR_EFFECT_SPIRALIN_RIGHT;
        case presentation::FadeEffect_SPIRALOUT_LEFT:       return STR_EFFECT_SPIRALOUT_LEFT;
        case presentation::FadeEffect_SPIRALOUT_RIGHT:      return STR_EFFECT_SPIRALOUT_RIGHT;
        case presentation::FadeEffect_DISSOLVE:                 return STR_EFFECT_DISSOLVE;
        case presentation::FadeEffect_WAVYLINE_FROM_LEFT:   return STR_EFFECT_WAVYLINE_FROM_LEFT;
        case presentation::FadeEffect_WAVYLINE_FROM_RIGHT:  return STR_EFFECT_WAVYLINE_FROM_RIGHT;
        case presentation::FadeEffect_WAVYLINE_FROM_TOP:        return STR_EFFECT_WAVYLINE_FROM_TOP;
        case presentation::FadeEffect_WAVYLINE_FROM_BOTTOM: return STR_EFFECT_WAVYLINE_FROM_BOTTOM;
        case presentation::FadeEffect_VERTICAL_LINES:        return STR_EFFECT_VERTICAL_LINES;
        case presentation::FadeEffect_HORIZONTAL_LINES:      return STR_EFFECT_HORIZONTAL_LINES;
        case presentation::FadeEffect_RANDOM:                return STR_EFFECT_RANDOM;
        case presentation::FadeEffect_STRETCH_FROM_LEFT:     return STR_EFFECT_STRETCH_FROM_LEFT;
        case presentation::FadeEffect_STRETCH_FROM_TOP:      return STR_EFFECT_STRETCH_FROM_TOP;
        case presentation::FadeEffect_STRETCH_FROM_RIGHT:    return STR_EFFECT_STRETCH_FROM_RIGHT;
        case presentation::FadeEffect_STRETCH_FROM_BOTTOM:   return STR_EFFECT_STRETCH_FROM_BOTTOM;

        default: DBG_ERROR( "Keine StringResource fuer FadeEffect vorhanden!" );
    }
    return( 0 );
}



