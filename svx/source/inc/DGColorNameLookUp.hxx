/*************************************************************************
 *
 *  $RCSfile: DGColorNameLookUp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-26 09:05:56 $
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

#ifndef _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX
#define _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include <hash_map>

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
    typedef std::hash_map<long int, ::rtl::OUString>
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
