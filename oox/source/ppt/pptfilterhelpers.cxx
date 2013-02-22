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
        { "wipe(up)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, sal_True },
        { "wipe(right)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, sal_False },
        { "wipe(left)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, sal_True },
        { "wipe(down)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, sal_False },
        { "wheel(1)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::ONEBLADE, sal_True },
        { "wheel(2)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::TWOBLADEVERTICAL, sal_True },
        { "wheel(3)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::THREEBLADE, sal_True },
        { "wheel(4)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::FOURBLADE, sal_True },
        { "wheel(8)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::EIGHTBLADE, sal_True },
        { "strips(downLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, sal_True },
        { "strips(upLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, sal_False },
        { "strips(downRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, sal_True },
        { "strips(upRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, sal_False },
        { "barn(inVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_False },
        { "barn(outVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True },
        { "barn(inHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_False },
        { "barn(outHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
        { "randombar(vertical)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True},
        { "randombar(horizontal)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
        { "checkerboard(down)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::DOWN, sal_True},
        { "checkerboard(across)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::ACROSS, sal_True },
        { "plus(out)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, sal_False },
        { "plus(in)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, sal_True },
        { "diamond(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, sal_True },
        { "diamond(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, sal_False },
        { "circle(out)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
        { "circle(in)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_False },
        { "box(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, sal_True },
        { "box(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, sal_False },
        { "wedge", ::com::sun::star::animations::TransitionType::FANWIPE, ::com::sun::star::animations::TransitionSubType::CENTERTOP, sal_True },
        { "blinds(vertical)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True },
        { "blinds(horizontal)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
        { "fade", ::com::sun::star::animations::TransitionType::FADE, ::com::sun::star::animations::TransitionSubType::CROSSFADE, sal_True },
        { "slide(fromTop)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMTOP, sal_True },
        { "slide(fromRight)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMRIGHT, sal_True },
        { "slide(fromLeft)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMLEFT, sal_True },
        { "slide(fromBottom)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMBOTTOM, sal_True },
        { "dissolve", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, sal_True },
        { "image", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, sal_True }, // TODO
        { NULL, 0, 0, sal_False }
    };

    const transition* transition::find( const OUString& rName )
    {
        const transition* p = gTransitions;

        while( p->mpName )
        {
            if( rName.compareToAscii( p->mpName ) == 0 )
                return p;

            p++;
        }

        return NULL;
    }


    bool convertMeasure( OUString& rString )
    {
        bool bRet = false;

        /* here we want to substitute all occurance of
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
                // copy the non matching inverval verbatim
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
                            sRes.append((sal_Unicode)'x');
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
                            sRes.append((sal_Unicode)'y');
                        }
                        bRet = true;
                        break;
                    default:
                        // this was ppt_ without an interesting thing after that
                        // just copy it verbatim
                        sRes.append("ppt_");
                        // we are going to ajust for ppt_@ after the swtich
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
