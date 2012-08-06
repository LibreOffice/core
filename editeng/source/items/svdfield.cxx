/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include <editeng/measfld.hxx>

SV_IMPL_PERSIST1(SdrMeasureField,SvxFieldData);

SdrMeasureField::~SdrMeasureField()
{
}

SvxFieldData* SdrMeasureField::Clone() const
{
    return new SdrMeasureField(*this);
}

int SdrMeasureField::operator==(const SvxFieldData& rSrc) const
{
    return eMeasureFieldKind==((SdrMeasureField&)rSrc).GetMeasureFieldKind();
}

void SdrMeasureField::Load(SvPersistStream& rIn)
{
    sal_uInt16 nFieldKind;
    rIn>>nFieldKind;
    eMeasureFieldKind=(SdrMeasureFieldKind)nFieldKind;
}

void SdrMeasureField::Save(SvPersistStream& rOut)
{
    rOut<<(sal_uInt16)eMeasureFieldKind;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
