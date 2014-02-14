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

#include "oox/drawingml/textrun.hxx"

#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/token/tokens.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox { namespace drawingml {

TextRun::TextRun() :
    mbIsLineBreak( false )
{
}

TextRun::~TextRun()
{
}

sal_Int32 TextRun::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > & xText,
        const Reference < XTextCursor > &xAt,
        const TextCharacterProperties& rTextCharacterStyle,
        float nDefaultCharHeight) const
{
    sal_Int32 nCharHeight = 0;
    try {
        Reference< XTextRange > xStart( xAt, UNO_QUERY );
        PropertySet aPropSet( xStart );

        TextCharacterProperties aTextCharacterProps( rTextCharacterStyle );
        aTextCharacterProps.assignUsed( maTextCharacterProperties );
        if ( aTextCharacterProps.moHeight.has() )
            nCharHeight = aTextCharacterProps.moHeight.get();
        else
            // UNO API has the character height as float, DML has it as int, but in hundreds.
            aTextCharacterProps.moHeight = static_cast<sal_Int32>(nDefaultCharHeight * 100);
        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );

        if( maTextCharacterProperties.maHyperlinkPropertyMap.empty() )
        {
            if( mbIsLineBreak )
            {
                OSL_TRACE( "OOX: TextRun::insertAt() insert line break" );
                xText->insertControlCharacter( xStart, ControlCharacter::LINE_BREAK, sal_False );
            }
            else
            {
                OUString aLatinFontName, aSymbolFontName;
                sal_Int16 nSymbolFontFamily = 0, nSymbolFontPitch = 0;

                if ( !aTextCharacterProps.maSymbolFont.getFontData( aSymbolFontName, nSymbolFontPitch, nSymbolFontFamily, rFilterBase ) )
                    xText->insertString( xStart, getText(), sal_False );
                else if ( !getText().isEmpty() )
                {   // !!#i113673<<<
                    sal_Int16 nLatinFontPitch = 0, nLatinFontFamily = 0;
                    aTextCharacterProps.maLatinFont.getFontData( aLatinFontName, nLatinFontPitch, nLatinFontFamily, rFilterBase );

                    sal_Int32 nIndex = 0;
                    while ( true )
                    {
                        sal_Int32 nCount = 0;
                        sal_Bool bSymbol = ( getText()[ nIndex ] & 0xff00 ) == 0xf000;
                        if ( bSymbol )
                        {
                            do
                            {
                                nCount++;
                            }
                            while( ( ( nCount + nIndex ) < getText().getLength() ) && ( ( getText()[ nCount + nIndex ] & 0xff00 ) == 0xf000 ) );
                            aPropSet.setAnyProperty( PROP_CharFontName, Any( aSymbolFontName ) );
                            aPropSet.setAnyProperty( PROP_CharFontPitch, Any( nSymbolFontPitch ) );
                            aPropSet.setAnyProperty( PROP_CharFontFamily, Any( nSymbolFontFamily ) );
                        }
                        else
                        {
                            do
                            {
                                nCount++;
                            }
                            while( ( ( nCount + nIndex ) < getText().getLength() ) && ( ( getText()[ nCount + nIndex ] & 0xff00 ) != 0xf000 ) );
                            aPropSet.setAnyProperty( PROP_CharFontName, Any( aLatinFontName ) );
                            aPropSet.setAnyProperty( PROP_CharFontPitch, Any( nLatinFontPitch ) );
                            aPropSet.setAnyProperty( PROP_CharFontFamily, Any( nLatinFontFamily ) );
                        }
                        OUString aSubString( getText().copy( nIndex, nCount ) );
                        xText->insertString( xStart, aSubString, sal_False );
                        nIndex += nCount;

                        if ( nIndex >= getText().getLength() )
                            break;

                        xStart = xAt;
                        aPropSet = PropertySet( xStart );
                        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );
                    }
                }
            }
        }
        else
        {
            OSL_TRACE( "OOX: URL field" );
            Reference< XMultiServiceFactory > xFactory( rFilterBase.getModel(), UNO_QUERY );
            Reference< XTextField > xField( xFactory->createInstance( "com.sun.star.text.TextField.URL" ), UNO_QUERY );
            if( xField.is() )
            {
                Reference< XTextCursor > xTextFieldCursor = xText->createTextCursor();
                xTextFieldCursor->gotoEnd( sal_False );

                PropertySet aFieldProps( xField );
                aFieldProps.setProperties( maTextCharacterProperties.maHyperlinkPropertyMap );
                aFieldProps.setProperty( PROP_Representation, getText() );
                xText->insertTextContent( xStart, xField, sal_False );

                xTextFieldCursor->gotoEnd( sal_True );

                if ( !maTextCharacterProperties.maCharColor.isUsed() )
                    aTextCharacterProps.maCharColor.setSchemeClr( XML_hlink );
                if ( !maTextCharacterProperties.moUnderline.has() )
                    aTextCharacterProps.moUnderline.set( XML_sng );

                PropertySet aFieldTextPropSet( xTextFieldCursor );
                aTextCharacterProps.pushToPropSet( aFieldTextPropSet, rFilterBase );

                oox::core::TextField aTextField;
                aTextField.xText = xText;
                aTextField.xTextCursor = xTextFieldCursor;
                aTextField.xTextField = xField;
                rFilterBase.getTextFieldStack().push_back( aTextField );
            }
            else
            {
                OSL_TRACE( "OOX: URL field couldn't be created" );
                xText->insertString( xStart, getText(), sal_False );
            }
        }
    }
    catch( const Exception&  )
    {
        OSL_TRACE("OOX:  TextRun::insertAt() exception");
    }

    return nCharHeight;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
