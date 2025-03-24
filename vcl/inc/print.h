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

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/QueueInfo.hxx>
#include "salprn.hxx"

#include <vector>
#include <unordered_map>

class JobSetup;

namespace vcl
{ class PrinterListener; }

struct ImplPrnQueueData
{
    std::unique_ptr<QueueInfo>           mpQueueInfo;
    std::unique_ptr<SalPrinterQueueInfo> mpSalQueueInfo;

// unlike other similar places, we need to ifdef this to keep old GCC baseline happy
#ifdef _MSC_VER
    ImplPrnQueueData() {}
    ImplPrnQueueData(ImplPrnQueueData&&) = default;
  
    ImplPrnQueueData& operator=( ImplPrnQueueData const & ) = delete; // MSVC2017 workaround
    ImplPrnQueueData( ImplPrnQueueData const & ) = delete; // MSVC2017 workaround
#endif
};

class VCL_PLUGIN_PUBLIC ImplPrnQueueList
{
public:
    std::unordered_map< OUString, sal_Int32 > m_aNameToIndex;
    std::vector< ImplPrnQueueData >     m_aQueueInfos;
    std::vector< OUString >             m_aPrinterList;

    ImplPrnQueueList() {}
    ~ImplPrnQueueList();

    ImplPrnQueueList& operator=( ImplPrnQueueList const & ) = delete; // MSVC2017 workaround
    ImplPrnQueueList( ImplPrnQueueList const & ) = delete; // MSVC2017 workaround

void                    Add( std::unique_ptr<SalPrinterQueueInfo> pData );
    ImplPrnQueueData*       Get( const OUString& rPrinter );
};

void ImplDeletePrnQueueList();
void ImplUpdateJobSetupPaper( JobSetup& rJobSetup );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
