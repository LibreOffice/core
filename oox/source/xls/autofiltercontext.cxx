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

#include "oox/xls/autofiltercontext.hxx"

#include "oox/xls/autofilterbuffer.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

using ::oox::core::ContextHandlerRef;
using ::rtl::OUString;

// ============================================================================

FilterSettingsContext::FilterSettingsContext( WorksheetContextBase& rParent, FilterSettingsBase& rFilterSettings ) :
    WorksheetContextBase( rParent ),
    mrFilterSettings( rFilterSettings )
{
}

ContextHandlerRef FilterSettingsContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( filters ):
            if( nElement == XLS_TOKEN( filter ) ) return this;
        break;
        case XLS_TOKEN( customFilters ):
            if( nElement == XLS_TOKEN( customFilter ) ) return this;
        break;
    }
    return 0;
}

void FilterSettingsContext::onStartElement( const AttributeList& rAttribs )
{
    mrFilterSettings.importAttribs( getCurrentElement(), rAttribs );
}

ContextHandlerRef FilterSettingsContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& /*rStrm*/ )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_DISCRETEFILTERS:
            if( nRecId == BIFF12_ID_DISCRETEFILTER ) return this;
        break;
        case BIFF12_ID_CUSTOMFILTERS:
            if( nRecId == BIFF12_ID_CUSTOMFILTER ) return this;
        break;
    }
    return 0;
}

void FilterSettingsContext::onStartRecord( SequenceInputStream& rStrm )
{
    mrFilterSettings.importRecord( getCurrentElement(), rStrm );
}

// ============================================================================

FilterColumnContext::FilterColumnContext( WorksheetContextBase& rParent, FilterColumn& rFilterColumn ) :
    WorksheetContextBase( rParent ),
    mrFilterColumn( rFilterColumn )
{
}

ContextHandlerRef FilterColumnContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    if( getCurrentElement() == XLS_TOKEN( filterColumn ) ) switch( nElement )
    {
        case XLS_TOKEN( filters ):
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< DiscreteFilter >() );
        case XLS_TOKEN( top10 ):
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< Top10Filter >() );
        case XLS_TOKEN( customFilters ):
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< CustomFilter >() );
    }
    return 0;
}

void FilterColumnContext::onStartElement( const AttributeList& rAttribs )
{
    mrFilterColumn.importFilterColumn( rAttribs );
}

ContextHandlerRef FilterColumnContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& /*rStrm*/ )
{
    if( getCurrentElement() == BIFF12_ID_FILTERCOLUMN ) switch( nRecId )
    {
        case BIFF12_ID_DISCRETEFILTERS:
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< DiscreteFilter >() );
        case BIFF12_ID_TOP10FILTER:
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< Top10Filter >() );
        case BIFF12_ID_CUSTOMFILTERS:
            return new FilterSettingsContext( *this, mrFilterColumn.createFilterSettings< CustomFilter >() );
    }
    return 0;
}

void FilterColumnContext::onStartRecord( SequenceInputStream& rStrm )
{
    mrFilterColumn.importFilterColumn( rStrm );
}

// ============================================================================

AutoFilterContext::AutoFilterContext( WorksheetFragmentBase& rFragment, AutoFilter& rAutoFilter ) :
    WorksheetContextBase( rFragment ),
    mrAutoFilter( rAutoFilter )
{
}

ContextHandlerRef AutoFilterContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    if( (getCurrentElement() == XLS_TOKEN( autoFilter )) && (nElement == XLS_TOKEN( filterColumn )) )
        return new FilterColumnContext( *this, mrAutoFilter.createFilterColumn() );
    return 0;
}

void AutoFilterContext::onStartElement( const AttributeList& rAttribs )
{
    mrAutoFilter.importAutoFilter( rAttribs, getSheetIndex() );
}

ContextHandlerRef AutoFilterContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& /*rStrm*/ )
{
    if( (getCurrentElement() == BIFF12_ID_AUTOFILTER) && (nRecId == BIFF12_ID_FILTERCOLUMN) )
        return new FilterColumnContext( *this, mrAutoFilter.createFilterColumn() );
    return 0;
}

void AutoFilterContext::onStartRecord( SequenceInputStream& rStrm )
{
    mrAutoFilter.importAutoFilter( rStrm, getSheetIndex() );
}

// ============================================================================

BiffAutoFilterContext::BiffAutoFilterContext( const WorksheetHelper& rHelper, AutoFilter& rAutoFilter ) :
    BiffWorksheetContextBase( rHelper ),
    mrAutoFilter( rAutoFilter )
{
}

void BiffAutoFilterContext::importRecord( BiffInputStream& rStrm )
{
    switch( rStrm.getRecId() )
    {
        // nothing to read for BIFF_ID_AUTOFILTER
        case BIFF_ID_FILTERCOLUMN:  mrAutoFilter.createFilterColumn().importFilterColumn( rStrm );  break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
