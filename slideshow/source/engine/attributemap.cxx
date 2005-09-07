/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributemap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:23:50 $
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

// must be first
#include <canvas/debug.hxx>
#include <attributemap.hxx>

#include <tools.hxx>


namespace presentation
{
    namespace internal
    {
        typedef ValueMap< AttributeType > AnimateAttributeMap;

        AttributeType mapAttributeName( const ::rtl::OUString& rAttrName )
        {
            /** Maps attribute name to AttributeType enum.

                String entries are all case-insensitive and MUST
                BE STORED lowercase.

                String entries MUST BE SORTED in ascending order!
            */
            static AnimateAttributeMap::MapEntry lcl_attributeMap[] =
                {
                    { "charcolor", ATTRIBUTE_CHAR_COLOR },

                    { "charfontname", ATTRIBUTE_CHAR_FONT_NAME },

                    { "charheight", ATTRIBUTE_CHAR_HEIGHT },

                    { "charposture", ATTRIBUTE_CHAR_POSTURE },

                    // TODO(Q1): This should prolly be changed in PPT import
                    // { "charrotation", ATTRIBUTE_CHAR_ROTATION },
                    { "charrotation", ATTRIBUTE_ROTATE },

                    { "charunderline", ATTRIBUTE_CHAR_UNDERLINE },

                    { "charweight", ATTRIBUTE_CHAR_WEIGHT },

                    { "color", ATTRIBUTE_COLOR },

                    { "dimcolor", ATTRIBUTE_DIMCOLOR },

                    { "fillcolor", ATTRIBUTE_FILL_COLOR },

                    { "fillstyle", ATTRIBUTE_FILL_STYLE },

                    { "height", ATTRIBUTE_HEIGHT },

                    { "linecolor", ATTRIBUTE_LINE_COLOR },

                    { "linestyle", ATTRIBUTE_LINE_STYLE },

                    { "opacity", ATTRIBUTE_OPACITY },

                    { "rotate", ATTRIBUTE_ROTATE },

                    { "skewx", ATTRIBUTE_SKEW_X },

                    { "skewy", ATTRIBUTE_SKEW_Y },

                    { "visibility", ATTRIBUTE_VISIBILITY },

                    { "width", ATTRIBUTE_WIDTH },

                    { "x", ATTRIBUTE_POS_X },

                    { "y", ATTRIBUTE_POS_Y }
                };

            static AnimateAttributeMap aMap( lcl_attributeMap,
                                             sizeof(lcl_attributeMap)/sizeof(AnimateAttributeMap::MapEntry),
                                             false );

            AttributeType eAttributeType;

            // determine the type from the attribute name
            if( !aMap.lookup( rAttrName,
                              eAttributeType ) )
            {
                OSL_TRACE( "mapAttributeName(): attribute name %s not found in map.",
                           ::rtl::OUStringToOString( rAttrName,
                                                     RTL_TEXTENCODING_ASCII_US ).getStr() );
                return ATTRIBUTE_INVALID;
            }

            return eAttributeType;
        }

    }
}
