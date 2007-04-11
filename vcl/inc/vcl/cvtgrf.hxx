/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cvtgrf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:50:31 $
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

#ifndef _SV_CVTGRF_HXX
#define _SV_CVTGRF_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_SALCTYPE_H
#include <vcl/salctype.hxx>
#endif

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

