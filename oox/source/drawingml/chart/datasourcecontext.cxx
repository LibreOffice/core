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

#include "drawingml/chart/datasourcecontext.hxx"

#include "oox/drawingml/chart/datasourcemodel.hxx"

#include <comphelper/processfactory.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <svl/zforlist.hxx>
#include <osl/diagnose.h>

namespace oox {
namespace drawingml {
namespace chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

using namespace ::com::sun::star;

DoubleSequenceContext::DoubleSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel ) :
    DataSequenceContextBase( rParent, rModel ),
    mnPtIndex( -1 ),
    mpNumberFormatter( NULL )
{
}

DoubleSequenceContext::~DoubleSequenceContext()
{
    if( mpNumberFormatter != NULL )
    {
        delete mpNumberFormatter;
    }
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
                    return 0;
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
    }
    return 0;
}

void DoubleSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C_TOKEN( formatCode ):
            mrModel.maFormatCode = rChars;
        break;
        case C_TOKEN( v ):
            if( mnPtIndex >= 0 )
            {
                /* Import categories as String even though it could
                 * be values.
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
                            short nType;
                            pNumFrmt->PutEntry( aFormatCode, nCheckPos, nType, nKey );
                            bNoKey = (nCheckPos != 0);
                        }
                        if( bNoKey )
                        {
                            mrModel.maData[ mnPtIndex ] <<= rChars;
                        }
                        else
                        {
                            double fValue = rChars.toDouble();
                            ::Color* pColor = NULL;
                            OUString aFormattedValue;
                            pNumFrmt->GetOutputString( fValue, nKey, aFormattedValue, &pColor );
                            mrModel.maData[ mnPtIndex ] <<= aFormattedValue;
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
    if( mpNumberFormatter == NULL )
    {
        uno::Reference<uno::XComponentContext> rContext =
                                this->getFilter().getComponentContext();
        mpNumberFormatter =
                new SvNumberFormatter(rContext, LANGUAGE_DONTKNOW);
    }
    return mpNumberFormatter;
}


StringSequenceContext::StringSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel )
    : DataSequenceContextBase( rParent, rModel )
    , mnPtIndex(-1)
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
            switch( nElement )
            {
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger( XML_val, -1 );
                    return 0;
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
    }
    return 0;
}

void StringSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C_TOKEN( v ):
            if( mnPtIndex >= 0 )
                mrModel.maData[ mnPtIndex ] <<= rChars;
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

ContextHandlerRef DataSourceContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( cat ):
        case C_TOKEN( xVal ):
            switch( nElement )
            {
                case C_TOKEN( multiLvlStrRef ):
                case C_TOKEN( strLit ):
                case C_TOKEN( strRef ):
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
    return 0;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
