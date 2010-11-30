/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "common.hxx"
#include "misc.hxx"
#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <vector>


namespace css = ::com::sun::star;

namespace xmlscript
{

struct Style
{
    sal_uInt32 _backgroundColor;
    sal_uInt32 _textColor;
    sal_uInt32 _textLineColor;
    sal_Int16 _border;
    sal_Int32 _borderColor;
    css::awt::FontDescriptor _descr;
    sal_uInt16 _fontRelief;
    sal_uInt16 _fontEmphasisMark;
    sal_uInt32 _fillColor;
    sal_Int16 _visualEffect;

    // current highest mask: 0x40
    short _all;
    short _set;

    ::rtl::OUString _id;

    inline Style( short all_ ) SAL_THROW( () )
        : _fontRelief( css::awt::FontRelief::NONE )
        , _fontEmphasisMark( css::awt::FontEmphasisMark::NONE )
        , _all( all_ )
        , _set( 0 )
        {}

    css::uno::Reference< css::xml::sax::XAttributeList > createElement();
};
class StyleBag
{
    ::std::vector< Style * > _styles;

public:
    ~StyleBag() SAL_THROW( () );

    ::rtl::OUString getStyleId( Style const & rStyle ) SAL_THROW( () );

    void dump( css::uno::Reference< css::xml::sax::XExtendedDocumentHandler >
               const & xOut );
};

class ElementDescriptor
    : public ::xmlscript::XMLElement
{
    css::uno::Reference< css::beans::XPropertySet > _xProps;
    css::uno::Reference< css::beans::XPropertyState > _xPropState;
    css::uno::Reference< css::frame::XModel > _xDocument;

public:
    inline ElementDescriptor(
        css::uno::Reference< css::beans::XPropertySet > const & xProps,
        css::uno::Reference< css::beans::XPropertyState > const & xPropState,
        ::rtl::OUString const & name, css::uno::Reference< css::frame::XModel > const & xDocument )
        SAL_THROW( () )
        : XMLElement( name )
        , _xProps( xProps )
        , _xPropState( xPropState )
        , _xDocument( xDocument )
        {}
    inline ElementDescriptor(
        ::rtl::OUString const & name )
        SAL_THROW( () )
        : XMLElement( name )
        {}

    template<typename T>
    inline void read(
        ::rtl::OUString const & propName, ::rtl::OUString const & attrName,
        bool forceAttribute = false );

    //
    template<typename T>
    inline bool readProp( T * ret, ::rtl::OUString const & rPropName );
    css::uno::Any readProp( ::rtl::OUString const & rPropName );
    //
    void readDefaults( bool supportPrintable = true, bool supportVisible = true );
    //
    void readStringAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    inline void readDoubleAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName )
        { read<double>( rPropName, rAttrName ); }
    inline void readLongAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName,
        bool forceAttribute = false )
        { read<sal_Int32>( rPropName, rAttrName, forceAttribute ); }
    void readHexLongAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    inline void readShortAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName )
        { read<sal_Int32>( rPropName, rAttrName ); }
    inline void readBoolAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName )
        { read<sal_Bool>( rPropName, rAttrName ); }

    void readAlignAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readVerticalAlignAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readImageURLAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readImageAlignAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readImagePositionAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readDateFormatAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readTimeFormatAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readOrientationAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readButtonTypeAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readLineEndFormatAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    void readSelectionTypeAttr(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName );
    //
    void readDataAwareAttr(
        ::rtl::OUString const & rAttrName );
    inline void addBoolAttr(
        ::rtl::OUString const & rAttrName, sal_Bool bValue )
        { addAttribute( rAttrName, ::rtl::OUString::valueOf(bValue) ); }
    void addNumberFormatAttr(
        css::uno::Reference< css::beans::XPropertySet >
        const & xFormatProperties,
        ::rtl::OUString const & rAttrName );

    //
    void readEvents() SAL_THROW( (css::uno::Exception) );
    //
    void readDialogModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readBullitinBoard( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readMultiPageModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFrameModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readPageModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readButtonModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readEditModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readCheckBoxModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readRadioButtonModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readComboBoxModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readCurrencyFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readDateFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFileControlModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readTreeControlModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFixedTextModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readGroupBoxModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readImageControlModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readListBoxModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readNumericFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readPatternFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFormattedFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readTimeFieldModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFixedLineModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readProgressBarModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readScrollBarModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readSpinButtonModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
    void readFixedHyperLinkModel( StyleBag * all_styles )
        SAL_THROW( (css::uno::Exception) );
};

template<typename T>
inline void ElementDescriptor::read(
    ::rtl::OUString const & propName, ::rtl::OUString const & attrName,
    bool forceAttribute )
{
    if (forceAttribute ||
        css::beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( propName ))
    {
        css::uno::Any a( _xProps->getPropertyValue( propName ) );
        T v = T();
        if (a >>= v)
            addAttribute( attrName, ::rtl::OUString::valueOf(v) );
        else
            OSL_ENSURE( 0, "### unexpected property type!" );
    }
}

template<typename T>
inline bool ElementDescriptor::readProp(
    T * ret, ::rtl::OUString const & rPropName )
{
    _xProps->getPropertyValue( rPropName ) >>= *ret;
    return css::beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( rPropName );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
