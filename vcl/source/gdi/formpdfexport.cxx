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


#include <memory>
#include <vcl/formpdfexport.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/lineend.hxx>
#include <unordered_map>
#include <sal/log.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/unohelp.hxx>

#include <algorithm>
#include <iterator>


static vcl::Font CreateFont( const css::awt::FontDescriptor& rDescr )
{
    vcl::Font aFont;
    if ( !rDescr.Name.isEmpty() )
        aFont.SetFamilyName( rDescr.Name );
    if ( !rDescr.StyleName.isEmpty() )
        aFont.SetStyleName( rDescr.StyleName );
    if ( rDescr.Height )
        aFont.SetFontSize( Size( rDescr.Width, rDescr.Height ) );
    if ( static_cast<FontFamily>(rDescr.Family) != FAMILY_DONTKNOW )
        aFont.SetFamily( static_cast<FontFamily>(rDescr.Family) );
    if ( static_cast<rtl_TextEncoding>(rDescr.CharSet) != RTL_TEXTENCODING_DONTKNOW )
        aFont.SetCharSet( static_cast<rtl_TextEncoding>(rDescr.CharSet) );
    if ( static_cast<FontPitch>(rDescr.Pitch) != PITCH_DONTKNOW )
        aFont.SetPitch( static_cast<FontPitch>(rDescr.Pitch) );
    if ( rDescr.CharacterWidth )
        aFont.SetWidthType(vcl::unohelper::ConvertFontWidth(rDescr.CharacterWidth));
    if ( rDescr.Weight )
        aFont.SetWeight(vcl::unohelper::ConvertFontWeight(rDescr.Weight));
    if ( rDescr.Slant != css::awt::FontSlant_DONTKNOW )
        aFont.SetItalic(vcl::unohelper::ConvertFontSlant(rDescr.Slant));
    if ( static_cast<FontLineStyle>(rDescr.Underline) != LINESTYLE_DONTKNOW )
        aFont.SetUnderline( static_cast<FontLineStyle>(rDescr.Underline) );
    if ( static_cast<FontStrikeout>(rDescr.Strikeout) != STRIKEOUT_DONTKNOW )
        aFont.SetStrikeout( static_cast<FontStrikeout>(rDescr.Strikeout) );

    // Not DONTKNOW
    aFont.SetOrientation( Degree10(static_cast<sal_Int16>(rDescr.Orientation * 10)) );
    aFont.SetKerning( static_cast<FontKerning>(rDescr.Kerning) );
    aFont.SetWordLineMode( rDescr.WordLineMode );

    return aFont;
}

