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

#pragma once

#include "excelhandlers.hxx"
#include "autofilterbuffer.hxx"

namespace oox::xls {

class AutoFilter;
class FilterColumn;
class FilterSettingsBase;

class FilterSettingsContext final : public WorksheetContextBase
{
public:
    explicit            FilterSettingsContext( WorksheetContextBase& rParent, FilterSettingsBase& rFilterSettings );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

    FilterSettingsBase& mrFilterSettings;
};

class FilterColumnContext final : public WorksheetContextBase
{
public:
    explicit            FilterColumnContext( WorksheetContextBase& rParent, FilterColumn& rFilterColumn );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

    FilterColumn&       mrFilterColumn;
};

// class SortConditionContext

class SortConditionContext final : public WorksheetContextBase
{
public:
    explicit            SortConditionContext( WorksheetContextBase& rFragment, SortCondition& rSortCondition );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

    SortCondition&      mrSortCondition;
};

// class SortStateContext

class SortStateContext final : public WorksheetContextBase
{
public:
    explicit            SortStateContext( WorksheetContextBase& rFragment, AutoFilter& rAutoFilter );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

    AutoFilter&         mrAutoFilter;
};

// class AutoFilterContext

class AutoFilterContext final : public WorksheetContextBase
{
public:
    explicit            AutoFilterContext( WorksheetFragmentBase& rFragment, AutoFilter& rAutoFilter );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

    AutoFilter&         mrAutoFilter;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
