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
#ifndef _SVX_SRCHCFG_HXX
#define _SVX_SRCHCFG_HXX

#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/svldllapi.h>

//-----------------------------------------------------------------------------
struct SvxSearchConfig_Impl;
struct SvxSearchEngineData
{
    rtl::OUString   sEngineName;

    rtl::OUString   sAndPrefix;
    rtl::OUString   sAndSuffix;
    rtl::OUString   sAndSeparator;
    sal_Int32       nAndCaseMatch;

    rtl::OUString   sOrPrefix;
    rtl::OUString   sOrSuffix;
    rtl::OUString   sOrSeparator;
    sal_Int32       nOrCaseMatch;

    rtl::OUString   sExactPrefix;
    rtl::OUString   sExactSuffix;
    rtl::OUString   sExactSeparator;
    sal_Int32       nExactCaseMatch;

    SvxSearchEngineData() :
        nAndCaseMatch(0),
        nOrCaseMatch(0),
        nExactCaseMatch(0){}

    sal_Bool operator==(const SvxSearchEngineData& rData);
};
class SVL_DLLPUBLIC SvxSearchConfig : public utl::ConfigItem
{
    SvxSearchConfig_Impl* pImpl;

public:
    SvxSearchConfig(sal_Bool bEnableNotify = sal_True);
    virtual ~SvxSearchConfig();

    void            Load();
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    sal_uInt16                  Count();
    const SvxSearchEngineData&  GetData(sal_uInt16 nPos);
    const SvxSearchEngineData*  GetData(const rtl::OUString& rEngineName);
    void                        SetData(const SvxSearchEngineData& rData);
    void                        RemoveData(const rtl::OUString& rEngineName);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
