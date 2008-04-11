/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cvtgrf.hxx,v $
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

#ifndef _SV_CVTGRF_HXX
#define _SV_CVTGRF_HXX

#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <vcl/salctype.hxx>

// --------------------
// - GraphicConverter -
// --------------------

struct  ConvertData;
class   Graphic;

class VCL_DLLPUBLIC GraphicConverter
{
private:

    Link                maFilterHdl;
    ConvertData*        mpConvertData;

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE ULONG    ImplConvert( ULONG nInFormat, void* pInBuffer, ULONG nInBufSize,
                                     void** ppOutBuffer, ULONG nOutFormat );
//#endif // __PRIVATE

public:

                        GraphicConverter();
                        ~GraphicConverter();

    static ULONG        Import( SvStream& rIStm, Graphic& rGraphic, ULONG nFormat = CVT_UNKNOWN );
    static ULONG        Export( SvStream& rOStm, const Graphic& rGraphic, ULONG nFormat );

    ConvertData*        GetConvertData() { return mpConvertData; }

    void                SetFilterHdl( const Link& rLink ) { maFilterHdl = rLink; }
    const Link&         GetFilterHdl() const { return maFilterHdl; }
};

#endif // _SV_CVTGRF_HXX

