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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/salbtype.hxx"

#include "aqua/salcolorutils.hxx"

// =======================================================================

SalColor GetSalColor( const float* pQuartzColor )
{
    return MAKE_SALCOLOR( sal_uInt8( pQuartzColor[0] * 255.0), sal_uInt8(  pQuartzColor[1] * 255.0 ), sal_uInt8( pQuartzColor[2] * 255.0 ) );
}

void SetSalColor( const SalColor& rColor, float* pQuartzColor )
{
    pQuartzColor[0] = (float) SALCOLOR_RED(rColor) / 255.0;
    pQuartzColor[1] = (float) SALCOLOR_GREEN(rColor) / 255.0;
    pQuartzColor[2] = (float) SALCOLOR_BLUE(rColor) / 255.0;
    pQuartzColor[3] = 1.0;
}

// =======================================================================

