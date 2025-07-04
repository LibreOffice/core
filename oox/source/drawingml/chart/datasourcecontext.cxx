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

#include <oox/drawingml/chart/datasourcemodel.hxx>

#include <drawingml/chart/seriesmodel.hxx>
#include <drawingml/chart/datasourcecontext.hxx>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <osl/diagnose.h>

namespace oox::drawingml::chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

using namespace ::com::sun::star;

DoubleSequenceContext::DoubleSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel ) :
    DataSequenceContextBase( rParent, rModel ),
    mnPtIndex( -1 )
{
}

DoubleSequenceContext::~DoubleSequenceContext()
{
}

ContextHandlerRef DoubleSequenceContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( numRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                case C_TOKEN( numCache ):
                    return this;
            }
        break;

        case C_TOKEN( numCache ):
        case C_TOKEN( numLit ):
            switch( nElement )
            {
                case C_TOKEN( formatCode ):
                    return this;
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger( XML_val, -1 );
                    return nullptr;
                case C_TOKEN( pt ):
                    mnPtIndex = rAttribs.getInteger( XML_idx, -1 );
                    return this;
            }
        break;

        case C_TOKEN( pt ):
            switch( nElement )
            {
                case C_TOKEN( v ):
                    return this;
            }
        break;
        case CX_TOKEN(numDim) :
            switch (nElement) {
                case CX_TOKEN(f):
                    return this;
                case CX_TOKEN(lvl):
                    mrModel.mnPointCount = rAttribs.getInteger(XML_ptCount, -1);
                    mrModel.maFormatCode = rAttribs.getString(XML_formatCode, "");
                    return this;
            }
            break;
        case CX_TOKEN(lvl) :
            switch(nElement) {
                case CX_TOKEN(pt):
                    mnPtIndex = rAttribs.getInteger(XML_idx, -1);
                    return this;
            }
            break;
    }
    return nullptr;
}

void DoubleSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
        case CX_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C_TOKEN( formatCode ):
            mrModel.maFormatCode = rChars;
        break;
        case C_TOKEN( v ):
        case CX_TOKEN(pt):
            if( mnPtIndex >= 0 )
            {
                /* Import categories as String even though it could
                 * be values except when the format code indicates that they are dates.
                 * n#810508: xVal needs to be imported as double
                 * TODO: NumberFormat conversion, remove the check then.
                 */
                if( isParentElement( C_TOKEN( cat ), 4 ) )
                {
                    // workaround for bug n#889755
                    SvNumberFormatter* pNumFrmt = getNumberFormatter();
                    if( pNumFrmt )
                    {
                        sal_uInt32 nKey = pNumFrmt->GetEntryKey( mrModel.maFormatCode );
                        bool bNoKey = ( nKey == NUMBERFORMAT_ENTRY_NOT_FOUND );
                        if( bNoKey )
                        {
                            OUString aFormatCode = mrModel.maFormatCode;
                            sal_Int32 nCheckPos = 0;
                            SvNumFormatType nType;
                            pNumFrmt->PutEntry( aFormatCode, nCheckPos, nType, nKey );
                            bNoKey = (nCheckPos != 0);
                            if (!bNoKey)
                                mrModel.meFormatType = nType;
                        }
                        if( bNoKey )
                        {
                            mrModel.maData[ mnPtIndex ] <<= rChars;
                        }
                        else
                        {
                            double fValue = rChars.toDouble();
                            if (mrModel.meFormatType == SvNumFormatType::DATE)
                                mrModel.maData[ mnPtIndex ] <<= fValue;
                            else
                            {
                                const ::Color* pColor = nullptr;
                                OUString aFormattedValue;
                                // tdf#91250: use UNLIMITED_PRECISION in case of GENERAL Number Format of category axis labels
                                if( pNumFrmt->GetStandardPrec() != SvNumberFormatter::UNLIMITED_PRECISION )
                                    pNumFrmt->ChangeStandardPrec(SvNumberFormatter::UNLIMITED_PRECISION);
                                pNumFrmt->GetOutputString( fValue, nKey, aFormattedValue, &pColor );
                                mrModel.maData[ mnPtIndex ] <<= aFormattedValue;
                            }
                        }
                    }
                    else
                    {
                        mrModel.maData[ mnPtIndex ] <<= rChars;
                    }
                }
                else
                {
                    mrModel.maData[ mnPtIndex ] <<= rChars.toDouble();
                }
            }
        break;
    }
}


SvNumberFormatter* DoubleSequenceContext::getNumberFormatter()
{
    if( mpNumberFormatter == nullptr )
    {
        uno::Reference<uno::XComponentContext> rContext =
                                getFilter().getComponentContext();
        mpNumberFormatter.reset(
                new SvNumberFormatter(rContext, LANGUAGE_SYSTEM) );
    }
    return mpNumberFormatter.get();
}


StringSequenceContext::StringSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel )
    : DataSequenceContextBase( rParent, rModel )
    , mnPtIndex(-1)
    , mbReadC15(false)
{
}

StringSequenceContext::~StringSequenceContext()
{
}

