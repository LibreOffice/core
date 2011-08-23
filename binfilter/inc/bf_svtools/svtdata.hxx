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

#ifndef _SVTOOLS_SVTDATA_HXX
#define _SVTOOLS_SVTDATA_HXX

#include <com/sun/star/lang/Locale.hpp>
#include <tools/resid.hxx>

class ResMgr;

namespace binfilter
{

class SfxItemDesruptorList_Impl;
class SfxItemPool;

//============================================================================
class ImpSvtData
{
public:
    const SfxItemPool * pStoringPool;
    SfxItemDesruptorList_Impl * pItemDesruptList;

    ResMgr *		pResMgr;
private:
    ImpSvtData():
        pStoringPool(0), pItemDesruptList(0), pResMgr(0)
    {}

    ~ImpSvtData();

public:
    ResMgr * GetResMgr(const ::com::sun::star::lang::Locale aLocale);
    ResMgr * GetResMgr(); // VCL dependant, only available in SVT, not in SVL!

    static ImpSvtData & GetSvtData();
};

class SvtResId: public ResId
{
public:
    SvtResId(USHORT nId, const ::com::sun::star::lang::Locale aLocale):
        ResId(nId, *ImpSvtData::GetSvtData().GetResMgr(aLocale)) {}

    SvtResId(USHORT nId): ResId(nId, *ImpSvtData::GetSvtData().GetResMgr()) {}
     // VCL dependant, only available in SVT, not in SVL!
};

#define SvpResId SvtResId

}

#endif //  _SVTOOLS_SVTDATA_HXX

