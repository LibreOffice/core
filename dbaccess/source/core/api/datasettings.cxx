/*************************************************************************
 *
 *  $RCSfile: datasettings.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:02:13 $
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

#ifndef _DBA_CORE_DATASETTINGS_HXX_
#include "datasettings.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_PROPERTYHELPER_HXX_
#include "propertyhelper.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTEMPHASISMARK_HPP_
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTRELIEF_HPP_
#include <com/sun/star/awt/FontRelief.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................
float ConvertFontWeight( ::FontWeight eWeight )
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

    OSL_ENSURE(0, "Unknown FontWeigth" );
    return ::com::sun::star::awt::FontWeight::DONTKNOW;
}
// -----------------------------------------------------------------------------
float ConvertFontWidth( ::FontWidth eWidth )
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

    OSL_ENSURE(0, "Unknown FontWidth" );
    return ::com::sun::star::awt::FontWidth::DONTKNOW;
}
//------------------------------------------------------------------
::com::sun::star::awt::FontDescriptor ImplCreateFontDescriptor( const Font& rFont )
{
    ::com::sun::star::awt::FontDescriptor aFD;
    aFD.Name            = rFont.GetName();
    aFD.StyleName       = rFont.GetStyleName();
    aFD.Height          = (sal_Int16)rFont.GetSize().Height();
    aFD.Width           = (sal_Int16)rFont.GetSize().Width();
    aFD.Family          = rFont.GetFamily();
    aFD.CharSet         = rFont.GetCharSet();
    aFD.Pitch           = rFont.GetPitch();
    aFD.CharacterWidth  = ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight          = ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant           = (::com::sun::star::awt::FontSlant)rFont.GetItalic();
    aFD.Underline       = rFont.GetUnderline();
    aFD.Strikeout       = rFont.GetStrikeout();
    aFD.Orientation     = rFont.GetOrientation();
    aFD.Kerning         = rFont.IsKerning();
    aFD.WordLineMode    = rFont.IsWordLineMode();
    aFD.Type            = 0;   // ??? => Nur an Metric...
    return aFD;
}


//==========================================================================
//= ODataSettings
//==========================================================================
//--------------------------------------------------------------------------
void ODataSettings::registerProperties(ODataSettings_Base* _pItem)
{
    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &_pItem->m_sFilter, ::getCppuType(&_pItem->m_sFilter));

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &_pItem->m_sOrder, ::getCppuType(&_pItem->m_sOrder));

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &_pItem->m_bApplyFilter, ::getBooleanCppuType());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &_pItem->m_aFont, ::getCppuType(&_pItem->m_aFont));

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextLineColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
        &_pItem->m_nFontEmphasis, ::getCppuType(&_pItem->m_nFontEmphasis));

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,&_pItem->m_nFontRelief, ::getCppuType(&_pItem->m_nFontRelief));

    registerProperty(PROPERTY_FONTNAME,         PROPERTY_ID_FONTNAME,        PropertyAttribute::BOUND,&_pItem->m_aFont.Name,            ::getCppuType(&_pItem->m_aFont.Name));
    registerProperty(PROPERTY_FONTHEIGHT,       PROPERTY_ID_FONTHEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Height,          ::getCppuType(&_pItem->m_aFont.Height));
    registerProperty(PROPERTY_FONTWIDTH,        PROPERTY_ID_FONTWIDTH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Width,           ::getCppuType(&_pItem->m_aFont.Width));
    registerProperty(PROPERTY_FONTSTYLENAME,    PROPERTY_ID_FONTSTYLENAME,   PropertyAttribute::BOUND,&_pItem->m_aFont.StyleName,       ::getCppuType(&_pItem->m_aFont.StyleName));
    registerProperty(PROPERTY_FONTFAMILY,       PROPERTY_ID_FONTFAMILY,      PropertyAttribute::BOUND,&_pItem->m_aFont.Family,          ::getCppuType(&_pItem->m_aFont.Family));
    registerProperty(PROPERTY_FONTCHARSET,      PROPERTY_ID_FONTCHARSET,     PropertyAttribute::BOUND,&_pItem->m_aFont.CharSet,         ::getCppuType(&_pItem->m_aFont.CharSet));
    registerProperty(PROPERTY_FONTPITCH,        PROPERTY_ID_FONTPITCH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Pitch,           ::getCppuType(&_pItem->m_aFont.Pitch));
    registerProperty(PROPERTY_FONTCHARWIDTH,    PROPERTY_ID_FONTCHARWIDTH,   PropertyAttribute::BOUND,&_pItem->m_aFont.CharacterWidth,  ::getCppuType(&_pItem->m_aFont.CharacterWidth));
    registerProperty(PROPERTY_FONTWEIGHT,       PROPERTY_ID_FONTWEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Weight,          ::getCppuType(&_pItem->m_aFont.Weight));
    registerProperty(PROPERTY_FONTSLANT,        PROPERTY_ID_FONTSLANT,       PropertyAttribute::BOUND,&_pItem->m_aFont.Slant,           ::getCppuType(&_pItem->m_aFont.Slant));
    registerProperty(PROPERTY_FONTUNDERLINE,    PROPERTY_ID_FONTUNDERLINE,   PropertyAttribute::BOUND,&_pItem->m_aFont.Underline,       ::getCppuType(&_pItem->m_aFont.Underline));
    registerProperty(PROPERTY_FONTSTRIKEOUT,    PROPERTY_ID_FONTSTRIKEOUT,   PropertyAttribute::BOUND,&_pItem->m_aFont.Strikeout,       ::getCppuType(&_pItem->m_aFont.Strikeout));
    registerProperty(PROPERTY_FONTORIENTATION,  PROPERTY_ID_FONTORIENTATION, PropertyAttribute::BOUND,&_pItem->m_aFont.Orientation,     ::getCppuType(&_pItem->m_aFont.Orientation));
    registerProperty(PROPERTY_FONTKERNING,      PROPERTY_ID_FONTKERNING,     PropertyAttribute::BOUND,&_pItem->m_aFont.Kerning,         ::getCppuType(&_pItem->m_aFont.Kerning));
    registerProperty(PROPERTY_FONTWORDLINEMODE, PROPERTY_ID_FONTWORDLINEMODE,PropertyAttribute::BOUND,&_pItem->m_aFont.WordLineMode,    ::getCppuType(&_pItem->m_aFont.WordLineMode));
    registerProperty(PROPERTY_FONTTYPE,         PROPERTY_ID_FONTTYPE,        PropertyAttribute::BOUND,&_pItem->m_aFont.Type,            ::getCppuType(&_pItem->m_aFont.Type));
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper)
    :OPropertyStateContainer(_rBHelper)
    ,ODataSettings_Base()
{
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyStateContainer(_rBHelper)
    ,ODataSettings_Base(_rSource)
{
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base()
    :m_aFont(::comphelper::getDefaultFont())
    ,m_bApplyFilter(sal_False)
    ,m_nFontEmphasis(::com::sun::star::awt::FontEmphasisMark::NONE)
    ,m_nFontRelief(::com::sun::star::awt::FontRelief::NONE)
{
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base(const ODataSettings_Base& _rSource)
{
    m_sFilter       = _rSource.m_sFilter;
    m_sOrder        = _rSource.m_sOrder;
    m_bApplyFilter  = _rSource.m_bApplyFilter;
    m_aFont         = _rSource.m_aFont;
    m_aRowHeight    = _rSource.m_aRowHeight;
    m_aTextColor    = _rSource.m_aTextColor;
    m_aTextLineColor= _rSource.m_aTextLineColor;
    m_nFontEmphasis = _rSource.m_nFontEmphasis;
    m_nFontRelief   = _rSource.m_nFontRelief;
}
// -----------------------------------------------------------------------------
Any ODataSettings::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aRet;
    static ::com::sun::star::awt::FontDescriptor aFD = ::comphelper::getDefaultFont();
    switch( _nHandle )
    {
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
            aRet <<= ::rtl::OUString();         break;
        case PROPERTY_ID_FONT:
            aRet <<= aFD;           break;
        case PROPERTY_ID_APPLYFILTER:
            aRet <<= sal_False;         break;
        case PROPERTY_ID_TEXTRELIEF:
            aRet <<= ::com::sun::star::awt::FontRelief::NONE;           break;
        case PROPERTY_ID_TEXTEMPHASIS:
            aRet <<= ::com::sun::star::awt::FontEmphasisMark::NONE;         break;
        case PROPERTY_ID_FONTNAME:
            aRet <<= aFD.Name;          break;
        case PROPERTY_ID_FONTHEIGHT:
            aRet <<= aFD.Height;break;
        case PROPERTY_ID_FONTWIDTH:
            aRet <<= aFD.Width;break;
        case PROPERTY_ID_FONTSTYLENAME:
            aRet <<= aFD.StyleName;break;
        case PROPERTY_ID_FONTFAMILY:
            aRet <<= aFD.Family;break;
        case PROPERTY_ID_FONTCHARSET:
            aRet <<= aFD.CharSet;break;
        case PROPERTY_ID_FONTPITCH:
            aRet <<= aFD.Pitch;break;
        case PROPERTY_ID_FONTCHARWIDTH:
            aRet <<= aFD.CharacterWidth;break;
        case PROPERTY_ID_FONTWEIGHT:
            aRet <<= aFD.Weight;break;
        case PROPERTY_ID_FONTSLANT:
            aRet <<= aFD.Slant; break;
        case PROPERTY_ID_FONTUNDERLINE:
            aRet <<= aFD.Underline;break;
        case PROPERTY_ID_FONTSTRIKEOUT:
            aRet <<= aFD.Strikeout;break;
        case PROPERTY_ID_FONTORIENTATION:
            aRet <<= aFD.Orientation;break;
        case PROPERTY_ID_FONTKERNING:
            aRet <<= aFD.Kerning;break;
        case PROPERTY_ID_FONTWORDLINEMODE:
            aRet <<= aFD.WordLineMode;break;
        case PROPERTY_ID_FONTTYPE:
            aRet <<= aFD.Type;break;
    }

    return aRet;
}
//........................................................................
}   // namespace dbaccess
//........................................................................

