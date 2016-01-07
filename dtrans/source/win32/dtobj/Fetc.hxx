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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_FETC_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_FETC_HXX

#include <sal/types.h>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/**********************************************************************
 stl container elements must fulfill the following requirements:
 1. they need a copy ctor and assignement operator(?)
 2. they must be comparable
 because the FORMATETC structure has a pointer to a TARGETDEVICE
 structure we need a simple wrapper class to fulfill these needs
***********************************************************************/

class CFormatEtc
{
public:
    CFormatEtc( );
    explicit CFormatEtc( const FORMATETC& aFormatEtc );
    CFormatEtc( CLIPFORMAT cf, DWORD tymed = TYMED_HGLOBAL, DVTARGETDEVICE* ptd = NULL, DWORD dwAspect = DVASPECT_CONTENT, LONG lindex = -1 );
    CFormatEtc( const CFormatEtc& theOther );

    ~CFormatEtc( );

    CFormatEtc& operator=( const CFormatEtc& theOther );
    operator FORMATETC*( );
    operator FORMATETC( );

    void getFORMATETC( LPFORMATETC lpFormatEtc );

    CLIPFORMAT getClipformat( ) const;
    DWORD      getTymed( ) const;
    void       getTargetDevice( DVTARGETDEVICE** ptd ) const;
    DWORD      getDvAspect( ) const;
    LONG       getLindex( ) const;

    void setClipformat( CLIPFORMAT cf );
    void setTymed( DWORD tymed );
    void setTargetDevice( DVTARGETDEVICE* ptd );
    void setDvAspect( DWORD dwAspect );
    void setLindex( LONG lindex );

private:
    FORMATETC m_FormatEtc;

    friend sal_Int32 operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
    friend sal_Int32 operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );
};

sal_Int32 operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
sal_Int32 operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
