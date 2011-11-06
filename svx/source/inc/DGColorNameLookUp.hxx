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



#ifndef _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX
#define _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX

#include <rtl/ustrbuf.hxx>
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
