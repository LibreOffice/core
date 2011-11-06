/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

