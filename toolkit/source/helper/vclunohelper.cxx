/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclunohelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:06:38 $
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

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/window.hxx>

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XPOINTER_HPP_
#include <com/sun/star/awt/XPointer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIMPLEFONTMETRIC_HPP_
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_EMBEDMAPUNITS_HPP_
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#endif

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>
#include <toolkit/awt/vclxpointer.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/controls/unocontrolcontainermodel.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

//  ----------------------------------------------------
//  class VCLUnoHelper
//  ----------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> VCLUnoHelper::CreateToolkit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( szServiceName2_Toolkit ) );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> xToolkit;
    if ( xI.is() )
        xToolkit = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>( xI, ::com::sun::star::uno::UNO_QUERY );

    return xToolkit;
}

BitmapEx VCLUnoHelper::GetBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>& rxBitmap )
{
    BitmapEx aBmp;

    VCLXBitmap* pVCLBitmap = VCLXBitmap::GetImplementation( rxBitmap );
    if ( pVCLBitmap )
        aBmp = pVCLBitmap->GetBitmap();
    else
    {
        Bitmap aDIB, aMask;
        {
            ::com::sun::star::uno::Sequence<sal_Int8> aBytes = rxBitmap->getDIB();
            SvMemoryStream aMem( (char*) aBytes.getArray(), aBytes.getLength(), STREAM_READ );
            aMem >> aDIB;
        }
        {
            ::com::sun::star::uno::Sequence<sal_Int8> aBytes = rxBitmap->getMaskDIB();
            SvMemoryStream aMem( (char*) aBytes.getArray(), aBytes.getLength(), STREAM_READ );
            aMem >> aMask;
        }
        aBmp = BitmapEx( aDIB, aMask );
    }
    return aBmp;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap> VCLUnoHelper::CreateBitmap( const BitmapEx& rBitmap )
{
    VCLXBitmap* pBmp = new VCLXBitmap;
    pBmp->SetBitmap( rBitmap );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap> xBmp = pBmp;
    return xBmp;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Region VCLUnoHelper::GetRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion )
{
    Region aRegion;
    VCLXRegion* pVCLRegion = VCLXRegion::GetImplementation( rxRegion );
    if ( pVCLRegion )
        aRegion = pVCLRegion->GetRegion();
    else
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > aRects = rxRegion->getRectangles();
        sal_Int32 nRects = aRects.getLength();
        for ( sal_Int32 n = 0; n < nRects; n++ )
            aRegion.Union( VCLRectangle( aRects.getArray()[n] ) );
    }
    return aRegion;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> VCLUnoHelper::GetInterface( Window* pWindow )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xWin;
    if ( pWindow )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xPeer = pWindow->GetComponentInterface();
        xWin = xWin.query( xPeer );
    }
    return xWin;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer> VCLUnoHelper::CreatePointer()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer> xPointer = new VCLXPointer;
    return xPointer;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice>& rxDevice )
{
    OutputDevice* pOutDev = NULL;
    VCLXDevice* pDev = VCLXDevice::GetImplementation( rxDevice );
    if ( pDev )
        pOutDev = pDev->GetOutputDevice();
    return pOutDev;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>& rxGraphics )
{
    OutputDevice* pOutDev = NULL;
    VCLXGraphics* pGrf = VCLXGraphics::GetImplementation( rxGraphics );
    if ( pGrf )
        pOutDev = pGrf->GetOutputDevice();
    return pOutDev;
}

