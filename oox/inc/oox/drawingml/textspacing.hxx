/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_DRAWINGNML__TEXTSPACING_HXX
#define OOX_DRAWINGNML__TEXTSPACING_HXX

#include <rtl/ustring.hxx>

#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

namespace oox { namespace drawingml {


    /** carries a CT_TextSpacing */
    class TextSpacing
    {
    public:
        enum {
            POINTS = 0,
            PERCENT
        };
        TextSpacing()
            : nUnit( POINTS ), nValue( 0 ), bHasValue( sal_False )
            {
            }
        TextSpacing( sal_Int32 nPoints ) : nUnit( POINTS ), nValue( nPoints ), bHasValue( sal_True ){};
        ::com::sun::star::style::LineSpacing toLineSpacing() const
            {
                ::com::sun::star::style::LineSpacing aSpacing;
                aSpacing.Mode = ( nUnit == PERCENT
                                                    ? ::com::sun::star::style::LineSpacingMode::PROP
                                                    :	::com::sun::star::style::LineSpacingMode::MINIMUM );
                aSpacing.Height = static_cast< sal_Int16 >( nUnit == PERCENT ? nValue / 1000 :  nValue );
                return aSpacing;
            }
        sal_Int32 toMargin( float fFontSize ) const
            {
                if ( nUnit == PERCENT )
                {
                    double fMargin = ( fFontSize * 2540 + 36 ) / 72;
                    fMargin *= nValue;
                    fMargin /= 100000;
                    return static_cast< sal_Int32 >( fMargin );
                }
                else
                    return nValue;
            }
        sal_Int32 nUnit;
        sal_Int32 nValue;
        sal_Bool  bHasValue;
    };

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
