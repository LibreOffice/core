/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Fetc.hxx,v $
 * $Revision: 1.7 $
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


#ifndef _FETC_HXX_
#define _FETC_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

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
 2. they must be compareable
 because the FORMATETC structure has a pointer to a TARGETDEVICE
 structure we need a simple wrapper class to fulfill these needs
***********************************************************************/

class CFormatEtc
{
public:
    CFormatEtc( );
    CFormatEtc( const FORMATETC& aFormatEtc );
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
