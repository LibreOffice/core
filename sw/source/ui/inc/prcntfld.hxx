/*************************************************************************
 *
 *  $RCSfile: prcntfld.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-03-15 10:44:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _PRCNTFLD_HXX
#define _PRCNTFLD_HXX

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class PercentField : public MetricField
{
    long        nRefValue;      // 100%-Wert fuer Umrechnung (in Twips)
    long        nOldMax, nOldMin, nOldSpinSize, nOldBaseValue;
    long        nLastPercent, nLastValue;
    USHORT      nOldDigits;
    FieldUnit   eOldUnit;

    long        ImpPower10(USHORT n);

    virtual void  SetValue(long nNewValue, FieldUnit eInUnit = FUNIT_NONE);
public:
    PercentField( Window* pWin, const ResId& rResId );

    void        SetPrcntValue(long nNewValue, FieldUnit eInUnit = FUNIT_NONE);
    void        SetUserValue(long nNewValue, FieldUnit eInUnit = FUNIT_NONE);
    void        SetBaseValue(long nNewValue, FieldUnit eInUnit = FUNIT_NONE);
    long        GetValue(FieldUnit eOutUnit = FUNIT_NONE);
    BOOL        IsValueModified();

    void        SetMax(long nNewMax, FieldUnit eInUnit = FUNIT_NONE);
    void        SetMin(long nNewMin, FieldUnit eInUnit = FUNIT_NONE);

    long        Normalize(long nValue);
    long        Denormalize(long nValue);

    void        SetRefValue(long nValue);
    inline long GetRefValue() const { return nRefValue; }
    long        GetRealValue(FieldUnit eOutUnit = FUNIT_NONE);

    long        Convert(long nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void        ShowPercent(BOOL bPercent);

    USHORT      GetOldDigits() const {return nOldDigits;}

};

#endif // _PRCNTFLD_HXX

