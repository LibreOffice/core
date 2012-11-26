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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <editeng/pathtextportion.hxx>
#include <editeng/outliner.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>

//////////////////////////////////////////////////////////////////////////////

PathTextPortion::PathTextPortion(DrawPortionInfo& rInfo)
:   maOffset(rInfo.mrStartPos.X(), rInfo.mrStartPos.Y()),
    maText(rInfo.mrText),
    mnTextStart(rInfo.mnTextStart),
    mnTextLength(rInfo.mnTextLen),
    mnParagraph(rInfo.mnPara),
    mnIndex(rInfo.mnIndex),
    maFont(rInfo.mrFont),
    maDblDXArray(),
    maLocale(rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale()),
    mbRTL(rInfo.mrFont.IsVertical() ? false : rInfo.IsRTL())
{
    if(mnTextLength && rInfo.mpDXArray)
    {
        maDblDXArray.reserve(mnTextLength);

        for(xub_StrLen a(0); a < mnTextLength; a++)
        {
            maDblDXArray.push_back((double)rInfo.mpDXArray[a]);
        }
    }
}

bool PathTextPortion::operator<(const PathTextPortion& rComp) const
{
    if(mnParagraph < rComp.mnParagraph)
    {
        return true;
    }

    if(maOffset.getX() < rComp.maOffset.getX())
    {
        return true;
    }

    return (maOffset.getY() < rComp.maOffset.getY());
}

xub_StrLen PathTextPortion::getPortionIndex(xub_StrLen nIndex, xub_StrLen nLength) const
{
    if(mbRTL)
    {
        return (mnTextStart + (mnTextLength - (nIndex + nLength)));
    }
    else
    {
        return (mnTextStart + nIndex);
    }
}

double PathTextPortion::getDisplayLength(xub_StrLen nIndex, xub_StrLen nLength) const
{
    drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
    double fRetval(0.0);

    if(maFont.IsVertical())
    {
        fRetval = aTextLayouter.getTextHeight() * (double)nLength;
    }
    else
    {
        fRetval = aTextLayouter.getTextWidth(maText, getPortionIndex(nIndex, nLength), nLength);
    }

    return fRetval;
}

//////////////////////////////////////////////////////////////////////////////
// eof
