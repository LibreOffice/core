/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
