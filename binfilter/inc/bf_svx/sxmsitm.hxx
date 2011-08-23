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
#ifndef _SXMSITM_HXX
#define _SXMSITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sxsiitm.hxx>
namespace binfilter {

// Massstabsvorgabe. Wenn bereits am Model ein Massstab (UIScale) gesetzt
// ist wird dieser nicht ueberschrieben. Stattdessen werden beide Massstaebe
// miteinander multipliziert.
class SdrMeasureScaleItem: public SdrScaleItem {
public:
    SdrMeasureScaleItem()                   : SdrScaleItem(SDRATTR_MEASURESCALE,Fraction(1,1)) {}
    SdrMeasureScaleItem(const Fraction& rFr): SdrScaleItem(SDRATTR_MEASURESCALE,rFr) {}
    SdrMeasureScaleItem(SvStream& rIn)      : SdrScaleItem(SDRATTR_MEASURESCALE,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
