/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

namespace oox::xls {

class PivotCache;
class PivotCacheField;
class PivotCacheItem;

class PivotCacheFieldContext : public WorkbookContextBase
{
public:
    explicit            PivotCacheFieldContext(
                            WorkbookFragmentBase& rFragment,
                            PivotCacheField& rCacheField );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;

private:
    PivotCacheField&    mrCacheField;
};

class PivotCacheDefinitionFragment : public WorkbookFragmentBase
{
public:
    explicit            PivotCacheDefinitionFragment(
                            const WorkbookHelper& rHelper,
                            const OUString& rFragmentPath,
                            PivotCache& rPivotCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;
    virtual void        finalizeImport() override;

private:
    PivotCache&         mrPivotCache;
};

class PivotCacheRecordsFragment : public WorkbookFragmentBase
{
public:
    /** The sheet globals name the sheet that takes the records as cells. They are null when the
        source sheet is part of the document. */
    explicit PivotCacheRecordsFragment(const WorkbookHelper& rHelper, const OUString& rFragmentPath,
                                       PivotCache& rPivotCache, WorksheetGlobalsRef xSheetGlobals);

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;
    virtual void finalizeImport() override;

private:
    void                startCacheRecord();
    void addRecordItem(const PivotCacheItem& rItem);
    void                importPCRecord( SequenceInputStream& rStrm );
    void                importPCRecordItem( sal_Int32 nRecId, SequenceInputStream& rStrm );

private:
    PivotCache& mrPivotCache;
    WorksheetGlobalsRef mxSheetGlobals; /// Sheet that takes the records as cells, or null.
    sal_Int32 mnColIdx = 0; /// Relative column index in source data.
    sal_Int32 mnRowIdx = 0; /// Relative row index in source data.
    bool mbInRecord = false;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
