/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salctype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:05:38 $
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

#ifndef _SV_SALCTYPE_HXX
#define _SV_SALCTYPE_HXX

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

// -----------
// - Defines -
// -----------

#define CVT_UNKNOWN (0x00000000UL)
#define CVT_BMP     (0x00000001UL)
#define CVT_GIF     (0x00000002UL)
#define CVT_JPG     (0x00000003UL)
#define CVT_MET     (0x00000004UL)
#define CVT_PCT     (0x00000005UL)
#define CVT_PNG     (0x00000006UL)
#define CVT_SVM     (0x00000007UL)
#define CVT_TIF     (0x00000008UL)
#define CVT_WMF     (0x00000009UL)
#define CVT_EMF     (0x0000000aUL)

// ---------------
// - ConvertData -
// ---------------

class SvStream;

struct ConvertData
{
private:

                        ConvertData();

public:

    Graphic             maGraphic;
    SvStream&           mrStm;
    ULONG               mnFormat;

                        ConvertData( const Graphic& rGraphic, SvStream& rStm, ULONG nFormat ) :
                            maGraphic( rGraphic ), mrStm( rStm ), mnFormat( nFormat ) {}
                        ~ConvertData() {}
};

// ------------
// - Callback -
// ------------

typedef ULONG (*SALGRFCVTPROC)( void* pInst,
                                ULONG nInFormat, void* pInBuffer, ULONG nInBufSize,
                                ULONG nOutFormat, void** ppOutBuffer );

#endif // _SV_SALCTYPE_HXX