ContextHandlerRef StringSequenceContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( multiLvlStrRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                case C_TOKEN( multiLvlStrCache ):
                    return this;
            }
        break;

        case C15_TOKEN( datalabelsRange ):
            mbReadC15 = true;
            switch( nElement )
            {
                case C15_TOKEN( f ):
                case C15_TOKEN( dlblRangeCache ):
                    return this;
            }
        break;

        case C_TOKEN( strRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                case C_TOKEN( strCache ):
                    return this;
            }
        break;

        case C_TOKEN( strCache ):
        case C_TOKEN( strLit ):
        case C15_TOKEN( dlblRangeCache ):
            if (nElement == C15_TOKEN( dlblRangeCache ) && !mbReadC15)
                break;

            switch( nElement )
            {
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger( XML_val, -1 );
                    return nullptr;
                case C_TOKEN( pt ):
                    mnPtIndex = rAttribs.getInteger( XML_idx, -1 );
                    return this;
            }
        break;

        case C_TOKEN( multiLvlStrCache ):
            switch (nElement)
            {
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger(XML_val, -1);
                    mrModel.mnLevelCount--; // normalize level count
                    return nullptr;
                case C_TOKEN( lvl ):
                    mrModel.mnLevelCount++;
                    return this;
            }
            break;

        case C_TOKEN( lvl ):
        case CX_TOKEN( lvl ):
            switch (nElement)
            {
                case C_TOKEN(pt):
                case CX_TOKEN(pt):
                    mnPtIndex = rAttribs.getInteger(XML_idx, -1);
                    return this;
            }
            break;

        case C_TOKEN( pt ):
            switch( nElement )
            {
                case C_TOKEN( v ):
                    return this;
            }
        break;
        case CX_TOKEN(strDim) :
            switch (nElement) {
                case CX_TOKEN(f):
                    return this;
                case CX_TOKEN(lvl):
                    mrModel.mnPointCount = rAttribs.getInteger(XML_ptCount, -1);
                    return this;
            }
    }
    return nullptr;
}

void StringSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C15_TOKEN( f ):
            if (mbReadC15)
                mrModel.maFormula = rChars;
        break;
        case C_TOKEN( v ):
        case CX_TOKEN(pt):
            if( mnPtIndex >= 0 ) {
                assert(mrModel.mnPointCount > 0);
                mrModel.maData[ (mrModel.mnLevelCount-1) * mrModel.mnPointCount + mnPtIndex ] <<= rChars;
            }
        break;
    }
}

DataSourceContext::DataSourceContext( ContextHandler2Helper& rParent, DataSourceModel& rModel ) :
    ContextBase< DataSourceModel >( rParent, rModel )
{
}

DataSourceContext::~DataSourceContext()
{
}

ContextHandlerRef DataSourceContext::onCreateContext( sal_Int32 nElement, const
        AttributeList&)
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( cat ):
        case C_TOKEN( xVal ):
        case C_TOKEN( ext ):
            switch( nElement )
            {
                case C_TOKEN( multiLvlStrRef ):
                case C_TOKEN( strLit ):
                case C_TOKEN( strRef ):
                case C15_TOKEN( datalabelsRange ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new StringSequenceContext( *this, mrModel.mxDataSeq.create() );

                case C_TOKEN( numLit ):
                case C_TOKEN( numRef ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new DoubleSequenceContext( *this, mrModel.mxDataSeq.create() );
            }
        break;

        case C_TOKEN( plus ):
        case C_TOKEN( minus ):
        case C_TOKEN( val ):
        case C_TOKEN( yVal ):
        case C_TOKEN( bubbleSize ):
            switch( nElement )
            {
                case C_TOKEN( numLit ):
                case C_TOKEN( numRef ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new DoubleSequenceContext( *this, mrModel.mxDataSeq.create() );
            }
        break;
    }
    return nullptr;
}

// =====
// DataSourceCxContext: handler for chartex data sources
// =====
DataSourceCxContext::DataSourceCxContext( ContextHandler2Helper& rParent,
        DataSourceCxModel& rModel ) :
    ContextBase< DataSourceCxModel >( rParent, rModel ),
    paCurSource(nullptr)
{
}

DataSourceCxContext::~DataSourceCxContext()
{
}

ContextHandlerRef DataSourceCxContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    switch( getCurrentElement() )
    {
        case CX_TOKEN(chartData) :
            switch (nElement) {
                case CX_TOKEN(externalData) :
                    return nullptr; // TODO
                case CX_TOKEN(data) :
                    paCurSource = &mrModel.maSourceMap.create(rAttribs.getInteger(XML_id, -1));
                    return this;
            }
            break;
        case CX_TOKEN(data) :
            switch (nElement) {
                case CX_TOKEN(numDim) :
                {
                    assert(paCurSource);
                    OUString sType = rAttribs.getString(XML_type, "val");
                    if (sType == "cat") {
                        DataSourceModel& rDataModel = paCurSource->create(DataSourceType::CATEGORIES);
                        return new DoubleSequenceContext(*this,
                                rDataModel.mxDataSeq.create());
                    } else {
                        // default is VALUES
                        DataSourceModel& rDataModel = paCurSource->create(DataSourceType::VALUES);
                        return new DoubleSequenceContext(*this,
                                rDataModel.mxDataSeq.create());
                    }
                    break;
                }

                case CX_TOKEN(strDim) :
                {
                    assert(paCurSource);
                    OUString sType = rAttribs.getString(XML_type, "cat");
                    if (sType == "val") {
                        DataSourceModel& rDataModel = paCurSource->create(DataSourceType::VALUES);
                        return new StringSequenceContext(*this,
                                rDataModel.mxDataSeq.create());
                    } else {
                        // default is CATEGORIES
                        DataSourceModel& rDataModel = paCurSource->create(DataSourceType::CATEGORIES);
                        return new StringSequenceContext(*this,
                                rDataModel.mxDataSeq.create());
                    }
                }
            }
            break;

    }
    return nullptr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
