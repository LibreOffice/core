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
                                                    :   ::com::sun::star::style::LineSpacingMode::MINIMUM );
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

