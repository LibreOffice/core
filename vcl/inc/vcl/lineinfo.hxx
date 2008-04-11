/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lineinfo.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_LINEINFO_HXX
#define _SV_LINEINFO_HXX

#include <vcl/dllapi.h>

#include <tools/gen.hxx>
#include <vcl/vclenum.hxx>

// ----------------
// - ImplLineInfo -
// ----------------

class SvStream;

struct ImplLineInfo
{
    ULONG               mnRefCount;
    LineStyle           meStyle;
    long                mnWidth;
    USHORT              mnDashCount;
    long                mnDashLen;
    USHORT              mnDotCount;
    long                mnDotLen;
    long                mnDistance;

                        ImplLineInfo();
                        ImplLineInfo( const ImplLineInfo& rImplLineInfo );

    friend SvStream&    operator>>( SvStream& rIStm, ImplLineInfo& rImplLineInfo );
    friend SvStream&    operator<<( SvStream& rOStm, const ImplLineInfo& rImplLineInfo );
};

// ------------
// - LineInfo -
// ------------

class VCL_DLLPUBLIC LineInfo
{
private:

    ImplLineInfo*   mpImplLineInfo;
    long            n1;
    long            n2;
    long            n3;

    SAL_DLLPRIVATE void ImplMakeUnique();

public:

                    LineInfo( LineStyle eLineStyle = LINE_SOLID, long nWidth = 0L );
                    LineInfo( const LineInfo& rLineInfo );
                    ~LineInfo();

    LineInfo&       operator=( const LineInfo& rLineInfo );
    BOOL            operator==( const LineInfo& rLineInfo ) const;
    BOOL            operator!=( const LineInfo& rLineInfo ) const { return !(LineInfo::operator==( rLineInfo ) ); }
    BOOL            IsSameInstance( const LineInfo& rLineInfo ) const { return( mpImplLineInfo == rLineInfo.mpImplLineInfo ); }

    void            SetStyle( LineStyle eStyle );
    LineStyle       GetStyle() const { return mpImplLineInfo->meStyle; }

    void            SetWidth( long nWidth );
    long            GetWidth() const { return mpImplLineInfo->mnWidth; }

    void            SetDashCount( USHORT nDashCount );
    USHORT          GetDashCount() const { return mpImplLineInfo->mnDashCount; }

    void            SetDashLen( long nDashLen );
    long            GetDashLen() const { return mpImplLineInfo->mnDashLen; }

    void            SetDotCount( USHORT nDotCount );
    USHORT          GetDotCount() const { return mpImplLineInfo->mnDotCount; }

    void            SetDotLen( long nDotLen );
    long            GetDotLen() const { return mpImplLineInfo->mnDotLen; }

    void            SetDistance( long nDistance );
    long            GetDistance() const { return mpImplLineInfo->mnDistance; }

    BOOL            IsDefault() const { return( !mpImplLineInfo->mnWidth && ( LINE_SOLID == mpImplLineInfo->meStyle ) ); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, LineInfo& rLineInfo );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const LineInfo& rLineInfo );
};

#endif  // _SV_LINEINFO_HXX
