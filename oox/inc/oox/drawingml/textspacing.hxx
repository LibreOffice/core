/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textspacing.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

