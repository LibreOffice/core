/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:59:11 $
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

#ifndef _SV_LINEINFO_HXX
#define _SV_LINEINFO_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
