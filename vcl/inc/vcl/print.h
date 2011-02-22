/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_PRINT_H
#define _SV_PRINT_H

#include "vcl/sv.h"
#include "vcl/dllapi.h"

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
