/*************************************************************************
 *
 *  $RCSfile: viscache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 17:52:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    void *  pJob;
    Size    aSize;      // Groesse in 100TH_MM
public:
                    Impl_OlePres( ULONG nF )
                        : nFormat( nF )
                        , nAdvFlags( 0x2 ) // in Dokument gefunden
                        , nJobLen( 0 )
                        , pJob( NULL )
                        , pBmp( NULL )
                        , pMtf( NULL )
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


