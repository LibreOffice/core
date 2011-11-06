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



#ifndef OOX_XLS_WORKBOOKFRAGMENT_HXX
#define OOX_XLS_WORKBOOKFRAGMENT_HXX

#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

class ExternalLink;

// ============================================================================

class WorkbookFragment : public WorkbookFragmentBase
{
public:
    explicit            WorkbookFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();

private:
    void                importExternalReference( const AttributeList& rAttribs );
    void                importDefinedName( const AttributeList& rAttribs );
    void                importPivotCache( const AttributeList& rAttribs );

    void                importExternalRef( SequenceInputStream& rStrm );
    void                importPivotCache( SequenceInputStream& rStrm );

    void                importExternalLinkFragment( ExternalLink& rExtLink );
    void                importPivotCacheDefFragment( const ::rtl::OUString& rRelId, sal_Int32 nCacheId );

private:
    DefinedNameRef      mxCurrName;
};

// ============================================================================

class BiffWorkbookFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffWorkbookFragment( const WorkbookHelper& rHelper, const ::rtl::OUString& rStrmName );

    /** Imports the entire workbook stream, including all contained worksheets. */
    virtual bool        importFragment();

private:
    /** Imports a complete BIFF4 workspace fragment (with embedded sheets). */
    bool                importWorkspaceFragment();
    /** Imports the workbook globals fragment from current stream position. */
    bool                importGlobalsFragment( ISegmentProgressBar& rProgressBar );
    /** Imports a sheet fragment with passed type from current stream position. */
    bool                importSheetFragment(
                            ISegmentProgressBar& rProgressBar,
                            BiffFragmentType eFragment, sal_Int16 nCalcSheet );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
