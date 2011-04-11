/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "svtools/svtdllapi.h"
#include <tools/resid.hxx>
#include <com/sun/star/lang/Locale.hpp>

class ResMgr;
class SfxItemDesruptorList_Impl;

//============================================================================
class ImpSvtData
{
public:
    SfxItemDesruptorList_Impl * pItemDesruptList;
    ResMgr *        pResMgr;

private:
    ImpSvtData():
        pItemDesruptList(0), pResMgr(0)
    {}

    ~ImpSvtData();

public:
    ResMgr * GetResMgr(const ::com::sun::star::lang::Locale aLocale);
    ResMgr * GetResMgr(); // VCL dependant, only available in SVT, not in SVL!

    static ImpSvtData & GetSvtData();
};

//============================================================================
class SVT_DLLPUBLIC SvtResId: public ResId
{
public:
    SvtResId(sal_uInt16 nId, const ::com::sun::star::lang::Locale aLocale);
    SvtResId(sal_uInt16 nId);
     // VCL dependant, only available in SVT, not in SVL!
};

#endif //  _SVTOOLS_SVTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
