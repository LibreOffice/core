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

#include "oox/xls/condformatcontext.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

// ============================================================================

OoxCondFormatContext::OoxCondFormatContext( OoxWorksheetFragmentBase& rFragment ) :
    OoxWorksheetContextBase( rFragment )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxCondFormatContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( conditionalFormatting ):
            return (nElement == XLS_TOKEN( cfRule )) ? this : 0;
        case XLS_TOKEN( cfRule ):
            return (nElement == XLS_TOKEN( formula )) ? this : 0;
    }
    return 0;
}

void OoxCondFormatContext::onStartElement( const AttributeList& rAttribs )
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

void OoxCondFormatContext::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( formula ):
            if( mxCondFmt.get() && mxRule.get() ) mxRule->appendFormula( rChars );
        break;
    }
}

ContextHandlerRef OoxCondFormatContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_CONDFORMATTING:
            return (nRecId == OOBIN_ID_CFRULE) ? this : 0;
    }
    return 0;
}

void OoxCondFormatContext::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_CONDFORMATTING:
            mxCondFmt = getCondFormats().importCondFormatting( rStrm );
        break;
        case OOBIN_ID_CFRULE:
            if( mxCondFmt.get() ) mxCondFmt->importCfRule( rStrm );
        break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
