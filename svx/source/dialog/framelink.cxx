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

#include <sal/config.h>

#include <cstdlib>

#include <svx/framelink.hxx>

#include <math.h>
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>
#include <editeng/borderline.hxx>
#include <svtools/borderhelper.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>


using namespace ::com::sun::star;
using namespace editeng;

namespace svx {
namespace frame {

// Classes
void Style::implEnsureImplStyle()
{
    if(!maImplStyle)
    {
        maImplStyle.reset(new implStyle());
    }
}

Style::Style() :
    maImplStyle()
{
}

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale ) :
    maImplStyle(new implStyle())
{
    maImplStyle->mnType = nType;
    maImplStyle->mfPatternScale = fScale;
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale ) :
    maImplStyle(new implStyle())
{
    maImplStyle->mnType = nType;
    maImplStyle->mfPatternScale = fScale;
    Set( rColorPrim, rColorSecn, rColorGap, bUseGapColor, nP, nD, nS );
}

Style::Style( const editeng::SvxBorderLine* pBorder, double fScale ) :
    maImplStyle()
{
    if(nullptr != pBorder)
    {
        maImplStyle.reset(new implStyle());
        maImplStyle->mfPatternScale = fScale;
        Set( pBorder, fScale );
    }
}

void Style::Clear()
{
    if(maImplStyle)
    {
        maImplStyle.reset();
    }
}

void Style::Set( double nP, double nD, double nS )
{
    /*  nP  nD  nS  ->  mfPrim  mfDist  mfSecn
        --------------------------------------
        any any 0       nP      0       0
        0   any >0      nS      0       0
        >0  0   >0      nP      0       0
        >0  >0  >0      nP      nD      nS
     */
    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->mfPrim = rtl::math::round(nP ? nP : nS, 2);
    pTarget->mfDist = rtl::math::round((nP && nS) ? nD : 0, 2);
    pTarget->mfSecn = rtl::math::round((nP && nD) ? nS : 0, 2);
}

void Style::Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS )
{
    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->maColorPrim = rColorPrim;
    pTarget->maColorSecn = rColorSecn;
    pTarget->maColorGap = rColorGap;
    pTarget->mbUseGapColor = bUseGapColor;
    Set( nP, nD, nS );
}

void Style::Set( const SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth )
{
    if(nullptr == pBorder)
    {
        Clear();
        return;
    }

    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->maColorPrim = pBorder->GetColorOut();
    pTarget->maColorSecn = pBorder->GetColorIn();
    pTarget->maColorGap = pBorder->GetColorGap();
    pTarget->mbUseGapColor = pBorder->HasGapColor();

    const sal_uInt16 nPrim(pBorder->GetOutWidth());
    const sal_uInt16 nDist(pBorder->GetDistance());
    const sal_uInt16 nSecn(pBorder->GetInWidth());

    pTarget->mnType = pBorder->GetBorderLineStyle();
    pTarget->mfPatternScale = fScale;

    if( !nSecn )    // no or single frame border
    {
        Set( std::min<double>(nPrim * fScale, nMaxWidth), 0, 0 );
    }
    else
    {
        Set(std::min<double>(nPrim * fScale, nMaxWidth), std::min<double>(nDist * fScale, nMaxWidth), std::min<double>(nSecn * fScale, nMaxWidth));
        // Enlarge the style if distance is too small due to rounding losses.
        double nPixWidth = std::min<double>((nPrim + nDist + nSecn) * fScale, nMaxWidth);

        if( nPixWidth > GetWidth() )
        {
            pTarget->mfDist = nPixWidth - pTarget->mfPrim - pTarget->mfSecn;
        }

        // Shrink the style if it is too thick for the control.
        while( GetWidth() > nMaxWidth )
        {
            // First decrease space between lines.
            if (pTarget->mfDist)
            {
                --(pTarget->mfDist);
                continue;
            }

            // Still too thick? Decrease the line widths.
            if (pTarget->mfPrim != 0.0 && rtl::math::approxEqual(pTarget->mfPrim, pTarget->mfSecn))
            {
                // Both lines equal - decrease both to keep symmetry.
                --(pTarget->mfPrim);
                --(pTarget->mfSecn);
                continue;
            }

            // Decrease each line for itself
            if (pTarget->mfPrim)
            {
                --(pTarget->mfPrim);
            }

            if ((GetWidth() > nMaxWidth) && pTarget->mfSecn != 0.0)
            {
                --(pTarget->mfSecn);
            }
        }
    }
}

void Style::SetRefMode( RefMode eRefMode )
{
    if(!maImplStyle)
    {
        if(RefMode::Centered == eRefMode)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->meRefMode = eRefMode;
}

void Style::SetColorPrim( const Color& rColor )
{
    if(!maImplStyle)
    {
        if(Color() == rColor)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->maColorPrim = rColor;
}

void Style::SetColorSecn( const Color& rColor )
{
    if(!maImplStyle)
    {
        if(Color() == rColor)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->maColorSecn = rColor;
}

void Style::SetType( SvxBorderLineStyle nType )
{
    if(!maImplStyle)
    {
        if(SvxBorderLineStyle::SOLID == nType)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->mnType = nType;
}

Style& Style::MirrorSelf()
{
    if(!maImplStyle)
    {
        return *this;
    }

    implStyle* pTarget = maImplStyle.get();

    if (pTarget->mfSecn)
    {
        std::swap( pTarget->mfPrim, pTarget->mfSecn );
        // also need to swap colors
        std::swap( pTarget->maColorPrim, pTarget->maColorSecn );
    }

    if( pTarget->meRefMode != RefMode::Centered )
    {
        pTarget->meRefMode = (pTarget->meRefMode == RefMode::Begin) ? RefMode::End : RefMode::Begin;
    }

    return *this;
}

bool Style::operator==( const Style& rOther) const
{
    if(!maImplStyle && !rOther.maImplStyle)
    {
        return true;
    }

    if(maImplStyle && rOther.maImplStyle && maImplStyle.get() == rOther.maImplStyle.get())
    {
        return true;
    }

    return (Prim() == rOther.Prim()
        && Dist() == rOther.Dist()
        && Secn() == rOther.Secn()
        && GetColorPrim() == rOther.GetColorPrim()
        && GetColorSecn() == rOther.GetColorSecn()
        && GetColorGap() == rOther.GetColorGap()
        && GetRefMode() == rOther.GetRefMode()
        && UseGapColor() == rOther.UseGapColor()
        && Type() == rOther.Type());
}

bool Style::operator<( const Style& rOther) const
{
    if(!maImplStyle && !rOther.maImplStyle)
    {
        // are equal
        return false;
    }

    // different total widths -> this<rOther, if this is thinner
    double nLW = GetWidth();
    double nRW = rOther.GetWidth();
    if( !rtl::math::approxEqual(nLW, nRW) ) return nLW < nRW;

    // one line double, the other single -> this<rOther, if this is single
    if( (Secn() == 0) != (rOther.Secn() == 0) ) return Secn() == 0;

    // both lines double with different distances -> this<rOther, if distance of this greater
    if( (Secn() && rOther.Secn()) && !rtl::math::approxEqual(Dist(), rOther.Dist()) ) return Dist() > rOther.Dist();

    // both lines single and 1 unit thick, only one is dotted -> this<rOther, if this is dotted
    if ((nLW == 1) && !Secn() && !rOther.Secn() && (Type() != rOther.Type())) return Type() > rOther.Type();

    // seem to be equal
    return false;
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
