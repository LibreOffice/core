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
#ifndef _TOOLS_RESARY_HXX
#define _TOOLS_RESARY_HXX

#include "boost/noncopyable.hpp"
#include <vector>
#include "tools/toolsdllapi.h"
#include <tools/string.hxx>
#include <tools/resid.hxx>

#define RESARRAY_INDEX_NOTFOUND (0xffffffff)

class TOOLS_DLLPUBLIC ResStringArray : private boost::noncopyable
{
private:
    struct ImplResStringItem
    {
        rtl::OUString m_aStr;
        long        m_nValue;

        ImplResStringItem( const rtl::OUString& rStr, long nValue = 0 ) :
        m_aStr( rStr ),
        m_nValue( nValue )
        {}
    };

    std::vector< ImplResStringItem >    m_aStrings;

public:
    ResStringArray( const ResId& rResId );
    ~ResStringArray();

    const rtl::OUString GetString( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_aStr : rtl::OUString(); }
    long                GetValue( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_nValue : -1; }
    sal_uInt32          Count() const { return sal_uInt32(m_aStrings.size()); }

    sal_uInt32          FindIndex( long nValue ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
