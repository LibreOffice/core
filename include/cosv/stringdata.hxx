/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
