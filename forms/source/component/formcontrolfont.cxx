/*************************************************************************
 *
 *  $RCSfile: formcontrolfont.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:08:13 $
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

#ifndef FORMS_SOURCE_COMPONENT_FORMCONTROLFONT_HXX
#include "formcontrolfont.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_
#include <toolkit/helper/emptyfontdescriptor.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTRELIEF_HPP_
#include <com/sun/star/awt/FontRelief.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTEMPHASISMARK_HPP_
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //------------------------------------------------------------------------------
    namespace
    {
        Any lcl_extractFontDescriptorAggregate( sal_Int32 _nHandle, const FontDescriptor& _rFont )
        {
            Any aValue;
            switch ( _nHandle )
            {
            case PROPERTY_ID_FONT_NAME:
                aValue <<= _rFont.Name;
                break;

            case PROPERTY_ID_FONT_STYLENAME:
                aValue <<= _rFont.StyleName;
                break;

            case PROPERTY_ID_FONT_FAMILY:
                aValue <<= (sal_Int16)_rFont.Family;
                break;

            case PROPERTY_ID_FONT_CHARSET:
                aValue <<= (sal_Int16)_rFont.CharSet;
                break;

            case PROPERTY_ID_FONT_CHARWIDTH:
                aValue <<= _rFont.CharacterWidth;
                break;

            case PROPERTY_ID_FONT_KERNING:
                aValue <<= _rFont.Kerning;
                break;

            case PROPERTY_ID_FONT_ORIENTATION:
                aValue <<= _rFont.Orientation;
                break;

            case PROPERTY_ID_FONT_PITCH:
                aValue <<= _rFont.Pitch;
                break;

            case PROPERTY_ID_FONT_TYPE:
                aValue <<= _rFont.Type;
                break;

            case PROPERTY_ID_FONT_WIDTH:
                aValue <<= _rFont.Width;
                break;

            case PROPERTY_ID_FONT_HEIGHT:
                aValue <<= (float)( _rFont.Height );
                break;

            case PROPERTY_ID_FONT_WEIGHT:
                aValue <<= (float)_rFont.Weight;
                break;

            case PROPERTY_ID_FONT_SLANT:
                aValue = makeAny(_rFont.Slant);
                break;

            case PROPERTY_ID_FONT_UNDERLINE:
                aValue <<= (sal_Int16)_rFont.Underline;
                break;

            case PROPERTY_ID_FONT_STRIKEOUT:
                aValue <<= (sal_Int16)_rFont.Strikeout;
                break;

            case PROPERTY_ID_FONT_WORDLINEMODE:
                aValue = makeAny( (sal_Bool)_rFont.WordLineMode );
                break;

            default:
                OSL_ENSURE( sal_False, "lcl_extractFontDescriptorAggregate: invalid handle!" );
                break;
            }
            return aValue;
        }
    }

    //=====================================================================
    //= FontControlModel
    //=====================================================================
    //---------------------------------------------------------------------
    FontControlModel::FontControlModel( bool _bToolkitCompatibleDefaults )
        :m_nFontRelief( FontRelief::NONE )
        ,m_nFontEmphasis( FontEmphasisMark::NONE )
        ,m_bToolkitCompatibleDefaults( _bToolkitCompatibleDefaults )
    {
    }

    //---------------------------------------------------------------------
    FontControlModel::FontControlModel( const FontControlModel* _pOriginal )
    {
        m_nFontRelief = _pOriginal->m_nFontRelief;
        m_nFontEmphasis = _pOriginal->m_nFontEmphasis;
        m_bToolkitCompatibleDefaults = _pOriginal->m_bToolkitCompatibleDefaults;
    }

    //---------------------------------------------------------------------
    bool FontControlModel::isFontRelatedProperty( sal_Int32 _nPropertyHandle ) const
    {
        return isFontAggregateProperty( _nPropertyHandle )
            || ( _nPropertyHandle == PROPERTY_ID_FONT )
            || ( _nPropertyHandle == PROPERTY_ID_FONTEMPHASISMARK )
            || ( _nPropertyHandle == PROPERTY_ID_FONTRELIEF )
            || ( _nPropertyHandle == PROPERTY_ID_TEXTLINECOLOR )
            || ( _nPropertyHandle == PROPERTY_ID_TEXTCOLOR );
    }

    //---------------------------------------------------------------------
    bool FontControlModel::isFontAggregateProperty( sal_Int32 _nPropertyHandle ) const
    {
        return ( _nPropertyHandle == PROPERTY_ID_FONT_NAME )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_STYLENAME )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_FAMILY )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_CHARSET )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_HEIGHT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_WEIGHT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_SLANT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_UNDERLINE )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_STRIKEOUT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_WORDLINEMODE );
    }

    //---------------------------------------------------------------------
    sal_Int32 FontControlModel::getTextColor( ) const
    {
        sal_Int32 nColor = COL_TRANSPARENT;
        m_aTextColor >>= nColor;
        return nColor;
    }

    //---------------------------------------------------------------------
    sal_Int32 FontControlModel::getTextLineColor( ) const
    {
        sal_Int32 nColor = COL_TRANSPARENT;
        m_aTextLineColor >>= nColor;
        return nColor;
    }

    //------------------------------------------------------------------------------
    void FontControlModel::describeFontRelatedProperties( Sequence< Property >& /* [out] */ _rProps ) const
    {
        sal_Int32 nPos = _rProps.getLength();
        _rProps.realloc( nPos + 21 );
        Property* pProperties = _rProps.getArray();

        DECL_PROP2      ( FONT,               FontDescriptor,   BOUND, MAYBEDEFAULT );
        DECL_PROP2      ( FONTEMPHASISMARK,   sal_Int16,        BOUND, MAYBEDEFAULT );
        DECL_PROP2      ( FONTRELIEF,         sal_Int16,        BOUND, MAYBEDEFAULT );
        DECL_PROP3      ( TEXTCOLOR,          sal_Int32,        BOUND, MAYBEDEFAULT, MAYBEVOID );
        DECL_PROP3      ( TEXTLINECOLOR,      sal_Int32,        BOUND, MAYBEDEFAULT, MAYBEVOID );

        DECL_PROP1      ( FONT_CHARWIDTH,     float,            MAYBEDEFAULT );
        DECL_BOOL_PROP1 ( FONT_KERNING,                         MAYBEDEFAULT );
        DECL_PROP1      ( FONT_ORIENTATION,   float,            MAYBEDEFAULT );
        DECL_PROP1      ( FONT_PITCH,         sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_TYPE,          sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_WIDTH,         sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_NAME,          ::rtl::OUString,  MAYBEDEFAULT );
        DECL_PROP1      ( FONT_STYLENAME,     ::rtl::OUString,  MAYBEDEFAULT );
        DECL_PROP1      ( FONT_FAMILY,        sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_CHARSET,       sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_HEIGHT,        float,            MAYBEDEFAULT );
        DECL_PROP1      ( FONT_WEIGHT,        float,            MAYBEDEFAULT );
        DECL_PROP1      ( FONT_SLANT,         sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_UNDERLINE,     sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_STRIKEOUT,     sal_Int16,        MAYBEDEFAULT );
        DECL_BOOL_PROP1 ( FONT_WORDLINEMODE,                    MAYBEDEFAULT );
    }

    //---------------------------------------------------------------------
    void FontControlModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch( _nHandle )
        {
        case PROPERTY_ID_TEXTCOLOR:
            _rValue = m_aTextColor;
            break;

        case PROPERTY_ID_FONTEMPHASISMARK:
            _rValue <<= m_nFontEmphasis;
            break;

        case PROPERTY_ID_FONTRELIEF:
            _rValue <<= m_nFontRelief;
            break;

        case PROPERTY_ID_TEXTLINECOLOR:
            _rValue = m_aTextLineColor;
            break;

        case PROPERTY_ID_FONT:
            _rValue = makeAny( m_aFont );
            break;

        default:
            _rValue = lcl_extractFontDescriptorAggregate( _nHandle, m_aFont );
            break;
        }
    }

    //---------------------------------------------------------------------
    sal_Bool FontControlModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue,
                sal_Int32 _nHandle, const Any& _rValue ) throw( IllegalArgumentException )
    {
        sal_Bool bModified = sal_False;
        switch( _nHandle )
        {
        case PROPERTY_ID_TEXTCOLOR:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aTextColor, ::getCppuType( static_cast< const sal_Int32* >( NULL ) ) );
            break;

        case PROPERTY_ID_TEXTLINECOLOR:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aTextLineColor, ::getCppuType( static_cast< sal_Int32* >( NULL ) ) );
            break;

        case PROPERTY_ID_FONTEMPHASISMARK:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_nFontEmphasis );
            break;

        case PROPERTY_ID_FONTRELIEF:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_nFontRelief );
            break;

        case PROPERTY_ID_FONT:
        {
            Any aWorkAroundGccLimitation = makeAny( m_aFont );
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, aWorkAroundGccLimitation, ::getCppuType( &m_aFont ) );
        }
        break;

        case PROPERTY_ID_FONT_NAME:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aFont.Name );
            break;

        case PROPERTY_ID_FONT_STYLENAME:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aFont.StyleName );
            break;

        case PROPERTY_ID_FONT_FAMILY:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Family );
            break;

        case PROPERTY_ID_FONT_CHARSET:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.CharSet );
            break;

        case PROPERTY_ID_FONT_CHARWIDTH:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, float( m_aFont.CharacterWidth ) );
            break;

        case PROPERTY_ID_FONT_KERNING:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Kerning );
            break;

        case PROPERTY_ID_FONT_ORIENTATION:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, float( m_aFont.Orientation ) );
            break;

        case PROPERTY_ID_FONT_PITCH:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Pitch );
            break;

        case PROPERTY_ID_FONT_TYPE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Type );
            break;

        case PROPERTY_ID_FONT_WIDTH:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Width );
            break;

        case PROPERTY_ID_FONT_HEIGHT:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, float( m_aFont.Height ) );
            break;

        case PROPERTY_ID_FONT_WEIGHT:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aFont.Weight );
            break;

        case PROPERTY_ID_FONT_SLANT:
            bModified = tryPropertyValueEnum( _rConvertedValue, _rOldValue, _rValue, m_aFont.Slant );
            break;

        case PROPERTY_ID_FONT_UNDERLINE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Underline );
            break;

        case PROPERTY_ID_FONT_STRIKEOUT:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_aFont.Strikeout );
            break;

        case PROPERTY_ID_FONT_WORDLINEMODE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Bool)m_aFont.WordLineMode );
            break;

        default:
            DBG_ERROR( "FontControlModel::convertFastPropertyValue: no font aggregate!" );
        }
        return bModified;
    }

    //------------------------------------------------------------------------------
    void FontControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception )
    {
        switch( _nHandle )
        {
        case PROPERTY_ID_TEXTCOLOR:
            m_aTextColor = _rValue;
            break;

        case PROPERTY_ID_TEXTLINECOLOR:
            m_aTextLineColor = _rValue;
            break;

        case PROPERTY_ID_FONTEMPHASISMARK:
            _rValue >>= m_nFontEmphasis;
            break;

        case PROPERTY_ID_FONTRELIEF:
            _rValue >>= m_nFontRelief;
            break;

        case PROPERTY_ID_FONT:
            _rValue >>= m_aFont;
            break;

        case PROPERTY_ID_FONT_NAME:
            _rValue >>= m_aFont.Name;
            break;

        case PROPERTY_ID_FONT_STYLENAME:
            _rValue >>= m_aFont.StyleName;
            break;

        case PROPERTY_ID_FONT_FAMILY:
            _rValue >>= m_aFont.Family;
            break;

        case PROPERTY_ID_FONT_CHARSET:
            _rValue >>= m_aFont.CharSet;
            break;

        case PROPERTY_ID_FONT_CHARWIDTH:
            _rValue >>= m_aFont.CharacterWidth;
            break;

        case PROPERTY_ID_FONT_KERNING:
            _rValue >>= m_aFont.Kerning;
            break;

        case PROPERTY_ID_FONT_ORIENTATION:
            _rValue >>= m_aFont.Orientation;
            break;

        case PROPERTY_ID_FONT_PITCH:
            _rValue >>= m_aFont.Pitch;
            break;

        case PROPERTY_ID_FONT_TYPE:
            _rValue >>= m_aFont.Type;
            break;

        case PROPERTY_ID_FONT_WIDTH:
            _rValue >>= m_aFont.Width;
            break;

        case PROPERTY_ID_FONT_HEIGHT:
        {
            float nHeight = 0;
            _rValue >>= nHeight;
            m_aFont.Height = (sal_Int16)nHeight;
        }
        break;

        case PROPERTY_ID_FONT_WEIGHT:
            _rValue >>= m_aFont.Weight;
            break;

        case PROPERTY_ID_FONT_SLANT:
            _rValue >>= m_aFont.Slant;
            break;

        case PROPERTY_ID_FONT_UNDERLINE:
            _rValue >>= m_aFont.Underline;
            break;

        case PROPERTY_ID_FONT_STRIKEOUT:
            _rValue >>= m_aFont.Strikeout;
            break;

        case PROPERTY_ID_FONT_WORDLINEMODE:
        {
            sal_Bool bWordLineMode = sal_False;
            _rValue >>= bWordLineMode;
            m_aFont.WordLineMode = bWordLineMode;
        }
        break;

        default:
            DBG_ERROR( "FontControlModel::setFastPropertyValue_NoBroadcast: invalid property!" );
        }
    }

    //------------------------------------------------------------------------------
    Any FontControlModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aReturn;
        // some defaults which are the same, not matter if we have toolkit-compatible
        // defaults or not
        bool bHandled = false;
        switch( _nHandle )
        {
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
            // void
            bHandled = true;
            break;

        case PROPERTY_ID_FONTEMPHASISMARK:
            aReturn <<= FontEmphasisMark::NONE;
            bHandled = true;
            break;

        case PROPERTY_ID_FONTRELIEF:
            aReturn <<= FontRelief::NONE;
            bHandled = true;
            break;
        }
        if ( bHandled )
            return aReturn;

        if ( m_bToolkitCompatibleDefaults )
        {
            EmptyFontDescriptor aEmpty;
            if ( PROPERTY_ID_FONT == _nHandle )
                return makeAny( (FontDescriptor)aEmpty );
            return lcl_extractFontDescriptorAggregate( _nHandle, aEmpty );
        }

        switch( _nHandle )
        {
        case PROPERTY_ID_FONT:
            aReturn <<= ::comphelper::getDefaultFont();
            break;

        case PROPERTY_ID_FONT_WORDLINEMODE:
            aReturn = makeBoolAny(sal_False);

        case PROPERTY_ID_FONT_NAME:
        case PROPERTY_ID_FONT_STYLENAME:
            aReturn <<= ::rtl::OUString();

        case PROPERTY_ID_FONT_FAMILY:
        case PROPERTY_ID_FONT_CHARSET:
        case PROPERTY_ID_FONT_SLANT:
        case PROPERTY_ID_FONT_UNDERLINE:
        case PROPERTY_ID_FONT_STRIKEOUT:
            aReturn <<= (sal_Int16)1;
            break;

        case PROPERTY_ID_FONT_KERNING:
            aReturn = makeBoolAny(sal_False);
            break;

        case PROPERTY_ID_FONT_PITCH:
        case PROPERTY_ID_FONT_TYPE:
        case PROPERTY_ID_FONT_WIDTH:
            aReturn <<= (sal_Int16)0;
            break;

        case PROPERTY_ID_FONT_HEIGHT:
        case PROPERTY_ID_FONT_WEIGHT:
        case PROPERTY_ID_FONT_CHARWIDTH:
        case PROPERTY_ID_FONT_ORIENTATION:
            aReturn <<= (float)0;
            break;

        default:
            DBG_ERROR( "FontControlModel::getPropertyDefaultByHandle: invalid property!" );
        }

        return aReturn;
    }

//.........................................................................
}   // namespace frm
//.........................................................................
