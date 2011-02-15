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

#ifndef _SVL_SVLDATA_HXX
#define _SVL_SVLDATA_HXX

#include <tools/simplerm.hxx>

class SfxItemPool;

//============================================================================
class ImpSvlData
{
public:
    const SfxItemPool * pStoringPool;
    void*           m_pThreadsafeRMs;
        // one SimpleResMgr for each language for which a resource was requested
        // (When using the 'non-simple' resmgr, the first request for any language wins, any
        // further request for any other language supply the resmgr of the first call.
        // For the simple resmgr we have a mgr for each language ever requested).

private:
    ImpSvlData():
        pStoringPool(0), m_pThreadsafeRMs(NULL)
    {}

    ~ImpSvlData();

public:
    SimpleResMgr * GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale);
    static ImpSvlData & GetSvlData();
};

//============================================================================
class SvtSimpleResId
{
    String  m_sValue;

public:
    SvtSimpleResId(sal_uInt16 nId, const ::com::sun::star::lang::Locale aLocale) : m_sValue(ImpSvlData::GetSvlData().GetSimpleRM(aLocale)->ReadString(nId)) { };

    operator String () const { return m_sValue; }
};



#endif //  _SVL_SVLDATA_HXX

