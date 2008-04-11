/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salbmp.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_SALBMP_HXX
#define _SV_SALBMP_HXX

#ifndef _TL_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <vcl/dllapi.h>

struct BitmapBuffer;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;

class VCL_DLLPUBLIC SalBitmap
{
public:
    SalBitmap() {}
    virtual ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    USHORT nBitCount,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    USHORT nNewBitCount ) = 0;
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual USHORT          GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;

};

#endif
