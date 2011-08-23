/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <algorithm>
#include <iterator>

#ifndef INCLUDED_DOCTOK_EXCEPTIONS
#include <resourcemodel/exceptions.hxx>
#endif
#include <WW8PieceTableImpl.hxx>
#include <WW8Clx.hxx>

namespace writerfilter {
namespace doctok
{
using namespace ::std;

ostream & operator << (ostream & o, const WW8PieceTable & rPieceTable)
{
    rPieceTable.dump(o);
        
    return o;
}

WW8PieceTableImpl::WW8PieceTableImpl(WW8Stream & rStream, 
                                     sal_uInt32 nOffset,
                                     sal_uInt32 nCount)
{
    WW8Clx aClx(rStream, nOffset, nCount);

    sal_uInt32 nPieceCount = aClx.getPieceCount();

    if (nPieceCount > 0)
    {
        for (sal_uInt32 n = 0; n < nPieceCount;  n++)
        {
            Cp aCp(aClx.getCp(n));
            Fc aFc(aClx.getFc(n), aClx.isComplexFc(n));
                
            CpAndFc aCpAndFc(aCp, aFc, PROP_DOC);

            mEntries.push_back(aCpAndFc);
        }
            
        CpAndFc aBack = mEntries.back();
        Cp aCp(aClx.getCp(aClx.getPieceCount()));
        Fc aFc(aBack.getFc() + (aCp - aBack.getCp()));
            
        CpAndFc aCpAndFc(aCp, aFc, PROP_DOC);

        mEntries.push_back(aCpAndFc);
    }
}

sal_uInt32 WW8PieceTableImpl::getCount() const
{
    return mEntries.size();
}

WW8PieceTableImpl::tEntries::const_iterator 
WW8PieceTableImpl::findCp(const Cp & rCp) const
{
    tEntries::const_iterator aResult = mEntries.end();
    tEntries::const_iterator aEnd = mEntries.end();

    for (tEntries::const_iterator aIt = mEntries.begin(); aIt != aEnd; 
         aIt++)
    {
        if (aIt->getCp() <= rCp)
        {
            aResult = aIt;
                
            //break;
        }
    }

    return aResult;
}

WW8PieceTableImpl::tEntries::const_iterator 
WW8PieceTableImpl::findFc(const Fc & rFc) const
{
    tEntries::const_iterator aResult = mEntries.end();
    tEntries::const_iterator aEnd = mEntries.end();

    if (mEntries.size() > 0)
    {
        if (rFc < mEntries.begin()->getFc())
            aResult = mEntries.begin();
        else
        {
            for (tEntries::const_iterator aIt = mEntries.begin(); 
                 aIt != aEnd; aIt++)
            {
                if (aIt->getFc() <= rFc)
                {
                    tEntries::const_iterator aItNext = aIt;
                    aItNext++;

                    if (aItNext != aEnd)
                    {
                        sal_uInt32 nOffset = rFc.get() - aIt->getFc().get();
                        sal_uInt32 nLength = aItNext->getCp() - aIt->getCp();

                        if (! aIt->isComplex())
                            nLength *= 2;

                        if (nOffset < nLength)
                        {
                            aResult = aIt;

                            break;
                        }
                    }
                    
                }
            }
        }
    }

    return aResult;
}

Cp WW8PieceTableImpl::getFirstCp() const
{
    Cp aResult;

    if (getCount() > 0)
        aResult = getCp(0);
    else
        throw ExceptionNotFound("WW8PieceTableImpl::getFirstCp");

    return aResult;
}

Fc WW8PieceTableImpl::getFirstFc() const
{
    Fc aResult;

    if (getCount() > 0)
        aResult = getFc(0);
    else
        throw ExceptionNotFound(" WW8PieceTableImpl::getFirstFc");

    return aResult;
}    

Cp WW8PieceTableImpl::getLastCp() const
{
    Cp aResult;

    if (getCount() > 0)
        aResult = getCp(getCount() - 1);
    else
        throw ExceptionNotFound("WW8PieceTableImpl::getLastCp");

    return aResult;
}

Fc WW8PieceTableImpl::getLastFc() const
{
    Fc aResult;

    if (getCount() > 0)
        aResult = getFc(getCount() - 1);
    else
        throw ExceptionNotFound("WW8PieceTableImpl::getLastFc");

    return aResult;
}    

Cp WW8PieceTableImpl::getCp(sal_uInt32 nIndex) const
{
    return mEntries[nIndex].getCp();
}

Fc WW8PieceTableImpl::getFc(sal_uInt32 nIndex) const
{
    return mEntries[nIndex].getFc();
}

Cp WW8PieceTableImpl::fc2cp(const Fc & rFc) const
{
    Cp cpResult;

    if (mEntries.size() > 0)
    {
        Fc aFc;

        if (rFc < mEntries.begin()->getFc())
            aFc = mEntries.begin()->getFc();
        else
            aFc = rFc;

        tEntries::const_iterator aIt = findFc(aFc);
            
        if (aIt != mEntries.end())
        {
            cpResult = aIt->getCp() + (aFc - aIt->getFc());
        }
        else
            throw ExceptionNotFound("WW8PieceTableImpl::fc2cp: " + aFc.toString());
    }

    return cpResult;
}
    
Fc WW8PieceTableImpl::cp2fc(const Cp & rCp) const
{
    Fc aResult;

    Cp2FcHashMap_t::iterator aItCp = mCp2FcCache.find(rCp);

    if (aItCp == mCp2FcCache.end())
    {
        tEntries::const_iterator aIt = findCp(rCp);
        
        if (aIt != mEntries.end())
        {
            aResult = aIt->getFc() + (rCp - aIt->getCp());
            mCp2FcCache[rCp] = aResult;
        }
        else
            throw ExceptionNotFound
                ("WW8PieceTableImpl::cp2fc: " + rCp.toString());
    }
    else
        aResult = mCp2FcCache[rCp];

    return aResult;
}

bool WW8PieceTableImpl::isComplex(const Cp & rCp) const
{
    bool bResult = false;

    tEntries::const_iterator aIt = findCp(rCp);

    if (aIt != mEntries.end())
        bResult = aIt->isComplex();

    return bResult;
}

bool WW8PieceTableImpl::isComplex(const Fc & rFc) const
{
    bool bResult = false;

    tEntries::const_iterator aIt = findFc(rFc);

    if (aIt != mEntries.end())
        bResult = aIt->isComplex();
        
    return bResult;
}

CpAndFc WW8PieceTableImpl::createCpAndFc
(const Cp & rCp, PropertyType eType) const
{
    return CpAndFc(rCp, cp2fc(rCp), eType);
}

CpAndFc WW8PieceTableImpl::createCpAndFc
(const Fc & rFc, PropertyType eType) const
{
    return CpAndFc(fc2cp(rFc), rFc, eType);
}

void WW8PieceTableImpl::dump(ostream & o) const
{
    o << "<piecetable>" << endl;
    copy(mEntries.begin(), mEntries.end(), ostream_iterator<CpAndFc>(o, "\n"));
    o << "</piecetable>" << endl;
}
}}
