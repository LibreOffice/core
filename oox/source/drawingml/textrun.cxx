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

#include <drawingml/textrun.hxx>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <sal/log.hxx>

#include <oox/helper/helper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace oox::drawingml {

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
        Reference< XTextRange > xStart = xAt;
        PropertySet aPropSet( xStart );

        Reference<XPropertyState> xState(xStart, UNO_QUERY);
        Any aOldFontName = xState->getPropertyDefault(u"CharFontName"_ustr);
        Any aOldFontPitch = xState->getPropertyDefault(u"CharFontPitch"_ustr);
        Any aOldFontFamily = xState->getPropertyDefault(u"CharFontFamily"_ustr);
        Any aOldFontCharSet = xState->getPropertyDefault(u"CharFontCharSet"_ustr);

        TextCharacterProperties aTextCharacterProps( rTextCharacterStyle );

        // If no text color specified lets anyway initialize it as default:
        // this will help to recover after hyperlink
        if (!aTextCharacterProps.maFillProperties.maFillColor.isUsed())
            aTextCharacterProps.maFillProperties.moFillType = XML_solidFill;

        aTextCharacterProps.assignUsed(maTextCharacterProperties);
        if ( aTextCharacterProps.moHeight.has_value() )
            nCharHeight = aTextCharacterProps.moHeight.value();
        else
            // UNO API has the character height as float, DML has it as int, but in hundreds.
            aTextCharacterProps.moHeight = static_cast<sal_Int32>(nDefaultCharHeight * 100);
        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );

        if( maTextCharacterProperties.maHyperlinkPropertyMap.empty() )
        {
            if( mbIsLineBreak )
            {
                SAL_WARN("oox",  "OOX: TextRun::insertAt() insert line break" );
                xText->insertControlCharacter( xStart, ControlCharacter::LINE_BREAK, false );
            }
            else if (!getText().isEmpty())
            {
                sal_Int32 nIndex = 0;
                sal_Int32 nMax = getText().getLength();
                while(true)
                {
                    bool bSymbol = (getText()[nIndex] & 0xff00) == 0xf000;
                    sal_Int32 nCount = 1;
                    while(nIndex + nCount < nMax
                            && ((getText()[nIndex + nCount] & 0xff00) == 0xf000) == bSymbol)
                        ++nCount;

                    OUString aFontName;
                    sal_Int16 nFontFamily = 0, nFontPitch = 0;
                    bool bSymbolEnc(false);
                    bool bReset = false;

                    // Direct formatting for symbols.
                    if (bSymbol && aTextCharacterProps.maSymbolFont.getFontData(aFontName, nFontPitch, nFontFamily, &bSymbolEnc, rFilterBase))

                    {
                        aPropSet.setAnyProperty(PROP_CharFontName, Any(aFontName));
                        aPropSet.setAnyProperty(PROP_CharFontPitch, Any(nFontPitch));
                        aPropSet.setAnyProperty(PROP_CharFontFamily, Any(nFontFamily));
                        aPropSet.setAnyProperty(PROP_CharFontCharSet, Any(bSymbolEnc ? css::awt::CharSet::SYMBOL : css::awt::CharSet::DONTKNOW));
                        bReset = true;
                    }

                    OUString aSubString(getText().copy(nIndex, nCount));
                    xText->insertString(xStart, aSubString, false);

                    aPropSet = PropertySet(xStart);
                    // Reset to whatever it was.
                    if (bReset)
                    {
                        aPropSet.setAnyProperty(PROP_CharFontName, aOldFontName);
                        aPropSet.setAnyProperty(PROP_CharFontPitch, aOldFontPitch);
                        aPropSet.setAnyProperty(PROP_CharFontFamily, aOldFontFamily);
                        aPropSet.setAnyProperty(PROP_CharFontCharSet, aOldFontCharSet);
                    }

                    nIndex += nCount;

                    if (nIndex >= nMax)
                        break;

                    aTextCharacterProps.pushToPropSet(aPropSet, rFilterBase);
                }
            }
        }
        else
        {
            SAL_WARN("oox",  "OOX: URL field" );
            Reference< XMultiServiceFactory > xFactory( rFilterBase.getModel(), UNO_QUERY );
            Reference< XTextField > xField( xFactory->createInstance( u"com.sun.star.text.TextField.URL"_ustr ), UNO_QUERY );
            if( xField.is() )
            {
                Reference< XTextCursor > xTextFieldCursor = xText->createTextCursor();
                xTextFieldCursor->gotoEnd( false );

                PropertySet aFieldProps( xField );
                aFieldProps.setProperties( maTextCharacterProperties.maHyperlinkPropertyMap );
                aFieldProps.setProperty( PROP_Representation, getText() );
                xText->insertTextContent( xStart, xField, false );

                xTextFieldCursor->gotoEnd( true );

                if (!maTextCharacterProperties.maHyperlinkPropertyMap.hasProperty(PROP_CharColor))
                    aTextCharacterProps.maFillProperties.maFillColor.setSchemeClr(XML_hlink);

                aTextCharacterProps.maFillProperties.moFillType = XML_solidFill;
                if ( !maTextCharacterProperties.moUnderline.has_value() )
                    aTextCharacterProps.moUnderline = XML_sng;

                PropertySet aFieldTextPropSet( xTextFieldCursor );
                aTextCharacterProps.pushToPropSet( aFieldTextPropSet, rFilterBase );
            }
            else
            {
                SAL_WARN("oox",  "OOX: URL field couldn't be created" );
                xText->insertString( xStart, getText(), false );
            }
        }
    }
    catch( const Exception&  )
    {
        TOOLS_WARN_EXCEPTION("oox", "OOX: TextRun::insertAt()");
    }

    return nCharHeight;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
