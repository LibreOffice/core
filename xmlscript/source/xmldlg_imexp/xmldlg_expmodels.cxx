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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
#include "exp_share.hxx"

#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <comphelper/componentcontext.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace xmlscript
{

static inline bool readBorderProps(
    ElementDescriptor * element, Style & style )
{
    if (element->readProp( &style._border, OUSTR("Border") )) {
        if (style._border == BORDER_SIMPLE /* simple */)
        {
            if (element->readProp( &style._borderColor, OUSTR("BorderColor") ))
                style._border = BORDER_SIMPLE_COLOR;
        }
        return true;
    }
    return false;
}

static inline bool readFontProps( ElementDescriptor * element, Style & style )
{
    bool ret = element->readProp(
        &style._descr, OUSTR("FontDescriptor") );
    ret |= element->readProp(
        &style._fontEmphasisMark, OUSTR("FontEmphasisMark") );
    ret |= element->readProp(
        &style._fontRelief, OUSTR("FontRelief") );
    return ret;
}

//__________________________________________________________________________________________________
void ElementDescriptor::readMultiPageModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiPageValue") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    Any aDecorationAny( _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Decoration") ) ) );
    bool bDecoration = sal_True;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":withtabs") ), OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );

    readEvents();
    uno::Reference< container::XNameContainer > xPagesContainer( _xProps, uno::UNO_QUERY );
    if ( xPagesContainer.is() && xPagesContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":bulletinboard") ), _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFrameModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );

    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    OUString aTitle;

    if ( readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ) ) >>= aTitle)
    {
        ElementDescriptor * title = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ), _xDocument );
        title->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                             aTitle );
        addSubElement( title );
    }

    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":bulletinboard") ), _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readPageModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    rtl::OUString aTitle;
    readStringAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ) );
    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":bulletinboard") ), _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
    readEvents();
}

