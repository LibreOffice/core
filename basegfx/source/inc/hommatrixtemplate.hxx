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

#ifndef INCLUDED_BASEGFX_SOURCE_INC_HOMMATRIXTEMPLATE_HXX
#define INCLUDED_BASEGFX_SOURCE_INC_HOMMATRIXTEMPLATE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <cmath>
#include <string.h>

#include <memory>

namespace basegfx
{
    namespace internal
    {

        inline double implGetDefaultValue(sal_uInt16 nRow, sal_uInt16 nColumn)
        {
            if(nRow == nColumn)
                return 1.0;
            return 0.0;
        }

        template < sal_uInt16 RowSize > class ImplMatLine
        {
            double                                          mfValue[RowSize];

        public:
            ImplMatLine()
            {
            }

            explicit ImplMatLine(sal_uInt16 nRow, ImplMatLine< RowSize >* pToBeCopied)
            {
                if(pToBeCopied)
                {
                    memcpy(&mfValue, pToBeCopied, sizeof(double) * RowSize);
                }
                else
                {
                    for(sal_uInt16 a(0); a < RowSize; a++)
                    {
                        mfValue[a] = implGetDefaultValue(nRow, a);
                    }
                }
            }

            double get(sal_uInt16 nColumn) const
            {
                return mfValue[nColumn];
            }

            void set(sal_uInt16 nColumn, const double& rValue)
            {
                mfValue[nColumn] = rValue;
            }
        };

        template < sal_uInt16 RowSize > class ImplHomMatrixTemplate
        {
            ImplMatLine< RowSize >                          maLine[RowSize - 1];
            std::unique_ptr<ImplMatLine< RowSize >> mutable mpLine;

        public:
            // Is last line used?
            bool isLastLineDefault() const
            {
                if(!mpLine)
                    return true;

                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    const double fDefault(implGetDefaultValue((RowSize - 1), a));
                    const double fLineValue(mpLine->get(a));

                    if(!::basegfx::fTools::equal(fDefault, fLineValue))
                    {
                        return false;
                    }
                }

                // reset last line, it equals default
                mpLine.reset();

                return true;
            }

            ImplHomMatrixTemplate()
            {
                // complete initialization with identity matrix, all lines
                // were initialized with a trailing 1 followed by 0's.
                for(sal_uInt16 a(0); a < RowSize-1; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                        maLine[a].set(b, implGetDefaultValue(a, b) );
                }
            }

            ImplHomMatrixTemplate(const ImplHomMatrixTemplate& rToBeCopied)
            {
                operator=(rToBeCopied);
            }

            ImplHomMatrixTemplate& operator=(const ImplHomMatrixTemplate& rToBeCopied)
            {
                if (this != &rToBeCopied)
                {
                    // complete initialization using copy
                    for(sal_uInt16 a(0); a < (RowSize - 1); a++)
                    {
                        memcpy(&maLine[a], &rToBeCopied.maLine[a], sizeof(ImplMatLine< RowSize >));
                    }
                    if(rToBeCopied.mpLine)
                    {
                        mpLine.reset( new ImplMatLine< RowSize >((RowSize - 1), rToBeCopied.mpLine.get()) );
                    }
                }
                return *this;
            }

            static sal_uInt16 getEdgeLength() { return RowSize; }

            double get(sal_uInt16 nRow, sal_uInt16 nColumn) const
            {
                if(nRow < (RowSize - 1))
                {
                    return maLine[nRow].get(nColumn);
                }

                if(mpLine)
                {
                    return mpLine->get(nColumn);
                }

                return implGetDefaultValue((RowSize - 1), nColumn);
            }

            void set(sal_uInt16 nRow, sal_uInt16 nColumn, const double& rValue)
            {
                if(nRow < (RowSize - 1))
                {
                    maLine[nRow].set(nColumn, rValue);
                }
                else if(mpLine)
                {
                    mpLine->set(nColumn, rValue);
                }
                else
                {
                    const double fDefault(implGetDefaultValue((RowSize - 1), nColumn));

                    if(!::basegfx::fTools::equal(fDefault, rValue))
                    {
                        mpLine.reset(new ImplMatLine< RowSize >((RowSize - 1), nullptr));
                        mpLine->set(nColumn, rValue);
                    }
                }
            }

