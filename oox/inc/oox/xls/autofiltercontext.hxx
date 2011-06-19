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

#ifndef OOX_XLS_AUTOFILTERCONTEXT_HXX
#define OOX_XLS_AUTOFILTERCONTEXT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

class AutoFilter;
class FilterColumn;
class FilterSettingsBase;

// ============================================================================

class FilterSettingsContext : public WorksheetContextBase
{
public:
    explicit            FilterSettingsContext( WorksheetContextBase& rParent, FilterSettingsBase& rFilterSettings );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        onStartRecord( SequenceInputStream& rStrm );

private:
    FilterSettingsBase& mrFilterSettings;
};

// ============================================================================

class FilterColumnContext : public WorksheetContextBase
{
public:
    explicit            FilterColumnContext( WorksheetContextBase& rParent, FilterColumn& rFilterColumn );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        onStartRecord( SequenceInputStream& rStrm );

private:
    FilterColumn&       mrFilterColumn;
};

// ============================================================================

class AutoFilterContext : public WorksheetContextBase
{
public:
    explicit            AutoFilterContext( WorksheetFragmentBase& rFragment, AutoFilter& rAutoFilter );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        onStartRecord( SequenceInputStream& rStrm );

private:
    AutoFilter&         mrAutoFilter;
};

// ============================================================================

class BiffAutoFilterContext : public BiffWorksheetContextBase
{
public:
    explicit            BiffAutoFilterContext( const WorksheetHelper& rHelper, AutoFilter& rAutoFilter );

protected:
    /** Imports all records related to the current auto filter. */
    virtual void        importRecord( BiffInputStream& rStrm );

private:
    AutoFilter&         mrAutoFilter;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
