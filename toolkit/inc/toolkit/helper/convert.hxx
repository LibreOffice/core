/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convert.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:54:04 $
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

#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#define _TOOLKIT_HELPER_CONVERT_HXX_

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#include <tools/gen.hxx>

inline ::com::sun::star::awt::Size AWTSize( const Size& rVCLSize )
{
    return ::com::sun::star::awt::Size( rVCLSize.Width(), rVCLSize.Height() );
}

inline ::Size VCLSize( const ::com::sun::star::awt::Size& rAWTSize )
{
    return ::Size( rAWTSize.Width, rAWTSize.Height );
}

inline ::com::sun::star::awt::Point AWTPoint( const ::Point& rVCLPoint )
{
    return ::com::sun::star::awt::Point( rVCLPoint.X(), rVCLPoint.Y() );
}

inline ::Point VCLPoint( const ::com::sun::star::awt::Point& rAWTPoint )
{
    return ::Point( rAWTPoint.X, rAWTPoint.Y );
}

inline ::com::sun::star::awt::Rectangle AWTRectangle( const ::Rectangle& rVCLRect )
{
    return ::com::sun::star::awt::Rectangle( rVCLRect.Left(), rVCLRect.Top(), rVCLRect.GetWidth(), rVCLRect.GetHeight() );
}

inline ::Rectangle VCLRectangle( const ::com::sun::star::awt::Rectangle& rAWTRect )
{
    return ::Rectangle( ::Point( rAWTRect.X, rAWTRect.Y ), ::Size( rAWTRect.Width, rAWTRect.Height ) );
}


#endif  // _TOOLKIT_HELPER_CONVERT_HXX_
