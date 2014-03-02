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

#include "condformatcontext.hxx"
#include "extlstcontext.hxx"

namespace oox {
namespace xls {



using ::oox::core::ContextHandlerRef;



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
            return (nElement == XLS_TOKEN( dataBar )) ? this : 0;
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



IconSetContext::IconSetContext( CondFormatContext& rFragment, CondFormatRuleRef xRule ) :
    WorksheetContextBase( rFragment ),
    mxRule( xRule )
{
}

ContextHandlerRef IconSetContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cfRule ):
            return (nElement == XLS_TOKEN( iconSet )) ? this : 0;
        case XLS_TOKEN( iconSet ):
            if (nElement == XLS_TOKEN( cfvo ))
                return this;
            else
                return 0;
    }
    return 0;
}

void IconSetContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( iconSet ):
            mxRule->getIconSet()->importAttribs( rAttribs );
        break;
        case XLS_TOKEN( cfvo ):
            mxRule->getIconSet()->importCfvo( rAttribs );
        break;
    }
}



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
            else if (nElement == XLS_TOKEN( iconSet ) )
                return new IconSetContext( *this, mxRule );
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
                mxCondFmt->setReadyForFinalize();
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



} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
