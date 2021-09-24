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
#pragma once

#include <tools/color.hxx>
#include <tools/long.hxx>
#include <tools/degree.hxx>
#include <vcl/dllapi.h>

#include <vcl/vclenum.hxx>
#include <o3tl/cow_wrapper.hxx>


class SvStream;

struct ImplHatch
{
    Color               maColor;
    HatchStyle          meStyle;
    tools::Long                mnDistance;
    Degree10            mnAngle;

    ImplHatch();

    bool operator==( const ImplHatch& rImplHatch ) const;
};

class VCL_DLLPUBLIC Hatch
{
public:

                    Hatch();
                    Hatch( const Hatch& rHatch );
                    Hatch( HatchStyle eStyle, const Color& rHatchColor, tools::Long nDistance, Degree10 nAngle10 );
                    ~Hatch();

    Hatch&          operator=( const Hatch& rHatch );
    bool            operator==( const Hatch& rHatch ) const;
    bool            operator!=( const Hatch& rHatch ) const { return !(Hatch::operator==( rHatch ) ); }

    HatchStyle      GetStyle() const { return mpImpl->meStyle; }

    void            SetColor( const Color& rColor  );
    const Color&    GetColor() const { return mpImpl->maColor; }

    void            SetDistance( tools::Long nDistance  );
    tools::Long            GetDistance() const { return mpImpl->mnDistance; }

    void            SetAngle( Degree10 nAngle10 );
    Degree10        GetAngle() const { return mpImpl->mnAngle; }

    friend SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch );
    friend SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch );

private:
    o3tl::cow_wrapper< ImplHatch >          mpImpl;
};


namespace std
{
    /** Specialise std::optional template for the case where we are wrapping a o3tl::cow_wrapper
        type, and we can make the pointer inside the cow_wrapper act as an empty value,
        and save ourselves some storage */
    template<>
    class optional<Hatch> final : public o3tl::cow_optional<Hatch>
    {
    public:
        using cow_optional::cow_optional; // inherit constructors
        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;
        ~optional();
        void reset();
    };
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
