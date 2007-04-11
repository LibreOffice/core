/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:56:11 $
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

#ifndef _SV_HATCH_HXX
#define _SV_HATCH_HXX

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#include <vcl/vclenum.hxx>

// --------------
// - Impl_Hatch -
// --------------

class SvStream;

struct ImplHatch
{
    ULONG               mnRefCount;
    Color               maColor;
    HatchStyle          meStyle;
    long                mnDistance;
    USHORT              mnAngle;

                        ImplHatch();
                        ImplHatch( const ImplHatch& rImplHatch );

    friend SvStream&    operator>>( SvStream& rIStm, ImplHatch& rImplHatch );
    friend SvStream&    operator<<( SvStream& rOStm, const ImplHatch& rImplHatch );
};

// ---------
// - Hatch -
// ---------

class VCL_DLLPUBLIC Hatch
{
private:

    ImplHatch*          mpImplHatch;
    SAL_DLLPRIVATE void ImplMakeUnique();

public:

                    Hatch();
                    Hatch( const Hatch& rHatch );
                    Hatch( HatchStyle eStyle, const Color& rHatchColor, long nDistance, USHORT nAngle10 = 0 );
                    ~Hatch();

    Hatch&          operator=( const Hatch& rHatch );
    BOOL            operator==( const Hatch& rHatch ) const;
    BOOL            operator!=( const Hatch& rHatch ) const { return !(Hatch::operator==( rHatch ) ); }
    BOOL            IsSameInstance( const Hatch& rHatch ) const { return( mpImplHatch == rHatch.mpImplHatch ); }

    void            SetStyle( HatchStyle eStyle );
    HatchStyle      GetStyle() const { return mpImplHatch->meStyle; }

    void            SetColor( const Color& rColor  );
    const Color&    GetColor() const { return mpImplHatch->maColor; }

    void            SetDistance( long nDistance  );
    long            GetDistance() const { return mpImplHatch->mnDistance; }

    void            SetAngle( USHORT nAngle10 );
    USHORT          GetAngle() const { return mpImplHatch->mnAngle; }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Hatch& rHatch );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Hatch& rHatch );
};

#endif  // _SV_HATCH_HXX
