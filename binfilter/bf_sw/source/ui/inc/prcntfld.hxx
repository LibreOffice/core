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
#ifndef _PRCNTFLD_HXX
#define _PRCNTFLD_HXX

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
namespace binfilter {

class PercentField : public MetricField
{
    long		nRefValue;		// 100%-Wert fuer Umrechnung (in Twips)
    long 		nOldMax, nOldMin, nOldSpinSize, nOldBaseValue;
    long		nLastPercent, nLastValue;
    USHORT		nOldDigits;
    FieldUnit	eOldUnit;
    sal_Bool    bLockAutoCalculation; //prevent recalcution of percent values when the 
                                        //reference value is changed


public:
     PercentField( Window* pWin, const ResId& rResId );








    void        LockAutoCalculation(sal_Bool bLock) {bLockAutoCalculation = bLock;}
    sal_Bool    IsAutoCalculationLocked()const {return bLockAutoCalculation;}
};

} //namespace binfilter
#endif // _PRCNTFLD_HXX

