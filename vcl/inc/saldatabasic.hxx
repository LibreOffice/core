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



#ifndef _SV_SALDATABASIC_HXX
#define _SV_SALDATABASIC_HXX

#include <svdata.hxx>
#include <salinst.hxx>
#include <osl/module.h>

#include "vcl/dllapi.h"

namespace psp
{
    class PrinterInfoManager;
}

class VCL_PLUGIN_PUBLIC SalData
{
public:
    SalInstance*                  m_pInstance; // pointer to instance
    oslModule                     m_pPlugin;   // plugin library handle
    psp::PrinterInfoManager*      m_pPIManager;

    SalData();
    virtual ~SalData();

};

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline void SetSalData( SalData* pData )
{ ImplGetSVData()->mpSalData = (void*)pData; }

inline SalData* GetSalData()
{ return (SalData*)ImplGetSVData()->mpSalData; }

#endif
