/*************************************************************************
 *
 *  $RCSfile: lineinfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_LINEINFO_CXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <lineinfo.hxx>
#endif

DBG_NAME( LineInfo );

// ----------------
// - ImplLineInfo -
// ----------------

ImplLineInfo::ImplLineInfo() :
    mnRefCount  ( 1 ),
    meStyle     ( LINE_SOLID ),
    mnWidth     ( 0 ),
    mnDashCount ( 0 ),
    mnDashLen   ( 0 ),
    mnDotCount  ( 0 ),
    mnDotLen    ( 0 ),
    mnDistance  ( 0 )
{
}

// -----------------------------------------------------------------------

ImplLineInfo::ImplLineInfo( const ImplLineInfo& rImplLineInfo ) :
    mnRefCount  ( 1 ),
    meStyle     ( rImplLineInfo.meStyle ),
    mnWidth     ( rImplLineInfo.mnWidth ),
    mnDashCount ( rImplLineInfo.mnDashCount ),
    mnDashLen   ( rImplLineInfo.mnDashLen ),
    mnDotCount  ( rImplLineInfo.mnDotCount ),
    mnDotLen    ( rImplLineInfo.mnDotLen ),
    mnDistance  ( rImplLineInfo.mnDistance )
{
}

// ------------
// - LineInfo -
// ------------

LineInfo::LineInfo( LineStyle eStyle, long nWidth )
{
    DBG_CTOR( LineInfo, NULL );
    mpImplLineInfo = new ImplLineInfo;
    mpImplLineInfo->meStyle = eStyle;
    mpImplLineInfo->mnWidth = nWidth;
}

// -----------------------------------------------------------------------

LineInfo::LineInfo( const LineInfo& rLineInfo )
{
    DBG_CTOR( LineInfo, NULL );
    DBG_CHKOBJ( &rLineInfo, LineInfo, NULL );
    mpImplLineInfo = rLineInfo.mpImplLineInfo;
    mpImplLineInfo->mnRefCount++;
}

// -----------------------------------------------------------------------

LineInfo::~LineInfo()
{
    DBG_DTOR( LineInfo, NULL );
    if( !( --mpImplLineInfo->mnRefCount ) )
        delete mpImplLineInfo;
}

// -----------------------------------------------------------------------

LineInfo& LineInfo::operator=( const LineInfo& rLineInfo )
{
    DBG_CHKTHIS( LineInfo, NULL );
    DBG_CHKOBJ( &rLineInfo, LineInfo, NULL );

    rLineInfo.mpImplLineInfo->mnRefCount++;

    if( !( --mpImplLineInfo->mnRefCount ) )
        delete mpImplLineInfo;

    mpImplLineInfo = rLineInfo.mpImplLineInfo;
    return *this;
}

// -----------------------------------------------------------------------

BOOL LineInfo::operator==( const LineInfo& rLineInfo ) const
{
    DBG_CHKTHIS( LineInfo, NULL );
    DBG_CHKOBJ( &rLineInfo, LineInfo, NULL );

    return( mpImplLineInfo == rLineInfo.mpImplLineInfo ||
            ( mpImplLineInfo->meStyle == rLineInfo.mpImplLineInfo->meStyle &&
              mpImplLineInfo->mnWidth == rLineInfo.mpImplLineInfo->mnWidth &&
              mpImplLineInfo->mnDashCount == rLineInfo.mpImplLineInfo->mnDashCount &&
              mpImplLineInfo->mnDashLen == rLineInfo.mpImplLineInfo->mnDashLen &&
              mpImplLineInfo->mnDotCount == rLineInfo.mpImplLineInfo->mnDotCount &&
              mpImplLineInfo->mnDotLen == rLineInfo.mpImplLineInfo->mnDotLen &&
              mpImplLineInfo->mnDistance == rLineInfo.mpImplLineInfo->mnDistance ) );
}

// -----------------------------------------------------------------------

void LineInfo::ImplMakeUnique()
{
    if( mpImplLineInfo->mnRefCount != 1 )
    {
        if( mpImplLineInfo->mnRefCount )
            mpImplLineInfo->mnRefCount--;

        mpImplLineInfo = new ImplLineInfo( *mpImplLineInfo );
    }
}

// -----------------------------------------------------------------------

void LineInfo::SetStyle( LineStyle eStyle )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->meStyle = eStyle;
}

// -----------------------------------------------------------------------

void LineInfo::SetWidth( long nWidth )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnWidth = nWidth;
}

// -----------------------------------------------------------------------

void LineInfo::SetDashCount( USHORT nDashCount )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnDashCount = nDashCount;
}

// -----------------------------------------------------------------------

void LineInfo::SetDashLen( long nDashLen )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnDashLen = nDashLen;
}

// -----------------------------------------------------------------------

void LineInfo::SetDotCount( USHORT nDotCount )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnDotCount = nDotCount;
}

// -----------------------------------------------------------------------

void LineInfo::SetDotLen( long nDotLen )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnDotLen = nDotLen;
}

// -----------------------------------------------------------------------

void LineInfo::SetDistance( long nDistance )
{
    DBG_CHKTHIS( LineInfo, NULL );
    ImplMakeUnique();
    mpImplLineInfo->mnDistance = nDistance;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplLineInfo& rImplLineInfo )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;

    rIStm >> nTmp16; rImplLineInfo.meStyle = (LineStyle) nTmp16;
    rIStm >> rImplLineInfo.mnWidth;

    if( aCompat.GetVersion() >= 2 )
    {
        // version 2
        rIStm >> rImplLineInfo.mnDashCount >> rImplLineInfo.mnDashLen;
        rIStm >> rImplLineInfo.mnDotCount >> rImplLineInfo.mnDotLen;
        rIStm >> rImplLineInfo.mnDistance;
    }

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplLineInfo& rImplLineInfo )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 2 );

    // version 1
    rOStm << (UINT16) rImplLineInfo.meStyle << rImplLineInfo.mnWidth;

    // since version2
    rOStm << rImplLineInfo.mnDashCount << rImplLineInfo.mnDashLen;
    rOStm << rImplLineInfo.mnDotCount << rImplLineInfo.mnDotLen;
    rOStm << rImplLineInfo.mnDistance;

    return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, LineInfo& rLineInfo )
{
    rLineInfo.ImplMakeUnique();
    return( rIStm >> *rLineInfo.mpImplLineInfo );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const LineInfo& rLineInfo )
{
    return( rOStm << *rLineInfo.mpImplLineInfo );
}
