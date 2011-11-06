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



#ifndef INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX
#define INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX

#include <rtl/ustring.hxx>

/* Definition of AttributeMap helper function */

namespace slideshow
{
    namespace internal
    {
        /** Type of to-be-animated attribute.

            This enum describes the type of an animated
            attribute.
        */
        enum AttributeType
        {
            ATTRIBUTE_INVALID,
            ATTRIBUTE_CHAR_COLOR,
            ATTRIBUTE_CHAR_FONT_NAME,
            ATTRIBUTE_CHAR_HEIGHT,
            ATTRIBUTE_CHAR_POSTURE,
            ATTRIBUTE_CHAR_ROTATION,
            ATTRIBUTE_CHAR_UNDERLINE,
            ATTRIBUTE_CHAR_WEIGHT,
            ATTRIBUTE_COLOR,
            ATTRIBUTE_DIMCOLOR,
            ATTRIBUTE_FILL_COLOR,
            ATTRIBUTE_FILL_STYLE,
            ATTRIBUTE_HEIGHT,
            ATTRIBUTE_LINE_COLOR,
            ATTRIBUTE_LINE_STYLE,
            ATTRIBUTE_OPACITY,
            ATTRIBUTE_ROTATE,
            ATTRIBUTE_SKEW_X,
            ATTRIBUTE_SKEW_Y,
            ATTRIBUTE_VISIBILITY,
            ATTRIBUTE_WIDTH,
            ATTRIBUTE_POS_X,
            ATTRIBUTE_POS_Y
        };

        /** Map attribute name to AttributeType enum

            @returns ATTRIBUTE_INVALID, if name was not found in the
            mapping table.
         */
        AttributeType mapAttributeName( const ::rtl::OUString& rAttrName );
    }
}

#endif /* INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX */
