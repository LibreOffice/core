/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/bitmap.hxx>

/************************************************************************
|*    Impl_CacheElement
|*    Impl_Cache
|*
|*    Beschreibung
*************************************************************************/
class Impl_OlePres
{
    sal_uLong   nFormat;
    sal_uInt16  nAspect;
    Bitmap *        pBmp;
    GDIMetaFile *   pMtf;

    sal_uInt32  nAdvFlags;
    sal_Int32   nJobLen;
    sal_uInt8*  pJob;
    Size    aSize;      // Groesse in 100TH_MM
public:
                    Impl_OlePres( sal_uLong nF )
                        : nFormat( nF )
                        , pBmp( NULL )
                        , pMtf( NULL )
                        , nAdvFlags( 0x2 )  // in Dokument gefunden
                        , nJobLen( 0 )
                        , pJob( NULL )
                    {}
                    ~Impl_OlePres()
                    {
                        delete pJob;
                        delete pBmp;
                        delete pMtf;
                    }
    void    SetMtf( const GDIMetaFile & rMtf )
            {
                if( pMtf )
                    delete pMtf;
                pMtf = new GDIMetaFile( rMtf );
            }
    Bitmap *GetBitmap() const { return pBmp; }
    GDIMetaFile *GetMetaFile() const { return pMtf; }
    sal_uLong   GetFormat() const { return nFormat; }
    void    SetAspect( sal_uInt16 nAsp ) { nAspect = nAsp; }
    sal_uLong   GetAdviseFlags() const { return nAdvFlags; }
    void    SetAdviseFlags( sal_uLong nAdv ) { nAdvFlags = nAdv; }
    void    SetSize( const Size & rSize ) { aSize = rSize; }
            /// return sal_False => unknown format
    sal_Bool    Read( SvStream & rStm );
    void    Write( SvStream & rStm );
};


