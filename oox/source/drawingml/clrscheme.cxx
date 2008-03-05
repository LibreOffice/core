/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clrscheme.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:16:20 $
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

// static
bool ClrScheme::getSystemColor( const sal_Int32 nSysClrToken, sal_Int32& rColor )
{
    //! TODO: get colors from system
    switch( nSysClrToken )
    {
        case XML_window:        rColor = 0xFFFFFF;  break;
        case XML_windowText:    rColor = 0x000000;  break;
        //! TODO: more colors to follow... (chapter 5.1.12.58)
        default:    return false;
    }
    return true;
}

} }
