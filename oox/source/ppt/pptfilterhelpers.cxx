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

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <rtl/ustrbuf.hxx>
#include "pptfilterhelpers.hxx"

namespace oox { namespace ppt {

    // BEGIN CUT&PASTE from sd pptanimations.hxx

    static const transition gTransitions[] =
    {
        { "wipe(up)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, true },
        { "wipe(right)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, false },
        { "wipe(left)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, true },
        { "wipe(down)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, false },
        { "wheel(1)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::ONEBLADE, true },
        { "wheel(2)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::TWOBLADEVERTICAL, true },
        { "wheel(3)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::THREEBLADE, true },
        { "wheel(4)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::FOURBLADE, true },
        { "wheel(8)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::EIGHTBLADE, true },
        { "strips(downLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, true },
        { "strips(upLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, false },
        { "strips(downRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, true },
        { "strips(upRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, false },
        { "barn(inVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, false },
        { "barn(outVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, true },
        { "barn(inHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, false },
        { "barn(outHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, true },
        { "randombar(vertical)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, true},
        { "randombar(horizontal)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, true },
        { "checkerboard(down)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::DOWN, true},
        { "checkerboard(across)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::ACROSS, true },
        { "plus(out)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, false },
        { "plus(in)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, true },
        { "diamond(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, true },
        { "diamond(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, false },
        { "circle(out)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, true },
        { "circle(in)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, false },
        { "box(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, true },
        { "box(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, false },
        { "wedge", ::com::sun::star::animations::TransitionType::FANWIPE, ::com::sun::star::animations::TransitionSubType::CENTERTOP, true },
        { "blinds(vertical)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, true },
        { "blinds(horizontal)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, true },
        { "fade", ::com::sun::star::animations::TransitionType::FADE, ::com::sun::star::animations::TransitionSubType::CROSSFADE, true },
        { "slide(fromTop)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMTOP, true },
        { "slide(fromRight)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMRIGHT, true },
        { "slide(fromLeft)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMLEFT, true },
        { "slide(fromBottom)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMBOTTOM, true },
        { "dissolve", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, true },
        { "image", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, true }, // TODO
        { NULL, 0, 0, false }
    };

    const transition* transition::find( const OUString& rName )
    {
        const transition* p = gTransitions;

        while( p->mpName )
        {
            if( rName.equalsAscii( p->mpName ) )
                return p;

            p++;
        }

        return NULL;
    }

    bool convertMeasure( OUString& rString )
    {
        bool bRet = false;

        /* here we want to substitute all occurrences of
         * [#]ppt_[xyhw] with
         * x,y,height and width respectively
         */
        sal_Int32 nIndex = 0;
        sal_Int32 nLastIndex = 0;

        nIndex = rString.indexOf("ppt_");
        // bail out early if there is no substitution to be made
        if(nIndex >= 0)
        {
            OUStringBuffer sRes(rString.getLength());

            do
            {
                // copy the non matching interval verbatim
                if(nIndex > nLastIndex)
                {
                    sRes.append(rString.getStr() + nLastIndex, (nIndex - nLastIndex));
                }
                // we are searching for ppt_[xywh] so we need and extra char behind the match
                if(nIndex + 4 < rString.getLength())
                {
                    switch(rString[nIndex + 4])
                    {
                    case (sal_Unicode)'h': // we found ppt_h
                        // if it was #ppt_h we already copied the #
                        // which we do not want in the target, so remove it
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes.remove(sRes.getLength() - 1, 1);
                        }
                        sRes.append("height");
                        bRet = true;
                        break;
                    case (sal_Unicode)'w':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes.remove(sRes.getLength() - 1, 1);
                        }
                        sRes.append("width");
                        bRet = true;
                        break;
                    case (sal_Unicode)'x':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes[sRes.getLength() - 1] = (sal_Unicode)'x';
                        }
                        else
                        {
                            sRes.append('x');
                        }
                        bRet = true;
                        break;
                    case (sal_Unicode)'y':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes[sRes.getLength() - 1] = (sal_Unicode)'y';
                        }
                        else
                        {
                            sRes.append('y');
                        }
                        bRet = true;
                        break;
                    default:
                        // this was ppt_ without an interesting thing after that
                        // just copy it verbatim
                        sRes.append("ppt_");
                        // we are going to adjust for ppt_@ after the switch
                        // so compensate for the fact we did not really process
                        // an extra character after ppt_
                        nIndex -= 1;
                        break;
                    }
                }
                else
                {
                    sRes.append("ppt_");
                    nIndex += 4;
                    nLastIndex = nIndex;
                    break;
                }
                nIndex += 5;
                nLastIndex = nIndex;
            }
            while((nIndex = rString.indexOf("ppt_", nIndex)) > 0);
            // copy the non matching tail if any
            if(nLastIndex < rString.getLength())
            {
                sRes.append(rString.getStr() + nLastIndex, rString.getLength() - nLastIndex );
            }
            rString = sRes.makeStringAndClear();
        }
        return bRet;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
