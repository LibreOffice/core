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

#include <drawingml/textparagraph.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <svtools/unitconv.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <oox/mathml/importutils.hxx>
#include <oox/helper/propertyset.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <oox/token/properties.hxx>

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace oox::drawingml {

TextParagraph::TextParagraph()
{
}

TextParagraph::~TextParagraph()
{
}

TextCharacterProperties TextParagraph::getCharacterStyle (
    const TextCharacterProperties& rTextStyleProperties,
    const TextListStyle& rTextListStyle) const
{
    TextParagraphPropertiesPtr pTextParagraphStyle = getParagraphStyle(rTextListStyle);

    TextCharacterProperties aTextCharacterStyle;
    if (pTextParagraphStyle)
        aTextCharacterStyle.assignUsed(pTextParagraphStyle->getTextCharacterProperties());
    aTextCharacterStyle.assignUsed(rTextStyleProperties);
    aTextCharacterStyle.assignUsed(maProperties.getTextCharacterProperties());
    return aTextCharacterStyle;
}

TextParagraphPropertiesPtr TextParagraph::getParagraphStyle(
    const TextListStyle& rTextListStyle) const
{
    sal_Int16 nLevel = maProperties.getLevel();

    SAL_INFO("oox", "TextParagraph::getParagraphStyle - level " << nLevel);

    const TextParagraphPropertiesArray& rListStyle = rTextListStyle.getListStyle();
    if (nLevel >= static_cast< sal_Int16 >(rListStyle.size()))
        nLevel = 0;
    TextParagraphPropertiesPtr pTextParagraphStyle;
    if (!rListStyle.empty())
        pTextParagraphStyle = rListStyle[nLevel];

    return pTextParagraphStyle;
}

void TextParagraph::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > &xText,
        const Reference < XTextCursor > &xAt,
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStyle& rTextListStyle, bool bFirst, float nDefaultCharHeight) const
{
    try {
        sal_Int32 nParagraphSize = 0;
        TextCharacterProperties aTextCharacterStyle = getCharacterStyle(rTextStyleProperties, rTextListStyle);

        if( !bFirst )
        {
            xText->insertControlCharacter( xAt, ControlCharacter::APPEND_PARAGRAPH, false );
            xAt->gotoEnd( true );
        }

        sal_Int32 nCharHeight = 0;
        sal_Int32 nCharHeightFirst = 0;
        if ( maRuns.empty() )
        {
            PropertySet aPropSet( xAt );

            TextCharacterProperties aTextCharacterProps( aTextCharacterStyle );
            aTextCharacterProps.assignUsed( maEndProperties );
            if ( aTextCharacterProps.moHeight.has() )
                nCharHeight = nCharHeightFirst = aTextCharacterProps.moHeight.get();
            aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );
        }
        else
        {
            for( TextRunVector::const_iterator aIt = maRuns.begin(), aEnd = maRuns.end(); aIt != aEnd; ++aIt )
            {
                sal_Int32 nLen = (*aIt)->getText().getLength();
                // n#759180: Force use, maEndProperties for the last segment
                // This is currently applied to only empty runs
                if( !nLen && ( ( aIt + 1 ) == aEnd ) )
                    (*aIt)->getTextCharacterProperties().assignUsed( maEndProperties );
                sal_Int32 nCharHeightCurrent = (*aIt)->insertAt( rFilterBase, xText, xAt, aTextCharacterStyle, nDefaultCharHeight );
                if(aIt == maRuns.begin())
                    nCharHeightFirst = nCharHeightCurrent;
                nCharHeight = std::max< sal_Int32 >( nCharHeight, nCharHeightCurrent);
                nParagraphSize += nLen;
            }
        }
        xAt->gotoEnd( true );

        PropertyMap aioBulletList;
        Reference< XPropertySet > xProps( xAt, UNO_QUERY);

        TextParagraphPropertiesPtr pTextParagraphStyle = getParagraphStyle(rTextListStyle);
        if ( pTextParagraphStyle )
        {
            TextParagraphProperties aParaProp;
            aParaProp.apply( *pTextParagraphStyle );
            aParaProp.apply( maProperties );

            // bullets have same color as following texts by default
            if( !aioBulletList.hasProperty( PROP_BulletColor ) && !maRuns.empty()
                && (*maRuns.begin())->getTextCharacterProperties().maFillProperties.moFillType.has() )
                aioBulletList.setProperty( PROP_BulletColor, (*maRuns.begin())->getTextCharacterProperties().maFillProperties.getBestSolidColor().getColor( rFilterBase.getGraphicHelper() ));
            if( !aioBulletList.hasProperty( PROP_BulletColor ) && aTextCharacterStyle.maFillProperties.moFillType.has() )
                aioBulletList.setProperty( PROP_BulletColor, aTextCharacterStyle.maFillProperties.getBestSolidColor().getColor( rFilterBase.getGraphicHelper() ));
            if( !aioBulletList.hasProperty( PROP_GraphicSize ) && !maRuns.empty()
                && aParaProp.getBulletList().maGraphic.hasValue())
            {
                tools::Long nFirstCharHeightMm = TransformMetric(nCharHeightFirst > 0 ? nCharHeightFirst : 1200, FieldUnit::POINT, FieldUnit::MM);
                float fBulletSizeRel = 1.f;
                double fBulletAspectRatio = 1.0;

                if( aParaProp.getBulletList().mnSize.hasValue() )
                    fBulletSizeRel = aParaProp.getBulletList().mnSize.get<sal_Int16>() / 100.f;

                if( aParaProp.getBulletList().mnAspectRatio.hasValue() )
                    fBulletAspectRatio = aParaProp.getBulletList().mnAspectRatio.get<double>();

                css::awt::Size aBulletSize;
                if( fBulletAspectRatio != 1.0 )
                {
                    aBulletSize.Height = std::lround(fBulletSizeRel * nFirstCharHeightMm * OOX_BULLET_LIST_SCALE_FACTOR);
                    aBulletSize.Width = aBulletSize.Height * fBulletAspectRatio;
                }
                else
                    aBulletSize.Width = aBulletSize.Height = std::lround(fBulletSizeRel * nFirstCharHeightMm * OOX_BULLET_LIST_SCALE_FACTOR);

                aioBulletList.setProperty( PROP_GraphicSize, aBulletSize);
            }

            float fCharacterSize = nCharHeight > 0 ? GetFontHeight ( nCharHeight ) : pTextParagraphStyle->getCharHeightPoints( 12 );
            aParaProp.pushToPropSet( &rFilterBase, xProps, aioBulletList, &pTextParagraphStyle->getBulletList(), true, fCharacterSize, true );
        }

        // empty paragraphs do not have bullets in ppt
        if ( !nParagraphSize )
        {
            xProps->setPropertyValue( "NumberingLevel", Any( static_cast< sal_Int16 >( -1 ) ) );
        }

// FIXME this is causing a lot of disruption (ie does not work). I wonder what to do -- Hub
//          Reference< XTextRange > xEnd( xAt, UNO_QUERY );
//      Reference< XPropertySet > xProps2( xEnd, UNO_QUERY );
//          mpEndProperties->pushToPropSet( xProps2 );
    }
    catch( Exception & )
    {
        SAL_INFO("oox", "exception in TextParagraph::insertAt");
    }
}

formulaimport::XmlStreamBuilder & TextParagraph::GetMathXml()
{
    if (!m_pMathXml)
    {
        m_pMathXml.reset(new formulaimport::XmlStreamBuilder);
    }
    return *m_pMathXml;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
