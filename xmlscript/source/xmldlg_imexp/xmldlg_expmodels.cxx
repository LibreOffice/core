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
#include "exp_share.hxx"
#include <misc.hxx>
#include <xmlscript/xmlns.h>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <o3tl/any.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

static bool readBorderProps(
    ElementDescriptor * element, Style & style )
{
    if (element->readProp( &style._border, u"Border"_ustr )) {
        if (style._border == BORDER_SIMPLE /* simple */)
        {
            if (element->readProp( &style._borderColor, u"BorderColor"_ustr ))
                style._border = BORDER_SIMPLE_COLOR;
        }
        return true;
    }
    return false;
}

static bool readFontProps( ElementDescriptor * element, Style & style )
{
    bool ret = element->readProp(
        &style._descr, u"FontDescriptor"_ustr );
    ret |= element->readProp(
        &style._fontEmphasisMark, u"FontEmphasisMark"_ustr );
    ret |= element->readProp(
        &style._fontRelief, u"FontRelief"_ustr );
    return ret;
}

void ElementDescriptor::readMultiPageModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr ,  all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( u"MultiPageValue"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    Any aDecorationAny( _xProps->getPropertyValue( u"Decoration"_ustr ) );
    bool bDecoration = true;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":withtabs"_ustr, u"false"_ustr );

    readEvents();
    uno::Reference< container::XNameContainer > xPagesContainer( _xProps, uno::UNO_QUERY );
    if ( xPagesContainer.is() && xPagesContainer->getElementNames().hasElements() )
    {
        rtl::Reference<ElementDescriptor> pElem = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":bulletinboard"_ustr, _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
}

void ElementDescriptor::readFrameModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );

    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr,  all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    OUString aTitle;

    if ( readProp( u"Label"_ustr ) >>= aTitle)
    {
        rtl::Reference<ElementDescriptor> title = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":title"_ustr, _xDocument );
        title->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value"_ustr, aTitle );
        addSubElement( title );
    }
    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().hasElements() )
    {
        rtl::Reference<ElementDescriptor> pElem = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":bulletinboard"_ustr, _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
    readEvents();
}

void ElementDescriptor::readPageModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( u"Title"_ustr, u"" XMLNS_DIALOGS_PREFIX ":title"_ustr );
    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().hasElements() )
    {
        rtl::Reference<ElementDescriptor> pElem = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":bulletinboard"_ustr, _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
    readEvents();
}

void ElementDescriptor::readButtonModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"DefaultButton"_ustr, u"" XMLNS_DIALOGS_PREFIX ":default"_ustr );
    readStringAttr( u"Label"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readButtonTypeAttr( u"PushButtonType"_ustr, u"" XMLNS_DIALOGS_PREFIX ":button-type"_ustr );
    readImageOrGraphicAttr(u"" XMLNS_DIALOGS_PREFIX ":image-src"_ustr);
    readImagePositionAttr( u"ImagePosition"_ustr, u"" XMLNS_DIALOGS_PREFIX ":image-position"_ustr );
    readImageAlignAttr( u"ImageAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":image-align"_ustr );

    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );

    if (extract_throw<bool>( _xProps->getPropertyValue( u"Toggle"_ustr ) ))
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":toggled"_ustr, u"1"_ustr );

    readBoolAttr( u"FocusOnClick"_ustr, u"" XMLNS_DIALOGS_PREFIX ":grab-focus"_ustr );
    readBoolAttr( u"MultiLine"_ustr,u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );

    // State
    sal_Int16 nState = 0;
    if (readProp( u"State"_ustr ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"false"_ustr );
            break;
        case 1:
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"true"_ustr );
            break;
        default:
            OSL_FAIL( "### unexpected radio state!" );
            break;
        }
    }

    readEvents();
}

void ElementDescriptor::readCheckBoxModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 | 0x40 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( u"VisualEffect"_ustr ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readStringAttr( u"Label"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readAlignAttr( u"Align"_ustr,  u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readImageOrGraphicAttr(u"" XMLNS_DIALOGS_PREFIX ":image-src"_ustr);
    readImagePositionAttr( u"ImagePosition"_ustr, u"" XMLNS_DIALOGS_PREFIX ":image-position"_ustr );
    readBoolAttr( u"MultiLine"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );

    bool bTriState = false;
    if ((readProp( u"TriState"_ustr ) >>= bTriState) && bTriState)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":tristate"_ustr, u"true"_ustr );
    }
    sal_Int16 nState = 0;
    if (_xProps->getPropertyValue( u"State"_ustr ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"false"_ustr );
            break;
        case 1:
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"true"_ustr );
            break;
        case 2: // tristate=true exported, checked omitted => don't know!
            OSL_ENSURE( bTriState, "### detected tristate value, but TriState is not set!" );
            break;
        default:
            OSL_FAIL( "### unexpected checkbox state!" );
            break;
        }
    }
    readEvents();
}

