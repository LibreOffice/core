/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    SAL_DLLPRIVATE sal_uLong    ImplConvert( sal_uLong nInFormat, void* pInBuffer, sal_uLong nInBufSize,
                                     void** ppOutBuffer, sal_uLong nOutFormat );
//#endif // __PRIVATE

public:

                        GraphicConverter();
                        ~GraphicConverter();

    static sal_uLong        Import( SvStream& rIStm, Graphic& rGraphic, sal_uLong nFormat = CVT_UNKNOWN );
    static sal_uLong        Export( SvStream& rOStm, const Graphic& rGraphic, sal_uLong nFormat );

    ConvertData*        GetConvertData() { return mpConvertData; }

    void                SetFilterHdl( const Link& rLink ) { maFilterHdl = rLink; }
    const Link&         GetFilterHdl() const { return maFilterHdl; }
};

#endif // _SV_CVTGRF_HXX

