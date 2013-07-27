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

#ifndef _SV_DIBTOOLS_HXX
#define _SV_DIBTOOLS_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <tools/rc.hxx>
#include <vcl/region.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SvStream;
class BitmapEx;
class Bitmap;

//////////////////////////////////////////////////////////////////////////////

bool VCL_DLLPUBLIC ReadDIB( // ReadDIB(rBitmap, rIStm, true);
    Bitmap& rTarget,
    SvStream& rIStm,
    bool bFileHeader);

bool VCL_DLLPUBLIC ReadDIBBitmapEx(
    BitmapEx& rTarget,
    SvStream& rIStm);

bool VCL_DLLPUBLIC ReadDIBV5(
    Bitmap& rTarget,
    Bitmap& rTargetAlpha,
    SvStream& rIStm);

//////////////////////////////////////////////////////////////////////////////

bool VCL_DLLPUBLIC WriteDIB( // WriteDIB(rBitmap, rOStm, false, true);
    const Bitmap& rSource,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader);

bool VCL_DLLPUBLIC WriteDIBBitmapEx(
    const BitmapEx& rSource,
    SvStream& rOStm);

bool VCL_DLLPUBLIC WriteDIBV5(
    const Bitmap& rSource,
    const Bitmap& rSourceAlpha,
    SvStream& rOStm);

//////////////////////////////////////////////////////////////////////////////

#endif // _SV_DIBTOOLS_HXX

//////////////////////////////////////////////////////////////////////////////
// eof

