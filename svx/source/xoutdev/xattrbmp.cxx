/*************************************************************************
 *
 *  $RCSfile: xattrbmp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pw $ $Date: 2000-10-12 11:54:52 $
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

#include <com/sun/star/awt/XBitmap.hpp>
#include <tools/stream.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <toolkit/unohlp.hxx>
#include "xattr.hxx"
#include "xtable.hxx"
#include "xoutx.hxx"
#include "xdef.hxx"
#include "unomid.hxx"
#include "unoprnms.hxx"
#include "unoapi.hxx"

#define GLOBALOVERFLOW

// ---------------
// class XOBitmap
// ---------------

/*************************************************************************
|*
|*    XOBitmap::XOBitmap()
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

XOBitmap::XOBitmap() :
    eType           ( XBITMAP_NONE ),
    eStyle          ( XBITMAP_STRETCH ),
    pPixelArray     ( NULL ),
    bGraphicDirty   ( FALSE )
{
}

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

XOBitmap::XOBitmap( const Bitmap& rBmp, XBitmapStyle eInStyle ) :
    eType           ( XBITMAP_IMPORT ),
    eStyle          ( eInStyle ),
    aGraphicObject  ( rBmp ),
    pPixelArray     ( NULL ),
    bGraphicDirty   ( FALSE )
{
}

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

XOBitmap::XOBitmap( const GraphicObject& rGraphicObject, XBitmapStyle eInStyle ) :
    eType           ( XBITMAP_IMPORT ),
    eStyle          ( eInStyle ),
    aGraphicObject  ( rGraphicObject ),
    pPixelArray     ( NULL ),
    bGraphicDirty   ( FALSE )
{
}

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( USHORT* pArray, const Color& aPixelColor,
|*          const Color& aBckgrColor, const Size& rSize = Size( 8, 8 ),
|*          XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

XOBitmap::XOBitmap( const USHORT* pArray, const Color& rPixelColor,
            const Color& rBckgrColor, const Size& rSize,
            XBitmapStyle eInStyle ) :
    eStyle          ( eInStyle ),
    pPixelArray     ( NULL ),
    aArraySize      ( rSize ),
    aPixelColor     ( rPixelColor ),
    aBckgrColor     ( rBckgrColor ),
    bGraphicDirty   ( TRUE )

{
    if( aArraySize.Width() == 8 && aArraySize.Height() == 8 )
    {
        eType = XBITMAP_8X8;
        pPixelArray = new USHORT[ 64 ];

        for( USHORT i = 0; i < 64; i++ )
            *( pPixelArray + i ) = *( pArray + i );
    }
    else
        DBG_ASSERT( 0, "Nicht unterstuetzte Bitmapgroesse" );
}

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( const XOBitmap& rXBmp )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

XOBitmap::XOBitmap( const XOBitmap& rXBmp ) :
    pPixelArray ( NULL )
{
    eType = rXBmp.eType;
    eStyle = rXBmp.eStyle;
    aGraphicObject = rXBmp.aGraphicObject;
    aArraySize = rXBmp.aArraySize;
    aPixelColor = rXBmp.aPixelColor;
    aBckgrColor = rXBmp.aBckgrColor;
    bGraphicDirty = rXBmp.bGraphicDirty;

    if( rXBmp.pPixelArray )
    {
        if( eType == XBITMAP_8X8 )
        {
            pPixelArray = new USHORT[ 64 ];

            for( USHORT i = 0; i < 64; i++ )
                *( pPixelArray + i ) = *( rXBmp.pPixelArray + i );
        }
    }
}

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

XOBitmap::~XOBitmap()
{
    if( pPixelArray )
        delete []pPixelArray;
}

/*************************************************************************
|*
|*    XOBitmap& XOBitmap::operator=( const XOBitmap& rXBmp )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

XOBitmap& XOBitmap::operator=( const XOBitmap& rXBmp )
{
    eType = rXBmp.eType;
    eStyle = rXBmp.eStyle;
    aGraphicObject = rXBmp.aGraphicObject;
    aArraySize = rXBmp.aArraySize;
    aPixelColor = rXBmp.aPixelColor;
    aBckgrColor = rXBmp.aBckgrColor;
    bGraphicDirty = rXBmp.bGraphicDirty;

    if( rXBmp.pPixelArray )
    {
        if( eType == XBITMAP_8X8 )
        {
            pPixelArray = new USHORT[ 64 ];

            for( USHORT i = 0; i < 64; i++ )
                *( pPixelArray + i ) = *( rXBmp.pPixelArray + i );
        }
    }
    return( *this );
}

/*************************************************************************
|*
|*    int XOBitmap::operator==( const XOBitmap& rXOBitmap ) const
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

int XOBitmap::operator==( const XOBitmap& rXOBitmap ) const
{
    if( eType != rXOBitmap.eType      ||
        eStyle != rXOBitmap.eStyle         ||
        aGraphicObject != rXOBitmap.aGraphicObject ||
        aArraySize != rXOBitmap.aArraySize     ||
        aPixelColor != rXOBitmap.aPixelColor ||
        aBckgrColor != rXOBitmap.aBckgrColor ||
        bGraphicDirty != rXOBitmap.bGraphicDirty )
    {
        return( FALSE );
    }

    if( pPixelArray && rXOBitmap.pPixelArray )
    {
        USHORT nCount = (USHORT) ( aArraySize.Width() * aArraySize.Height() );
        for( USHORT i = 0; i < nCount; i++ )
        {
            if( *( pPixelArray + i ) != *( rXOBitmap.pPixelArray + i ) )
                return( FALSE );
        }
    }
    return( TRUE );
}

/*************************************************************************
|*
|*    void SetPixelArray( const USHORT* pArray )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

void XOBitmap::SetPixelArray( const USHORT* pArray )
{
    if( eType == XBITMAP_8X8 )
    {
        if( pPixelArray )
            delete []pPixelArray;

        pPixelArray = new USHORT[ 64 ];

        for( USHORT i = 0; i < 64; i++ )
            *( pPixelArray + i ) = *( pArray + i );

        bGraphicDirty = TRUE;
    }
    else
        DBG_ASSERT( 0, "Nicht unterstuetzter Bitmaptyp" );
}

/*************************************************************************
|*
|*    Bitmap XOBitmap::GetBitmap()
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

Bitmap XOBitmap::GetBitmap() const
{
    return GetGraphicObject().GetGraphic().GetBitmap();
}

/*************************************************************************
|*
|*    Bitmap XOBitmap::GetGraphicObject()
|*
|*    Beschreibung
|*    Ersterstellung
|*    Letzte Aenderung
|*
*************************************************************************/

