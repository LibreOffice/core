/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salctype.hxx,v $
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

#ifndef _SV_SALCTYPE_HXX
#define _SV_SALCTYPE_HXX

#include <vcl/graph.hxx>

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
