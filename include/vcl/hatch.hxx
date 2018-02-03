/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_HATCH_HXX
#define INCLUDED_VCL_HATCH_HXX

#include <vcl/color.hxx>
#include <vcl/dllapi.h>

#include <vcl/vclenum.hxx>
#include <o3tl/cow_wrapper.hxx>


class SvStream;

struct ImplHatch
{
    Color               maColor;
    HatchStyle          meStyle;
    long                mnDistance;
    sal_uInt16          mnAngle;

    ImplHatch();
    ImplHatch( const ImplHatch& rImplHatch );

    bool operator==( const ImplHatch& rImplHatch ) const;
};

class VCL_DLLPUBLIC Hatch
{
public:

                    Hatch();
                    Hatch( const Hatch& rHatch );
                    Hatch( HatchStyle eStyle, const Color& rHatchColor, long nDistance, sal_uInt16 nAngle10 );
                    ~Hatch();

    Hatch&          operator=( const Hatch& rHatch );
    bool            operator==( const Hatch& rHatch ) const;
    bool            operator!=( const Hatch& rHatch ) const { return !(Hatch::operator==( rHatch ) ); }

    HatchStyle      GetStyle() const { return mpImplHatch->meStyle; }

    void            SetColor( const Color& rColor  );
    const Color&    GetColor() const { return mpImplHatch->maColor; }

    void            SetDistance( long nDistance  );
    long            GetDistance() const { return mpImplHatch->mnDistance; }

    void            SetAngle( sal_uInt16 nAngle10 );
    sal_uInt16          GetAngle() const { return mpImplHatch->mnAngle; }

    friend VCL_DLLPUBLIC SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch );
    friend VCL_DLLPUBLIC SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch );

private:
    o3tl::cow_wrapper< ImplHatch >          mpImplHatch;
};

#endif // INCLUDED_VCL_HATCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
