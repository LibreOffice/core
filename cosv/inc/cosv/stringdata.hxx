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

#ifndef COSV_STRINGDATA_HXX
#define COSV_STRINGDATA_HXX


#include <cosv/str_types.hxx>



namespace csv
{

/** @tpl CHAR
    The expression CHAR(0) has to be valid.
*/
template <class CHAR>
class StringData
{
  public:
    typedef StringData          self;

    typedef str::size           size_type;
    typedef str::position       position_type;

    // LIFECYCLE
                        StringData();
    /** @precond i_pData != 0
        @precond i_nValidLength <= strlen(i_pData)
    */
                        StringData(
                            const CHAR *        i_pData,
                            size_type           i_nValidLength );
                        ~StringData();
    // OPERATORS

    // OPERATIONS

    // INQUIRY
    const CHAR *        Data() const;

    /** @returns the allocated number of CHAR.
        This may be different from the number of bytes.
        There is actually allocated one more CHAR,
        which is guaranteed to be CHAR(0) in all circumstances.
    */
    size_type           Size() const;

  private:
    /*  Because this is used only within a refcounted structure,
        these functions are forbidden - at least yet.
    */
                        StringData(const self&);
    self &              operator=(const self&);

    // DATA
    DYN CHAR *          dpData;
    size_type           nSize;      /// The allocated size - 1 (for the finishing 0).
};



// IMPLEMENTATION

template <class CHAR>
StringData<CHAR>::StringData()
    :   dpData( new CHAR[1] ),
        nSize(0)
{
    *dpData = CHAR(0);
}

template <class CHAR>
StringData<CHAR>::StringData( const CHAR *        i_pData,
                              size_type           i_nValidLength )
    :   dpData( new CHAR[i_nValidLength + 1] ),
        nSize(i_nValidLength)
{
    memcpy( dpData, i_pData, i_nValidLength * sizeof(CHAR) );
    dpData[nSize] = CHAR(0);
}

template <class CHAR>
StringData<CHAR>::~StringData()
{
    delete [] dpData;
}

template <class CHAR>
const CHAR *
StringData<CHAR>::Data() const
{
    return dpData;
}

template <class CHAR>
typename StringData<CHAR>::size_type
StringData<CHAR>::Size() const
{
    return nSize;
}



}   // namespace csv


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
