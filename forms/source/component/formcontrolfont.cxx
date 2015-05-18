/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "formcontrolfont.hxx"
#include "property.hrc"
#include "property.hxx"
#include <cppuhelper/propshlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <tools/color.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>


namespace frm
{


    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


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
                aValue = makeAny( _rFont.WordLineMode );
                break;

            default:
                OSL_FAIL( "lcl_extractFontDescriptorAggregate: invalid handle!" );
                break;
            }
            return aValue;
        }
    }

    FontControlModel::FontControlModel( bool _bToolkitCompatibleDefaults )
        :m_nFontRelief( css::awt::FontRelief::NONE )
        ,m_nFontEmphasis( FontEmphasisMark::NONE )
        ,m_bToolkitCompatibleDefaults( _bToolkitCompatibleDefaults )
    {
    }


    FontControlModel::FontControlModel( const FontControlModel* _pOriginal )
    {
        m_aFont = _pOriginal->m_aFont;
        m_nFontRelief = _pOriginal->m_nFontRelief;
        m_nFontEmphasis = _pOriginal->m_nFontEmphasis;
        m_aTextLineColor = _pOriginal->m_aTextLineColor;
        m_aTextColor = _pOriginal->m_aTextColor;
        m_bToolkitCompatibleDefaults = _pOriginal->m_bToolkitCompatibleDefaults;
    }


    bool FontControlModel::isFontRelatedProperty( sal_Int32 _nPropertyHandle )
    {
        return isFontAggregateProperty( _nPropertyHandle )
            || ( _nPropertyHandle == PROPERTY_ID_FONT )
            || ( _nPropertyHandle == PROPERTY_ID_FONTEMPHASISMARK )
            || ( _nPropertyHandle == PROPERTY_ID_FONTRELIEF )
            || ( _nPropertyHandle == PROPERTY_ID_TEXTLINECOLOR )
            || ( _nPropertyHandle == PROPERTY_ID_TEXTCOLOR );
    }


    bool FontControlModel::isFontAggregateProperty( sal_Int32 _nPropertyHandle )
    {
        return ( _nPropertyHandle == PROPERTY_ID_FONT_CHARWIDTH )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_ORIENTATION )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_WIDTH )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_NAME )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_STYLENAME )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_FAMILY )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_CHARSET )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_HEIGHT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_WEIGHT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_SLANT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_UNDERLINE )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_STRIKEOUT )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_WORDLINEMODE )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_PITCH )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_KERNING )
            || ( _nPropertyHandle == PROPERTY_ID_FONT_TYPE );
    }


    sal_Int32 FontControlModel::getTextColor( ) const
    {
        sal_Int32 nColor = COL_TRANSPARENT;
        m_aTextColor >>= nColor;
        return nColor;
    }


    sal_Int32 FontControlModel::getTextLineColor( ) const
    {
        sal_Int32 nColor = COL_TRANSPARENT;
        m_aTextLineColor >>= nColor;
        return nColor;
    }


    void FontControlModel::describeFontRelatedProperties( Sequence< Property >& /* [out] */ _rProps)
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
        DECL_PROP1      ( FONT_NAME,          OUString,  MAYBEDEFAULT );
        DECL_PROP1      ( FONT_STYLENAME,     OUString,  MAYBEDEFAULT );
        DECL_PROP1      ( FONT_FAMILY,        sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_CHARSET,       sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_HEIGHT,        float,            MAYBEDEFAULT );
        DECL_PROP1      ( FONT_WEIGHT,        float,            MAYBEDEFAULT );
        DECL_PROP1      ( FONT_SLANT,         sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_UNDERLINE,     sal_Int16,        MAYBEDEFAULT );
        DECL_PROP1      ( FONT_STRIKEOUT,     sal_Int16,        MAYBEDEFAULT );
        DECL_BOOL_PROP1 ( FONT_WORDLINEMODE,                    MAYBEDEFAULT );
    }


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


    bool FontControlModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue,
                sal_Int32 _nHandle, const Any& _rValue ) throw( IllegalArgumentException )
    {
        bool bModified = false;
        switch( _nHandle )
        {
        case PROPERTY_ID_TEXTCOLOR:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aTextColor, cppu::UnoType<sal_Int32>::get() );
            break;

        case PROPERTY_ID_TEXTLINECOLOR:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aTextLineColor, cppu::UnoType<sal_Int32>::get() );
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
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, aWorkAroundGccLimitation, cppu::UnoType<decltype(m_aFont)>::get() );
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
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aFont.Kerning );
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
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aFont.WordLineMode );
            break;

        default:
            OSL_FAIL( "FontControlModel::convertFastPropertyValue: no font aggregate!" );
        }
        return bModified;
    }


    static void setFastPropertyValue_NoBroadcast_implimpl(
            FontDescriptor & rFont,
            sal_Int32 nHandle, const Any& rValue) throw (Exception)
    {
        switch (nHandle)
        {
        case PROPERTY_ID_FONT_NAME:
            rValue >>= rFont.Name;
            break;

        case PROPERTY_ID_FONT_STYLENAME:
            rValue >>= rFont.StyleName;
            break;

        case PROPERTY_ID_FONT_FAMILY:
            rValue >>= rFont.Family;
            break;

        case PROPERTY_ID_FONT_CHARSET:
            rValue >>= rFont.CharSet;
            break;

        case PROPERTY_ID_FONT_CHARWIDTH:
            rValue >>= rFont.CharacterWidth;
            break;

        case PROPERTY_ID_FONT_KERNING:
            rValue >>= rFont.Kerning;
            break;

        case PROPERTY_ID_FONT_ORIENTATION:
            rValue >>= rFont.Orientation;
            break;

        case PROPERTY_ID_FONT_PITCH:
            rValue >>= rFont.Pitch;
            break;

        case PROPERTY_ID_FONT_TYPE:
            rValue >>= rFont.Type;
            break;

        case PROPERTY_ID_FONT_WIDTH:
            rValue >>= rFont.Width;
            break;

        case PROPERTY_ID_FONT_HEIGHT:
        {
            float nHeight = 0;
            rValue >>= nHeight;
            rFont.Height = (sal_Int16)nHeight;
        }
        break;

        case PROPERTY_ID_FONT_WEIGHT:
            rValue >>= rFont.Weight;
            break;

        case PROPERTY_ID_FONT_SLANT:
            rValue >>= rFont.Slant;
            break;

        case PROPERTY_ID_FONT_UNDERLINE:
            rValue >>= rFont.Underline;
            break;

        case PROPERTY_ID_FONT_STRIKEOUT:
            rValue >>= rFont.Strikeout;
            break;

        case PROPERTY_ID_FONT_WORDLINEMODE:
        {
            bool bWordLineMode = false;
            rValue >>= bWordLineMode;
            rFont.WordLineMode = bWordLineMode;
        }
        break;

        default:
            assert(false); // isFontAggregateProperty
        }
    }

    void FontControlModel::setFastPropertyValue_NoBroadcast_impl(
            ::cppu::OPropertySetHelper & rBase,
            void (::cppu::OPropertySetHelper::*pSet)(sal_Int32, Any const&),
            sal_Int32 nHandle, const Any& rValue) throw (Exception)
    {
        if (isFontAggregateProperty(nHandle))
        {
            // need to fire a event for PROPERTY_ID_FONT too apparently, so:
            FontDescriptor font(getFont());

            // first set new value on backup copy
            setFastPropertyValue_NoBroadcast_implimpl(font, nHandle, rValue);

            // then set that as the actual property - will eventually call
            // this method recursively again...
            (rBase.*pSet)(PROPERTY_ID_FONT, makeAny(font));
#ifndef NDEBUG
            // verify that the nHandle property has the new value
            Any tmp;
            getFastPropertyValue(tmp, nHandle);
            assert(tmp == rValue || PROPERTY_ID_FONT_HEIGHT == nHandle /*rounded*/);
#endif
        }
        else
        {
            switch (nHandle)
            {
            case PROPERTY_ID_TEXTCOLOR:
                m_aTextColor = rValue;
                break;

            case PROPERTY_ID_TEXTLINECOLOR:
                m_aTextLineColor = rValue;
                break;

            case PROPERTY_ID_FONTEMPHASISMARK:
                rValue >>= m_nFontEmphasis;
                break;

            case PROPERTY_ID_FONTRELIEF:
                rValue >>= m_nFontRelief;
                break;

            case PROPERTY_ID_FONT:
                rValue >>= m_aFont;
                break;

            default:
                SAL_WARN("forms.component", "invalid property: " << nHandle);
            }
        }
    }


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
            aReturn <<= css::awt::FontRelief::NONE;
            bHandled = true;
            break;
        }
        if ( bHandled )
            return aReturn;

        if ( m_bToolkitCompatibleDefaults )
        {
            EmptyFontDescriptor aEmpty;
            if ( PROPERTY_ID_FONT == _nHandle )
                return makeAny( FontDescriptor(aEmpty) );
            return lcl_extractFontDescriptorAggregate( _nHandle, aEmpty );
        }

        switch( _nHandle )
        {
        case PROPERTY_ID_FONT:
            aReturn <<= ::comphelper::getDefaultFont();
            break;

        case PROPERTY_ID_FONT_WORDLINEMODE:
            aReturn = makeBoolAny(false);
            break;

        case PROPERTY_ID_FONT_NAME:
        case PROPERTY_ID_FONT_STYLENAME:
            aReturn <<= OUString();
            break;

        case PROPERTY_ID_FONT_FAMILY:
        case PROPERTY_ID_FONT_CHARSET:
        case PROPERTY_ID_FONT_SLANT:
        case PROPERTY_ID_FONT_UNDERLINE:
        case PROPERTY_ID_FONT_STRIKEOUT:
            aReturn <<= (sal_Int16)1;
            break;

        case PROPERTY_ID_FONT_KERNING:
            aReturn = makeBoolAny(false);
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
            OSL_FAIL( "FontControlModel::getPropertyDefaultByHandle: invalid property!" );
        }

        return aReturn;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
