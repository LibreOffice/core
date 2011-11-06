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



#ifndef OOX_XLS_CHARTSHEETFRAGMENT_HXX
#define OOX_XLS_CHARTSHEETFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

class ChartsheetFragment : public WorksheetFragmentBase
{
public:
    explicit            ChartsheetFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        initializeImport();
    virtual void        finalizeImport();

private:
    /** Imports the the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( SequenceInputStream& rStrm );
};

// ============================================================================

class BiffChartsheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffChartsheetFragment(
                            const WorksheetHelper& rHelper,
                            const BiffWorkbookFragmentBase& rParent );

    /** Imports the entire sheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
