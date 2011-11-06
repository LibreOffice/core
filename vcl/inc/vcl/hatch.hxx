/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_HATCH_HXX
#define _SV_HATCH_HXX

#include <tools/color.hxx>
#include <vcl/dllapi.h>

#include <vcl/vclenum.hxx>

// --------------
// - Impl_Hatch -
// --------------

class SvStream;

struct ImplHatch
{
    sal_uLong               mnRefCount;
    Color               maColor;
    HatchStyle          meStyle;
    long                mnDistance;
    sal_uInt16              mnAngle;

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
                    Hatch( HatchStyle eStyle, const Color& rHatchColor, long nDistance, sal_uInt16 nAngle10 = 0 );
                    ~Hatch();

    Hatch&          operator=( const Hatch& rHatch );
    sal_Bool            operator==( const Hatch& rHatch ) const;
    sal_Bool            operator!=( const Hatch& rHatch ) const { return !(Hatch::operator==( rHatch ) ); }
    sal_Bool            IsSameInstance( const Hatch& rHatch ) const { return( mpImplHatch == rHatch.mpImplHatch ); }

    void            SetStyle( HatchStyle eStyle );
    HatchStyle      GetStyle() const { return mpImplHatch->meStyle; }

    void            SetColor( const Color& rColor  );
    const Color&    GetColor() const { return mpImplHatch->maColor; }

    void            SetDistance( long nDistance  );
    long            GetDistance() const { return mpImplHatch->mnDistance; }

    void            SetAngle( sal_uInt16 nAngle10 );
    sal_uInt16          GetAngle() const { return mpImplHatch->mnAngle; }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Hatch& rHatch );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Hatch& rHatch );
};

#endif  // _SV_HATCH_HXX