Polygon VCLUnoHelper::CreatePolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY )
{
    sal_uInt32 nLen = DataX.getLength();
    const sal_Int32* pDataX = DataX.getConstArray();
    const sal_Int32* pDataY = DataY.getConstArray();
    Polygon aPoly( (sal_uInt16) nLen );
    for ( sal_uInt16 n = 0; n < nLen; n++ )
    {
        Point aPnt;
        aPnt.X() = pDataX[n];
        aPnt.Y() = pDataY[n];
        aPoly[n] = aPnt;
    }
    return aPoly;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> VCLUnoHelper::CreateControlContainer( Window* pWindow )
{
    UnoControlContainer* pContainer = new UnoControlContainer( pWindow->GetComponentInterface( sal_True ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > x = pContainer;

    UnoControlModel* pContainerModel = new UnoControlContainerModel;
    pContainer->setModel( (::com::sun::star::awt::XControlModel*)pContainerModel );

    return x;
}

float VCLUnoHelper::ConvertFontWidth( FontWidth eWidth )
{
    if( eWidth == WIDTH_DONTKNOW )
        return ::com::sun::star::awt::FontWidth::DONTKNOW;
    else if( eWidth == WIDTH_ULTRA_CONDENSED )
        return ::com::sun::star::awt::FontWidth::ULTRACONDENSED;
    else if( eWidth == WIDTH_EXTRA_CONDENSED )
        return ::com::sun::star::awt::FontWidth::EXTRACONDENSED;
    else if( eWidth == WIDTH_CONDENSED )
        return ::com::sun::star::awt::FontWidth::CONDENSED;
    else if( eWidth == WIDTH_SEMI_CONDENSED )
        return ::com::sun::star::awt::FontWidth::SEMICONDENSED;
    else if( eWidth == WIDTH_NORMAL )
        return ::com::sun::star::awt::FontWidth::NORMAL;
    else if( eWidth == WIDTH_SEMI_EXPANDED )
        return ::com::sun::star::awt::FontWidth::SEMIEXPANDED;
    else if( eWidth == WIDTH_EXPANDED )
        return ::com::sun::star::awt::FontWidth::EXPANDED;
    else if( eWidth == WIDTH_EXTRA_EXPANDED )
        return ::com::sun::star::awt::FontWidth::EXTRAEXPANDED;
    else if( eWidth == WIDTH_ULTRA_EXPANDED )
        return ::com::sun::star::awt::FontWidth::ULTRAEXPANDED;

    DBG_ERROR( "Unknown FontWidth" );
    return ::com::sun::star::awt::FontWidth::DONTKNOW;
}

FontWidth VCLUnoHelper::ConvertFontWidth( float f )
{
    if( f <= ::com::sun::star::awt::FontWidth::DONTKNOW )
        return WIDTH_DONTKNOW;
    else if( f <= ::com::sun::star::awt::FontWidth::ULTRACONDENSED )
        return WIDTH_ULTRA_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXTRACONDENSED )
        return WIDTH_EXTRA_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::CONDENSED )
        return WIDTH_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::SEMICONDENSED )
        return WIDTH_SEMI_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::NORMAL )
        return WIDTH_NORMAL;
    else if( f <= ::com::sun::star::awt::FontWidth::SEMIEXPANDED )
        return WIDTH_SEMI_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXPANDED )
        return WIDTH_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXTRAEXPANDED )
        return WIDTH_EXTRA_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::ULTRAEXPANDED )
        return WIDTH_ULTRA_EXPANDED;

    DBG_ERROR( "Unknown FontWidth" );
    return WIDTH_DONTKNOW;
}

float VCLUnoHelper::ConvertFontWeight( FontWeight eWeight )
{
    if( eWeight == WEIGHT_DONTKNOW )
        return ::com::sun::star::awt::FontWeight::DONTKNOW;
    else if( eWeight == WEIGHT_THIN )
        return ::com::sun::star::awt::FontWeight::THIN;
    else if( eWeight == WEIGHT_ULTRALIGHT )
        return ::com::sun::star::awt::FontWeight::ULTRALIGHT;
    else if( eWeight == WEIGHT_LIGHT )
        return ::com::sun::star::awt::FontWeight::LIGHT;
    else if( eWeight == WEIGHT_SEMILIGHT )
        return ::com::sun::star::awt::FontWeight::SEMILIGHT;
    else if( ( eWeight == WEIGHT_NORMAL ) || ( eWeight == WEIGHT_MEDIUM ) )
        return ::com::sun::star::awt::FontWeight::NORMAL;
    else if( eWeight == WEIGHT_SEMIBOLD )
        return ::com::sun::star::awt::FontWeight::SEMIBOLD;
    else if( eWeight == WEIGHT_BOLD )
        return ::com::sun::star::awt::FontWeight::BOLD;
    else if( eWeight == WEIGHT_ULTRABOLD )
        return ::com::sun::star::awt::FontWeight::ULTRABOLD;
    else if( eWeight == WEIGHT_BLACK )
        return ::com::sun::star::awt::FontWeight::BLACK;

    DBG_ERROR( "Unknown FontWeigth" );
    return ::com::sun::star::awt::FontWeight::DONTKNOW;
}

FontWeight VCLUnoHelper::ConvertFontWeight( float f )
{
    if( f <= ::com::sun::star::awt::FontWeight::DONTKNOW )
        return WEIGHT_DONTKNOW;
    else if( f <= ::com::sun::star::awt::FontWeight::THIN )
        return WEIGHT_THIN;
    else if( f <= ::com::sun::star::awt::FontWeight::ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::LIGHT )
        return WEIGHT_LIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::SEMILIGHT )
        return WEIGHT_SEMILIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::NORMAL )
        return WEIGHT_NORMAL;
    else if( f <= ::com::sun::star::awt::FontWeight::SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::BOLD )
        return WEIGHT_BOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::BLACK )
        return WEIGHT_BLACK;

    DBG_ERROR( "Unknown FontWeigth" );
    return WEIGHT_DONTKNOW;
}


::com::sun::star::awt::FontDescriptor VCLUnoHelper::CreateFontDescriptor( const Font& rFont )
{
    ::com::sun::star::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = (sal_Int16)rFont.GetSize().Height();
    aFD.Width = (sal_Int16)rFont.GetSize().Width();
    aFD.Family = rFont.GetFamily();
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = rFont.GetPitch();
    aFD.CharacterWidth = VCLUnoHelper::ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight= VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant = (::com::sun::star::awt::FontSlant)rFont.GetItalic();
    aFD.Underline = rFont.GetUnderline();
    aFD.Strikeout = rFont.GetStrikeout();
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Nur an Metric...
    return aFD;
}

