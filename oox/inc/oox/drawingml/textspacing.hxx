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
            : nUnit( POINTS ), nValue( 0 ), bHasValue( false )
            {
            }
        TextSpacing( sal_Int32 nPoints ) : nUnit( POINTS ), nValue( nPoints ), bHasValue( true ){};
        ::com::sun::star::style::LineSpacing toLineSpacing() const
            {
                ::com::sun::star::style::LineSpacing aSpacing;
                aSpacing.Mode = ( nUnit == PERCENT
                                                    ? ::com::sun::star::style::LineSpacingMode::PROP
                                                    :   ::com::sun::star::style::LineSpacingMode::MINIMUM );
                aSpacing.Height = static_cast< sal_Int16 >( nUnit == PERCENT ? nValue / 1000 :  nValue );
                return aSpacing;
            }
        sal_Int32 toMargin( float fFontSize ) const
            {
                if ( nUnit == PERCENT )
                    return (sal_Int32) ((((fFontSize*nValue)/1000)*254 + 360)/720);
                else
                    return nValue;
            }
        sal_Int32 nUnit;
        sal_Int32 nValue;
        bool  bHasValue;
    };

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
