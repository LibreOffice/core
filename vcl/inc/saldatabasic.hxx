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

inline void SetSalData( SalData* pData )
{ ImplGetSVData()->mpSalData = pData; }

inline SalData* GetSalData()
{ return ImplGetSVData()->mpSalData; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