void ElementDescriptor::readButtonModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultButton") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":default") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readVerticalAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":valign") ) );
    readButtonTypeAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("PushButtonType") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":button-type") ) );
    readImageURLAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-src") ) );


    readImagePositionAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-position") ) );
    readImageAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageAlign") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-align") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Toggle") ) ))
        addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":toggled"), OUSTR("1") );
    readBoolAttr( OUSTR("FocusOnClick"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":grab-focus") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );


    // State
    sal_Int16 nState = 0;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ) ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            break;
        case 1:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        default:
            OSL_FAIL( "### unexpected radio state!" );
            break;
        }
    }

    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readCheckBoxModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 | 0x40 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( OUSTR("VisualEffect") ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readVerticalAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":valign") ) );
    readImageURLAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-src") ) );
    readImagePositionAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-position") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );

    sal_Bool bTriState = sal_False;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TriState") ) ) >>= bTriState) && bTriState)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tristate") ),
                      OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
    }
    sal_Int16 nState = 0;
    if (_xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ) ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            break;
        case 1:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        case 2: // tristate=true exported, checked omitted => dont know!
            OSL_ENSURE( bTriState, "### detected tristate value, but TriState is not set!" );
            break;
        default:
            OSL_FAIL( "### unexpected checkbox state!" );
            break;
        }
    }
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readComboBoxModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Autocomplete") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":autocomplete") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxlength") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":linecount") ) );
    // Cell Range, Ref Cell etc.
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readDataAwareAttr( OUSTR( XMLNS_DIALOGS_PREFIX ":source-cell-range") );

    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ) ) >>= itemValues) &&
        itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menupopup") ), _xDocument );

        OUString const * pItemValues = itemValues.getConstArray();
        for ( sal_Int32 nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor(
                _xProps, _xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menuitem") ), _xDocument );
            item->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                                pItemValues[ nPos ] );
            popup->addSubElement( item );
        }

        addSubElement( popup );
    }
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readListBoxModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiSelection") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiselection") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":linecount") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readDataAwareAttr( OUSTR( XMLNS_DIALOGS_PREFIX ":source-cell-range") );
    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ) ) >>= itemValues) &&
        itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menupopup") ), _xDocument );

        OUString const * pItemValues = itemValues.getConstArray();
        sal_Int32 nPos;
        for ( nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor(
                _xProps, _xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menuitem") ), _xDocument );
            item->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                                pItemValues[ nPos ] );
            popup->addSubElement( item );
        }

        Sequence< sal_Int16 > selected;
        if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") ) ) >>= selected)
        {
            sal_Int16 const * pSelected = selected.getConstArray();
            for ( nPos = selected.getLength(); nPos--; )
            {
                ElementDescriptor * item = static_cast< ElementDescriptor * >(
                    popup->getSubElement( pSelected[ nPos ] ).get() );
                item->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":selected") ),
                                    OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            }
        }

        addSubElement( popup );
    }
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readRadioButtonModel( StyleBag * all_styles  )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 | 0x40 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( OUSTR("VisualEffect") ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readVerticalAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":valign") ) );
    readImageURLAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-src") ) );
    readImagePositionAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-position") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("GroupName") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":group-name") ) );

    sal_Int16 nState = 0;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ) ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            break;
        case 1:
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        default:
            OSL_FAIL( "### unexpected radio state!" );
            break;
        }
    }
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readGroupBoxModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    OUString aTitle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ) ) >>= aTitle)
    {
        ElementDescriptor * title = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ), _xDocument );
        title->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                             aTitle );
        addSubElement( title );
    }

    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFixedTextModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readVerticalAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":valign") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("NoLabel") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":nolabel") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFixedHyperLinkModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("URL") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":url") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Description") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":description") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readVerticalAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":valign") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("NoLabel") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":nolabel") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readEditModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HardLineBreaks") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":hard-linebreaks") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HScroll") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":hscroll") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VScroll") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":vscroll") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxlength") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readLineEndFormatAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineEndFormat") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":lineend-format") ) );
    sal_Int16 nEcho = 0;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("EchoChar") ) ) >>= nEcho)
    {
        sal_Unicode cEcho = (sal_Unicode)nEcho;
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":echochar") ),
                      OUString( &cEcho, 1 ) );
    }
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImageControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleImage") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":scale-image") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readImageURLAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":src") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFileControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTreeControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readSelectionTypeAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectionType") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":selectiontype") ) );

    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("RootDisplayed") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":rootdisplayed") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowsHandles") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":showshandles") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowsRootHandles") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":showsroothandles") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Editable") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":editable") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("InvokesStopNodeEditing") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":invokesstopnodeediting") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("RowHeight") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":rowheight") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readCurrencyFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrencySymbol") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":currency-symbol") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":decimal-accuracy") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":thousands-separator") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-step") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    readBoolAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("PrependCurrencySymbol") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":prepend-symbol") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":enforce-format") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDateFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readDateFormatAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DateFormat") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":date-format") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DateShowCentury") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":show-century") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Date") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMax") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":dropdown") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":text") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":enforce-format") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readNumericFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":decimal-accuracy") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":thousands-separator") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-step") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":enforce-format") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTimeFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readTimeFormatAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeFormat") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":time-format") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Time") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMax") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":text") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":enforce-format") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readPatternFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxlength") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EditMask") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":edit-mask") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LiteralMask") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":literal-mask") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFormattedFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUSTR("HideInactiveSelection"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":hide-inactive-selection") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":strict-format") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":text") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxlength") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );

    Any a( readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveDefault") ) ) );
    switch (a.getValueTypeClass())
    {
    case TypeClass_DOUBLE:
        addAttribute(
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-default") ),
            OUString::valueOf( *(double const *)a.getValue() ) );
        break;
    case TypeClass_STRING:
        addAttribute(
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-default") ),
            *(OUString const *)a.getValue() );
        break;
    default:
        break;
    }
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMin") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMax") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveValue") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );

    // format spec
    sal_Int32 nKey = 0;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FormatKey") ) ) >>= nKey)
    {
        Reference< util::XNumberFormatsSupplier > xSupplier;
        if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FormatsSupplier") ) ) >>= xSupplier)
        {
            addNumberFormatAttr(
                xSupplier->getNumberFormats()->getByKey( nKey ),
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
        }
    }
    readBoolAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("TreatAsNumber") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":treat-as-number") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":enforce-format") ) );

    readEvents();
}

void ElementDescriptor::readSpinButtonModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinIncrement") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":increment") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinValue") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":curval") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinValueMax") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxval") ) );
    readLongAttr( OUSTR("SpinValueMin"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":minval") );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Repeat") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":repeat") ) );
    readLongAttr( OUSTR("RepeatDelay"), OUSTR(XMLNS_DIALOGS_PREFIX ":repeat-delay") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readHexLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SymbolColor") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":symbol-color") ) );
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readEvents();
}

//__________________________________________________________________________________________________
void ElementDescriptor::readFixedLineModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readOrientationAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readProgressBarModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 | 0x10 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FillColor") ) ) >>= aStyle._descr)
        aStyle._set |= 0x10;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValue") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValueMin") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValueMax") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readScrollBarModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("BlockIncrement") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":pageincrement") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineIncrement") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":increment") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ScrollValue") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":curpos") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ScrollValueMax") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxpos") ) );
    readLongAttr( OUSTR("ScrollValueMin"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":minpos") );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleSize") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":visible-size") ) );
    readLongAttr( OUSTR("RepeatDelay"), OUSTR(XMLNS_DIALOGS_PREFIX ":repeat") );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LiveScroll") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":live-scroll") ) );
    readHexLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("SymbolColor") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":symbol-color") ) );
    readDataAwareAttr( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDialogModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect elements
    addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_DIALOGS_PREFIX) ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_URI) ) );
    addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_SCRIPT_PREFIX) ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_URI) ) );

    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ) ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                      all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults( false, false );
    readBoolAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Closeable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":closeable") ) );
    readBoolAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Moveable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":moveable") ) );
    readBoolAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Sizeable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":resizeable") ) );
    readStringAttr(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ) );

    Any aDecorationAny( _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Decoration") ) ) );
    bool bDecoration = sal_False;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":withtitlebar") ),
                      OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
    readImageURLAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                           OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-src") ) );
    readEvents();
}

