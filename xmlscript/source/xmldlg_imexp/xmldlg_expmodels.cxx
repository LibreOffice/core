/*************************************************************************
 *
 *  $RCSfile: xmldlg_expmodels.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:48:57 $
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
#include "exp_share.hxx"

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>


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
    readButtonTypeAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("PushButtonType") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":button-type") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-src") ) );
    readImageAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageAlign") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":image-align") ) );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Repeat") ) ))
        readLongAttr( OUSTR("RepeatDelay"),
                      OUSTR(XMLNS_DIALOGS_PREFIX ":repeat"), true /* force */ );
    if (extract_throw<bool>( _xProps->getPropertyValue( OUSTR("Toggle") ) ))
        addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":toggled"), OUSTR("1") );
    readBoolAttr( OUSTR("FocusOnClick"),
                  OUSTR(XMLNS_DIALOGS_PREFIX ":grab-focus") );

    // State
    sal_Int16 nState;
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
            OSL_ENSURE( 0, "### unexpected radio state!" );
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
    Style aStyle( 0x2 | 0x8 | 0x20 | 0x40 );
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._backgroundColor)
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

    sal_Bool bTriState = sal_False;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TriState") ) ) >>= bTriState) && bTriState)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tristate") ),
                      OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
    }
    sal_Int16 nState;
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
            OSL_ENSURE( 0, "### unexpected checkbox state!" );
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

    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ) ) >>= itemValues) &&
        itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menupopup") ) );

        OUString const * pItemValues = itemValues.getConstArray();
        for ( sal_Int32 nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor(
                _xProps, _xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menuitem") ) );
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

    // string item list
    Sequence< OUString > itemValues;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ) ) >>= itemValues) &&
        itemValues.getLength() > 0)
    {
        ElementDescriptor * popup = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menupopup") ) );

        OUString const * pItemValues = itemValues.getConstArray();
        sal_Int32 nPos;
        for ( nPos = 0; nPos < itemValues.getLength(); ++nPos )
        {
            ElementDescriptor * item = new ElementDescriptor(
                _xProps, _xPropState,
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":menuitem") ) );
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
void ElementDescriptor::readRadioButtonModel( StyleBag * all_styles )
    SAL_THROW( (Exception) )
{
    // collect styles
    Style aStyle( 0x2 | 0x8 | 0x20 | 0x40 );
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

    sal_Int16 nState;
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
            OSL_ENSURE( 0, "### unexpected radio state!" );
            break;
        }
    }
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
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ) );
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
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
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
    sal_Int16 nEcho;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("EchoChar") ) ) >>= nEcho)
    {
        sal_Unicode cEcho = (sal_Unicode)nEcho;
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":echochar") ),
                      OUString( &cEcho, 1 ) );
    }
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
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
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
    }
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMin") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-min") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMax") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value-max") ) );
    readDoubleAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveValue") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );

    // format spec
    sal_Int32 nKey;
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
    Style aStyle( 0x4 );
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
    readDefaults( false );
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
    readEvents();
}

}