namespace toolkitform
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::style;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    constexpr OUString FM_PROP_NAME = u"Name"_ustr;

    namespace
    {

        /** determines the FormComponentType of a form control
        */
        sal_Int16 classifyFormControl( const Reference< XPropertySet >& _rxModel )
        {
            static constexpr OUString FM_PROP_CLASSID = u"ClassId"_ustr;
            sal_Int16 nControlType = FormComponentType::CONTROL;

            Reference< XPropertySetInfo > xPSI;
            if ( _rxModel.is() )
                xPSI = _rxModel->getPropertySetInfo();
            if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_CLASSID ) )
            {
                if( ! (_rxModel->getPropertyValue( FM_PROP_CLASSID ) >>= nControlType) ) {
                    SAL_WARN("toolkit.helper", "classifyFormControl: unable to get property " << FM_PROP_CLASSID);
                }
            }

            return nControlType;
        }


        /** (default-)creates a PDF widget according to a given FormComponentType
        */
        std::unique_ptr<vcl::PDFWriter::AnyWidget> createDefaultWidget( sal_Int16 _nFormComponentType )
        {
            switch ( _nFormComponentType )
            {
            case FormComponentType::COMMANDBUTTON:
                return std::make_unique<vcl::PDFWriter::PushButtonWidget>();
            case FormComponentType::CHECKBOX:
                return std::make_unique<vcl::PDFWriter::CheckBoxWidget>();
            case FormComponentType::RADIOBUTTON:
                return std::make_unique<vcl::PDFWriter::RadioButtonWidget>();
            case FormComponentType::LISTBOX:
                return std::make_unique<vcl::PDFWriter::ListBoxWidget>();
            case FormComponentType::COMBOBOX:
                return std::make_unique<vcl::PDFWriter::ComboBoxWidget>();

            case FormComponentType::TEXTFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::DATEFIELD:
            case FormComponentType::TIMEFIELD:
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                return std::make_unique<vcl::PDFWriter::EditWidget>();
            }
            return nullptr;
        }


        /** determines a unique number for the radio group which the given radio
            button model belongs to

            The number is guaranteed to be
            <ul><li>unique within the document in which the button lives</li>
                <li>the same for subsequent calls with other radio button models,
                    which live in the same document, and belong to the same group</li>
            </ul>

            @precond
                the model must be part of the form component hierarchy in a document
        */
        sal_Int32 determineRadioGroupId( const Reference< XPropertySet >& _rxRadioModel )
        {
            OSL_ENSURE( classifyFormControl( _rxRadioModel ) == FormComponentType::RADIOBUTTON,
                "determineRadioGroupId: this *is* no radio button model!" );
            // The fact that radio button groups need to be unique within the complete
            // host document makes it somewhat difficult ...
            // Problem is that two form radio buttons belong to the same group if
            // - they have the same parent
            // - AND they have the same name or group name
            // This implies that we need some knowledge about (potentially) *all* radio button
            // groups in the document.

            // get the root-level container
            Reference< XChild > xChild( _rxRadioModel, UNO_QUERY );
            Reference< XForm > xParentForm( xChild.is() ? xChild->getParent() : Reference< XInterface >(), UNO_QUERY );
            OSL_ENSURE( xParentForm.is(), "determineRadioGroupId: no parent form -> group id!" );
            if ( !xParentForm.is() )
                return -1;

            while ( xParentForm.is() )
            {
                xChild = xParentForm.get();
                xParentForm.set(xChild->getParent(), css::uno::UNO_QUERY);
            }
            Reference< XIndexAccess > xRoot( xChild->getParent(), UNO_QUERY );
            OSL_ENSURE( xRoot.is(), "determineRadioGroupId: unable to determine the root of the form component hierarchy!" );
            if ( !xRoot.is() )
                return -1;

            // count the leafs in the hierarchy, until we encounter radio button
            ::std::vector< Reference< XIndexAccess > > aAncestors;
            ::std::vector< sal_Int32 >                 aPath;

            Reference< XInterface > xNormalizedLookup( _rxRadioModel, UNO_QUERY );
            Reference< XIndexAccess > xCurrentContainer( xRoot );
            sal_Int32 nStartWithChild = 0;
            sal_Int32 nGroupsEncountered = 0;
            do
            {
                std::unordered_map<OUString,sal_Int32> GroupNameMap;
                std::unordered_map<OUString,sal_Int32> SharedNameMap;
                sal_Int32 nCount = xCurrentContainer->getCount();
                sal_Int32 i;
                for ( i = nStartWithChild; i < nCount; ++i )
                {
                    Reference< XInterface > xElement( xCurrentContainer->getByIndex( i ), UNO_QUERY );
                    if ( !xElement.is() )
                    {
                        OSL_FAIL( "determineRadioGroupId: very suspicious!" );
                        continue;
                    }

                    Reference< XIndexAccess > xNewContainer( xElement, UNO_QUERY );
                    if ( xNewContainer.is() )
                    {
                        // step down the hierarchy
                        aAncestors.push_back( xCurrentContainer );
                        xCurrentContainer = xNewContainer;
                        aPath.push_back( i );
                        nStartWithChild = 0;
                        break;
                            // out of the inner loop, but continue with the outer loop
                    }

                    if ( xElement.get() == xNormalizedLookup.get() )
                    {
                        // Our radio button is in this container.
                        // Now take the time to ID this container's groups and return the button's groupId
                        for ( i = 0; i < nCount; ++i )
                        {
                            try
                            {
                                xElement.set( xCurrentContainer->getByIndex( i ), UNO_QUERY_THROW );
                                Reference< XServiceInfo > xModelSI( xElement, UNO_QUERY_THROW );
                                if ( xModelSI->supportsService(u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr) )
                                {
                                    Reference< XPropertySet >  aProps( xElement, UNO_QUERY_THROW );

                                    OUString sGroupName;
                                    aProps->getPropertyValue(u"GroupName"_ustr) >>= sGroupName;
                                    if ( !sGroupName.isEmpty() )
                                    {
                                        // map: unique key is the group name, so attempts to add a different ID value
                                        // for an existing group are ignored - keeping the first ID - perfect for this scenario.
                                        GroupNameMap.emplace( sGroupName, nGroupsEncountered + i );

                                        if ( xElement.get() == xNormalizedLookup.get() )
                                            return GroupNameMap[sGroupName];
                                    }
                                    else
                                    {
                                        // Old implementation didn't have a GroupName, just identical Control names.
                                        aProps->getPropertyValue( FM_PROP_NAME ) >>= sGroupName;
                                        SharedNameMap.emplace( sGroupName, nGroupsEncountered + i );

                                        if ( xElement.get() == xNormalizedLookup.get() )
                                            return SharedNameMap[sGroupName];
                                    }

                                }
                            }
                            catch( uno::Exception& )
                            {
                                DBG_UNHANDLED_EXCEPTION("toolkit");
                            }
                        }
                        SAL_WARN("toolkit.helper","determineRadioGroupId: did not find the radios element's group!" );
                    }
                }

                // we encounter this container the first time. In particular, we did not just step up
                if ( nStartWithChild == 0 )
                {
                    // Our control wasn't in this container, so consider every item to be a possible unique group.
                    // This is way too much: Not all of the elements in the current container will form groups.
                    // But anyway, this number is sufficient for our purpose, since sequential group ids are not required.
                    // Ultimately, the container contains *at most* this many groups.
                    nGroupsEncountered += nCount;
                }

                if (  i >= nCount )
                {
                    // the loop terminated because there were no more elements
                    // -> step up, if possible
                    if ( aAncestors.empty() )
                        break;

                    xCurrentContainer = aAncestors.back(); aAncestors.pop_back();
                    nStartWithChild = aPath.back() + 1; aPath.pop_back();
                }
            }
            while ( true );
            return -1;
        }


        /** copies a StringItemList to a PDF widget's list
        */
        void getStringItemVector( const Reference< XPropertySet >& _rxModel, ::std::vector< OUString >& _rVector )
        {
            Sequence< OUString > aListEntries;
            if( ! (_rxModel->getPropertyValue( u"StringItemList"_ustr ) >>= aListEntries) ) {
                SAL_WARN("toolkit.helper", "getStringItemVector: unable to get property StringItemList");
            }
            _rVector.insert( _rVector.end(), std::cbegin(aListEntries), std::cend(aListEntries) );
        }
    }


    /** creates a PDF compatible control descriptor for the given control
    */
    std::unique_ptr<vcl::PDFWriter::AnyWidget> describePDFControl( const Reference< XControl >& _rxControl,
        vcl::PDFExtOutDevData& i_pdfExportData )
    {
        std::unique_ptr<vcl::PDFWriter::AnyWidget> Descriptor;
        OSL_ENSURE( _rxControl.is(), "describePDFControl: invalid (NULL) control!" );
        if ( !_rxControl.is() )
            return Descriptor;

        try
        {
            Reference< XPropertySet > xModelProps( _rxControl->getModel(), UNO_QUERY );
            sal_Int16 nControlType = classifyFormControl( xModelProps );
            Descriptor = createDefaultWidget( nControlType );
            if (!Descriptor)
                // no PDF widget available for this
                return Descriptor;

            Reference< XPropertySetInfo > xPSI( xModelProps->getPropertySetInfo() );
            Reference< XServiceInfo > xSI( xModelProps, UNO_QUERY );
            OSL_ENSURE( xSI.is(), "describePDFControl: no service info!" );
                // if we survived classifyFormControl, then it's a real form control, and they all have
                // service infos


            // set the common widget properties


            // Name, Description, Text
            if( ! (xModelProps->getPropertyValue( FM_PROP_NAME ) >>= Descriptor->Name) ) {
                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_NAME);
            }
            if( ! (xModelProps->getPropertyValue( u"HelpText"_ustr ) >>= Descriptor->Description) ) {
                SAL_INFO("toolkit.helper", "describePDFControl: unable to get property HelpText");
            }
            Any aText;
            static constexpr OUString FM_PROP_TEXT = u"Text"_ustr;
            static constexpr OUString FM_PROP_LABEL = u"Label"_ustr;
            static constexpr OUString FM_PROP_VALUE = u"Value"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_TEXT ) )
                aText = xModelProps->getPropertyValue( FM_PROP_TEXT );
            else if ( xPSI->hasPropertyByName( FM_PROP_LABEL ) )
                aText = xModelProps->getPropertyValue( FM_PROP_LABEL );
            else if ( xPSI->hasPropertyByName( FM_PROP_VALUE ) )
            {
                double aValue;
                if (xModelProps->getPropertyValue( FM_PROP_VALUE ) >>= aValue)
                    aText <<= OUString::number(aValue);
            }

            if ( aText.hasValue() ) {
                if( ! (aText >>= Descriptor->Text) ) {
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to assign aText to Descriptor->Text");
                }
            }


            // readonly
            static constexpr OUString FM_PROP_READONLY = u"ReadOnly"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_READONLY ) )
                if( ! (xModelProps->getPropertyValue( FM_PROP_READONLY ) >>= Descriptor->ReadOnly) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_READONLY);


            // border
            {
                static constexpr OUString FM_PROP_BORDER = u"Border"_ustr;
                if ( xPSI->hasPropertyByName( FM_PROP_BORDER ) )
                {
                    sal_Int16 nBorderType = 0;
                    if( ! (xModelProps->getPropertyValue( FM_PROP_BORDER ) >>= nBorderType) )
                        SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_BORDER);
                    Descriptor->Border = ( nBorderType != 0 );

                    OUString sBorderColorPropertyName( u"BorderColor"_ustr );
                    if ( xPSI->hasPropertyByName( sBorderColorPropertyName ) )
                    {
                        Color nBorderColor = COL_TRANSPARENT;
                        if ( xModelProps->getPropertyValue( sBorderColorPropertyName ) >>= nBorderColor )
                            Descriptor->BorderColor = nBorderColor;
                        else
                            Descriptor->BorderColor = COL_BLACK;
                    }
                }
            }


            // background color
            static constexpr OUString FM_PROP_BACKGROUNDCOLOR = u"BackgroundColor"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_BACKGROUNDCOLOR ) )
            {
                Color nBackColor = COL_TRANSPARENT;
                xModelProps->getPropertyValue( FM_PROP_BACKGROUNDCOLOR ) >>= nBackColor;
                Descriptor->Background = true;
                Descriptor->BackgroundColor = nBackColor;
            }


            // text color
            static constexpr OUString FM_PROP_TEXTCOLOR = u"TextColor"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_TEXTCOLOR ) )
            {
                Color nTextColor = COL_TRANSPARENT;
                xModelProps->getPropertyValue( FM_PROP_TEXTCOLOR ) >>= nTextColor;
                Descriptor->TextColor = nTextColor;
            }


            // text style
            Descriptor->TextStyle = DrawTextFlags::NONE;

            // multi line and word break
            // The MultiLine property of the control is mapped to both the "MULTILINE" and
            // "WORDBREAK" style flags
            static constexpr OUString FM_PROP_MULTILINE = u"MultiLine"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_MULTILINE ) )
            {
                bool bMultiLine = false;
                if( ! (xModelProps->getPropertyValue( FM_PROP_MULTILINE ) >>= bMultiLine) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_MULTILINE);
                if ( bMultiLine )
                    Descriptor->TextStyle |= DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;
            }

            // horizontal alignment
            static constexpr OUString FM_PROP_ALIGN = u"Align"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_ALIGN ) )
            {
                sal_Int16 nAlign = awt::TextAlign::LEFT;
                xModelProps->getPropertyValue( FM_PROP_ALIGN ) >>= nAlign;
                // TODO: when the property is VOID - are there situations/UIs where this
                // means something else than LEFT?
                switch ( nAlign )
                {
                case awt::TextAlign::LEFT:  Descriptor->TextStyle |= DrawTextFlags::Left; break;
                case awt::TextAlign::CENTER:  Descriptor->TextStyle |= DrawTextFlags::Center; break;
                case awt::TextAlign::RIGHT:  Descriptor->TextStyle |= DrawTextFlags::Right; break;
                default:
                    OSL_FAIL( "describePDFControl: invalid text align!" );
                }
            }

            // vertical alignment
            {
                OUString sVertAlignPropertyName( u"VerticalAlign"_ustr );
                if ( xPSI->hasPropertyByName( sVertAlignPropertyName ) )
                {
                    VerticalAlignment nAlign = VerticalAlignment_MIDDLE;
                    xModelProps->getPropertyValue( sVertAlignPropertyName ) >>= nAlign;
                    switch ( nAlign )
                    {
                    case VerticalAlignment_TOP:  Descriptor->TextStyle |= DrawTextFlags::Top; break;
                    case VerticalAlignment_MIDDLE:  Descriptor->TextStyle |= DrawTextFlags::VCenter; break;
                    case VerticalAlignment_BOTTOM:  Descriptor->TextStyle |= DrawTextFlags::Bottom; break;
                    default:
                        OSL_FAIL( "describePDFControl: invalid vertical text align!" );
                    }
                }
            }

            // font
            static constexpr OUString FM_PROP_FONT = u"FontDescriptor"_ustr;
            if ( xPSI->hasPropertyByName( FM_PROP_FONT ) )
            {
                FontDescriptor aUNOFont;
                if( ! (xModelProps->getPropertyValue( FM_PROP_FONT ) >>= aUNOFont) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_FONT);
                Descriptor->TextFont = CreateFont( aUNOFont );
            }

            // tab order
            OUString aTabIndexString( u"TabIndex"_ustr );
            if ( xPSI->hasPropertyByName( aTabIndexString ) )
            {
                sal_Int16 nIndex = -1;
                if( ! (xModelProps->getPropertyValue( aTabIndexString ) >>= nIndex) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << aTabIndexString);
                Descriptor->TabOrder = nIndex;
            }


            // special widget properties

            // edits
            if ( Descriptor->getType() == vcl::PDFWriter::Edit )
            {
                vcl::PDFWriter::EditWidget* pEditWidget = static_cast< vcl::PDFWriter::EditWidget* >( Descriptor.get() );

                // multiline (already flagged in the TextStyle)
                pEditWidget->MultiLine = bool( Descriptor->TextStyle & DrawTextFlags::MultiLine );

                // password input
                OUString sEchoCharPropName( u"EchoChar"_ustr );
                if ( xPSI->hasPropertyByName( sEchoCharPropName ) )
                {
                    sal_Int16 nEchoChar = 0;
                    if ( ( xModelProps->getPropertyValue( sEchoCharPropName ) >>= nEchoChar ) && ( nEchoChar != 0 ) )
                        pEditWidget->Password = true;
                }

                // file select
                if ( xSI->supportsService( u"com.sun.star.form.component.FileControl"_ustr ) )
                    pEditWidget->FileSelect = true;

                // maximum text length
                static constexpr OUString FM_PROP_MAXTEXTLEN = u"MaxTextLen"_ustr;
                if ( xPSI->hasPropertyByName( FM_PROP_MAXTEXTLEN ) )
                {
                    sal_Int16 nMaxTextLength = 0;
                    if( ! (xModelProps->getPropertyValue( FM_PROP_MAXTEXTLEN ) >>= nMaxTextLength) )
                        SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_MAXTEXTLEN);
                    if ( nMaxTextLength <= 0 )
                        // "-1" has a special meaning for database-bound controls
                        nMaxTextLength = 0;
                    pEditWidget->MaxLen = nMaxTextLength;
                }

                switch ( nControlType )
                {
                    case FormComponentType::CURRENCYFIELD:
                    case FormComponentType::NUMERICFIELD:
                    {

                        pEditWidget->Format = vcl::PDFWriter::Number;

                        static constexpr OUString FM_PROP_CURRENCYSYMBOL = u"CurrencySymbol"_ustr;
                        if ( xPSI->hasPropertyByName( FM_PROP_CURRENCYSYMBOL ) )
                        {
                            OUString sCurrencySymbol;
                            if( ! (xModelProps->getPropertyValue( FM_PROP_CURRENCYSYMBOL ) >>= sCurrencySymbol) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_CURRENCYSYMBOL);
                            pEditWidget->CurrencySymbol = sCurrencySymbol;
                        }

                        static constexpr OUString FM_PROP_DECIMALACCURACY = u"DecimalAccuracy"_ustr;
                        if ( xPSI->hasPropertyByName( FM_PROP_DECIMALACCURACY ) )
                        {
                            sal_Int32 nDecimalAccuracy = 0;
                            if( ! (xModelProps->getPropertyValue( FM_PROP_DECIMALACCURACY ) >>= nDecimalAccuracy) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_DECIMALACCURACY);
                            pEditWidget->DecimalAccuracy = nDecimalAccuracy;
                        }

                        static constexpr OUString FM_PROP_PREPENDCURRENCYSYMBOL = u"PrependCurrencySymbol"_ustr;
                        if ( xPSI->hasPropertyByName( FM_PROP_PREPENDCURRENCYSYMBOL ) )
                        {
                            bool bPrependCurrencySymbol = true;
                            if( ! (xModelProps->getPropertyValue( FM_PROP_PREPENDCURRENCYSYMBOL ) >>= bPrependCurrencySymbol) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_PREPENDCURRENCYSYMBOL);
                            pEditWidget->PrependCurrencySymbol = bPrependCurrencySymbol;
                        }
                    } break;
                    case FormComponentType::TIMEFIELD:
                    {
                        pEditWidget->Format = vcl::PDFWriter::Time;

                        static constexpr OUString FM_PROP_TIMEFORMAT = u"TimeFormat"_ustr;
                        if ( xPSI->hasPropertyByName( FM_PROP_TIMEFORMAT ) )
                        {
                            sal_Int32 nTimeFormat = 0;
                            if( ! (xModelProps->getPropertyValue( FM_PROP_TIMEFORMAT ) >>= nTimeFormat) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_TIMEFORMAT);

                            switch ( nTimeFormat )
                            {
                                case 0:
                                    pEditWidget->TimeFormat = "HH:MM"; //13:45
                                    break;
                                case 1:
                                    pEditWidget->TimeFormat = "HH:MM:ss"; //13:45:00
                                    break;
                                case 2:
                                    pEditWidget->TimeFormat = "h:MMtt"; //01:45 PM
                                    break;
                                case 3:
                                    pEditWidget->TimeFormat = "h:MM:sstt"; //01:45:00 PM
                                    break;
                            }
                        }
                    } break;
                    case FormComponentType::DATEFIELD:
                    {
                        pEditWidget->Format = vcl::PDFWriter::Date;

                        static constexpr OUString FM_PROP_DATEFORMAT = u"DateFormat"_ustr;
                        if ( xPSI->hasPropertyByName( FM_PROP_DATEFORMAT ) )
                        {
                            sal_Int32 nDateFormat = 0;
                            if( ! (xModelProps->getPropertyValue( FM_PROP_DATEFORMAT ) >>= nDateFormat) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_DATEFORMAT);

                            switch ( nDateFormat )
                            {
                                case 0:
                                case 1:
                                    pEditWidget->DateFormat = "mm/dd/yy"; // Standard (short)
                                    break;
                                case 2:
                                case 3:
                                    pEditWidget->DateFormat = "mm/dd/yyyy"; // Standard (long)
                                    break;
                                case 4:
                                    pEditWidget->DateFormat = "dd/mm/yy"; // DD/MM/YY
                                    break;
                                case 5:
                                    pEditWidget->DateFormat = "mm/dd/yy"; // MM/DD/YY
                                    break;
                                case 6:
                                    pEditWidget->DateFormat = "yy/mm/dd"; // YY/MM/DD
                                    break;
                                case 7:
                                    pEditWidget->DateFormat = "dd/mm/yyyy"; // DD/MM/YYYY
                                    break;
                                case 8:
                                    pEditWidget->DateFormat = "mm/dd/yyyy"; // MM/DD/YYYY
                                    break;
                                case 9:
                                    pEditWidget->DateFormat = "yyyy/mm/dd"; // YYYY/MM/DD
                                    break;
                                case 10:
                                    pEditWidget->DateFormat = "yy-mm-dd"; // YY-MM-DD
                                    break;
                                case 11:
                                    pEditWidget->DateFormat = "yyyy-mm-dd"; // YYYY-MM-DD
                                    break;
                            }
                        }
                    } break;
                }
            }

            // buttons
            if ( Descriptor->getType() == vcl::PDFWriter::PushButton )
            {
                vcl::PDFWriter::PushButtonWidget* pButtonWidget = static_cast< vcl::PDFWriter::PushButtonWidget* >( Descriptor.get() );
                FormButtonType eButtonType = FormButtonType_PUSH;
                if( ! (xModelProps->getPropertyValue(u"ButtonType"_ustr) >>= eButtonType) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property ButtonType");
                static constexpr OUString FM_PROP_TARGET_URL = u"TargetURL"_ustr;
                if ( eButtonType == FormButtonType_SUBMIT )
                {
                    // if a button is a submit button, then it uses the URL at its parent form
                    Reference< XChild > xChild( xModelProps, UNO_QUERY );
                    Reference < XPropertySet > xParentProps;
                    if ( xChild.is() )
                        xParentProps.set(xChild->getParent(), css::uno::UNO_QUERY);
                    if ( xParentProps.is() )
                    {
                        Reference< XServiceInfo > xParentSI( xParentProps, UNO_QUERY );
                        if ( xParentSI.is() && xParentSI->supportsService(u"com.sun.star.form.component.HTMLForm"_ustr) )
                        {
                            if( ! (xParentProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= pButtonWidget->URL) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_TARGET_URL);
                            pButtonWidget->Submit = true;
                            FormSubmitMethod eMethod = FormSubmitMethod_POST;
                            if( ! (xParentProps->getPropertyValue(u"SubmitMethod"_ustr) >>= eMethod) )
                                SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_TARGET_URL);
                            pButtonWidget->SubmitGet = (eMethod == FormSubmitMethod_GET);
                        }
                    }
                }
                else if ( eButtonType == FormButtonType_URL )
                {
                    OUString sURL;
                    if( ! (xModelProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= sURL) )
                        SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_TARGET_URL);
                    const bool bDocumentLocalTarget = sURL.startsWith("#");
                    if ( bDocumentLocalTarget )
                    {
                        // Register the destination for future handling ...
                        pButtonWidget->Dest = i_pdfExportData.RegisterDest();

                        // and put it into the bookmarks, to ensure the future handling really happens
                        ::std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks( i_pdfExportData.GetBookmarks() );
                        vcl::PDFExtOutDevBookmarkEntry aBookmark;
                        aBookmark.nDestId = pButtonWidget->Dest;
                        aBookmark.aBookmark = sURL;
                        rBookmarks.push_back( aBookmark );
                    }
                    else
                        pButtonWidget->URL = sURL;

                    pButtonWidget->Submit = false;
                }

               // TODO:
                // In PDF files, buttons are either reset, url or submit buttons. So if we have a simple PUSH button
                // in a document, then this means that we do not export a SubmitToURL, which means that in PDF,
                // the button is used as reset button.
                // Is this desired? If no, we would have to reset Descriptor to NULL here, in case eButtonType
                // != FormButtonType_SUBMIT && != FormButtonType_RESET

                // the PDF exporter defaults the text style, if 0. To prevent this, we have to transfer the UNO
                // defaults to the PDF widget
                if ( pButtonWidget->TextStyle == DrawTextFlags::NONE )
                    pButtonWidget->TextStyle = DrawTextFlags::Center | DrawTextFlags::VCenter;
            }


            // check boxes
            static constexpr OUString FM_PROP_STATE = u"State"_ustr;
            if ( Descriptor->getType() == vcl::PDFWriter::CheckBox )
            {
                vcl::PDFWriter::CheckBoxWidget* pCheckBoxWidget = static_cast< vcl::PDFWriter::CheckBoxWidget* >( Descriptor.get() );
                sal_Int16 nState = 0;
                if( ! (xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_STATE);
                pCheckBoxWidget->Checked = ( nState != 0 );

                try
                {
                    xModelProps->getPropertyValue( u"RefValue"_ustr ) >>= pCheckBoxWidget->OnValue;
                }
                catch(...)
                {
                }

                try
                {
                    xModelProps->getPropertyValue( u"SecondaryRefValue"_ustr ) >>= pCheckBoxWidget->OffValue;
                }
                catch(...)
                {
                }
            }


            // radio buttons
            if ( Descriptor->getType() == vcl::PDFWriter::RadioButton )
            {
                vcl::PDFWriter::RadioButtonWidget* pRadioWidget = static_cast< vcl::PDFWriter::RadioButtonWidget* >( Descriptor.get() );
                sal_Int16 nState = 0;
                if( ! (xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property " << FM_PROP_STATE);
                pRadioWidget->Selected = ( nState != 0 );
                pRadioWidget->RadioGroup = determineRadioGroupId( xModelProps );
                try
                {
                    xModelProps->getPropertyValue( u"RefValue"_ustr ) >>= pRadioWidget->OnValue;
                }
                catch(...)
                {
                }

                try
                {
                    xModelProps->getPropertyValue( u"SecondaryRefValue"_ustr ) >>= pRadioWidget->OffValue;
                }
                catch(...)
                {
                }
            }


            // list boxes
            if ( Descriptor->getType() == vcl::PDFWriter::ListBox )
            {
                vcl::PDFWriter::ListBoxWidget* pListWidget = static_cast< vcl::PDFWriter::ListBoxWidget* >( Descriptor.get() );

                // drop down
                if( ! (xModelProps->getPropertyValue( u"Dropdown"_ustr ) >>= pListWidget->DropDown) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property Dropdown");

                // multi selection
                if( ! (xModelProps->getPropertyValue(u"MultiSelection"_ustr) >>= pListWidget->MultiSelect) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property MultiSelection");

                // entries
                getStringItemVector( xModelProps, pListWidget->Entries );

                // get selected items
                Sequence< sal_Int16 > aSelectIndices;
                if( ! (xModelProps->getPropertyValue(u"SelectedItems"_ustr) >>= aSelectIndices) )
                    SAL_WARN("toolkit.helper", "describePDFControl: unable to get property SelectedItems");
                if( aSelectIndices.hasElements() )
                {
                    pListWidget->SelectedEntries.resize( 0 );
                    auto nEntriesSize = static_cast<sal_Int16>(pListWidget->Entries.size());
                    std::copy_if(std::cbegin(aSelectIndices), std::cend(aSelectIndices), std::back_inserter(pListWidget->SelectedEntries),
                        [&nEntriesSize](const sal_Int16 nIndex) { return nIndex >= 0 && nIndex < nEntriesSize; });
                }
            }


            // combo boxes
            if ( Descriptor->getType() == vcl::PDFWriter::ComboBox )
            {
                vcl::PDFWriter::ComboBoxWidget* pComboWidget = static_cast< vcl::PDFWriter::ComboBoxWidget* >( Descriptor.get() );

                // entries
                getStringItemVector( xModelProps, pComboWidget->Entries );
            }


            // some post-processing

            // text line ends
            // some controls may (always or dependent on other settings) return UNIX line ends
            Descriptor->Text = convertLineEnd(Descriptor->Text, LINEEND_CRLF);
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "toolkit", "describePDFControl" );
        }
        return Descriptor;
    }


} // namespace toolkitform


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
