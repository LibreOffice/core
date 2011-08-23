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

#ifndef _SVDFIELD_HXX
#define _SVDFIELD_HXX

#ifndef _FLDITEM_HXX
#ifndef ITEMID_FIELD
#ifndef _EDITDATA_HXX
#include <bf_svx/editdata.hxx>  /* das include wird wg. EE_FEATURE_FIELD benoetigt */
#endif
#define ITEMID_FIELD EE_FEATURE_FIELD  /* wird fuer #include <bf_svx/flditem.hxx> benoetigt */
#endif
#include <bf_svx/flditem.hxx>
#endif
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Do not remove this, it is still used in src536a!
void SdrRegisterFieldClasses();
////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDR_MEASUREFIELD 50

class SdrMeasureObj;

enum SdrMeasureFieldKind {SDRMEASUREFIELD_VALUE,SDRMEASUREFIELD_UNIT,SDRMEASUREFIELD_ROTA90BLANCS};

class SdrMeasureField: public SvxFieldData {
    SdrMeasureFieldKind eMeasureFieldKind;
public:
    SV_DECL_PERSIST1(SdrMeasureField,SvxFieldData,SDR_MEASUREFIELD)
    SdrMeasureField(): eMeasureFieldKind(SDRMEASUREFIELD_VALUE) {}
    SdrMeasureField(SdrMeasureFieldKind eNewKind): eMeasureFieldKind(eNewKind) {}
    virtual ~SdrMeasureField();
    virtual SvxFieldData* Clone() const;
    virtual int operator==(const SvxFieldData&) const;
    SdrMeasureFieldKind GetMeasureFieldKind() const { return eMeasureFieldKind; }
    void SetMeasureFieldKind(SdrMeasureFieldKind eNewKind) { eMeasureFieldKind=eNewKind; }
    void TakeRepresentation(const SdrMeasureObj& rObj, XubString& rStr) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDFIELD_HXX

