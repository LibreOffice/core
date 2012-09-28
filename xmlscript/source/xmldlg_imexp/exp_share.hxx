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

    OUString _id;

    inline Style( short all_ ) SAL_THROW(())
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
    ~StyleBag() SAL_THROW(());

    OUString getStyleId( Style const & rStyle ) SAL_THROW(());

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
        OUString const & name, css::uno::Reference< css::frame::XModel > const & xDocument )
        SAL_THROW(())
        : XMLElement( name )
        , _xProps( xProps )
        , _xPropState( xPropState )
        , _xDocument( xDocument )
        {}
    inline ElementDescriptor(
        OUString const & name )
        SAL_THROW(())
        : XMLElement( name )
        {}

    template<typename T>
    inline void read(
        OUString const & propName, OUString const & attrName,
        bool forceAttribute = false );

    template<typename T>
    inline bool readProp( T * ret, OUString const & rPropName );
    css::uno::Any readProp( OUString const & rPropName );
    void readScrollableSettings();
    void readDefaults( bool supportPrintable = true, bool supportVisible = true );
    void readStringAttr(
        OUString const & rPropName, OUString const & rAttrName );
    inline void readDoubleAttr(
        OUString const & rPropName, OUString const & rAttrName )
        { read<double>( rPropName, rAttrName ); }
    inline void readLongAttr(
        OUString const & rPropName, OUString const & rAttrName,
        bool forceAttribute = false )
        { read<sal_Int32>( rPropName, rAttrName, forceAttribute ); }
    void readHexLongAttr(
        OUString const & rPropName, OUString const & rAttrName );
    inline void readShortAttr(
        OUString const & rPropName, OUString const & rAttrName )
        { read<sal_Int32>( rPropName, rAttrName ); }
    inline void readBoolAttr(
        OUString const & rPropName, OUString const & rAttrName )
        { read<sal_Bool>( rPropName, rAttrName ); }

    void readAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readVerticalAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readImageURLAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readImageAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readImagePositionAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readDateFormatAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readTimeFormatAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readOrientationAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readButtonTypeAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readLineEndFormatAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readSelectionTypeAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readDataAwareAttr(
        OUString const & rAttrName );
    inline void addBoolAttr(
        OUString const & rAttrName, sal_Bool bValue )
        { addAttribute( rAttrName, OUString::valueOf(bValue) ); }
    void addNumberFormatAttr(
        css::uno::Reference< css::beans::XPropertySet >
        const & xFormatProperties );

    void readEvents() SAL_THROW( (css::uno::Exception) );
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
    OUString const & propName, OUString const & attrName,
    bool forceAttribute )
{
    if (forceAttribute ||
        css::beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( propName ))
    {
        css::uno::Any a( _xProps->getPropertyValue( propName ) );
        T v = T();
        if (a >>= v)
            addAttribute( attrName, OUString::valueOf(v) );
        else
            OSL_FAIL( "### unexpected property type!" );
    }
}

template<typename T>
inline bool ElementDescriptor::readProp(
    T * ret, OUString const & rPropName )
{
    _xProps->getPropertyValue( rPropName ) >>= *ret;
    return css::beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( rPropName );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
