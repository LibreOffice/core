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

#pragma once

#include <sal/types.h>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>

/**********************************************************************
 stl container elements must fulfill the following requirements:
 1. they need a copy ctor and assignment operator(?)
 2. they must be comparable
 because the FORMATETC structure has a pointer to a TARGETDEVICE
 structure we need a simple wrapper class to fulfill these needs
***********************************************************************/

class CFormatEtc
{
public:
    CFormatEtc( );
    explicit CFormatEtc( const FORMATETC& aFormatEtc );
    CFormatEtc( CLIPFORMAT cf, DWORD tymed = TYMED_HGLOBAL, DVTARGETDEVICE* ptd = nullptr, DWORD dwAspect = DVASPECT_CONTENT, LONG lindex = -1 );
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

    friend bool operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
    friend bool operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );
};

bool operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
bool operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
