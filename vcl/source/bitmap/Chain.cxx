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

#include "Chain.hxx"
#include "ChainMove.hxx"
#include "PointArray.hxx"
#include "vecthelper.hxx"

#include <cstring>

namespace vcl::Vectorizer
{
Chain::Chain()
    : mnArraySize(1024)
    , mnCount(0)
    , mpCodes(new sal_uInt8[mnArraySize])
{
}

void Chain::GetSpace()
{
    const sal_uLong nOldArraySize = mnArraySize;
    sal_uInt8* pNewCodes;

    mnArraySize = mnArraySize << 1;
    pNewCodes = new sal_uInt8[mnArraySize];
    memcpy(pNewCodes, mpCodes.get(), nOldArraySize);
    mpCodes.reset(pNewCodes);
}

void Chain::BeginAdd(const Point& rStartPt)
{
    maPoly = tools::Polygon();
    maStartPt = rStartPt;
    mnCount = 0;
}

void Chain::EndAdd(sal_uLong nFlag)
{
    if (mnCount)
    {
        vcl::Vectorizer::PointArray aArr;

        if (nFlag & VECT_POLY_INLINE_INNER)
        {
            tools::Long nFirstX, nFirstY;
            tools::Long nLastX, nLastY;

            nFirstX = nLastX = maStartPt.X();
            nFirstY = nLastY = maStartPt.Y();
            aArr.SetSize(mnCount << 1);

            sal_uInt16 nPolyPos;
            sal_uLong i;
            for (i = 0, nPolyPos = 0; i < (mnCount - 1); i++)
            {
                const sal_uInt8 cMove = mpCodes[i];
                const sal_uInt8 cNextMove = mpCodes[i + 1];
                const vcl::Vectorizer::ChainMove& rMove = aMove[cMove];
                const vcl::Vectorizer::ChainMove& rMoveInner = aMoveInner[cMove];
                bool bDone = true;

                nLastX += rMove.nDX;
                nLastY += rMove.nDY;

                if (cMove < 4)
                {
                    if ((cMove == 0 && cNextMove == 3) || (cMove == 3 && cNextMove == 2)
                        || (cMove == 2 && cNextMove == 1) || (cMove == 1 && cNextMove == 0))
                    {
                    }
                    else if (cMove == 2 && cNextMove == 3)
                    {
                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY);
                    }
                    else if (cMove == 3 && cNextMove == 0)
                    {
                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY + 1);
                    }
                    else if (cMove == 0 && cNextMove == 1)
                    {
                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY);
                    }
                    else if (cMove == 1 && cNextMove == 2)
                    {
                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY - 1);
                    }
                    else
                        bDone = false;
                }
                else if (cMove == 7 && cNextMove == 0)
                {
                    aArr[nPolyPos].setX(nLastX - 1);
                    aArr[nPolyPos++].setY(nLastY);

                    aArr[nPolyPos].setX(nLastX);
                    aArr[nPolyPos++].setY(nLastY + 1);
                }
                else if (cMove == 4 && cNextMove == 1)
                {
                    aArr[nPolyPos].setX(nLastX);
                    aArr[nPolyPos++].setY(nLastY + 1);

                    aArr[nPolyPos].setX(nLastX + 1);
                    aArr[nPolyPos++].setY(nLastY);
                }
                else
                    bDone = false;