void ElementDescriptor::readBullitinBoard( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect elements
    ::std::vector< ElementDescriptor* > all_elements;
    // read out all props
    Reference<  container::XNameContainer > xDialogModel( _xProps, UNO_QUERY );
    if ( !xDialogModel.is() )
        return; // #TODO throw???
    Sequence< OUString > aElements( xDialogModel->getElementNames() );
    OUString const * pElements = aElements.getConstArray();

    ElementDescriptor * pRadioGroup = 0;

    sal_Int32 nPos;
    for ( nPos = 0; nPos < aElements.getLength(); ++nPos )
    {
        Any aControlModel( xDialogModel->getByName( pElements[ nPos ] ) );
        Reference< beans::XPropertySet > xProps;
        OSL_VERIFY( aControlModel >>= xProps );
        if (! xProps.is())
            continue;
        Reference< beans::XPropertyState > xPropState( xProps, UNO_QUERY );
        OSL_ENSURE( xPropState.is(), "no XPropertyState!" );
        if (! xPropState.is())
            continue;
        Reference< lang::XServiceInfo > xServiceInfo( xProps, UNO_QUERY );
        OSL_ENSURE( xServiceInfo.is(), "no XServiceInfo!" );
        if (! xServiceInfo.is())
            continue;

        ElementDescriptor * pElem = 0;

        // group up radio buttons
        if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ) ) )
        {
            if (! pRadioGroup) // open radiogroup
            {
                pRadioGroup = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":radiogroup") ), _xDocument );
                all_elements.push_back( pRadioGroup );
            }

            pElem = new ElementDescriptor(
                xProps, xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":radio") ), _xDocument );
            pElem->readRadioButtonModel( all_styles );
            pRadioGroup->addSubElement( pElem );
        }
        else // no radio
        {
            pRadioGroup = 0; // close radiogroup

            if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":button") ), _xDocument );
                pElem->readButtonModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checkbox") ), _xDocument );
                pElem->readCheckBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":combobox") ), _xDocument );
                pElem->readComboBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menulist") ), _xDocument );
                pElem->readListBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":titledbox") ), _xDocument );
                pElem->readGroupBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoMultiPageModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multipage") ), _xDocument );
                pElem->readMultiPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoFrameModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":frame") ), _xDocument );
                pElem->readFrameModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoPageModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":page") ), _xDocument );
                pElem->readPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":text") ), _xDocument );
                pElem->readFixedTextModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlEditModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":textfield") ), _xDocument );
                pElem->readEditModel( all_styles );
            }
            // FixedHyperLink
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedHyperlinkModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":linklabel") ), _xDocument );
                pElem->readFixedHyperLinkModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":img") ), _xDocument );
                pElem->readImageControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFileControlModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":filecontrol") ), _xDocument );
                pElem->readFileControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.tree.TreeControlModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":treecontrol") ), _xDocument );
                pElem->readTreeControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCurrencyFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":currencyfield") ), _xDocument );
                pElem->readCurrencyFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDateFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":datefield") ), _xDocument );
                pElem->readDateFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlNumericFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":numericfield") ), _xDocument );
                pElem->readNumericFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlTimeFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":timefield") ) , _xDocument);
                pElem->readTimeFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlPatternFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":patternfield") ), _xDocument );
                pElem->readPatternFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFormattedFieldModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":formattedfield") ), _xDocument );
                pElem->readFormattedFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":fixedline") ), _xDocument );
                pElem->readFixedLineModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlScrollBarModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":scrollbar") ), _xDocument );
                pElem->readScrollBarModel( all_styles );
            }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlSpinButtonModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spinbutton") ), _xDocument );
                pElem->readSpinButtonModel( all_styles );
             }
            else if (xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlProgressBarModel") ) ) )
            {
                pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":progressmeter") ), _xDocument );
                pElem->readProgressBarModel( all_styles );
            }

            if (pElem)
            {
                all_elements.push_back( pElem );
            }
            else
            {
                OSL_FAIL( "unknown control type!" );
                continue;
            }
        }
    }
    if (! all_elements.empty())
    {
        for ( std::size_t n = 0; n < all_elements.size(); ++n )
        {
            addSubElement( all_elements[ n ] );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
