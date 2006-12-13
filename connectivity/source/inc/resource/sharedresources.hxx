/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedresources.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:24:33 $
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

#ifndef CONNECTIVITY_SHAREDRESOURCES_HXX
#define CONNECTIVITY_SHAREDRESOURCES_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/solar.h>
#include <rtl/ustring.hxx>

//........................................................................
namespace connectivity
{
//........................................................................

    typedef USHORT  ResourceId;
    //====================================================================
    //= SharedResources
    //====================================================================
    /** helper class for accessing resources shared by different libraries
        in the connectivity module
    */
    class SharedResources
    {
    public:
        SharedResources();
        ~SharedResources();

        /** loads a string from the shared resource file
            @param  _nResId
                the resource ID of the string
            @return
                the string from the resource file
        */
        ::rtl::OUString
            getResurceString(
                ResourceId _nResId
            );

        /** loads a string from the shared resource file, and replaces
            a given ASCII pattern with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _pAsciiPatternToReplace
                the ASCII string which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute
                the String which should substitute the ASCII pattern.

            @return
                the string from the resource file, with applied string substitution
        */
        ::rtl::OUString
            getResourceStringWithSubstitution(
                ResourceId _nResId,
                const sal_Char* _pAsciiPatternToReplace,
                const ::rtl::OUString& _rStringToSubstitute
            );

        /** loads a string from the shared resource file, and replaces
            a given ASCII pattern with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _pAsciiPatternToReplace1
                the ASCII string (1) which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute1
                the String which should substitute the ASCII pattern (1)
            @param  _pAsciiPatternToReplace2
                the ASCII string (2) which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute2
                the String which should substitute the ASCII pattern (2)

            @return
                the string from the resource file, with applied string substitution
        */
        ::rtl::OUString
            getResourceStringWithSubstitution(
                ResourceId _nResId,
                const sal_Char* _pAsciiPatternToReplace1,
                const ::rtl::OUString& _rStringToSubstitute1,
                const sal_Char* _pAsciiPatternToReplace2,
                const ::rtl::OUString& _rStringToSubstitute2
            );

        /** loads a string from the shared resource file, and replaces
            a given ASCII pattern with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _pAsciiPatternToReplace1
                the ASCII string (1) which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute1
                the String which should substitute the ASCII pattern (1)
            @param  _pAsciiPatternToReplace2
                the ASCII string (2) which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute2
                the String which should substitute the ASCII pattern (2)
            @param  _pAsciiPatternToReplace3
                the ASCII string (3) which is to search in the string. Must not be <NULL/>.
            @param  _rStringToSubstitute3
                the String which should substitute the ASCII pattern (3)

            @return
                the string from the resource file, with applied string substitution
        */
        ::rtl::OUString
            getResourceStringWithSubstitution(
                ResourceId _nResId,
                const sal_Char* _pAsciiPatternToReplace1,
                const ::rtl::OUString& _rStringToSubstitute1,
                const sal_Char* _pAsciiPatternToReplace2,
                const ::rtl::OUString& _rStringToSubstitute2,
                const sal_Char* _pAsciiPatternToReplace3,
                const ::rtl::OUString& _rStringToSubstitute3
            );
    };

//........................................................................
} // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_SHAREDRESOURCES_HXX

