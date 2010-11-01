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

#ifndef CONNECTIVITY_SHAREDRESOURCES_HXX
#define CONNECTIVITY_SHAREDRESOURCES_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustring.hxx>
#include <list>
#include "connectivity/dbtoolsdllapi.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    typedef sal_uInt16  ResourceId;
    //====================================================================
    //= SharedResources
    //====================================================================
    /** helper class for accessing resources shared by different libraries
        in the connectivity module
    */
    class OOO_DLLPUBLIC_DBTOOLS SharedResources
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
            getResourceString(
                ResourceId _nResId
            ) const;

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
            ) const;

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
            ) const;

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
            ) const;

        /** loads a string from the shared resource file, and replaces a given ASCII pattern with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _aStringToSubstitutes
                A list of substitutions.

            @return
                the string from the resource file, with applied string substitution
        */
        ::rtl::OUString getResourceStringWithSubstitution( ResourceId _nResId,
                    const ::std::list< ::std::pair<const sal_Char* , ::rtl::OUString > > _aStringToSubstitutes) const;
    };

//........................................................................
} // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_SHAREDRESOURCES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
