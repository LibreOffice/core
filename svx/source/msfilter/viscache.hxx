/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viscache.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:20:54 $
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
    ULONG   nFormat;
    USHORT  nAspect;
    Bitmap *        pBmp;
    GDIMetaFile *   pMtf;

    UINT32  nAdvFlags;
    INT32   nJobLen;
    BYTE*   pJob;
    Size    aSize;      // Groesse in 100TH_MM
public:
                    Impl_OlePres( ULONG nF )
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
    ULONG   GetFormat() const { return nFormat; }
    void    SetAspect( USHORT nAsp ) { nAspect = nAsp; }
    ULONG   GetAdviseFlags() const { return nAdvFlags; }
    void    SetAdviseFlags( ULONG nAdv ) { nAdvFlags = nAdv; }
    void    SetSize( const Size & rSize ) { aSize = rSize; }
            /// return FALSE => unknown format
    BOOL    Read( SvStream & rStm );
    void    Write( SvStream & rStm );
};


