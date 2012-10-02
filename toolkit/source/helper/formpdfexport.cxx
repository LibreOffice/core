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


#include <toolkit/helper/formpdfexport.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/outdev.hxx>

#include <functional>
#include <algorithm>

//........................................................................
namespace toolkitform
{
//........................................................................

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
        //--------------------------------------------------------------------
        /** determines the FormComponentType of a form control
        */
        sal_Int16 classifyFormControl( const Reference< XPropertySet >& _rxModel ) SAL_THROW(( Exception ))
        {
            static const ::rtl::OUString FM_PROP_CLASSID(RTL_CONSTASCII_USTRINGPARAM("ClassId"));
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

        //--------------------------------------------------------------------
        /** (default-)creates a PDF widget according to a given FormComponentType
        */
        ::vcl::PDFWriter::AnyWidget* createDefaultWidget( sal_Int16 _nFormComponentType )
        {
            switch ( _nFormComponentType )
            {
            case FormComponentType::COMMANDBUTTON:
                return new ::vcl::PDFWriter::PushButtonWidget;
            case FormComponentType::CHECKBOX:
                return new ::vcl::PDFWriter::CheckBoxWidget;
            case FormComponentType::RADIOBUTTON:
                return new ::vcl::PDFWriter::RadioButtonWidget;
            case FormComponentType::LISTBOX:
                return new ::vcl::PDFWriter::ListBoxWidget;
            case FormComponentType::COMBOBOX:
                return new ::vcl::PDFWriter::ComboBoxWidget;

            case FormComponentType::TEXTFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::DATEFIELD:
            case FormComponentType::TIMEFIELD:
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                return new ::vcl::PDFWriter::EditWidget;
            }
            return NULL;
        }

        //--------------------------------------------------------------------
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
        sal_Int32 determineRadioGroupId( const Reference< XPropertySet >& _rxRadioModel ) SAL_THROW((Exception))
        {
            OSL_ENSURE( classifyFormControl( _rxRadioModel ) == FormComponentType::RADIOBUTTON,
                "determineRadioGroupId: this *is* no radio button model!" );
            // The fact that radio button groups need to be unique within the complete
            // host document makes it somewhat difficult ...
            // Problem is that two form radio buttons belong to the same group if
            // - they have the same parent
            // - AND they have the same name
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
                xParentForm = xParentForm.query( xChild->getParent() );
            }
            Reference< XIndexAccess > xRoot( xChild->getParent(), UNO_QUERY );
            OSL_ENSURE( xRoot.is(), "determineRadioGroupId: unable to determine the root of the form component hierarchy!" );
            if ( !xRoot.is() )
                return -1;

            // count the leafs in the hierarchy, until we encounter radio button
            ::std::vector< Reference< XIndexAccess > > aAncestors;
            ::std::vector< sal_Int32 >                 aPath;

            Reference< XInterface > xNormalizedLookup( _rxRadioModel, UNO_QUERY );
            ::rtl::OUString sRadioGroupName;
            OSL_VERIFY( _rxRadioModel->getPropertyValue( rtl::OUString(FM_PROP_NAME) ) >>= sRadioGroupName );

            Reference< XIndexAccess > xCurrentContainer( xRoot );
            sal_Int32 nStartWithChild = 0;
            sal_Int32 nGroupsEncountered = 0;
            do
            {
                Reference< XNameAccess > xElementNameAccess( xCurrentContainer, UNO_QUERY );
                OSL_ENSURE( xElementNameAccess.is(), "determineRadioGroupId: no name container?" );
                if ( !xElementNameAccess.is() )
                    return -1;

                if ( nStartWithChild == 0 )
                {   // we encounter this container the first time. In particular, we did not
                    // just step up
                    nGroupsEncountered += xElementNameAccess->getElementNames().getLength();
                        // this is way too much: Not all of the elements in the current container
                        // may form groups, especially if they're forms. But anyway, this number is
                        // sufficient for our purpose. Finally, the container contains *at most*
                        // that much groups
                }

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
                        // look up the name of the radio group in the list of all element names
                        Sequence< ::rtl::OUString > aElementNames( xElementNameAccess->getElementNames() );
                        const ::rtl::OUString* pElementNames = aElementNames.getConstArray();
                        const ::rtl::OUString* pElementNamesEnd = pElementNames + aElementNames.getLength();
                        while ( pElementNames != pElementNamesEnd )
                        {
                            if ( *pElementNames == sRadioGroupName )
                            {
                                sal_Int32 nLocalGroupIndex = pElementNames - aElementNames.getConstArray();
                                OSL_ENSURE( nLocalGroupIndex < xElementNameAccess->getElementNames().getLength(),
                                    "determineRadioGroupId: inconsistency!" );

                                sal_Int32 nGlobalGroupId = nGroupsEncountered - xElementNameAccess->getElementNames().getLength() + nLocalGroupIndex;
                                return nGlobalGroupId;
                            }
                            ++pElementNames;
                        }
                        OSL_FAIL( "determineRadioGroupId: did not find the radios element name!" );
                    }
                }

