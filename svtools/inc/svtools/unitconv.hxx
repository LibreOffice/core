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
#ifndef _SVT_UNITCONV_HXX
#define _SVT_UNITCONV_HXX

#include <vcl/field.hxx>
#include "svtools/svtdllapi.h"
#include <svl/poolitem.hxx>

// macro -----------------------------------------------------------------

// typedef ---------------------------------------------------------------

typedef long (*FUNC_CONVERT)(long);

// Functions -------------------------------------------------------------

SVT_DLLPUBLIC void      SetFieldUnit( MetricField& rCtrl, FieldUnit eUnit, sal_Bool bAll = sal_False );
SVT_DLLPUBLIC void      SetFieldUnit( MetricBox& rCtrl, FieldUnit eUnit, sal_Bool bAll = sal_False );

SVT_DLLPUBLIC long      CalcToUnit( float nIn, SfxMapUnit eUnit );
SVT_DLLPUBLIC long      CalcToPoint( long nIn, SfxMapUnit eUnit, sal_uInt16 nFaktor );

SVT_DLLPUBLIC long      ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl );
SVT_DLLPUBLIC long      ControlToItem( long nIn, SfxFieldUnit eCtrl, SfxMapUnit eItem );

SVT_DLLPUBLIC FieldUnit MapToFieldUnit( const SfxMapUnit eUnit );

SVT_DLLPUBLIC void      SetMetricValue( MetricField& rField, long lCoreValue, SfxMapUnit eUnit );
SVT_DLLPUBLIC long      GetCoreValue( const MetricField& rField, SfxMapUnit eUnit );

SVT_DLLPUBLIC long  PointToTwips( long nIn );

SVT_DLLPUBLIC long  TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
