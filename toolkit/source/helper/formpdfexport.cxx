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
#include <toolkit/helper/formpdfexport.hxx>
#include <tools/diagnose_ex.h>
#include <tools/lineend.hxx>
#include <unordered_map>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
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
#include <vcl/outdev.hxx>

#include <functional>
#include <algorithm>
#include <iterator>


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

    static const char FM_PROP_NAME[] = "Name";

    namespace
    {

        /** determines the FormComponentType of a form control
        */
        sal_Int16 classifyFormControl( const Reference< XPropertySet >& _rxModel )
        {
            static const char FM_PROP_CLASSID[] = "ClassId";
            sal_Int16 nControlType = FormComponentType::CONTROL;

            Reference< XPropertySetInfo > xPSI;
            if ( _rxModel.is() )
                xPSI = _rxModel->getPropertySetInfo();
            if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_CLASSID ) )
            {
                OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_CLASSID ) >>= nControlType );
            }

            return nControlType;
        }


        /** (default-)creates a PDF widget according to a given FormComponentType
        */
        vcl::PDFWriter::AnyWidget* createDefaultWidget( sal_Int16 _nFormComponentType )
        {
            switch ( _nFormComponentType )
            {
            case FormComponentType::COMMANDBUTTON:
                return new vcl::PDFWriter::PushButtonWidget;
            case FormComponentType::CHECKBOX:
                return new vcl::PDFWriter::CheckBoxWidget;
            case FormComponentType::RADIOBUTTON:
                return new vcl::PDFWriter::RadioButtonWidget;
            case FormComponentType::LISTBOX:
                return new vcl::PDFWriter::ListBoxWidget;
            case FormComponentType::COMBOBOX:
                return new vcl::PDFWriter::ComboBoxWidget;

            case FormComponentType::TEXTFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::DATEFIELD:
            case FormComponentType::TIMEFIELD:
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                return new vcl::PDFWriter::EditWidget;
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
                                if ( xModelSI->supportsService("com.sun.star.awt.UnoControlRadioButtonModel") )
                                {
                                    Reference< XPropertySet >  aProps( xElement, UNO_QUERY_THROW );

                                    OUString sGroupName;
                                    aProps->getPropertyValue("GroupName") >>= sGroupName;
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
                        SAL_WARN("toolkit","determineRadioGroupId: did not find the radios element's group!" );
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
            OSL_VERIFY( _rxModel->getPropertyValue( "StringItemList" ) >>= aListEntries );
            ::std::copy( aListEntries.begin(), aListEntries.end(),
                         ::std::back_insert_iterator< ::std::vector< OUString > >( _rVector ) );
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
            Descriptor.reset( createDefaultWidget( nControlType ) );
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
            OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_NAME ) >>= Descriptor->Name );
            OSL_VERIFY( xModelProps->getPropertyValue( "HelpText" ) >>= Descriptor->Description );
            Any aText;
            static const char FM_PROP_TEXT[] = "Text";
            static const char FM_PROP_LABEL[] = "Label";
            if ( xPSI->hasPropertyByName( FM_PROP_TEXT ) )
                aText = xModelProps->getPropertyValue( FM_PROP_TEXT );
            else if ( xPSI->hasPropertyByName( FM_PROP_LABEL ) )
                aText = xModelProps->getPropertyValue( FM_PROP_LABEL );
            if ( aText.hasValue() )
                OSL_VERIFY( aText >>= Descriptor->Text );


            // readonly
            static const char FM_PROP_READONLY[] = "ReadOnly";
            if ( xPSI->hasPropertyByName( FM_PROP_READONLY ) )
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_READONLY ) >>= Descriptor->ReadOnly );


            // border
            {
                static const char FM_PROP_BORDER[] = "Border";
                if ( xPSI->hasPropertyByName( FM_PROP_BORDER ) )
                {
                    sal_Int16 nBorderType = 0;
                    OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_BORDER ) >>= nBorderType );
                    Descriptor->Border = ( nBorderType != 0 );

                    OUString sBorderColorPropertyName( "BorderColor" );
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
            static const char FM_PROP_BACKGROUNDCOLOR[] = "BackgroundColor";
            if ( xPSI->hasPropertyByName( FM_PROP_BACKGROUNDCOLOR ) )
            {
                Color nBackColor = COL_TRANSPARENT;
                xModelProps->getPropertyValue( FM_PROP_BACKGROUNDCOLOR ) >>= nBackColor;
                Descriptor->Background = true;
                Descriptor->BackgroundColor = nBackColor;
            }


            // text color
            static const char FM_PROP_TEXTCOLOR[] = "TextColor";
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
            static const char FM_PROP_MULTILINE[] = "MultiLine";
            if ( xPSI->hasPropertyByName( FM_PROP_MULTILINE ) )
            {
                bool bMultiLine = false;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_MULTILINE ) >>= bMultiLine );
                if ( bMultiLine )
                    Descriptor->TextStyle |= DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;
            }

            // horizontal alignment
            static const char FM_PROP_ALIGN[] = "Align";
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
                OUString sVertAlignPropertyName( "VerticalAlign" );
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
            static const char FM_PROP_FONT[] = "FontDescriptor";
            if ( xPSI->hasPropertyByName( FM_PROP_FONT ) )
            {
                FontDescriptor aUNOFont;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_FONT ) >>= aUNOFont );
                Descriptor->TextFont = VCLUnoHelper::CreateFont( aUNOFont, vcl::Font() );
            }

            // tab order
            OUString aTabIndexString( "TabIndex" );
            if ( xPSI->hasPropertyByName( aTabIndexString ) )
            {
                sal_Int16 nIndex = -1;
                OSL_VERIFY( xModelProps->getPropertyValue( aTabIndexString ) >>= nIndex );
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
                OUString sEchoCharPropName( "EchoChar" );
                if ( xPSI->hasPropertyByName( sEchoCharPropName ) )
                {
                    sal_Int16 nEchoChar = 0;
                    if ( ( xModelProps->getPropertyValue( sEchoCharPropName ) >>= nEchoChar ) && ( nEchoChar != 0 ) )
                        pEditWidget->Password = true;
                }

                // file select
                if ( xSI->supportsService( "com.sun.star.form.component.FileControl" ) )
                    pEditWidget->FileSelect = true;

                // maximum text length
                static const char FM_PROP_MAXTEXTLEN[] = "MaxTextLen";
                if ( xPSI->hasPropertyByName( FM_PROP_MAXTEXTLEN ) )
                {
                    sal_Int16 nMaxTextLength = 0;
                    OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_MAXTEXTLEN ) >>= nMaxTextLength );
                    if ( nMaxTextLength <= 0 )
                        // "-1" has a special meaning for database-bound controls
                        nMaxTextLength = 0;
                    pEditWidget->MaxLen = nMaxTextLength;
                }
            }


            // buttons
            if ( Descriptor->getType() == vcl::PDFWriter::PushButton )
            {
                vcl::PDFWriter::PushButtonWidget* pButtonWidget = static_cast< vcl::PDFWriter::PushButtonWidget* >( Descriptor.get() );
                FormButtonType eButtonType = FormButtonType_PUSH;
                OSL_VERIFY( xModelProps->getPropertyValue("ButtonType") >>= eButtonType );
                static const char FM_PROP_TARGET_URL[] = "TargetURL";
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
                        if ( xParentSI.is() && xParentSI->supportsService("com.sun.star.form.component.HTMLForm") )
                        {
                            OSL_VERIFY( xParentProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= pButtonWidget->URL );
                            pButtonWidget->Submit = true;
                            FormSubmitMethod eMethod = FormSubmitMethod_POST;
                            OSL_VERIFY( xParentProps->getPropertyValue("SubmitMethod") >>= eMethod );
                            pButtonWidget->SubmitGet = (eMethod == FormSubmitMethod_GET);
                        }
                    }
                }
                else if ( eButtonType == FormButtonType_URL )
                {
                    OUString sURL;
                    OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= sURL );
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
            static const char FM_PROP_STATE[] = "State";
            if ( Descriptor->getType() == vcl::PDFWriter::CheckBox )
            {
                vcl::PDFWriter::CheckBoxWidget* pCheckBoxWidget = static_cast< vcl::PDFWriter::CheckBoxWidget* >( Descriptor.get() );
                sal_Int16 nState = 0;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState );
                pCheckBoxWidget->Checked = ( nState != 0 );
            }


            // radio buttons
            if ( Descriptor->getType() == vcl::PDFWriter::RadioButton )
            {
                vcl::PDFWriter::RadioButtonWidget* pRadioWidget = static_cast< vcl::PDFWriter::RadioButtonWidget* >( Descriptor.get() );
                sal_Int16 nState = 0;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState );
                pRadioWidget->Selected = ( nState != 0 );
                pRadioWidget->RadioGroup = determineRadioGroupId( xModelProps );
                try
                {
                    xModelProps->getPropertyValue( "RefValue" ) >>= pRadioWidget->OnValue;
                }
                catch(...)
                {
                    pRadioWidget->OnValue = "On";
                }
            }


            // list boxes
            if ( Descriptor->getType() == vcl::PDFWriter::ListBox )
            {
                vcl::PDFWriter::ListBoxWidget* pListWidget = static_cast< vcl::PDFWriter::ListBoxWidget* >( Descriptor.get() );

                // drop down
                OSL_VERIFY( xModelProps->getPropertyValue( "Dropdown" ) >>= pListWidget->DropDown );

                // multi selection
                OSL_VERIFY( xModelProps->getPropertyValue("MultiSelection") >>= pListWidget->MultiSelect );

                // entries
                getStringItemVector( xModelProps, pListWidget->Entries );

                // get selected items
                Sequence< sal_Int16 > aSelectIndices;
                OSL_VERIFY( xModelProps->getPropertyValue("SelectedItems") >>= aSelectIndices );
                if( aSelectIndices.getLength() > 0 )
                {
                    pListWidget->SelectedEntries.resize( 0 );
                    for( sal_Int32 i = 0; i < aSelectIndices.getLength(); i++ )
                    {
                        sal_Int16 nIndex = aSelectIndices.getConstArray()[i];
                        if( nIndex >= 0 && nIndex < static_cast<sal_Int16>(pListWidget->Entries.size()) )
                            pListWidget->SelectedEntries.push_back( nIndex );
                    }
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
            OSL_FAIL( "describePDFControl: caught an exception!" );
        }
        return Descriptor;
    }


} // namespace toolkitform


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