void ElementDescriptor::readComboBoxModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readBoolAttr( u"Autocomplete"_ustr, u"" XMLNS_DIALOGS_PREFIX ":autocomplete"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"Dropdown"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    readShortAttr( u"MaxTextLen"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxlength"_ustr );
    readShortAttr( u"LineCount"_ustr, u"" XMLNS_DIALOGS_PREFIX ":linecount"_ustr );
    // Cell Range, Ref Cell etc.
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":source-cell-range"_ustr );

    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( u"StringItemList"_ustr ) >>= itemValues) &&  itemValues.hasElements())
    {
        rtl::Reference<ElementDescriptor> popup = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":menupopup"_ustr, _xDocument );

        for (const auto& rItemValue : itemValues)
        {
            rtl::Reference<ElementDescriptor> item = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":menuitem"_ustr, _xDocument );
            item->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value"_ustr, rItemValue );
            popup->addSubElement( item );
        }

        addSubElement( popup );
    }
    readEvents();
}

void ElementDescriptor::readListBoxModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"MultiSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiselection"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"Dropdown"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    readShortAttr( u"LineCount"_ustr, u"" XMLNS_DIALOGS_PREFIX ":linecount"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":source-cell-range"_ustr );
    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( u"StringItemList"_ustr ) >>= itemValues) && itemValues.hasElements())
    {
        rtl::Reference<ElementDescriptor> popup = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":menupopup"_ustr, _xDocument );

        for (const auto& rItemValue : itemValues)
        {
            rtl::Reference<ElementDescriptor> item = new ElementDescriptor(_xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":menuitem"_ustr, _xDocument );
            item->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value"_ustr, rItemValue );
            popup->addSubElement( item );
        }

        Sequence< sal_Int16 > selected;
        if (readProp( u"SelectedItems"_ustr ) >>= selected)
        {
            sal_Int16 const * pSelected = selected.getConstArray();
            for ( sal_Int32 nPos = selected.getLength(); nPos--; )
            {
                ElementDescriptor * item = static_cast< ElementDescriptor * >(
                    popup->getSubElement( pSelected[ nPos ] ).get() );
                item->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":selected"_ustr, u"true"_ustr );
            }
        }

        addSubElement( popup );
    }
    readEvents();
}

void ElementDescriptor::readRadioButtonModel( StyleBag * all_styles  )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 | 0x40 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( u"VisualEffect"_ustr ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr , all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr(u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readStringAttr( u"Label"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readImageOrGraphicAttr(u"" XMLNS_DIALOGS_PREFIX ":image-src"_ustr);
    readImagePositionAttr( u"ImagePosition"_ustr, u"" XMLNS_DIALOGS_PREFIX ":image-position"_ustr );
    readBoolAttr( u"MultiLine"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );
    readStringAttr( u"GroupName"_ustr, u"" XMLNS_DIALOGS_PREFIX ":group-name"_ustr );

    sal_Int16 nState = 0;
    if (readProp( u"State"_ustr ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute(u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"false"_ustr );
            break;
        case 1:
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":checked"_ustr, u"true"_ustr );
            break;
        default:
            OSL_FAIL( "### unexpected radio state!" );
            break;
        }
    }
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readEvents();
}

void ElementDescriptor::readGroupBoxModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    OUString aTitle;
    if (readProp( u"Label"_ustr ) >>= aTitle)
    {
        rtl::Reference<ElementDescriptor> title = new ElementDescriptor( _xProps, _xPropState, u"" XMLNS_DIALOGS_PREFIX ":title"_ustr, _xDocument );
        title->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value"_ustr, aTitle );
        addSubElement( title );
    }

    readEvents();
}

void ElementDescriptor::readFixedTextModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( u"Label"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"MultiLine"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"NoLabel"_ustr, u"" XMLNS_DIALOGS_PREFIX ":nolabel"_ustr );
    readEvents();
}