const GraphicObject& XOBitmap::GetGraphicObject() const
{
    if( bGraphicDirty )
        ( (XOBitmap*) this )->Array2Bitmap();

    return aGraphicObject;
}

/*************************************************************************
|*
|*    void XOBitmap::Bitmap2Array()
|*
|*    Beschreibung      Umwandlung der Bitmap in Array, Hinter- u.
|*                      Vordergrundfarbe
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

void XOBitmap::Bitmap2Array()
{
    VirtualDevice   aVD;
    BOOL            bPixelColor = FALSE;
    const Bitmap    aBitmap( GetBitmap() );
    const USHORT    nLines = 8; // von Type abhaengig

    if( !pPixelArray )
        pPixelArray = new USHORT[ nLines * nLines ];

    aVD.SetOutputSizePixel( aBitmap.GetSizePixel() );
    aVD.DrawBitmap( Point(), aBitmap );
    aPixelColor = aBckgrColor = aVD.GetPixel( Point() );

    // Aufbau des Arrays und Ermittlung der Vorder-, bzw.
    // Hintergrundfarbe
    for( USHORT i = 0; i < nLines; i++ )
    {
        for( USHORT j = 0; j < nLines; j++ )
        {
            if ( aVD.GetPixel( Point( j, i ) ) == aBckgrColor )
                *( pPixelArray + j + i * nLines ) = 0;
            else
            {
                *( pPixelArray + j + i * nLines ) = 1;
                if( !bPixelColor )
                {
                    aPixelColor = aVD.GetPixel( Point( j, i ) );
                    bPixelColor = TRUE;
                }
            }
        }
    }
}

/*************************************************************************
|*
|*    void XOBitmap::Array2Bitmap()
|*
|*    Beschreibung      Umwandlung des Arrays, Hinter- u.
|*                      Vordergrundfarbe in eine Bitmap
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

void XOBitmap::Array2Bitmap()
{
    VirtualDevice   aVD;
    BOOL            bPixelColor = FALSE;
    USHORT          nLines = 8; // von Type abhaengig

    if( !pPixelArray )
        return;

    aVD.SetOutputSizePixel( Size( nLines, nLines ) );

    // Aufbau der Bitmap
    for( USHORT i = 0; i < nLines; i++ )
    {
        for( USHORT j = 0; j < nLines; j++ )
        {
            if( *( pPixelArray + j + i * nLines ) == 0 )
                aVD.DrawPixel( Point( j, i ), aBckgrColor );
            else
                aVD.DrawPixel( Point( j, i ), aPixelColor );
        }
    }

    aGraphicObject = GraphicObject( aVD.GetBitmap( Point(), Size( nLines, nLines ) ) );
    bGraphicDirty = FALSE;
}

// -----------------------
// class XFillBitmapItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XFillBitmapItem, NameOrIndex);

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(long nIndex,
|*                                   const Bitmap& rTheBitmap)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XFillBitmapItem::XFillBitmapItem(long nIndex,
                               const XOBitmap& rTheBitmap) :
    NameOrIndex( XATTR_FILLBITMAP, nIndex ),
    aXOBitmap( rTheBitmap )
{
}

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(const XubString& rName,
|*                                 const Bitmap& rTheBitmap)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XFillBitmapItem::XFillBitmapItem(const XubString& rName,
                               const XOBitmap& rTheBitmap) :
    NameOrIndex( XATTR_FILLBITMAP, rName ),
    aXOBitmap( rTheBitmap )
{
}

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem) :
    NameOrIndex( rItem ),
    aXOBitmap( rItem.aXOBitmap )
{
}

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

XFillBitmapItem::XFillBitmapItem( SvStream& rIn, USHORT nVer ) :
    NameOrIndex( XATTR_FILLBITMAP, rIn )
{
    if( nVer == 0 )
    {
        if (!IsIndex())
        {
            // Behandlung der alten Bitmaps
            Bitmap aBmp;

            rIn >> aBmp;

            aXOBitmap.SetBitmap( aBmp );
            aXOBitmap.SetBitmapStyle( XBITMAP_TILE );

            if( aBmp.GetSizePixel().Width() == 8 &&
                aBmp.GetSizePixel().Height() == 8 )
            {
                aXOBitmap.SetBitmapType( XBITMAP_8X8 );
                aXOBitmap.Bitmap2Array();
            }
            else
                aXOBitmap.SetBitmapType( XBITMAP_IMPORT );
        }
    }
    else if( nVer == 1 )
    {
        if (!IsIndex())
        {
            INT16 iTmp;
            rIn >> iTmp;
            aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
            rIn >> iTmp;
            aXOBitmap.SetBitmapType( (XBitmapType) iTmp );

            if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
            {
                Bitmap aBmp;
                rIn >> aBmp;
                aXOBitmap.SetBitmap( aBmp );
            }
            else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
            {
                USHORT* pArray = new USHORT[ 64 ];
                Color   aColor;

                for( USHORT i = 0; i < 64; i++ )
                    rIn >> *( pArray + i );
                aXOBitmap.SetPixelArray( pArray );

                rIn >> aColor;
                aXOBitmap.SetPixelColor( aColor );
                rIn >> aColor;
                aXOBitmap.SetBackgroundColor( aColor );

                delete []pArray;
            }
        }
    }
}

//*************************************************************************

XFillBitmapItem::XFillBitmapItem( SfxItemPool* pPool, const XOBitmap& rTheBitmap )
:   NameOrIndex( XATTR_FILLBITMAP, -1 ),
    aXOBitmap( rTheBitmap )
{
    SetName( CreateStandardName( pPool, XATTR_FILLBITMAP ) );
}

//*************************************************************************

XFillBitmapItem::XFillBitmapItem( SfxItemPool* pPool )
: NameOrIndex(XATTR_FILLBITMAP, -1 )
{
    SetName( CreateStandardName( pPool, XATTR_FILLBITMAP ) );
}

/*************************************************************************
|*
|*    XFillBitmapItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

SfxPoolItem* XFillBitmapItem::Clone(SfxItemPool* pPool) const
{
    return new XFillBitmapItem(*this);
}

/*************************************************************************
|*
|*    int XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

int XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aXOBitmap == ((const XFillBitmapItem&) rItem).aXOBitmap );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillBitmapItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

SfxPoolItem* XFillBitmapItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillBitmapItem( rIn, nVer );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillBitmapItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

SvStream& XFillBitmapItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (INT16) aXOBitmap.GetBitmapStyle();
        if( !aXOBitmap.GetBitmap() )
            rOut << (INT16) XBITMAP_NONE;
        else
        {
            rOut << (INT16) aXOBitmap.GetBitmapType();
            if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
            {
                const USHORT    nOldComprMode = rOut.GetCompressMode();
                USHORT          nNewComprMode = nOldComprMode;

                if( rOut.GetVersion() >= SOFFICE_FILEFORMAT_50 )
                    nNewComprMode |= COMPRESSMODE_ZBITMAP;
                else
                    nNewComprMode &= ~COMPRESSMODE_ZBITMAP;

                rOut.SetCompressMode( nNewComprMode );
                rOut << aXOBitmap.GetBitmap();
                rOut.SetCompressMode( nOldComprMode );
            }
            else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
            {
                USHORT* pArray = aXOBitmap.GetPixelArray();
                for( USHORT i = 0; i < 64; i++ )
                    rOut << (USHORT) *( pArray + i );

                rOut << aXOBitmap.GetPixelColor();
                rOut << aXOBitmap.GetBackgroundColor();
            }
        }
    }

    return rOut;
}

/*************************************************************************
|*
|*    const Bitmap& XFillBitmapItem::GetValue(const XBitmapTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

const XOBitmap& XFillBitmapItem::GetValue(const XBitmapTable* pTable) const
{
    if (!IsIndex())
        return aXOBitmap;
    else
        return pTable->Get(GetIndex())->GetXBitmap();
}


/*************************************************************************
|*
|*    USHORT XFillBitmapItem::GetVersion() const
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

USHORT XFillBitmapItem::GetVersion( USHORT nFileFormatVersion ) const
{
    // 2. Version
    return( 1 );
}

//------------------------------------------------------------------------

SfxItemPresentation XFillBitmapItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&           rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText += GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

sal_Bool XFillBitmapItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    if( nMemberId == MID_NAME )
    {
        rVal <<= ::rtl::OUString( GetName() );
    }
    else if( nMemberId == MID_GRAFURL )
    {
        XOBitmap aXOBitmap( GetValue() );
        ::rtl::OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
        aURL += ::rtl::OUString::createFromAscii( aXOBitmap.GetGraphicObject().GetUniqueID().GetBuffer() );
        rVal <<= aURL;
    }
    else
    {
        XOBitmap aXOBitmap( GetValue() );
        Bitmap aBmp( aXOBitmap.GetBitmap() );
        BitmapEx aBmpEx( aBmp );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp(
            VCLUnoHelper::CreateBitmap( aBmpEx ) );

        rVal <<= xBmp;
    }

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool XFillBitmapItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    if( nMemberId == MID_NAME )
    {
        ::rtl::OUString aName;
        if(rVal >>= aName)
        {
            SetName( aName );
            return sal_True;
        }
    }
#ifndef SVX_LIGHT
    else if( nMemberId == MID_GRAFURL )
    {
        ::rtl::OUString aURL;
        if(rVal >>= aURL)
        {
            GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
            XOBitmap aBMP( aGrafObj );
            SetValue( aBMP );
            return sal_True;
        }
    }
#endif
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp;
        if( rVal >>= xBmp)
        {
            BitmapEx aInputEx( VCLUnoHelper::GetBitmap( xBmp ) );
            Bitmap aInput( aInputEx.GetBitmap() );

            // Bitmap einsetzen
            aXOBitmap.SetBitmap( aInput );
            aXOBitmap.SetBitmapType(XBITMAP_IMPORT);

            if(aInput.GetSizePixel().Width() == 8
                && aInput.GetSizePixel().Height() == 8
                && aInput.GetColorCount() == 2)
            {
                aXOBitmap.Bitmap2Array();
                aXOBitmap.SetBitmapType(XBITMAP_8X8);
                aXOBitmap.SetPixelSize(aInput.GetSizePixel());
            }
            return sal_True;
        }
    }
    return sal_False;
}



