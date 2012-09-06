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
    if (element->readProp( &style._border, "Border" )) {
        if (style._border == BORDER_SIMPLE /* simple */)
        {
            if (element->readProp( &style._borderColor, "BorderColor" ))
                style._border = BORDER_SIMPLE_COLOR;
        }
        return true;
    }
    return false;
}

static inline bool readFontProps( ElementDescriptor * element, Style & style )
{
    bool ret = element->readProp(
        &style._descr, "FontDescriptor" );
    ret |= element->readProp(
        &style._fontEmphasisMark, "FontEmphasisMark" );
    ret |= element->readProp(
        &style._fontRelief, "FontRelief" );
    return ret;
}

//__________________________________________________________________________________________________
void ElementDescriptor::readMultiPageModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id" ,  all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( "MultiPageValue", XMLNS_DIALOGS_PREFIX ":value" );
    Any aDecorationAny( _xProps->getPropertyValue( "Decoration" ) );
    bool bDecoration = sal_True;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( XMLNS_DIALOGS_PREFIX ":withtabs", "false" );

    readEvents();
    uno::Reference< container::XNameContainer > xPagesContainer( _xProps, uno::UNO_QUERY );
    if ( xPagesContainer.is() && xPagesContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":bulletinboard", _xDocument );
        pElem->readBullitinBoard( all_styles );
        addSubElement( pElem );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFrameModel( StyleBag * all_styles )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );

    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id",  all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    OUString aTitle;

    if ( readProp( "Label" ) >>= aTitle)
    {
        ElementDescriptor * title = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":title", _xDocument );
        title->addAttribute( XMLNS_DIALOGS_PREFIX ":value", aTitle );
        addSubElement( title );
    }

    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":bulletinboard", _xDocument );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    OUString aTitle;
    readStringAttr( "Title", XMLNS_DIALOGS_PREFIX ":title" );
    uno::Reference< container::XNameContainer > xControlContainer( _xProps, uno::UNO_QUERY );
    if ( xControlContainer.is() && xControlContainer->getElementNames().getLength() )
    {
        ElementDescriptor * pElem = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":bulletinboard", _xDocument );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "DefaultButton", XMLNS_DIALOGS_PREFIX ":default" );
    readStringAttr( "Label", XMLNS_DIALOGS_PREFIX ":value" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readVerticalAlignAttr( "VerticalAlign", XMLNS_DIALOGS_PREFIX ":valign" );
    readButtonTypeAttr( "PushButtonType", XMLNS_DIALOGS_PREFIX ":button-type" );
    readImageURLAttr( "ImageURL", XMLNS_DIALOGS_PREFIX ":image-src" );


    readImagePositionAttr( "ImagePosition", XMLNS_DIALOGS_PREFIX ":image-position" );
    readImageAlignAttr( "ImageAlign", XMLNS_DIALOGS_PREFIX ":image-align" );

    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );

    if (extract_throw<bool>( _xProps->getPropertyValue( "Toggle" ) ))
        addAttribute( XMLNS_DIALOGS_PREFIX ":toggled", "1" );

    readBoolAttr( "FocusOnClick", XMLNS_DIALOGS_PREFIX ":grab-focus" );
    readBoolAttr( "MultiLine",XMLNS_DIALOGS_PREFIX ":multiline" );


    // State
    sal_Int16 nState = 0;
    if (readProp( "State" ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( XMLNS_DIALOGS_PREFIX ":checked", "false" );
            break;
        case 1:
            addAttribute( XMLNS_DIALOGS_PREFIX ":checked", "true" );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( "VisualEffect" ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readStringAttr( "Label", XMLNS_DIALOGS_PREFIX ":value" );
    readAlignAttr( "Align",  XMLNS_DIALOGS_PREFIX ":align" );
    readVerticalAlignAttr( "VerticalAlign", XMLNS_DIALOGS_PREFIX ":valign" );
    readImageURLAttr( "ImageURL", XMLNS_DIALOGS_PREFIX ":image-src" );
    readImagePositionAttr( "ImagePosition", XMLNS_DIALOGS_PREFIX ":image-position" );
    readBoolAttr( "MultiLine", XMLNS_DIALOGS_PREFIX ":multiline" );

    sal_Bool bTriState = sal_False;
    if ((readProp( "TriState" ) >>= bTriState) && bTriState)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":tristate", "true" );
    }
    sal_Int16 nState = 0;
    if (_xProps->getPropertyValue( "State" ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute( XMLNS_DIALOGS_PREFIX ":checked", "false" );
            break;
        case 1:
            addAttribute( XMLNS_DIALOGS_PREFIX ":checked", "true" );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":value" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readBoolAttr( "Autocomplete", XMLNS_DIALOGS_PREFIX ":autocomplete" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "Dropdown", XMLNS_DIALOGS_PREFIX ":spin" );
    readShortAttr( "MaxTextLen", XMLNS_DIALOGS_PREFIX ":maxlength" );
    readShortAttr( "LineCount", XMLNS_DIALOGS_PREFIX ":linecount" );
    // Cell Range, Ref Cell etc.
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":source-cell-range" );

    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( "StringItemList" ) >>= itemValues) &&  itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":menupopup", _xDocument );

        OUString const * pItemValues = itemValues.getConstArray();
        for ( sal_Int32 nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":menuitem", _xDocument );
            item->addAttribute( XMLNS_DIALOGS_PREFIX ":value", pItemValues[ nPos ] );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "MultiSelection", XMLNS_DIALOGS_PREFIX ":multiselection" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "Dropdown", XMLNS_DIALOGS_PREFIX ":spin" );
    readShortAttr( "LineCount", XMLNS_DIALOGS_PREFIX ":linecount" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":source-cell-range" );
    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( "StringItemList" ) >>= itemValues) && itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":menupopup", _xDocument );

        OUString const * pItemValues = itemValues.getConstArray();
        sal_Int32 nPos;
        for ( nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor(_xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":menuitem", _xDocument );
            item->addAttribute( XMLNS_DIALOGS_PREFIX ":value", pItemValues[ nPos ] );
            popup->addSubElement( item );
        }

        Sequence< sal_Int16 > selected;
        if (readProp( "SelectedItems" ) >>= selected)
        {
            sal_Int16 const * pSelected = selected.getConstArray();
            for ( nPos = selected.getLength(); nPos--; )
            {
                ElementDescriptor * item = static_cast< ElementDescriptor * >(
                    popup->getSubElement( pSelected[ nPos ] ).get() );
                item->addAttribute( XMLNS_DIALOGS_PREFIX ":selected", "true" );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (readProp( "VisualEffect" ) >>= aStyle._visualEffect)
        aStyle._set |= 0x40;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id" , all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr("Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readStringAttr( "Label", XMLNS_DIALOGS_PREFIX ":value" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readVerticalAlignAttr( "VerticalAlign", XMLNS_DIALOGS_PREFIX ":valign" );
    readImageURLAttr( "ImageURL", XMLNS_DIALOGS_PREFIX ":image-src" );
    readImagePositionAttr( "ImagePosition", XMLNS_DIALOGS_PREFIX ":image-position" );
    readBoolAttr( "MultiLine", XMLNS_DIALOGS_PREFIX ":multiline" );
    readStringAttr( "GroupName", XMLNS_DIALOGS_PREFIX ":group-name" );

    sal_Int16 nState = 0;
    if (readProp( "State" ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttribute(XMLNS_DIALOGS_PREFIX ":checked", "false" );
            break;
        case 1:
            addAttribute( XMLNS_DIALOGS_PREFIX ":checked", "true" );
            break;
        default:
            OSL_FAIL( "### unexpected radio state!" );
            break;
        }
    }
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readGroupBoxModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    OUString aTitle;
    if (readProp( "Label" ) >>= aTitle)
    {
        ElementDescriptor * title = new ElementDescriptor( _xProps, _xPropState, XMLNS_DIALOGS_PREFIX ":title", _xDocument );
        title->addAttribute( XMLNS_DIALOGS_PREFIX ":value", aTitle );
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
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( "Label", XMLNS_DIALOGS_PREFIX ":value" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readVerticalAlignAttr( "VerticalAlign", XMLNS_DIALOGS_PREFIX ":valign" );
    readBoolAttr( "MultiLine", XMLNS_DIALOGS_PREFIX ":multiline" );
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "NoLabel", XMLNS_DIALOGS_PREFIX ":nolabel" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFixedHyperLinkModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( "Label",XMLNS_DIALOGS_PREFIX ":value" );
    readStringAttr( "URL", XMLNS_DIALOGS_PREFIX ":url" );
    readStringAttr( "Description", XMLNS_DIALOGS_PREFIX ":description" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readVerticalAlignAttr( "VerticalAlign", XMLNS_DIALOGS_PREFIX ":valign" );
    readBoolAttr( "MultiLine", XMLNS_DIALOGS_PREFIX ":multiline" );
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "NoLabel", XMLNS_DIALOGS_PREFIX ":nolabel" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readEditModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readBoolAttr( "HardLineBreaks", XMLNS_DIALOGS_PREFIX ":hard-linebreaks" );
    readBoolAttr( "HScroll", XMLNS_DIALOGS_PREFIX ":hscroll" );
    readBoolAttr( "VScroll", XMLNS_DIALOGS_PREFIX ":vscroll" );
    readShortAttr( "MaxTextLen", XMLNS_DIALOGS_PREFIX ":maxlength" );
    readBoolAttr( "MultiLine", XMLNS_DIALOGS_PREFIX ":multiline" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":value" );
    readLineEndFormatAttr( "LineEndFormat", XMLNS_DIALOGS_PREFIX ":lineend-format" );
    sal_Int16 nEcho = 0;
    if (readProp( "EchoChar" ) >>= nEcho)
    {
        sal_Unicode cEcho = (sal_Unicode)nEcho;
        addAttribute( XMLNS_DIALOGS_PREFIX ":echochar", OUString( &cEcho, 1 ) );
    }
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImageControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "ScaleImage", XMLNS_DIALOGS_PREFIX ":scale-image" );
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readImageURLAttr( "ImageURL", XMLNS_DIALOGS_PREFIX ":src" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFileControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":value" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTreeControlModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readSelectionTypeAttr( "SelectionType", XMLNS_DIALOGS_PREFIX ":selectiontype" );

    readBoolAttr( "RootDisplayed", XMLNS_DIALOGS_PREFIX ":rootdisplayed" );
    readBoolAttr( "ShowsHandles", XMLNS_DIALOGS_PREFIX ":showshandles" );
    readBoolAttr( "ShowsRootHandles", XMLNS_DIALOGS_PREFIX ":showsroothandles" );
    readBoolAttr( "Editable", XMLNS_DIALOGS_PREFIX ":editable" );
    readBoolAttr( "InvokesStopNodeEditing", XMLNS_DIALOGS_PREFIX ":invokesstopnodeediting" );
    readLongAttr( "RowHeight", XMLNS_DIALOGS_PREFIX ":rowheight" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readCurrencyFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readStringAttr( "CurrencySymbol", XMLNS_DIALOGS_PREFIX ":currency-symbol" );
    readShortAttr( "DecimalAccuracy", XMLNS_DIALOGS_PREFIX ":decimal-accuracy" );
    readBoolAttr( "ShowThousandsSeparator", XMLNS_DIALOGS_PREFIX ":thousands-separator" );
    readDoubleAttr( "Value", XMLNS_DIALOGS_PREFIX ":value" );
    readDoubleAttr( "ValueMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readDoubleAttr(  "ValueMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readDoubleAttr( "ValueStep", XMLNS_DIALOGS_PREFIX ":value-step" );
    readBoolAttr(  "Spin", XMLNS_DIALOGS_PREFIX ":spin" );
    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );
    readBoolAttr("PrependCurrencySymbol", XMLNS_DIALOGS_PREFIX ":prepend-symbol" );
    readBoolAttr( "EnforceFormat", XMLNS_DIALOGS_PREFIX ":enforce-format" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDateFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readDateFormatAttr( "DateFormat", XMLNS_DIALOGS_PREFIX ":date-format" );
    readBoolAttr( "DateShowCentury", XMLNS_DIALOGS_PREFIX ":show-century" );
    readLongAttr( "Date", XMLNS_DIALOGS_PREFIX ":value" );
    readLongAttr( "DateMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readLongAttr( "DateMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readBoolAttr( "Spin", XMLNS_DIALOGS_PREFIX ":spin" );
    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );
    readBoolAttr( "Dropdown", XMLNS_DIALOGS_PREFIX ":dropdown" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":text" );
    readBoolAttr( "EnforceFormat", XMLNS_DIALOGS_PREFIX ":enforce-format" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readNumericFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readShortAttr( "DecimalAccuracy", XMLNS_DIALOGS_PREFIX ":decimal-accuracy" );
    readBoolAttr( "ShowThousandsSeparator", XMLNS_DIALOGS_PREFIX ":thousands-separator" );
    readDoubleAttr( "Value", XMLNS_DIALOGS_PREFIX ":value" );
    readDoubleAttr( "ValueMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readDoubleAttr( "ValueMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readDoubleAttr( "ValueStep", XMLNS_DIALOGS_PREFIX ":value-step" );
    readBoolAttr( "Spin", XMLNS_DIALOGS_PREFIX ":spin" );
    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );
    readBoolAttr( "EnforceFormat", XMLNS_DIALOGS_PREFIX ":enforce-format" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTimeFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop");
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readTimeFormatAttr( "TimeFormat", XMLNS_DIALOGS_PREFIX ":time-format" );
    readLongAttr( "Time", XMLNS_DIALOGS_PREFIX ":value" );
    readLongAttr( "TimeMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readLongAttr( "TimeMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readBoolAttr( "Spin", XMLNS_DIALOGS_PREFIX ":spin" );
    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":text" );
    readBoolAttr( "EnforceFormat", XMLNS_DIALOGS_PREFIX ":enforce-format" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readPatternFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection", XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":value" );
    readShortAttr( "MaxTextLen", XMLNS_DIALOGS_PREFIX ":maxlength" );
    readStringAttr( "EditMask", XMLNS_DIALOGS_PREFIX ":edit-mask" );
    readStringAttr( "LiteralMask", XMLNS_DIALOGS_PREFIX ":literal-mask" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFormattedFieldModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x4 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "ReadOnly", XMLNS_DIALOGS_PREFIX ":readonly" );
    readBoolAttr( "HideInactiveSelection",XMLNS_DIALOGS_PREFIX ":hide-inactive-selection" );
    readBoolAttr( "StrictFormat", XMLNS_DIALOGS_PREFIX ":strict-format" );
    readStringAttr( "Text", XMLNS_DIALOGS_PREFIX ":text" );
    readAlignAttr( "Align", XMLNS_DIALOGS_PREFIX ":align" );
    readShortAttr( "MaxTextLen", XMLNS_DIALOGS_PREFIX ":maxlength" );
    readBoolAttr( "Spin", XMLNS_DIALOGS_PREFIX ":spin" );
    if (extract_throw<bool>( _xProps->getPropertyValue( "Repeat" ) ))
        readLongAttr( "RepeatDelay",XMLNS_DIALOGS_PREFIX ":repeat", true /* force */ );

    Any a( readProp( "EffectiveDefault" ) );
    switch (a.getValueTypeClass())
    {
    case TypeClass_DOUBLE:
        addAttribute( XMLNS_DIALOGS_PREFIX ":value-default", OUString::valueOf( *(double const *)a.getValue() ) );
        break;
    case TypeClass_STRING:
        addAttribute( XMLNS_DIALOGS_PREFIX ":value-default", *(OUString const *)a.getValue() );
        break;
    default:
        break;
    }
    readDoubleAttr( "EffectiveMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readDoubleAttr( "EffectiveMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readDoubleAttr( "EffectiveValue", XMLNS_DIALOGS_PREFIX ":value" );

    // format spec
    sal_Int32 nKey = 0;
    if (readProp( "FormatKey" ) >>= nKey)
    {
        Reference< util::XNumberFormatsSupplier > xSupplier;
        if (readProp( "FormatsSupplier" ) >>= xSupplier)
        {
            addNumberFormatAttr(
                xSupplier->getNumberFormats()->getByKey( nKey ) );
        }
    }
    readBoolAttr( "TreatAsNumber", XMLNS_DIALOGS_PREFIX ":treat-as-number" );
    readBoolAttr( "EnforceFormat", XMLNS_DIALOGS_PREFIX ":enforce-format" );

    readEvents();
}

void ElementDescriptor::readSpinButtonModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( "Orientation", XMLNS_DIALOGS_PREFIX ":align" );
    readLongAttr( "SpinIncrement", XMLNS_DIALOGS_PREFIX ":increment" );
    readLongAttr( "SpinValue", XMLNS_DIALOGS_PREFIX ":curval" );
    readLongAttr( "SpinValueMax", XMLNS_DIALOGS_PREFIX ":maxval" );
    readLongAttr( "SpinValueMin", XMLNS_DIALOGS_PREFIX ":minval" );
    readLongAttr( "Repeat", XMLNS_DIALOGS_PREFIX ":repeat" );
    readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat-delay" );
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readHexLongAttr( "SymbolColor", XMLNS_DIALOGS_PREFIX ":symbol-color" );
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readEvents();
}

//__________________________________________________________________________________________________
void ElementDescriptor::readFixedLineModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 );
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readStringAttr( "Label", XMLNS_DIALOGS_PREFIX ":value" );
    readOrientationAttr( "Orientation", XMLNS_DIALOGS_PREFIX ":align" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readProgressBarModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 | 0x10 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (readProp( "FillColor" ) >>= aStyle._descr)
        aStyle._set |= 0x10;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readLongAttr( "ProgressValue", XMLNS_DIALOGS_PREFIX ":value" );
    readLongAttr( "ProgressValueMin", XMLNS_DIALOGS_PREFIX ":value-min" );
    readLongAttr( "ProgressValueMax", XMLNS_DIALOGS_PREFIX ":value-max" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readScrollBarModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x1 | 0x4 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readBorderProps( this, aStyle ))
        aStyle._set |= 0x4;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();
    readOrientationAttr( "Orientation", XMLNS_DIALOGS_PREFIX ":align" );
    readLongAttr( "BlockIncrement", XMLNS_DIALOGS_PREFIX ":pageincrement" );
    readLongAttr( "LineIncrement", XMLNS_DIALOGS_PREFIX ":increment" );
    readLongAttr( "ScrollValue", XMLNS_DIALOGS_PREFIX ":curpos" );
    readLongAttr( "ScrollValueMax", XMLNS_DIALOGS_PREFIX ":maxpos" );
    readLongAttr( "ScrollValueMin", XMLNS_DIALOGS_PREFIX ":minpos" );
    readLongAttr( "VisibleSize", XMLNS_DIALOGS_PREFIX ":visible-size" );
    readLongAttr( "RepeatDelay", XMLNS_DIALOGS_PREFIX ":repeat" );
    readBoolAttr( "Tabstop", XMLNS_DIALOGS_PREFIX ":tabstop" );
    readBoolAttr( "LiveScroll", XMLNS_DIALOGS_PREFIX ":live-scroll" );
    readHexLongAttr( "SymbolColor", XMLNS_DIALOGS_PREFIX ":symbol-color" );
    readDataAwareAttr( XMLNS_DIALOGS_PREFIX ":linked-cell" );
    readEvents();
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDialogModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect elements
    addAttribute( "xmlns:" XMLNS_DIALOGS_PREFIX, XMLNS_DIALOGS_URI );
    addAttribute( "xmlns:" XMLNS_SCRIPT_PREFIX, XMLNS_SCRIPT_URI );

    // collect styles
    Style aStyle( 0x1 | 0x2 | 0x8 | 0x20 );
    if (readProp( "BackgroundColor" ) >>= aStyle._backgroundColor)
        aStyle._set |= 0x1;
    if (readProp( "TextColor" ) >>= aStyle._textColor)
        aStyle._set |= 0x2;
    if (readProp( "TextLineColor" ) >>= aStyle._textLineColor)
        aStyle._set |= 0x20;
    if (readFontProps( this, aStyle ))
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults( false, false );
    readBoolAttr("Closeable", XMLNS_DIALOGS_PREFIX ":closeable" );
    readBoolAttr( "Moveable", XMLNS_DIALOGS_PREFIX ":moveable" );
    readBoolAttr( "Sizeable", XMLNS_DIALOGS_PREFIX ":resizeable" );
    readStringAttr( "Title", XMLNS_DIALOGS_PREFIX ":title" );

    readScrollableSettings();
    Any aDecorationAny( _xProps->getPropertyValue( "Decoration" ) );
    bool bDecoration = sal_False;
    if ( (aDecorationAny >>= bDecoration) && !bDecoration )
        addAttribute( XMLNS_DIALOGS_PREFIX ":withtitlebar", "false" );
    readImageURLAttr( "ImageURL", XMLNS_DIALOGS_PREFIX ":image-src" );
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
        if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
        {
            if (! pRadioGroup) // open radiogroup
            {
                pRadioGroup = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":radiogroup", _xDocument );
                all_elements.push_back( pRadioGroup );
            }

            pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":radio", _xDocument );
            pElem->readRadioButtonModel( all_styles );
            pRadioGroup->addSubElement( pElem );
        }
        else // no radio
        {
            pRadioGroup = 0; // close radiogroup

            if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":button", _xDocument );
                pElem->readButtonModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":checkbox", _xDocument );
                pElem->readCheckBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":combobox", _xDocument );
                pElem->readComboBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":menulist", _xDocument );
                pElem->readListBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":titledbox", _xDocument );
                pElem->readGroupBoxModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoMultiPageModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":multipage", _xDocument );
                pElem->readMultiPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoFrameModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":frame", _xDocument );
                pElem->readFrameModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoPageModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":page", _xDocument );
                pElem->readPageModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":text", _xDocument );
                pElem->readFixedTextModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":textfield", _xDocument );
                pElem->readEditModel( all_styles );
            }
            // FixedHyperLink
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedHyperlinkModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":linklabel", _xDocument );
                pElem->readFixedHyperLinkModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":img", _xDocument );
                pElem->readImageControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFileControlModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":filecontrol", _xDocument );
                pElem->readFileControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.tree.TreeControlModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":treecontrol", _xDocument );
                pElem->readTreeControlModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":currencyfield", _xDocument );
                pElem->readCurrencyFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":datefield", _xDocument );
                pElem->readDateFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":numericfield", _xDocument );
                pElem->readNumericFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":timefield", _xDocument);
                pElem->readTimeFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":patternfield", _xDocument );
                pElem->readPatternFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":formattedfield", _xDocument );
                pElem->readFormattedFieldModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":fixedline", _xDocument );
                pElem->readFixedLineModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
            {
                pElem = new ElementDescriptor(xProps, xPropState, XMLNS_DIALOGS_PREFIX ":scrollbar", _xDocument );
                pElem->readScrollBarModel( all_styles );
            }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlSpinButtonModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":spinbutton", _xDocument );
                pElem->readSpinButtonModel( all_styles );
             }
            else if (xServiceInfo->supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
            {
                pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":progressmeter", _xDocument );
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