void ElementDescriptor::readFixedHyperLinkModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( u"Label"_ustr,u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readStringAttr( u"URL"_ustr, u"" XMLNS_DIALOGS_PREFIX ":url"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"MultiLine"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"NoLabel"_ustr, u"" XMLNS_DIALOGS_PREFIX ":nolabel"_ustr );
    readEvents();
}

void ElementDescriptor::readEditModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"HardLineBreaks"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hard-linebreaks"_ustr );
    readBoolAttr( u"HScroll"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hscroll"_ustr );
    readBoolAttr( u"VScroll"_ustr, u"" XMLNS_DIALOGS_PREFIX ":vscroll"_ustr );
    readShortAttr( u"MaxTextLen"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxlength"_ustr );
    readBoolAttr( u"MultiLine"_ustr, u"" XMLNS_DIALOGS_PREFIX ":multiline"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readLineEndFormatAttr( u"LineEndFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":lineend-format"_ustr );
    sal_Int16 nEcho = 0;
    if (readProp( u"EchoChar"_ustr ) >>= nEcho)
    {
        sal_Unicode cEcho = static_cast<sal_Unicode>(nEcho);
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":echochar"_ustr, OUString( &cEcho, 1 ) );
    }
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readEvents();
}

void ElementDescriptor::readImageControlModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"ScaleImage"_ustr, u"" XMLNS_DIALOGS_PREFIX ":scale-image"_ustr );
    readImageScaleModeAttr( u"ScaleMode"_ustr, u"" XMLNS_DIALOGS_PREFIX ":scale-mode"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readImageOrGraphicAttr(u"" XMLNS_DIALOGS_PREFIX ":src"_ustr);
    readEvents();
}

void ElementDescriptor::readFileControlModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readEvents();
}

void ElementDescriptor::readTreeControlModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readSelectionTypeAttr( u"SelectionType"_ustr, u"" XMLNS_DIALOGS_PREFIX ":selectiontype"_ustr );

    readBoolAttr( u"RootDisplayed"_ustr, u"" XMLNS_DIALOGS_PREFIX ":rootdisplayed"_ustr );
    readBoolAttr( u"ShowsHandles"_ustr, u"" XMLNS_DIALOGS_PREFIX ":showshandles"_ustr );
    readBoolAttr( u"ShowsRootHandles"_ustr, u"" XMLNS_DIALOGS_PREFIX ":showsroothandles"_ustr );
    readBoolAttr( u"Editable"_ustr, u"" XMLNS_DIALOGS_PREFIX ":editable"_ustr );
    readBoolAttr( u"InvokesStopNodeEditing"_ustr, u"" XMLNS_DIALOGS_PREFIX ":invokesstopnodeediting"_ustr );
    readLongAttr( u"RowHeight"_ustr, u"" XMLNS_DIALOGS_PREFIX ":rowheight"_ustr );
    readEvents();
}

