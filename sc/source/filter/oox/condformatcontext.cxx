/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "condformatcontext.hxx"
#include "extlstcontext.hxx"

namespace oox {
namespace xls {

// ============================================================================

using ::oox::core::ContextHandlerRef;
using ::rtl::OUString;

// ============================================================================

ColorScaleContext::ColorScaleContext( CondFormatContext& rFragment, CondFormatRuleRef xRule ) :
    WorksheetContextBase( rFragment ),
    mxRule( xRule )
{
}

ContextHandlerRef ColorScaleContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cfRule ):
            return (nElement == XLS_TOKEN( colorScale )) ? this : 0;
        case XLS_TOKEN( colorScale ):
            if (nElement == XLS_TOKEN( cfvo ))
                return this;
            else if (nElement == XLS_TOKEN( color ))
                return this;
            else
                return 0;
    }
    return 0;
}

void ColorScaleContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cfvo ):
            mxRule->getColorScale()->importCfvo( rAttribs );
        break;
        case XLS_TOKEN( color ):
            mxRule->getColorScale()->importColor( rAttribs );
        break;
    }
}

void ColorScaleContext::onCharacters( const OUString&  )
{

}

// ============================================================================

DataBarContext::DataBarContext( CondFormatContext& rFragment, CondFormatRuleRef xRule ) :
    WorksheetContextBase( rFragment ),
    mxRule( xRule )
{
}

ContextHandlerRef DataBarContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cfRule ):
            return (nElement == XLS_TOKEN( colorScale )) ? this : 0;
        case XLS_TOKEN( dataBar ):
            if (nElement == XLS_TOKEN( cfvo ))
                return this;
            else if (nElement == XLS_TOKEN( color ))
                return this;
            else
                return 0;
    }
    return 0;
}

void DataBarContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( dataBar ):
            mxRule->getDataBar()->importAttribs( rAttribs );
        break;
        case XLS_TOKEN( cfvo ):
            mxRule->getDataBar()->importCfvo( rAttribs );
        break;
        case XLS_TOKEN( color ):
            mxRule->getDataBar()->importColor( rAttribs );
        break;
    }
}

// ============================================================================

CondFormatContext::CondFormatContext( WorksheetFragmentBase& rFragment ) :
    WorksheetContextBase( rFragment )
{
}

ContextHandlerRef CondFormatContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( conditionalFormatting ):
            return (nElement == XLS_TOKEN( cfRule )) ? this : 0;
        case XLS_TOKEN( cfRule ):
            if (nElement == XLS_TOKEN( formula ))
                return this;
            else if (nElement == XLS_TOKEN( colorScale ) )
                return new ColorScaleContext( *this, mxRule );
            else if (nElement == XLS_TOKEN( dataBar ) )
                return new DataBarContext( *this, mxRule );
            else if (nElement == XLS_TOKEN( extLst ) )
                return new ExtLstLocalContext( *this, mxRule->getDataBar()->getDataBarFormatData() );
            else
                return 0;
    }
    return 0;
}

void CondFormatContext::onEndElement()
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( conditionalFormatting ):
            if(mxCondFmt.get())
                mxCondFmt->finalizeImport();
            break;
    }
}

void CondFormatContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( conditionalFormatting ):
            mxCondFmt = getCondFormats().importConditionalFormatting( rAttribs );
        break;
        case XLS_TOKEN( cfRule ):
            if( mxCondFmt.get() ) mxRule = mxCondFmt->importCfRule( rAttribs );
        break;
    }
}

void CondFormatContext::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( formula ) ) && mxCondFmt.get() && mxRule.get() )
        mxRule->appendFormula( rChars );
}

ContextHandlerRef CondFormatContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_CONDFORMATTING:
            return (nRecId == BIFF12_ID_CFRULE) ? this : 0;
    }
    return 0;
}

void CondFormatContext::onStartRecord( SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_CONDFORMATTING:
            mxCondFmt = getCondFormats().importCondFormatting( rStrm );
        break;
        case BIFF12_ID_CFRULE:
            if( mxCondFmt.get() ) mxCondFmt->importCfRule( rStrm );
        break;
    }
}

void CondFormatContext::onEndRecord()
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_CONDFORMATTING:
            if( mxCondFmt.get() )
                mxCondFmt->finalizeImport();
            break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
