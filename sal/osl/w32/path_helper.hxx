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

#ifndef INCLUDED_SAL_OSL_W32_PATH_HELPER_HXX
#define INCLUDED_SAL_OSL_W32_PATH_HELPER_HXX

#include <sal/config.h>

#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/alloc.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

void osl_systemPathEnsureSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

void SAL_CALL osl_systemPathRemoveSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_is_logical_drive_pattern
 Returns whether a given path is only a logical drive pattern or not.
 A logical drive pattern is something like "a:\", "c:\".
 No logical drive pattern is something like "c:\test"
 ******************************************************************/

bool osl_systemPathIsLogicalDrivePattern(/*in*/ const rtl_uString* pustrPath);

namespace osl
{

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

inline void systemPathEnsureSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathEnsureSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

inline void systemPathRemoveSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathRemoveSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathIsLogicalDrivePattern
 ******************************************************************/

inline bool systemPathIsLogicalDrivePattern(/*in*/ const rtl::OUString& path)
{
    return osl_systemPathIsLogicalDrivePattern(path.pData);
}

/*******************************************************************
 LongPathBuffer
 ******************************************************************/
template< class T >
class LongPathBuffer
{
    T* m_pBuffer;
    sal_uInt32 m_nCharNum;

    LongPathBuffer();
    LongPathBuffer( const LongPathBuffer& );
    LongPathBuffer& operator=( const LongPathBuffer& );

public:
    explicit LongPathBuffer( sal_uInt32 nCharNum )
    : m_pBuffer( static_cast<T*>( rtl_allocateMemory( nCharNum * sizeof( T ) ) ) )
    , m_nCharNum( nCharNum )
    {
        SAL_WARN_IF(!m_pBuffer, "sal.file", "Cannot allocate the buffer" );
    }

    ~LongPathBuffer()
    {
        if ( m_pBuffer )
            rtl_freeMemory( m_pBuffer );
        m_pBuffer = nullptr;
    }

    sal_uInt32 getBufSizeInSymbols()
    {
        return m_nCharNum;
    }

    operator T* ()
    {
        return m_pBuffer;
    }

};

    template< class U, class T > U mingw_reinterpret_cast(LongPathBuffer<T>& a) { return reinterpret_cast<U>(static_cast<T*>(a)); }

} // end namespace osl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
