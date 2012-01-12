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



#ifndef _SV_PRINT_H
#define _SV_PRINT_H

#include "vcl/sv.h"
#include "vcl/dllapi.h"

#include "rtl/ustring.hxx"

#include <vector>
#include <hash_map>

struct SalPrinterQueueInfo;
class QueueInfo;
class JobSetup;

namespace vcl
{ class PrinterListener; }

// --------------------
// - ImplPrnQueueData -
// --------------------

struct ImplPrnQueueData
{
    QueueInfo*              mpQueueInfo;
    SalPrinterQueueInfo*    mpSalQueueInfo;
};

// --------------------
// - ImplPrnQueueList -
// --------------------

class VCL_PLUGIN_PUBLIC ImplPrnQueueList
{
public:
    std::hash_map< rtl::OUString, sal_Int32, rtl::OUStringHash >
                                        m_aNameToIndex;
    std::vector< ImplPrnQueueData >     m_aQueueInfos;
    std::vector< rtl::OUString >        m_aPrinterList;

    ImplPrnQueueList() {}
    ~ImplPrnQueueList();

    void                    Add( SalPrinterQueueInfo* pData );
    ImplPrnQueueData*       Get( const rtl::OUString& rPrinter );
};

// --------------
// - Prototypes -
// --------------

void ImplDeletePrnQueueList();
void SAL_DLLPRIVATE ImplUpdateJobSetupPaper( JobSetup& rJobSetup );


#endif // _SV_PRINT_H