            void testLastLine()
            {
                if(mpLine)
                {
                    bool bNecessary(false);

                    for(sal_uInt16 a(0);!bNecessary && a < RowSize; a++)
                    {
                        const double fDefault(implGetDefaultValue((RowSize - 1), a));
                        const double fLineValue(mpLine->get(a));

                        if(!::basegfx::fTools::equal(fDefault, fLineValue))
                        {
                            bNecessary = true;
                        }
                    }

                    if(!bNecessary)
                    {
                        mpLine.reset();
                    }
                }
            }

            // Left-upper decomposition
            bool ludcmp(sal_uInt16 nIndex[], sal_Int16& nParity)
            {
                double fBig, fSum, fDum;
                double fStorage[RowSize];
                sal_uInt16 a, b, c;

                // #i30874# Initialize nAMax (compiler warns)
                sal_uInt16 nAMax = 0;

                nParity = 1;

                // Calc the max of each line. If a line is empty,
                // stop immediately since matrix is not invertible then.
                for(a = 0; a < RowSize; a++)
                {
                    fBig = 0.0;

                    for(b = 0; b < RowSize; b++)
                    {
                        double fTemp(fabs(get(a, b)));

                        if(::basegfx::fTools::more(fTemp, fBig))
                        {
                            fBig = fTemp;
                        }
                    }

                    if(::basegfx::fTools::equalZero(fBig))
                    {
                        return false;
                    }

                    fStorage[a] = 1.0 / fBig;
                }

                // start normalizing
                for(b = 0; b < RowSize; b++)
                {
                    for(a = 0; a < b; a++)
                    {
                        fSum = get(a, b);

                        for(c = 0; c < a; c++)
                        {
                            fSum -= get(a, c) * get(c, b);
                        }

                        set(a, b, fSum);
                    }

                    fBig = 0.0;

                    for(a = b; a < RowSize; a++)
                    {
                        fSum = get(a, b);

                        for(c = 0; c < b; c++)
                        {
                            fSum -= get(a, c) * get(c, b);
                        }

                        set(a, b, fSum);
                        fDum = fStorage[a] * fabs(fSum);

                        if(::basegfx::fTools::moreOrEqual(fDum, fBig))
                        {
                            fBig = fDum;
                            nAMax = a;
                        }
                    }

                    if(b != nAMax)
                    {
                        for(c = 0; c < RowSize; c++)
                        {
                            fDum = get(nAMax, c);
                            set(nAMax, c, get(b, c));
                            set(b, c, fDum);
                        }

                        nParity = -nParity;
                        fStorage[nAMax] = fStorage[b];
                    }

                    nIndex[b] = nAMax;

                    // here the failure of precision occurs
                    const double fValBB(fabs(get(b, b)));

                    if(::basegfx::fTools::equalZero(fValBB))
                    {
                        return false;
                    }

                    if(b != (RowSize - 1))
                    {
                        fDum = 1.0 / get(b, b);

                        for(a = b + 1; a < RowSize; a++)
                        {
                            set(a, b, get(a, b) * fDum);
                        }
                    }
                }

                return true;
            }

            void lubksb(const sal_uInt16 nIndex[], double fRow[]) const
            {
                sal_uInt16 b, ip;
                sal_Int16 a, a2 = -1;
                double fSum;

                for(a = 0; a < RowSize; a++)
                {
                    ip = nIndex[a];
                    fSum = fRow[ip];
                    fRow[ip] = fRow[a];

                    if(a2 >= 0)
                    {
                        for(b = a2; b < a; b++)
                        {
                            fSum -= get(a, b) * fRow[b];
                        }
                    }
                    else if(!::basegfx::fTools::equalZero(fSum))
                    {
                        a2 = a;
                    }

                    fRow[a] = fSum;
                }

                for(a = (RowSize - 1); a >= 0; a--)
                {
                    fSum = fRow[a];

                    for(b = a + 1; b < RowSize; b++)
                    {
                        fSum -= get(a, b) * fRow[b];
                    }

                    const double fValueAA(get(a, a));

                    if(!::basegfx::fTools::equalZero(fValueAA))
                    {
                        fRow[a] = fSum / get(a, a);
                    }
                }
            }

