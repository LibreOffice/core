/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgutil.hxx,v $
 * $Revision: 1.11 $
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

// include ---------------------------------------------------------------
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
SVT_DLLPUBLIC MapUnit   FieldToMapUnit( const SfxFieldUnit eUnit );

SVT_DLLPUBLIC long      ConvertValueToMap( long nVal, SfxMapUnit eUnit );
SVT_DLLPUBLIC long      ConvertValueToUnit( long nVal, SfxMapUnit eUnit );

SVT_DLLPUBLIC void      SetMetricValue( MetricField& rField, long lCoreValue, SfxMapUnit eUnit );
SVT_DLLPUBLIC long      GetCoreValue( const MetricField& rField, SfxMapUnit eUnit );

SVT_DLLPUBLIC long  PointToTwips( long nIn );

#if 0
// to Twips
SVT_DLLPUBLIC long  CMToTwips( long nIn );
SVT_DLLPUBLIC long  MMToTwips( long nIn );
SVT_DLLPUBLIC long  InchToTwips( long nIn );
SVT_DLLPUBLIC long  PicaToTwips( long nIn );

// to CM
SVT_DLLPUBLIC long  TwipsToCM( long nIn );
SVT_DLLPUBLIC long  InchToCM( long nIn );
SVT_DLLPUBLIC long  MMToCM( long nIn );
SVT_DLLPUBLIC long  PointToCM( long nIn );
SVT_DLLPUBLIC long  PicaToCM( long nIn );

// to MM
SVT_DLLPUBLIC long  TwipsToMM( long nIn );
SVT_DLLPUBLIC long  CMToMM( long nIn );
SVT_DLLPUBLIC long  InchToMM( long nIn );
SVT_DLLPUBLIC long  PointToMM( long nIn );
SVT_DLLPUBLIC long  PicaToMM( long nIn );

// to Inch
SVT_DLLPUBLIC long  TwipsToInch(long nIn );
SVT_DLLPUBLIC long  CMToInch(long nIn );
SVT_DLLPUBLIC long  MMToInch(long nIn );
SVT_DLLPUBLIC long  PointToInch(long nIn );
SVT_DLLPUBLIC long  PicaToInch(long nIn );

// to Point
SVT_DLLPUBLIC long  TwipsToPoint(long nIn );
SVT_DLLPUBLIC long  InchToPoint(long nIn );
SVT_DLLPUBLIC long  CMToPoint(long nIn );
SVT_DLLPUBLIC long  MMToPoint(long nIn );
SVT_DLLPUBLIC long  PicaToPoint(long nIn );

// To Pica
long    TwipsToPica(long nIn );
long    InchToPica(long nIn );
long    PointToPica(long nIn );
long    CMToPica(long nIn );
long    MMToPica(long nIn );

#endif

SVT_DLLPUBLIC long  TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew );

#endif

