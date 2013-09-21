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
#ifndef _SV_DIBTOOLS_HXX
#define _SV_DIBTOOLS_HXX

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
