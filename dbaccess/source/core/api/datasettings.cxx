/*************************************************************************
 *
 *  $RCSfile: datasettings.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-16 16:04:57 $
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
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _DBA_CONFIGNODE_HXX_
#include "confignode.hxx"
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

#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................
::com::sun::star::awt::FontDescriptor ODataSettings::m_aAppFont;
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
    aFD.Height          = rFont.GetSize().Height();
    aFD.Width           = rFont.GetSize().Width();
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
}
//--------------------------------------------------------------------------
ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
{
    m_aAppFont = ImplCreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
    ,ODataSettings_Base(_rSource)
{
    m_aAppFont = ImplCreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base()
    : m_bApplyFilter(sal_False)
    ,m_aFont(::comphelper::getDefaultFont())
{
    m_aAppFont = ImplCreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
    //  registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base(const ODataSettings_Base& _rSource)
{
    //  registerProperties();

    m_sFilter       = _rSource.m_sFilter;
    m_sOrder        = _rSource.m_sOrder;
    m_bApplyFilter  = _rSource.m_bApplyFilter;
    m_aFont         = _rSource.m_aFont;
    m_aRowHeight    = _rSource.m_aRowHeight;
    m_aTextColor    = _rSource.m_aTextColor;
}

//--------------------------------------------------------------------------
void ODataSettings_Base::storeTo(const OConfigurationNode& _rConfigLocation) const
{
    if (!_rConfigLocation.isValid() || _rConfigLocation.isReadonly())
    {
        OSL_ASSERT("ODataSettings_Base::storeTo : invalid config key (NULL or readonly) !");
        return;
    }

    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FILTER, makeAny(m_sFilter));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_ORDER, makeAny(m_sOrder));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_APPLYFILTER, ::cppu::bool2any(m_bApplyFilter));
    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_ROW_HEIGHT, m_aRowHeight);

    if(m_aAppFont.Name != m_aFont.Name)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_NAME, makeAny(m_aFont.Name));

    if(m_aAppFont.Height != m_aFont.Height)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_HEIGHT, makeAny(m_aFont.Height));
    if(m_aAppFont.Width != m_aFont.Width)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_WIDTH, makeAny(m_aFont.Width));
    if(m_aAppFont.StyleName != m_aFont.StyleName)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_STYLENAME, makeAny(m_aFont.StyleName));
    if(m_aAppFont.Family != m_aFont.Family)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_FAMILY, makeAny(m_aFont.Family));
    if(m_aAppFont.CharSet != m_aFont.CharSet)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_CHARSET, makeAny(m_aFont.CharSet));
    if(m_aAppFont.Pitch != m_aFont.Pitch)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_PITCH, makeAny(m_aFont.Pitch));
    if(m_aAppFont.CharacterWidth != m_aFont.CharacterWidth)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH, makeAny(m_aFont.CharacterWidth));
    if(m_aAppFont.Weight != m_aFont.Weight)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_WEIGHT, makeAny(m_aFont.Weight));
    if(m_aAppFont.Slant != m_aFont.Slant)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_SLANT, makeAny((sal_Int16)m_aFont.Slant));
    if(m_aAppFont.Underline != m_aFont.Underline)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_UNDERLINE, makeAny(m_aFont.Underline));
    if(m_aAppFont.Strikeout != m_aFont.Strikeout)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_STRIKEOUT, makeAny(m_aFont.Strikeout));
    if(m_aAppFont.Orientation != m_aFont.Orientation)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_ORIENTATION, makeAny(m_aFont.Orientation));
    if(m_aAppFont.Kerning != m_aFont.Kerning)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_KERNING, makeAny(m_aFont.Kerning));
    if(m_aAppFont.WordLineMode != m_aFont.WordLineMode)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_WORDLINEMODE, makeAny(m_aFont.WordLineMode));
    if(m_aAppFont.Type != m_aFont.Type)
        _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_FONT_TYPE, makeAny(m_aFont.Type));

    _rConfigLocation.setNodeValue(CONFIGKEY_DEFSET_TEXTCOLOR, m_aTextColor);
}

//--------------------------------------------------------------------------
void ODataSettings_Base::loadFrom(const OConfigurationNode& _rConfigLocation)
{
    if (!_rConfigLocation.isValid())
    {
        OSL_ASSERT("ODataSettings_Base::loadFrom: invalid config key (NULL) !");
        return;
    }

    OSL_VERIFY(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FILTER)       >>= m_sFilter);
    OSL_VERIFY(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_ORDER)        >>= m_sOrder);
    m_bApplyFilter = ::cppu::any2bool(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_APPLYFILTER));

    if(!(_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_NAME) >>= m_aFont.Name) || !m_aFont.Name.getLength())
        m_aFont.Name = m_aAppFont.Name;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_HEIGHT) >>= m_aFont.Height))
        m_aFont.Height = m_aAppFont.Height;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_WIDTH) >>= m_aFont.Width))
        m_aFont.Width = m_aAppFont.Width;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_STYLENAME) >>= m_aFont.StyleName))
        m_aFont.StyleName = m_aAppFont.StyleName;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_FAMILY) >>= m_aFont.Family))
        m_aFont.Family = m_aAppFont.Family;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_CHARSET) >>= m_aFont.CharSet))
        m_aFont.CharSet = m_aAppFont.CharSet;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_PITCH) >>= m_aFont.Pitch))
        m_aFont.Pitch = m_aAppFont.Pitch;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH) >>= m_aFont.CharacterWidth))
        m_aFont.CharacterWidth = m_aAppFont.CharacterWidth;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_WEIGHT) >>= m_aFont.Weight))
        m_aFont.Weight = m_aAppFont.Weight;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_UNDERLINE) >>= m_aFont.Underline))
        m_aFont.Underline = m_aAppFont.Underline;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_STRIKEOUT) >>= m_aFont.Strikeout))
        m_aFont.Strikeout = m_aAppFont.Strikeout;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_ORIENTATION) >>= m_aFont.Orientation))
        m_aFont.Orientation = m_aAppFont.Orientation;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_KERNING) >>= m_aFont.Kerning))
        m_aFont.Kerning = m_aAppFont.Kerning;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_WORDLINEMODE) >>= m_aFont.WordLineMode))
        m_aFont.WordLineMode = m_aAppFont.WordLineMode;;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_TYPE) >>= m_aFont.Type))
        m_aFont.Type = m_aAppFont.Type;;
    sal_Int16 nTemp = 0;
    if(! (_rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_FONT_SLANT) >>= nTemp))
        m_aFont.Slant = m_aAppFont.Slant;
    else
        m_aFont.Slant = (FontSlant)nTemp;



    m_aRowHeight = _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_ROW_HEIGHT);
    m_aTextColor = _rConfigLocation.getNodeValue(CONFIGKEY_DEFSET_TEXTCOLOR);
}

//........................................................................
}   // namespace dbaccess
//........................................................................

