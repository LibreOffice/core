/*************************************************************************
 *
 *  $RCSfile: attributemap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:13:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_ATTRIBUTEMAP_HXX
#define _SLIDESHOW_ATTRIBUTEMAP_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

/* Definition of AttributeMap helper function */

namespace presentation
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

#endif /* _SLIDESHOW_ATTRIBUTEMAP_HXX */
