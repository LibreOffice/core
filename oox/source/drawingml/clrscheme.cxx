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

#include "oox/drawingml/clrscheme.hxx"
#include "tokens.hxx"

namespace oox { namespace drawingml {

sal_Bool ClrMap::getColorMap( sal_Int32& nClrToken )
{
    sal_Int32 nMapped = 0;
    std::map < sal_Int32, sal_Int32 >::const_iterator aIter( maClrMap.find( nClrToken ) );
    if ( aIter != maClrMap.end() )
        nMapped = (*aIter).second;
    if ( nMapped )
    {
        nClrToken = nMapped;
        return sal_True;
    }
    else
        return sal_False;
}

void ClrMap::setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken )
{
    maClrMap[ nClrToken ] = nMappedClrToken;
}

//-----------------------------------------------------------------------------------------

ClrScheme::ClrScheme()
{
}
ClrScheme::~ClrScheme()
{
}

sal_Bool ClrScheme::getColor( sal_Int32 nSchemeClrToken, sal_Int32& rColor ) const
{
    switch( nSchemeClrToken )
    {
        case XML_bg1 : nSchemeClrToken = XML_lt1; break;
        case XML_bg2 : nSchemeClrToken = XML_lt2; break;
        case XML_tx1 : nSchemeClrToken = XML_dk1; break;
        case XML_tx2 : nSchemeClrToken = XML_dk2; break;
    }
    std::map < sal_Int32, sal_Int32 >::const_iterator aIter( maClrScheme.find( nSchemeClrToken ) );
    if ( aIter != maClrScheme.end() )
        rColor = (*aIter).second;
    return aIter != maClrScheme.end();
}

void ClrScheme::setColor( sal_Int32 nSchemeClrToken, sal_Int32 nColor )
{
    maClrScheme[ nSchemeClrToken ] = nColor;
}

} }
