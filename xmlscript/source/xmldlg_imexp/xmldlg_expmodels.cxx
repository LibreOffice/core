/*************************************************************************
 *
 *  $RCSfile: xmldlg_expmodels.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-20 14:05:25 $
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

namespace xmlscript
{

//__________________________________________________________________________________________________
void ElementDescriptor::readButtonModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultButton") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":default") ) );

    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
}
//__________________________________________________________________________________________________
void ElementDescriptor::readCheckBoxModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );

    sal_Bool bTriState = sal_False;
    if ((readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TriState") ) ) >>= bTriState) && bTriState)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tristate") ),
                 OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
    }
    sal_Int16 nState;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ) ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            break;
        case 1:
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        case 2: // tristate=true exported, checked omitted => dont know!
            OSL_ENSURE( bTriState, "### detected tristate value, but TriState is not set!" );
            break;
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readComboBoxModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ) ) >>= aStyle._border )
        aStyle._set |= 0x4;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Autocomplete") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":autocomplete") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin-button") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":maxlength") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
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
            item->addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                           pItemValues[ nPos ] );
            popup->addSubElem( item );
        }

        addSubElem( popup );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readListBoxModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ) ) >>= aStyle._border )
        aStyle._set |= 0x4;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiSelection") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiselection") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":readonly") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":spin-button") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
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
            item->addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                           pItemValues[ nPos ] );
            popup->addSubElem( item );
        }

        Sequence< sal_Int16 > selected;
        if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") ) ) >>= selected)
        {
            sal_Int16 const * pSelected = selected.getConstArray();
            for ( nPos = selected.getLength(); nPos--; )
            {
                ElementDescriptor * item = static_cast< ElementDescriptor * >(
                    popup->getSubElemAt( pSelected[ nPos ] ).get() );
                item->addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":selected") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            }
        }

        addSubElem( popup );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readRadioButtonModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );

    sal_Int16 nState;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ) ) >>= nState)
    {
        switch (nState)
        {
        case 0:
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            break;
        case 1:
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":checked") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        case 2: // tristate=true exported, checked omitted => dont know!
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tristate") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
            break;
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readGroupBoxModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    // defaults \ tabstop
    sal_Bool bEnabled;
    if (_xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Enabled") ) ) >>= bEnabled)
    {
        if (! bEnabled)
        {
            addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":disabled") ),
                     OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
        }
    }
    else
    {
        OSL_ENSURE( 0, "unexpected property type for \"Enabled\": not bool!" );
    }

    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Printable") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":printable") ) );
//      readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
//                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tabstop") ) );

    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("PositionX") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":left") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("PositionY") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":top") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":width") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":height") ) );

    OUString aTitle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ) ) >>= aTitle)
    {
        ElementDescriptor * title = new ElementDescriptor(
            _xProps, _xPropState,
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":title") ) );
        title->addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ),
                        aTitle );
        addSubElem( title );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFixedTextModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ) ) >>= aStyle._border )
        aStyle._set |= 0x4;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":value") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":multiline") ) );
    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
}
//__________________________________________________________________________________________________
void ElementDescriptor::readEditModel( StyleBag * all_styles )
    throw (Exception)
{
    // collect styles
    Style aStyle;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ) ) >>= aStyle._backgroundColor )
        aStyle._set |= 0x1;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ) ) >>= aStyle._textColor )
        aStyle._set |= 0x2;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ) ) >>= aStyle._border )
        aStyle._set |= 0x4;
    if (readProp( OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ) ) >>= aStyle._descr )
        aStyle._set |= 0x8;
    if (aStyle._set)
    {
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ),
                 all_styles->getStyleId( aStyle ) );
    }

    // collect elements
    readDefaults();

    readAlignAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":align") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HardLineBreaks") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":hard-linebreaks") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HScroll") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":hscroll") ) );
    readBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("VScroll") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":vscroll") ) );
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
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
        addAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":echochar") ),
                 OUString( &cEcho, 1 ) );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readCurrencyFieldModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDateFieldModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readFileControlModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImageControlModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readNumericFieldModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readPatternFieldModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTimeFieldModel( StyleBag * all_styles )
    throw (Exception)
{
}
//__________________________________________________________________________________________________
void ElementDescriptor::readContainerModel( StyleBag * all_styles )
    throw (Exception)
{
}

};
