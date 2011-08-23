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

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <bf_svtools/lingucfg.hxx>
#endif
namespace binfilter {

// init static member
static SvtLinguConfig* mpImplLinguConfig = 0L;
static sal_uInt32 mnImplUseCount = 0L;

/*N*/ void ImplCreateOnDemand()
/*N*/ {
/*N*/ 	if(!mpImplLinguConfig && mnImplUseCount)
/*N*/ 	{
/*N*/ 		mpImplLinguConfig = new SvtLinguConfig();
/*N*/ 	}
/*N*/ }

/*N*/ SwLinguConfig::SwLinguConfig()
/*N*/ {
/*N*/ 	mnImplUseCount++;
/*N*/ }

/*N*/ SwLinguConfig::~SwLinguConfig()
/*N*/ {
/*N*/ 	mnImplUseCount--;
/*N*/ 
/*N*/ 	if(!mnImplUseCount && mpImplLinguConfig)
/*N*/ 	{
/*N*/ 		delete mpImplLinguConfig;
/*N*/ 		mpImplLinguConfig = 0L;
/*N*/ 	}
/*N*/ }


/*N*/ sal_Bool SwLinguConfig::GetOptions(SvtLinguOptions &rOptions) const
/*N*/ {
/*N*/ 	ImplCreateOnDemand();
/*N*/ 	return mpImplLinguConfig->GetOptions(rOptions);
/*N*/ }


// eof
}
