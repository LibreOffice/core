/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datasourcecontext.cxx,v $
 *
 * $Revision: 1.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/chart/datasourcecontext.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

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

void DoubleSequenceContext::onEndElement( const OUString& rChars )
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
                mrModel.maData[ mnPtIndex ] <<= rChars.toDouble();
        break;
    }
}

// ============================================================================

StringSequenceContext::StringSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel ) :
    DataSequenceContextBase( rParent, rModel )
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

void StringSequenceContext::onEndElement( const OUString& rChars )
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

// ============================================================================

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

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

