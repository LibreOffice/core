/*************************************************************************
 *
 *  $RCSfile: hommatrixtemplate.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: thb $ $Date: 2003-09-26 07:54:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _HOMMATRIX_TEMPLATE_HXX
#define _HOMMATRIX_TEMPLATE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _FTOOLS_HXX
#include <ftools.hxx>
#endif

#include <math.h>
#include <string.h>

inline double implGetDefaultValue(sal_uInt16 nRow, sal_uInt16 nColumn)
{
    if(nRow == nColumn)
        return 1.0;
    return 0.0;
}

template < int _RowSize > class ImplMatLine
{
    enum { RowSize = _RowSize };

    double                                          mfValue[RowSize];

public:
    ImplMatLine(sal_uInt16 nRow = 0L, ImplMatLine< RowSize >* pToBeCopied = 0L)
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

template < int _RowSize > class ImplHomMatrixTemplate
{
    enum { RowSize = _RowSize };

    sal_uInt32                                      mnRefCount;

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

            if(!fTools::equal(fDefault, fLineValue))
            {
                return false;
            }
        }

        // reset last line, it equals default
        delete ((ImplHomMatrixTemplate< RowSize >*)this)->mpLine;
        ((ImplHomMatrixTemplate< RowSize >*)this)->mpLine = 0L;

        return true;
    }

    // This constructor is only used form the static identity matrix, thus
    // the RefCount is set to 1 to never 'delete' this static incarnation.
    ImplHomMatrixTemplate()
    :   mnRefCount(1),
        mpLine(0L)
    {
        // complete initialization with identity matrix, all lines
        // were initialized with a trailing 1 followed by 0's.
        for(sal_uInt16 a(1); a < (RowSize - 1); a++)
        {
            maLine[a].set(0, 0.0);
            maLine[a].set(a, 1.0);
        }
    }

    ImplHomMatrixTemplate(const ImplHomMatrixTemplate& rToBeCopied)
    :   mnRefCount(0),
        mpLine(0L)
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

    const sal_uInt32 getRefCount() const { return mnRefCount; }
    void incRefCount() { mnRefCount++; }
    void decRefCount() { mnRefCount--; }

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

            if(!fTools::equal(fDefault, rValue))
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

                if(!fTools::equal(fDefault, fLineValue))
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
        sal_uInt16 a, b, c, nAMax;

        nParity = 1;

        // Calc the max of each line. If a line is empty,
        // stop immediately since matrix is not invertible then.
        for(a = 0; a < RowSize; a++)
        {
            fBig = 0.0;

            for(b = 0; b < RowSize; b++)
            {
                double fTemp(fabs(get(a, b)));

                if(fTools::more(fTemp, fBig))
                {
                    fBig = fTemp;
                }
            }

            if(fTools::equalZero(fBig))
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

                if(fTools::moreOrEqual(fDum, fBig))
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

            if(fTools::equalZero(fValBB))
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
            else if(!fTools::equalZero(fSum))
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

            if(!fTools::equalZero(fValueAA))
            {
                fRow[a] = fSum / get(a, a);
            }
        }
    }

    bool isIdentity() const
    {
        // last line needs no testing if not existing
        const sal_uInt16 nMaxLine = (mpLine) ? RowSize : (RowSize - 1);

        for(sal_uInt16 a(0); a < nMaxLine; a++)
        {
            for(sal_uInt16 b(0); b < RowSize; b++)
            {
                const double fDefault(implGetDefaultValue(a, b));
                const double fValueAB(get(a, b));

                if(!fTools::equal(fDefault, fValueAB))
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

        if(fTools::equalZero(fHomValue))
        {
            return true;
        }

        const double fOne(1.0);

        if(fTools::equal(fOne, fHomValue))
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
            for(sal_uInt16 b(0); b < RowSize; b++)
            {
                fArray[b] = implGetDefaultValue(a, b);
            }

            // expand line
            rWork.lubksb(nIndex, fArray);

            // copy line transposed to this matrix
            for(b = 0; b < RowSize; b++)
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
            const sal_uInt16 nMaxLine = (aWork.mpLine) ? RowSize : (RowSize - 1);

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
        const sal_uInt16 nMaxLine = (mpLine) ? RowSize : (RowSize - 1);

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

    bool isEqual(const ImplHomMatrixTemplate& rMat)
    {
        const sal_uInt16 nMaxLine = (mpLine || rMat.mpLine) ? RowSize : (RowSize - 1);

        for(sal_uInt16 a(0); a < nMaxLine; a++)
        {
            for(sal_uInt16 b(0); b < RowSize; b++)
            {
                const double fValueA(get(a, b));
                const double fValueB(rMat.get(a, b));

                if(!fTools::equal(fValueA, fValueB))
                {
                    return false;
                }
            }
        }

        return true;
    }
};

#endif // _HOMMATRIX_TEMPLATE_HXX
