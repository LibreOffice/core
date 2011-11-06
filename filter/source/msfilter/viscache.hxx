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
    void    Write( SvStream & rStm );
};


