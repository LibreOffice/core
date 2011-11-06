/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