                if ( !( i < nCount ) )
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

        //--------------------------------------------------------------------
        /** copies a StringItemList to a PDF widget's list
        */
        void getStringItemVector( const Reference< XPropertySet >& _rxModel, ::std::vector< ::rtl::OUString >& _rVector )
        {
            static const ::rtl::OUString FM_PROP_STRINGITEMLIST(RTL_CONSTASCII_USTRINGPARAM("StringItemList"));
            Sequence< ::rtl::OUString > aListEntries;
            OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_STRINGITEMLIST ) >>= aListEntries );
            ::std::copy( aListEntries.getConstArray(), aListEntries.getConstArray() + aListEntries.getLength(),
                ::std::back_insert_iterator< ::std::vector< ::rtl::OUString > >( _rVector ) );
        }
    }

    //--------------------------------------------------------------------
    /** creates a PDF compatible control descriptor for the given control
    */
    void TOOLKIT_DLLPUBLIC describePDFControl( const Reference< XControl >& _rxControl,
        ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget >& _rpDescriptor, ::vcl::PDFExtOutDevData& i_pdfExportData ) SAL_THROW(())
    {
        _rpDescriptor.reset( NULL );
        OSL_ENSURE( _rxControl.is(), "describePDFControl: invalid (NULL) control!" );
        if ( !_rxControl.is() )
            return;

        try
        {
            Reference< XPropertySet > xModelProps( _rxControl->getModel(), UNO_QUERY );
            sal_Int16 nControlType = classifyFormControl( xModelProps );
            _rpDescriptor.reset( createDefaultWidget( nControlType ) );
            if ( !_rpDescriptor.get() )
                // no PDF widget available for this
                return;

            Reference< XPropertySetInfo > xPSI( xModelProps->getPropertySetInfo() );
            Reference< XServiceInfo > xSI( xModelProps, UNO_QUERY );
            OSL_ENSURE( xSI.is(), "describePDFControl: no service info!" );
                // if we survived classifyFormControl, then it's a real form control, and they all have
                // service infos

            // ================================
            // set the common widget properties

            // --------------------------------
            // Name, Description, Text
            OSL_VERIFY( xModelProps->getPropertyValue( rtl::OUString(FM_PROP_NAME) ) >>= _rpDescriptor->Name );
            static const ::rtl::OUString FM_PROP_HELPTEXT(RTL_CONSTASCII_USTRINGPARAM("HelpText"));
            OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_HELPTEXT ) >>= _rpDescriptor->Description );
            Any aText;
            static const ::rtl::OUString FM_PROP_TEXT(RTL_CONSTASCII_USTRINGPARAM("Text"));
            static const ::rtl::OUString FM_PROP_LABEL(RTL_CONSTASCII_USTRINGPARAM("Label"));
            if ( xPSI->hasPropertyByName( FM_PROP_TEXT ) )
                aText = xModelProps->getPropertyValue( FM_PROP_TEXT );
            else if ( xPSI->hasPropertyByName( FM_PROP_LABEL ) )
                aText = xModelProps->getPropertyValue( FM_PROP_LABEL );
            if ( aText.hasValue() )
                OSL_VERIFY( aText >>= _rpDescriptor->Text );

            // --------------------------------
            // readonly
            static const ::rtl::OUString FM_PROP_READONLY(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            if ( xPSI->hasPropertyByName( FM_PROP_READONLY ) )
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_READONLY ) >>= _rpDescriptor->ReadOnly );

            // --------------------------------
            // border
            {
                static const ::rtl::OUString FM_PROP_BORDER(RTL_CONSTASCII_USTRINGPARAM("Border"));
                if ( xPSI->hasPropertyByName( FM_PROP_BORDER ) )
                {
                    sal_Int16 nBorderType = 0;
                    OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_BORDER ) >>= nBorderType );
                    _rpDescriptor->Border = ( nBorderType != 0 );

                    ::rtl::OUString sBorderColorPropertyName( RTL_CONSTASCII_USTRINGPARAM( "BorderColor" ) );
                    if ( xPSI->hasPropertyByName( sBorderColorPropertyName ) )
                    {
                        sal_Int32 nBoderColor = COL_TRANSPARENT;
                        if ( xModelProps->getPropertyValue( sBorderColorPropertyName ) >>= nBoderColor )
                            _rpDescriptor->BorderColor = Color( nBoderColor );
                        else
                            _rpDescriptor->BorderColor = Color( COL_BLACK );
                    }
                }
            }

            // --------------------------------
            // background color
            static const ::rtl::OUString FM_PROP_BACKGROUNDCOLOR(RTL_CONSTASCII_USTRINGPARAM("BackgroundColor"));
            if ( xPSI->hasPropertyByName( FM_PROP_BACKGROUNDCOLOR ) )
            {
                sal_Int32 nBackColor = COL_TRANSPARENT;
                xModelProps->getPropertyValue( FM_PROP_BACKGROUNDCOLOR ) >>= nBackColor;
                _rpDescriptor->Background = true;
                _rpDescriptor->BackgroundColor = Color( nBackColor );
            }

            // --------------------------------
            // text color
            static const ::rtl::OUString FM_PROP_TEXTCOLOR(RTL_CONSTASCII_USTRINGPARAM("TextColor"));
            if ( xPSI->hasPropertyByName( FM_PROP_TEXTCOLOR ) )
            {
                sal_Int32 nTextColor = COL_TRANSPARENT;
                xModelProps->getPropertyValue( FM_PROP_TEXTCOLOR ) >>= nTextColor;
                _rpDescriptor->TextColor = Color( nTextColor );
            }

            // --------------------------------
            // text style
            _rpDescriptor->TextStyle = 0;
            // ............................
            // multi line and word break
            // The MultiLine property of the control is mapped to both the "MULTILINE" and
            // "WORDBREAK" style flags
            static const ::rtl::OUString FM_PROP_MULTILINE(RTL_CONSTASCII_USTRINGPARAM("MultiLine"));
            if ( xPSI->hasPropertyByName( FM_PROP_MULTILINE ) )
            {
                sal_Bool bMultiLine = sal_False;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_MULTILINE ) >>= bMultiLine );
                if ( bMultiLine )
                    _rpDescriptor->TextStyle |= TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;
            }
            // ............................
            // horizontal alignment
            static const ::rtl::OUString FM_PROP_ALIGN(RTL_CONSTASCII_USTRINGPARAM("Align"));
            if ( xPSI->hasPropertyByName( FM_PROP_ALIGN ) )
            {
                sal_Int16 nAlign = awt::TextAlign::LEFT;
                xModelProps->getPropertyValue( FM_PROP_ALIGN ) >>= nAlign;
                // TODO: when the property is VOID - are there situations/UIs where this
                // means something else than LEFT?
                switch ( nAlign )
                {
                case awt::TextAlign::LEFT:  _rpDescriptor->TextStyle |= TEXT_DRAW_LEFT; break;
                case awt::TextAlign::CENTER:  _rpDescriptor->TextStyle |= TEXT_DRAW_CENTER; break;
                case awt::TextAlign::RIGHT:  _rpDescriptor->TextStyle |= TEXT_DRAW_RIGHT; break;
                default:
                    OSL_FAIL( "describePDFControl: invalid text align!" );
                }
            }
            // ............................
            // vertical alignment
            {
                ::rtl::OUString sVertAlignPropertyName( RTL_CONSTASCII_USTRINGPARAM( "VerticalAlign" ) );
                if ( xPSI->hasPropertyByName( sVertAlignPropertyName ) )
                {
                    sal_Int16 nAlign = VerticalAlignment_MIDDLE;
                    xModelProps->getPropertyValue( sVertAlignPropertyName ) >>= nAlign;
                    switch ( nAlign )
                    {
                    case VerticalAlignment_TOP:  _rpDescriptor->TextStyle |= TEXT_DRAW_TOP; break;
                    case VerticalAlignment_MIDDLE:  _rpDescriptor->TextStyle |= TEXT_DRAW_VCENTER; break;
                    case VerticalAlignment_BOTTOM:  _rpDescriptor->TextStyle |= TEXT_DRAW_BOTTOM; break;
                    default:
                        OSL_FAIL( "describePDFControl: invalid vertical text align!" );
                    }
                }
            }

            // font
            static const ::rtl::OUString FM_PROP_FONT(RTL_CONSTASCII_USTRINGPARAM("FontDescriptor"));
            if ( xPSI->hasPropertyByName( FM_PROP_FONT ) )
            {
                FontDescriptor aUNOFont;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_FONT ) >>= aUNOFont );
                _rpDescriptor->TextFont = VCLUnoHelper::CreateFont( aUNOFont, Font() );
            }

            // tab order
            rtl::OUString aTabIndexString( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) );
            if ( xPSI->hasPropertyByName( aTabIndexString ) )
            {
                sal_Int16 nIndex = -1;
                OSL_VERIFY( xModelProps->getPropertyValue( aTabIndexString ) >>= nIndex );
                _rpDescriptor->TabOrder = nIndex;
            }

            // ================================
            // special widget properties
            // --------------------------------
            // edits
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::Edit )
            {
                ::vcl::PDFWriter::EditWidget* pEditWidget = static_cast< ::vcl::PDFWriter::EditWidget* >( _rpDescriptor.get() );
                // ............................
                // multiline (already flagged in the TextStyle)
                pEditWidget->MultiLine = ( _rpDescriptor->TextStyle & TEXT_DRAW_MULTILINE ) != 0;
                // ............................
                // password input
                ::rtl::OUString sEchoCharPropName( RTL_CONSTASCII_USTRINGPARAM( "EchoChar" ) );
                if ( xPSI->hasPropertyByName( sEchoCharPropName ) )
                {
                    sal_Int16 nEchoChar = 0;
                    if ( ( xModelProps->getPropertyValue( sEchoCharPropName ) >>= nEchoChar ) && ( nEchoChar != 0 ) )
                        pEditWidget->Password = true;
                }
                // ............................
                // file select
                static const ::rtl::OUString FM_SUN_COMPONENT_FILECONTROL(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FileControl"));
                if ( xSI->supportsService( FM_SUN_COMPONENT_FILECONTROL ) )
                    pEditWidget->FileSelect = true;
                // ............................
                // maximum text length
                static const ::rtl::OUString FM_PROP_MAXTEXTLEN(RTL_CONSTASCII_USTRINGPARAM("MaxTextLen"));
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

            // --------------------------------
            // buttons
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::PushButton )
            {
                ::vcl::PDFWriter::PushButtonWidget* pButtonWidget = static_cast< ::vcl::PDFWriter::PushButtonWidget* >( _rpDescriptor.get() );
                FormButtonType eButtonType = FormButtonType_PUSH;
                OSL_VERIFY( xModelProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ) ) ) >>= eButtonType );
                static const ::rtl::OUString FM_PROP_TARGET_URL(RTL_CONSTASCII_USTRINGPARAM("TargetURL"));
                if ( eButtonType == FormButtonType_SUBMIT )
                {
                    // if a button is a submit button, then it uses the URL at it's parent form
                    Reference< XChild > xChild( xModelProps, UNO_QUERY );
                    Reference < XPropertySet > xParentProps;
                    if ( xChild.is() )
                        xParentProps = xParentProps.query( xChild->getParent() );
                    if ( xParentProps.is() )
                    {
                        Reference< XServiceInfo > xParentSI( xParentProps, UNO_QUERY );
                        if ( xParentSI.is() && xParentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.HTMLForm" ) ) ) )
                        {
                            OSL_VERIFY( xParentProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= pButtonWidget->URL );
                            pButtonWidget->Submit = true;
                            FormSubmitMethod eMethod = FormSubmitMethod_POST;
                            OSL_VERIFY( xParentProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SubmitMethod" ) ) ) >>= eMethod );
                            pButtonWidget->SubmitGet = (eMethod == FormSubmitMethod_GET);
                        }
                    }
                }
                else if ( eButtonType == FormButtonType_URL )
                {
                    ::rtl::OUString sURL;
                    OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_TARGET_URL ) >>= sURL );
                    const bool bDocumentLocalTarget = !sURL.isEmpty() && ( sURL.getStr()[0] == '#' );
                    if ( bDocumentLocalTarget )
                    {
                        // Register the destination for for future handling ...
                        pButtonWidget->Dest = i_pdfExportData.RegisterDest();

                        // and put it into the bookmarks, to ensure the future handling really happens
                        ::std::vector< ::vcl::PDFExtOutDevBookmarkEntry >& rBookmarks( i_pdfExportData.GetBookmarks() );
                        ::vcl::PDFExtOutDevBookmarkEntry aBookmark;
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
                // Is this desired? If no, we would have to reset _rpDescriptor to NULL here, in case eButtonType
                // != FormButtonType_SUBMIT && != FormButtonType_RESET

                // the PDF exporter defaults the text style, if 0. To prevent this, we have to transfer the UNO
                // defaults to the PDF widget
                if ( !pButtonWidget->TextStyle )
                    pButtonWidget->TextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER;
            }

            // --------------------------------
            // check boxes
            static const ::rtl::OUString FM_PROP_STATE(RTL_CONSTASCII_USTRINGPARAM("State"));
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::CheckBox )
            {
                ::vcl::PDFWriter::CheckBoxWidget* pCheckBoxWidget = static_cast< ::vcl::PDFWriter::CheckBoxWidget* >( _rpDescriptor.get() );
                sal_Int16 nState = 0;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState );
                pCheckBoxWidget->Checked = ( nState != 0 );
            }

            // --------------------------------
            // radio buttons
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::RadioButton )
            {
                ::vcl::PDFWriter::RadioButtonWidget* pRadioWidget = static_cast< ::vcl::PDFWriter::RadioButtonWidget* >( _rpDescriptor.get() );
                sal_Int16 nState = 0;
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_STATE ) >>= nState );
                pRadioWidget->Selected = ( nState != 0 );
                pRadioWidget->RadioGroup = determineRadioGroupId( xModelProps );
                try
                {
                    static const ::rtl::OUString FM_PROP_REFVALUE(RTL_CONSTASCII_USTRINGPARAM("RefValue"));
                    xModelProps->getPropertyValue( FM_PROP_REFVALUE ) >>= pRadioWidget->OnValue;
                }
                catch(...)
                {
                    pRadioWidget->OnValue = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "On" ) );
                }
            }

            // --------------------------------
            // list boxes
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::ListBox )
            {
                ::vcl::PDFWriter::ListBoxWidget* pListWidget = static_cast< ::vcl::PDFWriter::ListBoxWidget* >( _rpDescriptor.get() );
                // ............................
                // drop down
                static const ::rtl::OUString FM_PROP_DROPDOWN(RTL_CONSTASCII_USTRINGPARAM("Dropdown"));
                OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_DROPDOWN ) >>= pListWidget->DropDown );
                // ............................
                // multi selection
                OSL_VERIFY( xModelProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiSelection" ) ) ) >>= pListWidget->MultiSelect );
                // ............................
                // entries
                getStringItemVector( xModelProps, pListWidget->Entries );
                // since we explicitly list the entries in the order in which they appear, they should not be
                // resorted by the PDF viewer
                pListWidget->Sort = false;

                // get selected items
                Sequence< sal_Int16 > aSelectIndices;
                OSL_VERIFY( xModelProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectedItems" ) ) ) >>= aSelectIndices );
                if( aSelectIndices.getLength() > 0 )
                {
                    pListWidget->SelectedEntries.resize( 0 );
                    for( sal_Int32 i = 0; i < aSelectIndices.getLength(); i++ )
                    {
                        sal_Int16 nIndex = aSelectIndices.getConstArray()[i];
                        if( nIndex >= 0 && nIndex < (sal_Int16)pListWidget->Entries.size() )
                            pListWidget->SelectedEntries.push_back( nIndex );
                    }
                }
            }

            // --------------------------------
            // combo boxes
            if ( _rpDescriptor->getType() == ::vcl::PDFWriter::ComboBox )
            {
                ::vcl::PDFWriter::ComboBoxWidget* pComboWidget = static_cast< ::vcl::PDFWriter::ComboBoxWidget* >( _rpDescriptor.get() );
                // ............................
                // entries
                getStringItemVector( xModelProps, pComboWidget->Entries );
                // same reasoning as above
                pComboWidget->Sort = false;
            }

            // ================================
            // some post-processing
            // --------------------------------
            // text line ends
            // some controls may (always or dependent on other settings) return UNIX line ends
            _rpDescriptor->Text = convertLineEnd(_rpDescriptor->Text, LINEEND_CRLF);
        }
        catch( const Exception& )
        {
            OSL_FAIL( "describePDFControl: caught an exception!" );
        }
    }

//........................................................................
} // namespace toolkitform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
