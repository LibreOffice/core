/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svtdata.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SVTOOLS_SVTDATA_HXX
#define _SVTOOLS_SVTDATA_HXX

#include <tools/resid.hxx>
#include <tools/simplerm.hxx>

class ResMgr;
class SfxItemDesruptorList_Impl;
class SfxItemPool;
class Twain;

//============================================================================
class ImpSvtData
{
public:
    Twain * pTwain;
    const SfxItemPool * pStoringPool;
    SfxItemDesruptorList_Impl * pItemDesruptList;

    ResMgr *        pResMgr;
    ResMgr *        pPatchResMgr;

    void*           m_pThreadsafeRMs;
        // one SimpleResMgr for each language for which a resource was requested
        // (When using the 'non-simple' resmgr, the first request for any language wins, any
        // further request for any other language supply the resmgr of the first call.
        // For the simple resmgr we have a mgr for each language ever requested).

private:
    ImpSvtData():
        pTwain(0), pStoringPool(0), pItemDesruptList(0), pResMgr(0),
        pPatchResMgr(NULL), m_pThreadsafeRMs(NULL)
    {}

    ~ImpSvtData();

public:
    ResMgr * GetResMgr(const ::com::sun::star::lang::Locale aLocale);
    ResMgr * GetResMgr(); // VCL dependant, only available in SVT, not in SVL!

    ResMgr * GetPatchResMgr();
    ResMgr * GetPatchResMgr(const ::com::sun::star::lang::Locale& aLocale);


    SimpleResMgr * GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale);

    static ImpSvtData & GetSvtData();
};

//============================================================================

class SvpResId: public ResId
{
public:
    SvpResId( USHORT nId, const ::com::sun::star::lang::Locale aLocale ):
        ResId( nId, *ImpSvtData::GetSvtData().GetResMgr( aLocale ) ) {}

     // VCL dependant, only available in SVT, not in SVL!
    SvpResId( USHORT nId );
};


class SvtResId: public ResId
{
public:
    SvtResId(USHORT nId, const ::com::sun::star::lang::Locale aLocale):
        ResId(nId, *ImpSvtData::GetSvtData().GetResMgr(aLocale)) {}

    SvtResId(USHORT nId): ResId(nId, *ImpSvtData::GetSvtData().GetResMgr()) {}
     // VCL dependant, only available in SVT, not in SVL!
};

//============================================================================
class SvtSimpleResId
{
    String  m_sValue;

public:
    SvtSimpleResId(USHORT nId, const ::com::sun::star::lang::Locale aLocale) : m_sValue(ImpSvtData::GetSvtData().GetSimpleRM(aLocale)->ReadString(nId)) { };

    operator String () const { return m_sValue; }
};



#endif //  _SVTOOLS_SVTDATA_HXX

