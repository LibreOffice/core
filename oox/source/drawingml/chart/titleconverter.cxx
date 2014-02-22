/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/drawingml/chart/titleconverter.hxx"

#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"
#include "oox/helper/containerhelper.hxx"
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/drawing/Alignment.hpp>

#include "oox/drawingml/chart/modelbase.hxx"
namespace oox {
namespace drawingml {
namespace chart {



using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;



TextConverter::TextConverter( const ConverterRoot& rParent, TextModel& rModel ) :
    ConverterBase< TextModel >( rParent, rModel )
{
}

TextConverter::~TextConverter()
{
}

Reference< XDataSequence > TextConverter::createDataSequence( const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( mrModel.mxDataSeq.is() )
    {
        DataSequenceConverter aDataSeqConv( *this, *mrModel.mxDataSeq );
        xDataSeq = aDataSeqConv.createDataSequence( rRole );
    }
    return xDataSeq;
}

Sequence< Reference< XFormattedString > > TextConverter::createStringSequence(
        const OUString& rDefaultText, const ModelRef< TextBody >& rxTextProp, ObjectType eObjType )
{
    OSL_ENSURE( !mrModel.mxDataSeq || !mrModel.mxTextBody, "TextConverter::createStringSequence - linked string and rich text found" );
    ::std::vector< Reference< XFormattedString > > aStringVec;
    if( mrModel.mxTextBody.is() )
    {
        
        const TextParagraphVector& rTextParas = mrModel.mxTextBody->getParagraphs();
        for( TextParagraphVector::const_iterator aPIt = rTextParas.begin(), aPEnd = rTextParas.end(); aPIt != aPEnd; ++aPIt )
        {
            const TextParagraph& rTextPara = **aPIt;
            const TextCharacterProperties& rParaProps = rTextPara.getProperties().getTextCharacterProperties();
            for( TextRunVector::const_iterator aRIt = rTextPara.getRuns().begin(), aREnd = rTextPara.getRuns().end(); aRIt != aREnd; ++aRIt )
            {
                const TextRun& rTextRun = **aRIt;
                bool bAddNewLine = (aRIt + 1 == aREnd) && (aPIt + 1 != aPEnd);
                Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, rTextRun.getText(), bAddNewLine );
                PropertySet aPropSet( xFmtStr );
                TextCharacterProperties aRunProps( rParaProps );
                aRunProps.assignUsed( rTextRun.getTextCharacterProperties() );
                getFormatter().convertTextFormatting( aPropSet, aRunProps, eObjType );
            }
        }
    }
    else
    {
        OUString aString;
        
        if( mrModel.mxDataSeq.is() && !mrModel.mxDataSeq->maData.empty() )
            mrModel.mxDataSeq->maData.begin()->second >>= aString;
        
        if( aString.isEmpty() )
            aString = rDefaultText;

        
        if( !aString.isEmpty() )
        {
            Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, aString, false );
            PropertySet aPropSet( xFmtStr );
            getFormatter().convertTextFormatting( aPropSet, rxTextProp, eObjType );
        }
    }

    return ContainerHelper::vectorToSequence( aStringVec );
}

Reference< XFormattedString > TextConverter::appendFormattedString(
        ::std::vector< Reference< XFormattedString > >& orStringVec, const OUString& rString, bool bAddNewLine ) const
{
    Reference< XFormattedString2 > xFmtStr;
    try
    {
        xFmtStr = FormattedString::create( ConverterRoot::getComponentContext() );
        xFmtStr->setString( bAddNewLine ? (rString + OUString( '\n' )) : rString );
        orStringVec.push_back( xFmtStr );
    }
    catch( Exception& )
    {
    }
    return xFmtStr;
}



TitleConverter::TitleConverter( const ConverterRoot& rParent, TitleModel& rModel ) :
    ConverterBase< TitleModel >( rParent, rModel )
{
}

TitleConverter::~TitleConverter()
{
}

void TitleConverter::convertFromModel( const Reference< XTitled >& rxTitled, const OUString& rAutoTitle, ObjectType eObjType, sal_Int32 nMainIdx, sal_Int32 nSubIdx )
{
    if( rxTitled.is() )
    {
        
        TextModel& rText = mrModel.mxText.getOrCreate();
        TextConverter aTextConv( *this, rText );
        Sequence< Reference< XFormattedString > > aStringSeq = aTextConv.createStringSequence( rAutoTitle, mrModel.mxTextProp, eObjType );
        if( aStringSeq.hasElements() ) try
        {
            
            Reference< XTitle > xTitle( createInstance( "com.sun.star.chart2.Title" ), UNO_QUERY_THROW );
            xTitle->setText( aStringSeq );
            rxTitled->setTitleObject( xTitle );

            
            PropertySet aPropSet( xTitle );
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, eObjType );

            
            OSL_ENSURE( !mrModel.mxTextProp || !rText.mxTextBody, "TitleConverter::convertFromModel - multiple text properties" );
            ModelRef< TextBody > xTextProp = mrModel.mxTextProp.is() ? mrModel.mxTextProp : rText.mxTextBody;
            getFormatter().convertTextRotation( aPropSet, xTextProp, true );

            
            registerTitleLayout( xTitle, mrModel.mxLayout, eObjType, nMainIdx, nSubIdx );
        }
        catch( Exception& )
        {
        }
    }
}



LegendConverter::LegendConverter( const ConverterRoot& rParent, LegendModel& rModel ) :
    ConverterBase< LegendModel >( rParent, rModel )
{
}

LegendConverter::~LegendConverter()
{
}

void LegendConverter::convertFromModel( const Reference< XDiagram >& rxDiagram )
{
    if( rxDiagram.is() ) try
    {
        namespace cssc = ::com::sun::star::chart;
        namespace cssc2 = ::com::sun::star::chart2;

        
        Reference< XLegend > xLegend( createInstance( "com.sun.star.chart2.Legend" ), UNO_QUERY_THROW );
        rxDiagram->setLegend( xLegend );
        PropertySet aPropSet( xLegend );
        aPropSet.setProperty( PROP_Show, true );

        
        getFormatter().convertFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_LEGEND );

        
        cssc2::LegendPosition eLegendPos = cssc2::LegendPosition_CUSTOM;
        cssc::ChartLegendExpansion eLegendExpand = cssc::ChartLegendExpansion_CUSTOM;
        RelativePosition eRelPos;
        bool bTopRight=false;
        switch( mrModel.mnPosition )
        {
            case XML_l:
                eLegendPos = cssc2::LegendPosition_LINE_START;
                eLegendExpand = cssc::ChartLegendExpansion_HIGH;
            break;
            case XML_r:
                eLegendPos = cssc2::LegendPosition_LINE_END;
                eLegendExpand = cssc::ChartLegendExpansion_HIGH;
                break;
            case XML_tr:    
                eLegendPos = LegendPosition_CUSTOM;
                eRelPos.Primary = 1;
                eRelPos.Secondary =0;
                eRelPos.Anchor = Alignment_TOP_RIGHT;
                bTopRight=true;

            break;
            case XML_t:
                eLegendPos = cssc2::LegendPosition_PAGE_START;
                eLegendExpand = cssc::ChartLegendExpansion_WIDE;
            break;
            case XML_b:
                eLegendPos = cssc2::LegendPosition_PAGE_END;
                eLegendExpand = cssc::ChartLegendExpansion_WIDE;
            break;
        }
        bool bManualLayout=false;
        
        if( mrModel.mxLayout.get() )
        {
            LayoutConverter aLayoutConv( *this, *mrModel.mxLayout );
            
            if( aLayoutConv.convertFromModel( aPropSet ) )
                eLegendExpand = cssc::ChartLegendExpansion_CUSTOM;
            bManualLayout = !aLayoutConv.getAutoLayout();
        }

        
        aPropSet.setProperty( PROP_AnchorPosition, eLegendPos );
        aPropSet.setProperty( PROP_Expansion, eLegendExpand );

        if(eLegendPos == LegendPosition_CUSTOM && bTopRight && bManualLayout==false)
            aPropSet.setProperty( PROP_RelativePosition , makeAny(eRelPos));
    }
    catch( Exception& )
    {
    }
}



} 
} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