            bool isIdentity() const
            {
                // last line needs no testing if not existing
                const sal_uInt16 nMaxLine(
                    sal::static_int_cast<sal_uInt16>(mpLine ? RowSize : (RowSize - 1)) );

                for(sal_uInt16 a(0); a < nMaxLine; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                    {
                        const double fDefault(implGetDefaultValue(a, b));
                        const double fValueAB(get(a, b));

                        if(!::basegfx::fTools::equal(fDefault, fValueAB))
                        {
                            return false;
                        }
                    }
                }

                return true;
            }

            bool isInvertible() const
            {
                ImplHomMatrixTemplate aWork(*this);
                sal_uInt16 nIndex[RowSize];
                sal_Int16 nParity;

                return aWork.ludcmp(nIndex, nParity);
            }

            void doInvert(const ImplHomMatrixTemplate& rWork, const sal_uInt16 nIndex[])
            {
                double fArray[RowSize];

                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    // prepare line
                    sal_uInt16 b;
                    for( b = 0; b < RowSize; b++)
                    {
                        fArray[b] = implGetDefaultValue(a, b);
                    }

                    // expand line
                    rWork.lubksb(nIndex, fArray);

                    // copy line transposed to this matrix
                    for( b = 0; b < RowSize; b++)
                    {
                        set(b, a, fArray[b]);
                    }
                }

                // evtl. get rid of last matrix line
                testLastLine();
            }

            double doDeterminant() const
            {
                ImplHomMatrixTemplate aWork(*this);
                sal_uInt16 nIndex[RowSize];
                sal_Int16 nParity;
                double fRetval(0.0);

                if(aWork.ludcmp(nIndex, nParity))
                {
                    fRetval = static_cast<double>(nParity);

                    // last line needs no multiply if not existing; default value would be 1.
                    const sal_uInt16 nMaxLine(
                        sal::static_int_cast<sal_uInt16>(aWork.mpLine ? RowSize : (RowSize - 1)) );

                    for(sal_uInt16 a(0); a < nMaxLine; a++)
                    {
                        fRetval *= aWork.get(a, a);
                    }
                }

                return fRetval;
            }

            void doAddMatrix(const ImplHomMatrixTemplate& rMat)
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                    {
                        set(a, b, get(a, b) + rMat.get(a, b));
                    }
                }

                testLastLine();
            }

            void doSubMatrix(const ImplHomMatrixTemplate& rMat)
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                    {
                        set(a, b, get(a, b) - rMat.get(a, b));
                    }
                }

                testLastLine();
            }

            void doMulMatrix(const double& rfValue)
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                    {
                        set(a, b, get(a, b) * rfValue);
                    }
                }

                testLastLine();
            }

            void doMulMatrix(const ImplHomMatrixTemplate& rMat)
            {
                // create a copy as source for the original values
                const ImplHomMatrixTemplate aCopy(*this);

                // TODO: maybe optimize cases where last line is [0 0 1].

                double fValue(0.0);

                for(sal_uInt16 a(0); a < RowSize; ++a)
                {
                    for(sal_uInt16 b(0); b < RowSize; ++b)
                    {
                        fValue = 0.0;

                        for(sal_uInt16 c(0); c < RowSize; ++c)
                            fValue += aCopy.get(c, b) * rMat.get(a, c);

                        set(a, b, fValue);
                    }
                }

                testLastLine();
            }

            bool isEqual(const ImplHomMatrixTemplate& rMat) const
            {
                const sal_uInt16 nMaxLine(
                    sal::static_int_cast<sal_uInt16>((mpLine || rMat.mpLine) ? RowSize : (RowSize - 1)) );

                for(sal_uInt16 a(0); a < nMaxLine; a++)
                {
                    for(sal_uInt16 b(0); b < RowSize; b++)
                    {
                        const double fValueA(get(a, b));
                        const double fValueB(rMat.get(a, b));

                        if(!::basegfx::fTools::equal(fValueA, fValueB))
                        {
                            return false;
                        }
                    }
                }

                return true;
            }
        };

    } // namespace internal
} // namespace basegfx

#endif // INCLUDED_BASEGFX_SOURCE_INC_HOMMATRIXTEMPLATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
