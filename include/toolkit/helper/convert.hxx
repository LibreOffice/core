/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
