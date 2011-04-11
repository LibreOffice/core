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
#ifndef _TOOLS_RESARY_HXX
#define _TOOLS_RESARY_HXX

#include "tools/toolsdllapi.h"
#include <tools/resid.hxx>
#include <tools/rc.hxx>

// ---------------------
// - ImplResStringItem -
// ---------------------

struct ImplResStringItem
{
    XubString   maStr;
    long        mnValue;

#ifdef _TOOLS_RESARY_CXX
                ImplResStringItem( const XubString& rStr ) :
                    maStr( rStr ) {}
#endif
};

// ------------------
// - ResStringArray -
// ------------------

#define RESARRAY_INDEX_NOTFOUND (0xffffffff)

class TOOLS_DLLPUBLIC ResStringArray
{
    private:
    // ---------------------
    // - ImplResStringItem -
    // ---------------------
    struct ImplResStringItem
    {
        XubString   m_aStr;
        long        m_nValue;

        ImplResStringItem( const XubString& rStr, long nValue = 0 ) :
        m_aStr( rStr ),
        m_nValue( nValue )
        {}
    };

    std::vector< ImplResStringItem >    m_aStrings;

    public:
    ResStringArray( const ResId& rResId );
    ~ResStringArray();

    const XubString&    GetString( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_aStr : String::EmptyString(); }
    long                GetValue( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_nValue : -1; }
    sal_uInt32          Count() const { return sal_uInt32(m_aStrings.size()); }

    sal_uInt32          FindIndex( long nValue ) const;

    private:
    ResStringArray( const ResStringArray& );
    ResStringArray&     operator=( const ResStringArray& );
};

#endif  // _TOOLS_RESARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