Font VCLUnoHelper::CreateFont( const ::com::sun::star::awt::FontDescriptor& rDescr, const Font& rInitFont )
{
    Font aFont( rInitFont );
    if ( rDescr.Name.getLength() )
        aFont.SetName( rDescr.Name );
    if ( rDescr.StyleName.getLength() )
        aFont.SetStyleName( rDescr.StyleName );
    if ( rDescr.Height )
        aFont.SetSize( Size( rDescr.Width, rDescr.Height ) );
    if ( (FontFamily)rDescr.Family != FAMILY_DONTKNOW )
        aFont.SetFamily( (FontFamily)rDescr.Family );
    if ( (CharSet)rDescr.CharSet != RTL_TEXTENCODING_DONTKNOW )
        aFont.SetCharSet( (CharSet)rDescr.CharSet );
    if ( (FontPitch)rDescr.Pitch != PITCH_DONTKNOW )
        aFont.SetPitch( (FontPitch)rDescr.Pitch );
    if ( rDescr.CharacterWidth )
        aFont.SetWidthType( VCLUnoHelper::ConvertFontWidth( rDescr.CharacterWidth ) );
    if ( rDescr.Weight )
        aFont.SetWeight( VCLUnoHelper::ConvertFontWeight( rDescr.Weight ) );
    if ( (FontItalic)rDescr.Slant != ITALIC_DONTKNOW )
        aFont.SetItalic( (FontItalic)rDescr.Slant );
    if ( (FontUnderline)rDescr.Underline != UNDERLINE_DONTKNOW )
        aFont.SetUnderline( (FontUnderline)rDescr.Underline );
    if ( (FontStrikeout)rDescr.Strikeout != STRIKEOUT_DONTKNOW )
        aFont.SetStrikeout( (FontStrikeout)rDescr.Strikeout );

    // Kein DONTKNOW
    aFont.SetOrientation( (short)rDescr.Orientation );
    aFont.SetKerning( rDescr.Kerning );
    aFont.SetWordLineMode( rDescr.WordLineMode );

    return aFont;
}

Font VCLUnoHelper::CreateFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont )
{
    Font aFont;
    VCLXFont* pVCLXFont = VCLXFont::GetImplementation( rxFont );
    if ( pVCLXFont )
        aFont = pVCLXFont->GetFont();
    return aFont;
}


::com::sun::star::awt::SimpleFontMetric VCLUnoHelper::CreateFontMetric( const FontMetric& rFontMetric )
{
    ::com::sun::star::awt::SimpleFontMetric aFM;
    aFM.Ascent = (sal_Int16)rFontMetric.GetAscent();
    aFM.Descent = (sal_Int16)rFontMetric.GetDescent();
    aFM.Leading = (sal_Int16)rFontMetric.GetIntLeading();
    aFM.Slant = (sal_Int16)rFontMetric.GetSlant();
    aFM.FirstChar = 0x0020;
    aFM.LastChar = 0xFFFD;
    return aFM;
}

sal_Bool VCLUnoHelper::IsZero( ::com::sun::star::awt::Rectangle rRect )
{
    return ( !rRect.X && !rRect.Y && !rRect.Width && !rRect.Height );
}

MapUnit VCLUnoHelper::UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit )
{
    switch( nUnoEmbedMapUnit )
    {
        case ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_MM:
            return MAP_100TH_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_MM:
            return MAP_10TH_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_MM:
            return MAP_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_CM:
            return MAP_CM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_1000TH_INCH:
            return MAP_1000TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_INCH:
            return MAP_100TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_INCH:
            return MAP_10TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_INCH:
            return MAP_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::POINT:
            return MAP_POINT;
        case ::com::sun::star::embed::EmbedMapUnits::TWIP:
            return MAP_TWIP;
        case ::com::sun::star::embed::EmbedMapUnits::PIXEL:
            return MAP_PIXEL;
    }

    OSL_ENSURE( sal_False, "Unexpected UNO map mode is provided!\n" );
    return MAP_LASTENUMDUMMY;
}

sal_Int32 VCLUnoHelper::VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit )
{
    switch( nVCLMapUnit )
    {
        case MAP_100TH_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_MM;
        case MAP_10TH_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_MM;
        case MAP_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_MM;
        case MAP_CM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_CM;
        case MAP_1000TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_1000TH_INCH;
        case MAP_100TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_INCH;
        case MAP_10TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_INCH;
        case MAP_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_INCH;
        case MAP_POINT:
            return ::com::sun::star::embed::EmbedMapUnits::POINT;
        case MAP_TWIP:
            return ::com::sun::star::embed::EmbedMapUnits::TWIP;
        case MAP_PIXEL:
            return ::com::sun::star::embed::EmbedMapUnits::PIXEL;
        default: ; // avoid compiler warning
    }

    OSL_ENSURE( sal_False, "Unexpected VCL map mode is provided!\n" );
    return -1;
}


