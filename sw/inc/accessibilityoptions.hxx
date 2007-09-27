/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibilityoptions.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:55:38 $
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
#ifndef _ACCESSIBILITYOPTIONS_HXX
#define _ACCESSIBILITYOPTIONS_HXX

#include <tools/solar.h>

struct SwAccessibilityOptions
{
    BOOL bIsAlwaysAutoColor         :1;
    BOOL bIsStopAnimatedText        :1;
    BOOL bIsStopAnimatedGraphics    :1;

    SwAccessibilityOptions() :
        bIsAlwaysAutoColor(FALSE),
        bIsStopAnimatedText(FALSE),
        bIsStopAnimatedGraphics(FALSE) {}

    inline BOOL IsAlwaysAutoColor() const       { return bIsAlwaysAutoColor; }
    inline void SetAlwaysAutoColor( BOOL b )    { bIsAlwaysAutoColor = b; }

    inline BOOL IsStopAnimatedGraphics() const       { return bIsStopAnimatedText;}
    inline void SetStopAnimatedGraphics( BOOL b )    { bIsStopAnimatedText = b; }

    inline BOOL IsStopAnimatedText() const       { return bIsStopAnimatedGraphics; }
    inline void SetStopAnimatedText( BOOL b )    { bIsStopAnimatedGraphics = b;}
};
#endif

