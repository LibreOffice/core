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
#include <math.h>
#include <string.h>

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

        template < unsigned int _RowSize > class ImplMatLine
        {
            enum { RowSize = _RowSize };

            double                                          mfValue[RowSize];

        public:
            ImplMatLine()
            {
            }

            explicit ImplMatLine(sal_uInt16 nRow, ImplMatLine< RowSize >* pToBeCopied = 0L)
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

        template < unsigned int _RowSize > class ImplHomMatrixTemplate
        {
            enum { RowSize = _RowSize };

            ImplMatLine< RowSize >                          maLine[RowSize - 1];
            ImplMatLine< RowSize >*                         mpLine;

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
                delete ((ImplHomMatrixTemplate< RowSize >*)this)->mpLine;
                ((ImplHomMatrixTemplate< RowSize >*)this)->mpLine = 0L;

                return true;
            }

            ImplHomMatrixTemplate()
                :   mpLine(0L)
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
                :   mpLine(0L)
            {
                // complete initialization using copy
                for(sal_uInt16 a(0); a < (RowSize - 1); a++)
                {
                    memcpy(&maLine[a], &rToBeCopied.maLine[a], sizeof(ImplMatLine< RowSize >));
                }

                if(rToBeCopied.mpLine)
                {
                    mpLine = new ImplMatLine< RowSize >((RowSize - 1), rToBeCopied.mpLine);
                }
            }

            ~ImplHomMatrixTemplate()
            {
                if(mpLine)
                {
                    delete mpLine;
                }
            }

            sal_uInt16 getEdgeLength() const { return RowSize; }

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
                        mpLine = new ImplMatLine< RowSize >((RowSize - 1), 0L);
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
                        delete mpLine;
                        mpLine = 0L;
                    }
                }
            }

            // Left-upper decompositon
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

            bool isNormalized() const
            {
                if(!mpLine)
                    return true;

                const double fHomValue(get((RowSize - 1), (RowSize - 1)));

                if(::basegfx::fTools::equalZero(fHomValue))
                {
                    return true;
                }

                const double fOne(1.0);

                if(::basegfx::fTools::equal(fOne, fHomValue))
                {
                    return true;
                }

                return false;
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

            void doNormalize()
            {
                if(mpLine)
                {
                    const double fHomValue(get((RowSize - 1), (RowSize - 1)));

                    for(sal_uInt16 a(0); a < RowSize; a++)
                    {
                        for(sal_uInt16 b(0); b < RowSize; b++)
                        {
                            set(a, b, get(a, b) / fHomValue);
                        }
                    }

                    // evtl. get rid of last matrix line
                    testLastLine();
                }
            }

            double doDeterminant() const
            {
                ImplHomMatrixTemplate aWork(*this);
                sal_uInt16 nIndex[RowSize];
                sal_Int16 nParity;
                double fRetval(0.0);

                if(aWork.ludcmp(nIndex, nParity))
                {
                    fRetval = (double)nParity;

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

            double doTrace() const
            {
                double fTrace = (mpLine) ? 0.0 : 1.0;
                const sal_uInt16 nMaxLine(
                    sal::static_int_cast<sal_uInt16>(mpLine ? RowSize : (RowSize - 1)) );

                for(sal_uInt16 a(0); a < nMaxLine; a++)
                {
                    fTrace += get(a, a);
                }

                return fTrace;
            }

            void doTranspose()
            {
                for(sal_uInt16 a(0); a < (RowSize - 1); a++)
                {
                    for(sal_uInt16 b(a + 1); b < RowSize; b++)
                    {
                        const double fTemp(get(a, b));
                        set(a, b, get(b, a));
                        set(b, a, fTemp);
                    }
                }

                testLastLine();
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
