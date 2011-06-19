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

#ifndef _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX
#define _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX

#include <rtl/ustrbuf.hxx>
#include <boost/unordered_map.hpp>

namespace accessibility {

/** This is a color name lookup targeted to be used by the accessibility
    <type>DescriptionGenerator</type> class (hence the DG prefix).  It
    encapsulates a <type>com.sun.star.drawing.ColorTable</type> and provides
    an inverse look up of color names for given a numerical color
    descriptions--the RGB values encoded as integer.

    <p>The class itself is designed as singleton so that the
    <type>com.sun.star.drawing.ColorTable</type> object needs to be created
    only once.</p>

    <p>The singleton instance of this class lives at the moment until the
    application terminates.  However, the color table from which it takes
    its values may change during this time.  Reacting to these changes
    remains a task for the future.</p>
*/
class DGColorNameLookUp
{
public:
    /** Return the single instance of this class.  Use this to look up
        color names with the <member>LookUpColor()</member> method.
    */
    static DGColorNameLookUp& Instance (void);

    /** Return the color name of the color expressed by the given integer.
        @param nColor
            This integer is the sum of the 8 Bit red value shifted left 16
            Bits, the green value shifted left 8 Bits, and the unshifted
            blue value.
        @return
            The returned string is either the color name of the specified
            color or, when no name exists, a string of the form "#RRGGBB"
            with two hexadecimal digits for each color component.
    */
    ::rtl::OUString LookUpColor (long int nColor) const;

private:
    /// Define hash map type to convert numerical color values to names.
    typedef boost::unordered_map<long int, ::rtl::OUString>
        tColorValueToNameMap;

    /// This ma translates from numerical color values to names.
    tColorValueToNameMap maColorValueToNameMap;

    /** The pointer to the only instance of this class.  It is NULL until
        the <member>Instance</member> is called for the first time.
    */
    static DGColorNameLookUp* mpInstance;

    /// Create a new (the only) instance of this class.
    DGColorNameLookUp (void);

    /// The destructor is never called.
    ~DGColorNameLookUp (void);

    /// The copy constructor is not implemented.
    DGColorNameLookUp (const DGColorNameLookUp&);

    /// The assignment operator is not implemented.
    DGColorNameLookUp& operator= (const DGColorNameLookUp&);
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
