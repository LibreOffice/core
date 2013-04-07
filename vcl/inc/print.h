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

#ifndef _SV_PRINT_H
#define _SV_PRINT_H

#include "tools/solar.h"
#include "vcl/dllapi.h"

#include "rtl/ustring.hxx"

#include <vector>
#include <boost/unordered_map.hpp>

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
    boost::unordered_map< OUString, sal_Int32, OUStringHash >
                                        m_aNameToIndex;
    std::vector< ImplPrnQueueData >     m_aQueueInfos;
    std::vector< OUString >        m_aPrinterList;

    ImplPrnQueueList() {}
    ~ImplPrnQueueList();

    void                    Add( SalPrinterQueueInfo* pData );
    ImplPrnQueueData*       Get( const OUString& rPrinter );
};

// --------------
// - Prototypes -
// --------------

void ImplDeletePrnQueueList();
void SAL_DLLPRIVATE ImplUpdateJobSetupPaper( JobSetup& rJobSetup );


#endif // _SV_PRINT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
