/*************************************************************************
 *
 *  $RCSfile: datasettings.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:00 $
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
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
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
using namespace ::utl;

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
void ODataSettings::registerProperties()
{
    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &m_sFilter, ::getCppuType(&m_sFilter));

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &m_sOrder, ::getCppuType(&m_sOrder));

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &m_bApplyFilter, ::getBooleanCppuType());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &m_aFont, ::getCppuType(&m_aFont));

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextLineColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
        &m_nFontEmphasis, ::getCppuType(&m_nFontEmphasis));

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,
        &m_nFontRelief, ::getCppuType(&m_nFontRelief));
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
    ,ODataSettings_Base()
{
    registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
    ,ODataSettings_Base(_rSource)
{
    registerProperties();
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

//--------------------------------------------------------------------------
void ODataSettings_Base::storeTo(const OConfigurationNode& _rConfigLocation) const
{
    if (!_rConfigLocation.isValid() || _rConfigLocation.isReadonly())
    {
        OSL_ENSURE(sal_False, "ODataSettings_Base::storeTo : invalid config key (NULL or readonly) !");
        return;
    }

    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FILTER, makeAny(m_sFilter));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_ORDER, makeAny(m_sOrder));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_APPLYFILTER, ::cppu::bool2any(m_bApplyFilter));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_ROW_HEIGHT, m_aRowHeight);

    Any aNullAny;
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_NAME, makeAny( m_aFont.Name ) );
    sal_Bool bValidFont = 0 != m_aFont.Name.getLength();

    Any aEmpty;
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_HEIGHT,        bValidFont ? makeAny( m_aFont.Height )          : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_WIDTH,         bValidFont ? makeAny( m_aFont.Width )           : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_STYLENAME,     bValidFont ? makeAny( m_aFont.StyleName )       : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_FAMILY,        bValidFont ? makeAny( m_aFont.Family )          : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_CHARSET,       bValidFont ? makeAny( m_aFont.CharSet )         : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_PITCH,         bValidFont ? makeAny( m_aFont.Pitch )           : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH,bValidFont ? makeAny( m_aFont.CharacterWidth )  : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_WEIGHT,        bValidFont ? makeAny( m_aFont.Weight )          : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_SLANT,         bValidFont ? makeAny( (sal_Int16)m_aFont.Slant ): aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_UNDERLINE,     bValidFont ? makeAny( m_aFont.Underline )       : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_STRIKEOUT,     bValidFont ? makeAny( m_aFont.Strikeout )       : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_ORIENTATION,   bValidFont ? makeAny( m_aFont.Orientation )     : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_KERNING,       bValidFont ? makeAny( m_aFont.Kerning )         : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_WORDLINEMODE,  bValidFont ? makeAny( m_aFont.WordLineMode )    : aEmpty );
    _rConfigLocation.setNodeValue( CONFIGKEY_DEFSET_FONT_TYPE,          bValidFont ? makeAny( m_aFont.Type )            : aEmpty );

    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_TEXTCOLOR, m_aTextColor);
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_UNDERLINECOLOR, m_aTextLineColor);
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTEREMPHASIS, makeAny(m_nFontEmphasis));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTERRELIEF, makeAny(m_nFontRelief));
}

//--------------------------------------------------------------------------
void ODataSettings_Base::loadFrom(const OConfigurationNode& _rConfigLocation)
{
    if (!_rConfigLocation.isValid())
    {
        OSL_ENSURE(sal_False, "ODataSettings_Base::loadFrom: invalid config key (NULL) !");
        return;
    }

    OSL_VERIFY(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FILTER)       >>= m_sFilter);
    OSL_VERIFY(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_ORDER)        >>= m_sOrder);
    m_bApplyFilter = ::cppu::any2bool(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_APPLYFILTER));

    // default the font
    m_aFont = ::comphelper::getDefaultFont();

    _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_NAME) >>= m_aFont.Name;
    if ( m_aFont.Name.getLength() )
    {   // all other settings are only used if the name is valid
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_HEIGHT )       >>= m_aFont.Height;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_WIDTH )        >>= m_aFont.Width;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_STYLENAME )    >>= m_aFont.StyleName;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_FAMILY )       >>= m_aFont.Family;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_CHARSET )      >>= m_aFont.CharSet;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_PITCH )        >>= m_aFont.Pitch;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH )>>= m_aFont.CharacterWidth;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_WEIGHT )       >>= m_aFont.Weight;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_UNDERLINE )    >>= m_aFont.Underline;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_STRIKEOUT )    >>= m_aFont.Strikeout;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_ORIENTATION )  >>= m_aFont.Orientation;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_KERNING )      >>= m_aFont.Kerning;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_WORDLINEMODE ) >>= m_aFont.WordLineMode;
        _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_TYPE )         >>= m_aFont.Type;

        sal_Int16 nTemp = 0;
        if ( _rConfigLocation.getNodeValue( CONFIGKEY_DEFSET_FONT_SLANT ) >>= nTemp )
            m_aFont.Slant = (FontSlant)nTemp;
    }

    m_aRowHeight = _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_ROW_HEIGHT);
    m_aTextColor = _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_TEXTCOLOR);

    m_aTextLineColor = _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_UNDERLINECOLOR);
    _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTEREMPHASIS) >>= m_nFontEmphasis;
    _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTERRELIEF) >>= m_nFontRelief;
}

//........................................................................
}   // namespace dbaccess
//........................................................................

