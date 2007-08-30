/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbafont.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 10:04:31 $
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
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <org/openoffice/excel/XlColorIndex.hpp>
#include <org/openoffice/excel/XlUnderlineStyle.hpp>
#include "vbafont.hxx"
#include "scitems.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

// use local constants there is no need to expose these constants
// externally. Looking at the Format->Character dialog it seem that
// these may infact be even be calculated. Leave hardcoded for now
// #FIXEME #TBD investigate the code for dialog mentioned above

// The font baseline is not specified.
const short NORMAL = 0;

// specifies a superscripted.
const short SUPERSCRIPT = 33;

// specifies a subscripted.
const short SUBSCRIPT = -33;

// specifies a hight of superscripted font
 const sal_Int8 SUPERSCRIPTHEIGHT = 58;

// specifies a hight of subscripted font
const sal_Int8 SUBSCRIPTHEIGHT = 58;

// specifies a hight of normal font
const short NORMALHEIGHT = 100;

ScVbaFont::ScVbaFont( const ScVbaPalette& dPalette, uno::Reference< beans::XPropertySet > xPropertySet, SfxItemSet* pDataSet  ) throw ( uno::RuntimeException ) : mxFont( xPropertySet, css::uno::UNO_QUERY_THROW ), mPalette( dPalette ),  mpDataSet( pDataSet )
{
}

void
ScVbaFont::setSuperscript( const uno::Any& aValue ) throw ( uno::RuntimeException )
{

    sal_Bool bValue = sal_False;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

        if( bValue )
    {
        nValue = SUPERSCRIPT;
            nValue2 = SUPERSCRIPTHEIGHT;
    }
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), ( uno::Any )nValue );
     mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapementHeight" ) ), ( uno::Any )nValue2 );
}

uno::Any
ScVbaFont::getSuperscript() throw ( uno::RuntimeException )
{
    short nValue = 0;
    mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ) ) >>= nValue;
    return uno::makeAny( ( nValue == SUPERSCRIPT ) );
}

void
ScVbaFont::setSubscript( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    sal_Bool bValue = sal_False;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

        if( bValue )
    {
        nValue= SUBSCRIPT;
            nValue2 = SUBSCRIPTHEIGHT;
    }

     mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapementHeight" ) ), ( uno::Any )nValue2 );
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), ( uno::Any )nValue );

}

uno::Any
ScVbaFont::getSubscript() throw ( uno::RuntimeException )
{
    short nValue = NORMAL;
    mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ) ) >>= nValue;
    return uno::makeAny( ( nValue == SUBSCRIPT ) );
}

void
ScVbaFont::setSize( const uno::Any& aValue ) throw( uno::RuntimeException )
{
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) ), aValue );
}

uno::Any
ScVbaFont::getSize() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_HEIGHT, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();
        return mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) ) );
}

void
ScVbaFont::setColorIndex( const uno::Any& aValue ) throw( uno::RuntimeException )
{
    sal_Int32 lValue=0;
    aValue >>= lValue;
    // #TODO #FIXME is behavior random or just default, maybe it depends
    // on the parent object ( which we don't really take care of right now
    if ( !lValue || lValue == excel::XlColorIndex::xlColorIndexAutomatic )
        lValue = 1;
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ), ( uno::Any )mPalette.getPalette()->getByIndex( --lValue ) );
}


uno::Any
ScVbaFont::getColorIndex() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_COLOR, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();

//XXX
        sal_Int32 nColor = -1;
        uno::Any aValue= mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ) );
    aValue >>= nColor;
    uno::Reference< container::XIndexAccess > xIndex = mPalette.getPalette();
    sal_Int32 nElems = xIndex->getCount();
    sal_Int32 nIndex = -1;
    for ( sal_Int32 count=0; count<nElems; ++count )
           {
        sal_Int32 nPaletteColor = 0;
        xIndex->getByIndex( count ) >>= nPaletteColor;
        if ( nPaletteColor == nColor )
        {
            nIndex = count + 1; // 1 based
            break;
        }
    }
    return uno::makeAny( nIndex );
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ScVbaFont::setStandardFontSize( const uno::Any& /*aValue*/ ) throw( uno::RuntimeException )
{
//XXX #TODO# #FIXME#
    //mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharSize" ) ), ( uno::Any )fValue );
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("setStandardFontSize not supported") ), uno::Reference< uno::XInterface >() );
}


uno::Any
ScVbaFont::getStandardFontSize() throw ( uno::RuntimeException )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("getStandardFontSize not supported") ), uno::Reference< uno::XInterface >() );
    return uno::Any();
}


void
ScVbaFont::setStandardFont( const uno::Any& /*aValue*/ ) throw( uno::RuntimeException )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("setStandardFont not supported") ), uno::Reference< uno::XInterface >() );
}


uno::Any
ScVbaFont::getStandardFont() throw ( uno::RuntimeException )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("getStandardFont not supported") ), uno::Reference< uno::XInterface >() );
    return uno::Any();
}

