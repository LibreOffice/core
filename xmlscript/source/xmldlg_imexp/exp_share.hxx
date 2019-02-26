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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_EXP_SHARE_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_EXP_SHARE_HXX

#include "common.hxx"
#include <misc.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <memory>
#include <vector>


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

    explicit Style( short all_ )
        : _backgroundColor(0)
        , _textColor(0)
        , _textLineColor(0)
        , _border(0)
        , _borderColor(0)
        , _fontRelief(css::awt::FontRelief::NONE)
        , _fontEmphasisMark(css::awt::FontEmphasisMark::NONE)
        , _fillColor(0)
        , _visualEffect(0)
        , _all(all_)
        , _set(0)
    {
    }

    css::uno::Reference< css::xml::sax::XAttributeList > createElement();
};
class StyleBag
{
    ::std::vector< std::unique_ptr<Style> > _styles;

public:
    ~StyleBag() ;

    OUString getStyleId( Style const & rStyle );

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
    ElementDescriptor(
        css::uno::Reference< css::beans::XPropertySet > const & xProps,
        css::uno::Reference< css::beans::XPropertyState > const & xPropState,
        OUString const & name, css::uno::Reference< css::frame::XModel > const & xDocument )
        : XMLElement( name )
        , _xProps( xProps )
        , _xPropState( xPropState )
        , _xDocument( xDocument )
        {}
    explicit ElementDescriptor(
        OUString const & name )
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
    void readDoubleAttr(
        OUString const & rPropName, OUString const & rAttrName )
        { read<double>( rPropName, rAttrName ); }
    void readLongAttr(
        OUString const & rPropName, OUString const & rAttrName,
        bool forceAttribute = false )
        { read<sal_Int32>( rPropName, rAttrName, forceAttribute ); }
    void readHexLongAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readShortAttr(
        OUString const & rPropName, OUString const & rAttrName )
        { read<sal_Int32>( rPropName, rAttrName ); }
    inline void readBoolAttr(
        OUString const & rPropName, OUString const & rAttrName );

    void readAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readVerticalAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readImageAlignAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readImagePositionAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readDateAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readDateFormatAttr(
        OUString const & rPropName, OUString const & rAttrName );
    void readTimeAttr(
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
    void readImageScaleModeAttr(
        OUString const & rPropName, OUString const & rAttrName );

    void readDataAwareAttr(OUString const & rAttrName );
    void readImageOrGraphicAttr(OUString const & rAttrName );

    void addBoolAttr(
        OUString const & rAttrName, bool bValue )
        { addAttribute( rAttrName, OUString::boolean(bValue) ); }
    void addNumberFormatAttr(
        css::uno::Reference< css::beans::XPropertySet >
        const & xFormatProperties );

    void readEvents();
    void readDialogModel( StyleBag * all_styles );
    void readBullitinBoard( StyleBag * all_styles );
    void readMultiPageModel( StyleBag * all_styles );
    void readFrameModel( StyleBag * all_styles );
    void readPageModel( StyleBag * all_styles );
    void readButtonModel( StyleBag * all_styles );
    void readEditModel( StyleBag * all_styles );
    void readCheckBoxModel( StyleBag * all_styles );
    void readRadioButtonModel( StyleBag * all_styles );
    void readComboBoxModel( StyleBag * all_styles );
    void readCurrencyFieldModel( StyleBag * all_styles );
    void readDateFieldModel( StyleBag * all_styles );
    void readFileControlModel( StyleBag * all_styles );
    void readTreeControlModel( StyleBag * all_styles );
    void readFixedTextModel( StyleBag * all_styles );
    void readGroupBoxModel( StyleBag * all_styles );
    void readImageControlModel( StyleBag * all_styles );
    void readListBoxModel( StyleBag * all_styles );
    void readNumericFieldModel( StyleBag * all_styles );
    void readPatternFieldModel( StyleBag * all_styles );
    void readFormattedFieldModel( StyleBag * all_styles );
    void readTimeFieldModel( StyleBag * all_styles );
    void readFixedLineModel( StyleBag * all_styles );
    void readProgressBarModel( StyleBag * all_styles );
    void readScrollBarModel( StyleBag * all_styles );
    void readSpinButtonModel( StyleBag * all_styles );
    void readFixedHyperLinkModel( StyleBag * all_styles );
    void readGridControlModel( StyleBag * all_styles );
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
            addAttribute( attrName, OUString::number(v) );
        else
            OSL_FAIL( "### unexpected property type!" );
    }
}

template<>
inline void ElementDescriptor::read<sal_Bool>(
    OUString const & propName, OUString const & attrName,
    bool forceAttribute )
{
    if (forceAttribute ||
        css::beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( propName ))
    {
        css::uno::Any a( _xProps->getPropertyValue( propName ) );
        bool v;
        if (a >>= v)
            addAttribute( attrName, OUString::boolean(v) );
        else
            OSL_FAIL( "### unexpected property type!" );
    }
}

inline void ElementDescriptor::readBoolAttr(
    OUString const & rPropName, OUString const & rAttrName )
{
    read<sal_Bool>( rPropName, rAttrName );
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

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_EXP_SHARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
