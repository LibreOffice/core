/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: convert.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#define _TOOLKIT_HELPER_CONVERT_HXX_

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>

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