void
ScVbaFont::setFontStyle( const uno::Any& /*aValue*/ ) throw( uno::RuntimeException )
{
//XXX #TODO# #FIXME#
    //mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharSize" ) ), ( uno::Any )aValue );
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("setFontStyle not supported") ), uno::Reference< uno::XInterface >() );
}


uno::Any
ScVbaFont::getFontStyle() throw ( uno::RuntimeException )
{
//XXX
    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("getFontStyle not supported") ), uno::Reference< uno::XInterface >() );
    return uno::Any();
}

void
ScVbaFont::setBold( const uno::Any& aValue ) throw( uno::RuntimeException )
{
    sal_Bool bValue = sal_False;
    aValue >>= bValue;
    double fBoldValue = awt::FontWeight::NORMAL;
    if( bValue )
        fBoldValue = awt::FontWeight::BOLD;
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) ), ( uno::Any )fBoldValue );

}

uno::Any
ScVbaFont::getBold() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_WEIGHT, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();

    double fValue = 0.0;
     mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) ) ) >>= fValue;
    return uno::makeAny( fValue == awt::FontWeight::BOLD );
}

void
ScVbaFont::setUnderline( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    // default
    sal_Int32 nValue = excel::XlUnderlineStyle::xlUnderlineStyleNone;
    aValue >>= nValue;
    switch ( nValue )
    {
// NOTE:: #TODO #FIMXE
// xlUnderlineStyleDoubleAccounting & xlUnderlineStyleSingleAccounting
// don't seem to be supported in Openoffice.
// The import filter converts them to single or double underlines as appropriate
// So, here at the moment we are similarly silently converting
// xlUnderlineStyleSingleAccounting to xlUnderlineStyleSingle.

        case excel::XlUnderlineStyle::xlUnderlineStyleNone:
            nValue = awt::FontUnderline::NONE;
            break;
        case excel::XlUnderlineStyle::xlUnderlineStyleSingle:
        case excel::XlUnderlineStyle::xlUnderlineStyleSingleAccounting:
            nValue = awt::FontUnderline::SINGLE;
            break;
        case excel::XlUnderlineStyle::xlUnderlineStyleDouble:
        case excel::XlUnderlineStyle::xlUnderlineStyleDoubleAccounting:
            nValue = awt::FontUnderline::DOUBLE;
            break;
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Unknown value for Underline")), uno::Reference< uno::XInterface >() );
    }

    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ), ( uno::Any )nValue );

}

uno::Any
ScVbaFont::getUnderline() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_UNDERLINE, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();

    sal_Int32 nValue = awt::FontUnderline::NONE;
    mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ) ) >>= nValue;
    switch ( nValue )
    {
        case  awt::FontUnderline::DOUBLE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleDouble;
            break;
        case  awt::FontUnderline::SINGLE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleSingle;
            break;
        case  awt::FontUnderline::NONE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleNone;
            break;
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Unknown value retrieved for Underline") ), uno::Reference< uno::XInterface >() );

    }
    return uno::makeAny( nValue );
}

void
ScVbaFont::setStrikethrough( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    sal_Bool bValue = sal_False;
    aValue >>= bValue;
    short nValue = awt::FontStrikeout::NONE;
    if( bValue )
        nValue = awt::FontStrikeout::SINGLE;
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharStrikeout" ) ), ( uno::Any )nValue );
}

uno::Any
ScVbaFont::getStrikethrough() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_CROSSEDOUT, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();
    short nValue = 0;
    mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharStrikeout" ) ) ) >>= nValue;
    return uno::Any( nValue == awt::FontStrikeout::SINGLE );
}

void
ScVbaFont::setShadow( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ), aValue );
}

uno::Any
ScVbaFont::getShadow() throw (uno::RuntimeException)
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_SHADOWED, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();
    return mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ) );
}

void
ScVbaFont::setItalic( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    sal_Bool bValue = sal_False;
    aValue >>= bValue;
    short nValue = awt::FontSlant_NONE;
    if( bValue )
        nValue = awt::FontSlant_ITALIC;
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) ), ( uno::Any )nValue );
}

uno::Any
ScVbaFont::getItalic() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT_POSTURE, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();

    short nValue = 0;
    mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) ) ) >>= nValue;
    return uno::makeAny( nValue == awt::FontSlant_ITALIC );
}

void
ScVbaFont::setName( const uno::Any& aValue ) throw ( uno::RuntimeException )
{
    rtl::OUString sString;
    aValue >>= sString;
    mxFont->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), aValue);
}

uno::Any
ScVbaFont::getName() throw ( uno::RuntimeException )
{
    if ( mpDataSet )
        if (  mpDataSet->GetItemState( ATTR_FONT, TRUE, NULL) == SFX_ITEM_DONTCARE )
            return aNULL();
    return mxFont->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ) );
}
