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

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <cassert>
#include <cmath>

namespace basegfx::internal
    {

        inline constexpr double implGetDefaultValue(sal_uInt16 nRow, sal_uInt16 nColumn)
        {
            if(nRow == nColumn)
                return 1.0;
            return 0.0;
        }

        template < sal_uInt16 RowSize > class ImplMatLine
        {
            double                                          mfValue[RowSize];

        public:
            ImplMatLine() = default;

            explicit ImplMatLine(sal_uInt16 nRow)
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    mfValue[a] = implGetDefaultValue(nRow, a);
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
            ImplMatLine< RowSize >                          maLine[RowSize];

        public:
            // Is last line used?
            bool isLastLineDefault() const
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
                {
                    const double fDefault(implGetDefaultValue((RowSize - 1), a));
                    const double fLineValue(maLine[RowSize-1].get(a));

                    if(fDefault != fLineValue)
                    {
                        return false;
                    }
                }
                return true;
            }

            ImplHomMatrixTemplate()
            {
                // complete initialization with identity matrix, all lines
                // were initialized with a trailing 1 followed by 0's.
                for(sal_uInt16 a(0); a < RowSize; a++)
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
                    for(sal_uInt16 a(0); a < RowSize; a++)
                    {
                        maLine[a] = rToBeCopied.maLine[a];
                    }
                }
                return *this;
            }

            static sal_uInt16 getEdgeLength() { return RowSize; }

            double get(sal_uInt16 nRow, sal_uInt16 nColumn) const
            {
                return maLine[nRow].get(nColumn);
            }

            void set(sal_uInt16 nRow, sal_uInt16 nColumn, const double& rValue)
            {
                maLine[nRow].set(nColumn, rValue);
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

                    assert(fBig != 0 && "help coverity see it's not zero");
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
                for(sal_uInt16 a(0); a < RowSize; a++)
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
                    for(sal_uInt16 a(0); a < RowSize; a++)
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
            }

            bool isEqual(const ImplHomMatrixTemplate& rMat) const
            {
                for(sal_uInt16 a(0); a < RowSize; a++)
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

} // namespace basegfx::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
