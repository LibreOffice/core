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



#ifndef OOX_XLS_SHAREDSTRINGSBUFFER_HXX
#define OOX_XLS_SHAREDSTRINGSBUFFER_HXX

#include "oox/xls/richstring.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Collects all strings from the shared strings substream. */
class SharedStringsBuffer : public WorkbookHelper
{
public:
    explicit            SharedStringsBuffer( const WorkbookHelper& rHelper );

    /** Creates and returns a new string entry. */
    RichStringRef       createRichString();
    /** Imports the complete shared string table from a BIFF file. */
    void                importSst( BiffInputStream& rStrm );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Returns the specified string. */
    RichStringRef       getString( sal_Int32 nStringId ) const;

private:
    typedef RefVector< RichString > StringVector;
    StringVector        maStrings;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