                if (!bDone)
                {
                    aArr[nPolyPos].setX(nLastX + rMoveInner.nDX);
                    aArr[nPolyPos++].setY(nLastY + rMoveInner.nDY);
                }
            }

            aArr[nPolyPos].setX(nFirstX + 1);
            aArr[nPolyPos++].setY(nFirstY + 1);
            aArr.SetRealSize(nPolyPos);
        }
        else if (nFlag & VECT_POLY_INLINE_OUTER)
        {
            tools::Long nFirstX, nFirstY;
            tools::Long nLastX, nLastY;

            nFirstX = nLastX = maStartPt.X();
            nFirstY = nLastY = maStartPt.Y();
            aArr.SetSize(mnCount << 1);

            sal_uInt16 nPolyPos;
            sal_uLong i;
            for (i = 0, nPolyPos = 0; i < (mnCount - 1); i++)
            {
                const sal_uInt8 cMove = mpCodes[i];
                const sal_uInt8 cNextMove = mpCodes[i + 1];
                const vcl::Vectorizer::ChainMove& rMove = aMove[cMove];
                const vcl::Vectorizer::ChainMove& rMoveOuter = aMoveOuter[cMove];
                bool bDone = true;

                nLastX += rMove.nDX;
                nLastY += rMove.nDY;

                if (cMove < 4)
                {
                    if ((cMove == 0 && cNextMove == 1) || (cMove == 1 && cNextMove == 2)
                        || (cMove == 2 && cNextMove == 3) || (cMove == 3 && cNextMove == 0))
                    {
                    }
                    else if (cMove == 0 && cNextMove == 3)
                    {
                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY);
                    }
                    else if (cMove == 3 && cNextMove == 2)
                    {
                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY);

                        aArr[nPolyPos].setX(nLastX + 1);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY + 1);
                    }
                    else if (cMove == 2 && cNextMove == 1)
                    {
                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY + 1);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY);
                    }
                    else if (cMove == 1 && cNextMove == 0)
                    {
                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY);

                        aArr[nPolyPos].setX(nLastX - 1);
                        aArr[nPolyPos++].setY(nLastY - 1);

                        aArr[nPolyPos].setX(nLastX);
                        aArr[nPolyPos++].setY(nLastY - 1);
                    }
                    else
                        bDone = false;
                }
                else if (cMove == 7 && cNextMove == 3)
                {
                    aArr[nPolyPos].setX(nLastX);
                    aArr[nPolyPos++].setY(nLastY - 1);

                    aArr[nPolyPos].setX(nLastX + 1);
                    aArr[nPolyPos++].setY(nLastY);
                }
                else if (cMove == 6 && cNextMove == 2)
                {
                    aArr[nPolyPos].setX(nLastX + 1);
                    aArr[nPolyPos++].setY(nLastY);

                    aArr[nPolyPos].setX(nLastX);
                    aArr[nPolyPos++].setY(nLastY + 1);
                }
                else
                    bDone = false;

                if (!bDone)
                {
                    aArr[nPolyPos].setX(nLastX + rMoveOuter.nDX);
                    aArr[nPolyPos++].setY(nLastY + rMoveOuter.nDY);
                }
            }

            aArr[nPolyPos].setX(nFirstX - 1);
            aArr[nPolyPos++].setY(nFirstY - 1);
            aArr.SetRealSize(nPolyPos);
        }
        else
        {
            tools::Long nLastX = maStartPt.X(), nLastY = maStartPt.Y();

            aArr.SetSize(mnCount + 1);
            aArr[0] = Point(nLastX, nLastY);

            for (sal_uLong i = 0; i < mnCount;)
            {
                const vcl::Vectorizer::ChainMove& rMove = aMove[mpCodes[i]];
                nLastX += rMove.nDX;
                nLastY += rMove.nDY;
                aArr[++i] = Point(nLastX, nLastY);
            }

            aArr.SetRealSize(mnCount + 1);
        }

        PostProcess(aArr);
    }
    else
        maPoly.SetSize(0);
}

void Chain::PostProcess(const vcl::Vectorizer::PointArray& rArr)
{
    vcl::Vectorizer::PointArray aNewArr1;
    vcl::Vectorizer::PointArray aNewArr2;
    Point* pLast;
    Point* pLeast;
    sal_uLong nNewPos;
    sal_uLong nCount = rArr.GetRealSize();
    sal_uLong n;

    // pass 1
    aNewArr1.SetSize(nCount);
    pLast = &(aNewArr1[0]);
    pLast->setX(vcl::Vectorizer::BACK_MAP(rArr[0].X()));
    pLast->setY(vcl::Vectorizer::BACK_MAP(rArr[0].Y()));

    for (n = nNewPos = 1; n < nCount;)
    {
        const Point& rPt = rArr[n++];
        const tools::Long nX = vcl::Vectorizer::BACK_MAP(rPt.X());
        const tools::Long nY = vcl::Vectorizer::BACK_MAP(rPt.Y());

        if (nX != pLast->X() || nY != pLast->Y())
        {
            pLast = pLeast = &(aNewArr1[nNewPos++]);
            pLeast->setX(nX);
            pLeast->setY(nY);
        }
    }

    nCount = nNewPos;
    aNewArr1.SetRealSize(nCount);

    // pass 2
    aNewArr2.SetSize(nCount);
    pLast = &(aNewArr2[0]);
    *pLast = aNewArr1[0];

    for (n = nNewPos = 1; n < nCount;)
    {
        pLeast = &(aNewArr1[n++]);

        if (pLeast->X() == pLast->X())
        {
            while (n < nCount && aNewArr1[n].X() == pLast->X())
                pLeast = &(aNewArr1[n++]);
        }
        else if (pLeast->Y() == pLast->Y())
        {
            while (n < nCount && aNewArr1[n].Y() == pLast->Y())
                pLeast = &(aNewArr1[n++]);
        }

        pLast = pLeast;
        aNewArr2[nNewPos++] = *pLast;
    }

    aNewArr2.SetRealSize(nNewPos);
    aNewArr2.CreatePoly(maPoly);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
