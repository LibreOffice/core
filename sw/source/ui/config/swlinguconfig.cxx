/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: swlinguconfig.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// #107253#
#include <swlinguconfig.hxx>
#include <svtools/lingucfg.hxx>

using namespace ::com::sun::star;

// init static member
static SvtLinguConfig* mpImplLinguConfig = 0L;
static sal_uInt32 mnImplUseCount = 0L;

void ImplCreateOnDemand()
{
    if(!mpImplLinguConfig && mnImplUseCount)
    {
        mpImplLinguConfig = new SvtLinguConfig();
    }
}

SwLinguConfig::SwLinguConfig()
{
    mnImplUseCount++;
}

SwLinguConfig::~SwLinguConfig()
{
    mnImplUseCount--;

    if(!mnImplUseCount && mpImplLinguConfig)
    {
        delete mpImplLinguConfig;
        mpImplLinguConfig = 0L;
    }
}

sal_Bool SwLinguConfig::SetProperty(const rtl::OUString &rPropertyName, const uno::Any &rValue)
{
    ImplCreateOnDemand();
    return mpImplLinguConfig->SetProperty(rPropertyName, rValue);
}

sal_Bool SwLinguConfig::GetOptions(SvtLinguOptions &rOptions) const
{
    ImplCreateOnDemand();
    return mpImplLinguConfig->GetOptions(rOptions);
}

uno::Any SwLinguConfig::GetProperty(const rtl::OUString &rPropertyName) const
{
    ImplCreateOnDemand();
    return mpImplLinguConfig->GetProperty(rPropertyName);
}

// eof
