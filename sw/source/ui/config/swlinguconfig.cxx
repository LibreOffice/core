/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swlinguconfig.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:29:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif

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
