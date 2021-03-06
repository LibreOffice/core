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

namespace oox::xls {

class PivotCache;
class PivotCacheField;

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

class PivotCacheRecordsFragment : public WorksheetFragmentBase
{
public:
    explicit            PivotCacheRecordsFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath,
                            const PivotCache& rPivotCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;

private:
    void                startCacheRecord();
    void                importPCRecord( SequenceInputStream& rStrm );
    void                importPCRecordItem( sal_Int32 nRecId, SequenceInputStream& rStrm );

private:
    const PivotCache&   mrPivotCache;
    sal_Int32           mnColIdx;           /// Relative column index in source data.
    sal_Int32           mnRowIdx;           /// Relative row index in source data.
    bool                mbInRecord;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
