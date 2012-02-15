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



#ifndef _SV_SALCOLORUTILS_HXX
#define _SV_SALCOLORUTILS_HXX

#ifndef _LIMITS_H
    #include <limits.h>
#endif

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

#include "vcl/salbtype.hxx"
#include "vcl/salgtype.hxx"
#include "salconst.h"
#include "salmathutils.hxx"

// ------------------------------------------------------------------

SalColor GetSalColor( const float* pQuartzColor );

void SetSalColor( const SalColor& rColor, float* pQuartzColor );

// ------------------------------------------------------------------

#endif  // _SV_SALCOLORUTILS_HXX
