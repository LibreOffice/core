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
#pragma once

#include <svtools/svtdllapi.h>
#include <tools/mapunit.hxx>
#include <tools/fldunit.hxx>
#include <tools/long.hxx>

namespace weld { class MetricSpinButton; }

// macro -----------------------------------------------------------------

// typedef ---------------------------------------------------------------

typedef tools::Long (*FUNC_CONVERT)(tools::Long);

// Functions -------------------------------------------------------------

SVT_DLLPUBLIC void      SetFieldUnit(weld::MetricSpinButton& rCtrl, FieldUnit eUnit, bool bAll = false);

SVT_DLLPUBLIC tools::Long      CalcToUnit( float nIn, MapUnit eUnit );
SVT_DLLPUBLIC tools::Long      CalcToPoint( tools::Long nIn, MapUnit eUnit, sal_uInt16 nFactor );

SVT_DLLPUBLIC tools::Long      ItemToControl( tools::Long nIn, MapUnit eItem, FieldUnit eCtrl );
SVT_DLLPUBLIC tools::Long      ControlToItem( tools::Long nIn, FieldUnit eCtrl, MapUnit eItem );

SVT_DLLPUBLIC FieldUnit MapToFieldUnit( const MapUnit eUnit );

SVT_DLLPUBLIC void      SetMetricValue(weld::MetricSpinButton& rField, int lCoreValue, MapUnit eUnit);
SVT_DLLPUBLIC int       GetCoreValue(const weld::MetricSpinButton& rField, MapUnit eUnit);

SVT_DLLPUBLIC tools::Long  TransformMetric( tools::Long nVal, FieldUnit aOld, FieldUnit aNew );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