void ElementDescriptor::readCurrencyFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readStringAttr( u"CurrencySymbol"_ustr, u"" XMLNS_DIALOGS_PREFIX ":currency-symbol"_ustr );
    readShortAttr( u"DecimalAccuracy"_ustr, u"" XMLNS_DIALOGS_PREFIX ":decimal-accuracy"_ustr );
    readBoolAttr( u"ShowThousandsSeparator"_ustr, u"" XMLNS_DIALOGS_PREFIX ":thousands-separator"_ustr );
    readDoubleAttr( u"Value"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readDoubleAttr( u"ValueMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readDoubleAttr(  u"ValueMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readDoubleAttr( u"ValueStep"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-step"_ustr );
    readBoolAttr(  u"Spin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );
    readBoolAttr(u"PrependCurrencySymbol"_ustr, u"" XMLNS_DIALOGS_PREFIX ":prepend-symbol"_ustr );
    readBoolAttr( u"EnforceFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":enforce-format"_ustr );
    readEvents();
}

void ElementDescriptor::readDateFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readDateFormatAttr( u"DateFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":date-format"_ustr );
    readBoolAttr( u"DateShowCentury"_ustr, u"" XMLNS_DIALOGS_PREFIX ":show-century"_ustr );
    readDateAttr( u"Date"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readDateAttr( u"DateMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readDateAttr( u"DateMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readBoolAttr( u"Spin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );
    readBoolAttr( u"Dropdown"_ustr, u"" XMLNS_DIALOGS_PREFIX ":dropdown"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":text"_ustr );
    readBoolAttr( u"EnforceFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":enforce-format"_ustr );
    readEvents();
}

void ElementDescriptor::readNumericFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readShortAttr( u"DecimalAccuracy"_ustr, u"" XMLNS_DIALOGS_PREFIX ":decimal-accuracy"_ustr );
    readBoolAttr( u"ShowThousandsSeparator"_ustr, u"" XMLNS_DIALOGS_PREFIX ":thousands-separator"_ustr );
    readDoubleAttr( u"Value"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readDoubleAttr( u"ValueMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readDoubleAttr( u"ValueMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readDoubleAttr( u"ValueStep"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-step"_ustr );
    readBoolAttr( u"Spin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );
    readBoolAttr( u"EnforceFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":enforce-format"_ustr );
    readEvents();
}

void ElementDescriptor::readTimeFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr);
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readTimeFormatAttr( u"TimeFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":time-format"_ustr );
    readTimeAttr( u"Time"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readTimeAttr( u"TimeMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readTimeAttr( u"TimeMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readBoolAttr( u"Spin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":text"_ustr );
    readBoolAttr( u"EnforceFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":enforce-format"_ustr );
    readEvents();
}

void ElementDescriptor::readPatternFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr, u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readShortAttr( u"MaxTextLen"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxlength"_ustr );
    readStringAttr( u"EditMask"_ustr, u"" XMLNS_DIALOGS_PREFIX ":edit-mask"_ustr );
    readStringAttr( u"LiteralMask"_ustr, u"" XMLNS_DIALOGS_PREFIX ":literal-mask"_ustr );
    readEvents();
}

void ElementDescriptor::readFormattedFieldModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"ReadOnly"_ustr, u"" XMLNS_DIALOGS_PREFIX ":readonly"_ustr );
    readBoolAttr( u"HideInactiveSelection"_ustr,u"" XMLNS_DIALOGS_PREFIX ":hide-inactive-selection"_ustr );
    readBoolAttr( u"StrictFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":strict-format"_ustr );
    readStringAttr( u"Text"_ustr, u"" XMLNS_DIALOGS_PREFIX ":text"_ustr );
    readAlignAttr( u"Align"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readShortAttr( u"MaxTextLen"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxlength"_ustr );
    readBoolAttr( u"Spin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":spin"_ustr );
    if (extract_throw<bool>( _xProps->getPropertyValue( u"Repeat"_ustr ) ))
        readLongAttr( u"RepeatDelay"_ustr,u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr, true /* force */ );

    Any a( readProp( u"EffectiveDefault"_ustr ) );
    switch (a.getValueTypeClass())
    {
    case TypeClass_DOUBLE:
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value-default"_ustr, OUString::number( *o3tl::forceAccess<double>(a) ) );
        break;
    case TypeClass_STRING:
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":value-default"_ustr, *o3tl::forceAccess<OUString>(a) );
        break;
    default:
        break;
    }
    readDoubleAttr( u"EffectiveMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readDoubleAttr( u"EffectiveMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readDoubleAttr( u"EffectiveValue"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );

    // format spec
    sal_Int32 nKey = 0;
    if (readProp( u"FormatKey"_ustr ) >>= nKey)
    {
        Reference< util::XNumberFormatsSupplier > xSupplier;
        if (readProp( u"FormatsSupplier"_ustr ) >>= xSupplier)
        {
            addNumberFormatAttr(
                xSupplier->getNumberFormats()->getByKey( nKey ) );
        }
    }
    readBoolAttr( u"TreatAsNumber"_ustr, u"" XMLNS_DIALOGS_PREFIX ":treat-as-number"_ustr );
    readBoolAttr( u"EnforceFormat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":enforce-format"_ustr );

    readEvents();
}

void ElementDescriptor::readSpinButtonModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( u"Orientation"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readLongAttr( u"SpinIncrement"_ustr, u"" XMLNS_DIALOGS_PREFIX ":increment"_ustr );
    readLongAttr( u"SpinValue"_ustr, u"" XMLNS_DIALOGS_PREFIX ":curval"_ustr );
    readLongAttr( u"SpinValueMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxval"_ustr );
    readLongAttr( u"SpinValueMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":minval"_ustr );
    readLongAttr( u"Repeat"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr );
    readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat-delay"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readHexLongAttr( u"SymbolColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":symbol-color"_ustr );
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readEvents();
}

void ElementDescriptor::readFixedLineModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( u"Label"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readOrientationAttr( u"Orientation"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readEvents();
}

void ElementDescriptor::readProgressBarModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 | 0x10 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readProp( u"FillColor"_ustr ) >>= aStyle._descr)
        aStyle._set |= 0x10;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( u"ProgressValue"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value"_ustr );
    readLongAttr( u"ProgressValueMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-min"_ustr );
    readLongAttr( u"ProgressValueMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":value-max"_ustr );
    readEvents();
}

void ElementDescriptor::readScrollBarModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( u"Orientation"_ustr, u"" XMLNS_DIALOGS_PREFIX ":align"_ustr );
    readLongAttr( u"BlockIncrement"_ustr, u"" XMLNS_DIALOGS_PREFIX ":pageincrement"_ustr );
    readLongAttr( u"LineIncrement"_ustr, u"" XMLNS_DIALOGS_PREFIX ":increment"_ustr );
    readLongAttr( u"ScrollValue"_ustr, u"" XMLNS_DIALOGS_PREFIX ":curpos"_ustr );
    readLongAttr( u"ScrollValueMax"_ustr, u"" XMLNS_DIALOGS_PREFIX ":maxpos"_ustr );
    readLongAttr( u"ScrollValueMin"_ustr, u"" XMLNS_DIALOGS_PREFIX ":minpos"_ustr );
    readLongAttr( u"VisibleSize"_ustr, u"" XMLNS_DIALOGS_PREFIX ":visible-size"_ustr );
    readLongAttr( u"RepeatDelay"_ustr, u"" XMLNS_DIALOGS_PREFIX ":repeat"_ustr );
    readBoolAttr( u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr );
    readBoolAttr( u"LiveScroll"_ustr, u"" XMLNS_DIALOGS_PREFIX ":live-scroll"_ustr );
    readHexLongAttr( u"SymbolColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":symbol-color"_ustr );
    readDataAwareAttr( u"" XMLNS_DIALOGS_PREFIX ":linked-cell"_ustr );
    readEvents();
}

void ElementDescriptor::readGridControlModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp(u"BackgroundColor"_ustr) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readProp(u"TextColor"_ustr) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp(u"TextLineColor"_ustr) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr,all_styles->getStyleId( aStyle ) );
    }
    // collect elements
    readDefaults();
    readBoolAttr(u"Tabstop"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tabstop"_ustr);
    readVerticalAlignAttr( u"VerticalAlign"_ustr, u"" XMLNS_DIALOGS_PREFIX ":valign"_ustr);
    readSelectionTypeAttr( u"SelectionModel"_ustr, u"" XMLNS_DIALOGS_PREFIX ":selectiontype"_ustr);
    readBoolAttr( u"ShowColumnHeader"_ustr, u"" XMLNS_DIALOGS_PREFIX ":showcolumnheader"_ustr);
    readBoolAttr( u"ShowRowHeader"_ustr, u"" XMLNS_DIALOGS_PREFIX ":showrowheader"_ustr);
    readHexLongAttr( u"GridLineColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":gridline-color"_ustr);
    readBoolAttr( u"UseGridLines"_ustr, u"" XMLNS_DIALOGS_PREFIX ":usegridlines"_ustr );
    readHexLongAttr( u"HeaderBackgroundColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":headerbackground-color"_ustr);
    readHexLongAttr( u"HeaderTextColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":headertext-color"_ustr);
    readHexLongAttr( u"ActiveSelectionBackgroundColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":activeselectionbackground-color"_ustr);
    readHexLongAttr( u"ActiveSelectionTextColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":activeselectiontext-color"_ustr);
    readHexLongAttr( u"InactiveSelectionBackgroundColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":inactiveselectionbackground-color"_ustr);
    readHexLongAttr( u"InactiveSelectionTextColor"_ustr, u"" XMLNS_DIALOGS_PREFIX ":inactiveselectiontext-color"_ustr);
    readEvents();
}

void ElementDescriptor::readDialogModel( StyleBag * all_styles )
{
    // collect elements
    addAttribute( u"xmlns:" XMLNS_DIALOGS_PREFIX ""_ustr, XMLNS_DIALOGS_URI );
    addAttribute( u"xmlns:" XMLNS_SCRIPT_PREFIX ""_ustr, XMLNS_SCRIPT_URI );

    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( u"BackgroundColor"_ustr ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( u"TextColor"_ustr ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( u"TextLineColor"_ustr ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults( false, false );
    readBoolAttr(u"Closeable"_ustr, u"" XMLNS_DIALOGS_PREFIX ":closeable"_ustr );
    readBoolAttr( u"Moveable"_ustr, u"" XMLNS_DIALOGS_PREFIX ":moveable"_ustr );
    readBoolAttr( u"Sizeable"_ustr, u"" XMLNS_DIALOGS_PREFIX ":resizeable"_ustr );
    readStringAttr( u"Title"_ustr, u"" XMLNS_DIALOGS_PREFIX ":title"_ustr );

    readScrollableSettings();
    Any aDecorationAny( _xProps->getPropertyValue( u"Decoration"_ustr ) );
    bool bDecoration = false;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":withtitlebar"_ustr, u"false"_ustr );
    readImageOrGraphicAttr(u"" XMLNS_DIALOGS_PREFIX ":image-src"_ustr);
    readEvents();
}

void ElementDescriptor::readBullitinBoard( StyleBag * all_styles )
{
    // collect elements
    ::std::vector< rtl::Reference<ElementDescriptor> > all_elements;
    // read out all props
    Reference<  container::XNameContainer > xDialogModel( _xProps, UNO_QUERY );
    if ( !xDialogModel.is() )
        return; // #TODO throw???
    const Sequence< OUString > aElements( xDialogModel->getElementNames() );

    rtl::Reference<ElementDescriptor> pRadioGroup;

    for ( const auto& rElement : aElements )
    {
        Any aControlModel( xDialogModel->getByName( rElement ) );
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

        rtl::Reference<ElementDescriptor> pElem;

        // group up radio buttons
        if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr ) )
        {
            if (! pRadioGroup) // open radiogroup
            {
                pRadioGroup = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":radiogroup"_ustr, _xDocument );
                all_elements.push_back( pRadioGroup );
            }

            pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":radio"_ustr, _xDocument );
            pElem->readRadioButtonModel( all_styles );
            pRadioGroup->addSubElement( pElem );
        }
        else // no radio
        {
            pRadioGroup = nullptr; // close radiogroup

            if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlButtonModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":button"_ustr, _xDocument );
                pElem->readButtonModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":checkbox"_ustr, _xDocument );
                pElem->readCheckBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlComboBoxModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":combobox"_ustr, _xDocument );
                pElem->readComboBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlListBoxModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":menulist"_ustr, _xDocument );
                pElem->readListBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlGroupBoxModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":titledbox"_ustr, _xDocument );
                pElem->readGroupBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoMultiPageModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":multipage"_ustr, _xDocument );
                pElem->readMultiPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoFrameModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":frame"_ustr, _xDocument );
                pElem->readFrameModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoPageModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":page"_ustr, _xDocument );
                pElem->readPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedTextModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":text"_ustr, _xDocument );
                pElem->readFixedTextModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlEditModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":textfield"_ustr, _xDocument );
                pElem->readEditModel( all_styles );
            }
            // FixedHyperLink
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedHyperlinkModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":linklabel"_ustr, _xDocument );
                pElem->readFixedHyperLinkModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlImageControlModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":img"_ustr, _xDocument );
                pElem->readImageControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFileControlModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":filecontrol"_ustr, _xDocument );
                pElem->readFileControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.tree.TreeControlModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":treecontrol"_ustr, _xDocument );
                pElem->readTreeControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlCurrencyFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":currencyfield"_ustr, _xDocument );
                pElem->readCurrencyFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlDateFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":datefield"_ustr, _xDocument );
                pElem->readDateFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlNumericFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":numericfield"_ustr, _xDocument );
                pElem->readNumericFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlTimeFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":timefield"_ustr, _xDocument);
                pElem->readTimeFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlPatternFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":patternfield"_ustr, _xDocument );
                pElem->readPatternFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFormattedFieldModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":formattedfield"_ustr, _xDocument );
                pElem->readFormattedFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedLineModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":fixedline"_ustr, _xDocument );
                pElem->readFixedLineModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlScrollBarModel"_ustr ) )
            {
                pElem = new ElementDescriptor(xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":scrollbar"_ustr, _xDocument );
                pElem->readScrollBarModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlSpinButtonModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":spinbutton"_ustr, _xDocument );
                pElem->readSpinButtonModel( all_styles );
             }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlProgressBarModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":progressmeter"_ustr, _xDocument );
                pElem->readProgressBarModel( all_styles );
            }
            else if (xServiceInfo->supportsService( u"com.sun.star.awt.grid.UnoControlGridModel"_ustr ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":table"_ustr, _xDocument );
                pElem->readGridControlModel( all_styles );
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
    for (const rtl::Reference<ElementDescriptor> & p : all_elements)
    {
        addSubElement( p );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
