/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Fetc.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:04:59 $
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


#ifndef _FETC_HXX_
#define _FETC_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
