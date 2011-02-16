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

#ifndef _EDIT_MEASFIELD_HXX
#define _EDIT_MEASFIELD_HXX

#include <editeng/eeitem.hxx>

#ifndef _FLDITEM_HXX
#ifndef ITEMID_FIELD
#include <editeng/editdata.hxx>  /* this include is needed due to EE_FEATURE_FIELD */
#define ITEMID_FIELD EE_FEATURE_FIELD  /* is needed for #include <editeng/flditem.hxx> */
#endif
#include <editeng/flditem.hxx>
#endif
#include "editeng/editengdllapi.h"

#define SDR_MEASUREFIELD 50

class SdrMeasureObj;

enum SdrMeasureFieldKind {SDRMEASUREFIELD_VALUE,SDRMEASUREFIELD_UNIT,SDRMEASUREFIELD_ROTA90BLANCS};

class EDITENG_DLLPUBLIC SdrMeasureField: public SvxFieldData {
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
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
