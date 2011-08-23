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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <bf_sfx2/sfxsids.hrc>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <bf_svtools/itempool.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif


#ifndef CONFIG_HXX
#include "config.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif
namespace binfilter {

/////////////////////////////////////////////////////////////////

/*N*/ SmConfig::SmConfig()
/*N*/ {
/*N*/ }


/*N*/ SmConfig::~SmConfig()
/*N*/ {
/*N*/ }




/*N*/ void SmConfig::ConfigToItemSet(SfxItemSet &rSet) const
/*N*/ {
/*N*/ 	const SfxItemPool *pPool = rSet.GetPool();
/*N*/
/*N*/ 	rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTSIZE),
/*N*/                            (UINT16) GetPrintSize()));
/*N*/ 	rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTZOOM),
/*N*/                            (UINT16) GetPrintZoomFactor()));
/*N*/
/*N*/     rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTITLE), IsPrintTitle()));
/*N*/     rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTEXT),  IsPrintFormulaText()));
/*N*/     rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTFRAME), IsPrintFrame()));
/*N*/     rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTOREDRAW), IsAutoRedraw()));
/*N*/     rSet.Put(SfxBoolItem(pPool->GetWhich(SID_NO_RIGHT_SPACES), IsIgnoreSpacesRight()));
/*N*/ }


/////////////////////////////////////////////////////////////////


}
