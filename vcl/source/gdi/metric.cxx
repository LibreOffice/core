/*************************************************************************
 *
 *  $RCSfile: metric.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#include <metric.hxx>

// =======================================================================

FontInfo::FontInfo()
{
    mpImplMetric                = new ImplFontMetric;
    mpImplMetric->mnRefCount    = 1;
    mpImplMetric->meType        = TYPE_DONTKNOW;
    mpImplMetric->mbDevice      = FALSE;
    mpImplMetric->mnAscent      = 0;
    mpImplMetric->mnDescent     = 0;
    mpImplMetric->mnLeading     = 0;
    mpImplMetric->mnLineHeight  = 0;
    mpImplMetric->mnSlant       = 0;
    mpImplMetric->mnFirstChar   = 0;
    mpImplMetric->mnLastChar    = 0;
}

// -----------------------------------------------------------------------

FontInfo::FontInfo( const FontInfo& rInfo ) :
    Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->mnRefCount++;
}

// -----------------------------------------------------------------------

FontInfo::~FontInfo()
{
    // Eventuell Metric loeschen
    if ( mpImplMetric->mnRefCount > 1 )
        mpImplMetric->mnRefCount--;
    else
        delete mpImplMetric;
}

// -----------------------------------------------------------------------

FontInfo& FontInfo::operator=( const FontInfo& rInfo )
{
    Font::operator=( rInfo );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rInfo.mpImplMetric->mnRefCount++;

    // Sind wir nicht die letzten ?
    if ( mpImplMetric->mnRefCount > 1 )
        mpImplMetric->mnRefCount--;
    else
        delete mpImplMetric;

    mpImplMetric = rInfo.mpImplMetric;

    return *this;
}

// -----------------------------------------------------------------------

BOOL FontInfo::operator==( const FontInfo& rInfo ) const
{
    if ( !Font::operator==( rInfo ) )
        return FALSE;

    if ( mpImplMetric == rInfo.mpImplMetric )
        return TRUE;

    if ( (mpImplMetric->meType      == rInfo.mpImplMetric->meType       ) &&
         (mpImplMetric->mbDevice    == rInfo.mpImplMetric->mbDevice     ) &&
         (mpImplMetric->mnAscent    == rInfo.mpImplMetric->mnAscent     ) &&
         (mpImplMetric->mnDescent   == rInfo.mpImplMetric->mnDescent    ) &&
         (mpImplMetric->mnLeading   == rInfo.mpImplMetric->mnLeading    ) &&
         (mpImplMetric->mnSlant     == rInfo.mpImplMetric->mnSlant      ) &&
         (mpImplMetric->mnFirstChar == rInfo.mpImplMetric->mnFirstChar  ) &&
         (mpImplMetric->mnLastChar  == rInfo.mpImplMetric->mnLastChar   ) )
        return TRUE;
    else
        return FALSE;
}
