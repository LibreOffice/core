/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salbmp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:05:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALBMP_HXX
#define _SV_SALBMP_HXX

#ifndef _TL_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
