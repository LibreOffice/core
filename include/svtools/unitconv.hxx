/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVTOOLS_UNITCONV_HXX
#define INCLUDED_SVTOOLS_UNITCONV_HXX

#include <svtools/svtdllapi.h>
#include <tools/mapunit.hxx>
#include <tools/fldunit.hxx>

namespace weld { class MetricSpinButton; }

class MetricField;

// macro -----------------------------------------------------------------

// typedef ---------------------------------------------------------------

typedef long (*FUNC_CONVERT)(long);

// Functions -------------------------------------------------------------

SVT_DLLPUBLIC void      SetFieldUnit(weld::MetricSpinButton& rCtrl, FieldUnit eUnit, bool bAll = false);
SVT_DLLPUBLIC void      SetFieldUnit( MetricField& rCtrl, FieldUnit eUnit, bool bAll = false );

SVT_DLLPUBLIC long      CalcToUnit( float nIn, MapUnit eUnit );
SVT_DLLPUBLIC long      CalcToPoint( long nIn, MapUnit eUnit, sal_uInt16 nFactor );

SVT_DLLPUBLIC long      ItemToControl( long nIn, MapUnit eItem, FieldUnit eCtrl );
SVT_DLLPUBLIC long      ControlToItem( long nIn, FieldUnit eCtrl, MapUnit eItem );

SVT_DLLPUBLIC FieldUnit MapToFieldUnit( const MapUnit eUnit );

SVT_DLLPUBLIC void      SetMetricValue(weld::MetricSpinButton& rField, int lCoreValue, MapUnit eUnit);
SVT_DLLPUBLIC void      SetMetricValue( MetricField& rField, long lCoreValue, MapUnit eUnit );
SVT_DLLPUBLIC int       GetCoreValue(const weld::MetricSpinButton& rField, MapUnit eUnit);
SVT_DLLPUBLIC long      GetCoreValue( const MetricField& rField, MapUnit eUnit );

SVT_DLLPUBLIC long  PointToTwips( long nIn );

SVT_DLLPUBLIC long  TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
