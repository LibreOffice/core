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



#ifndef OOX_XLS_EXCELVBAPROJECT_HXX
#define OOX_XLS_EXCELVBAPROJECT_HXX

#include "oox/ole/vbaproject.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
        namespace sheet { class XSpreadsheetDocument; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Special implementation of the VBA project for the Excel filters. */
class OOX_DLLPUBLIC ExcelVbaProject : public ::oox::ole::VbaProject
{
public:
    explicit            ExcelVbaProject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& rxDocument );

protected:
    /** Adds dummy modules for sheets without imported code name. */
    virtual void        prepareImport();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                        mxDocument;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
