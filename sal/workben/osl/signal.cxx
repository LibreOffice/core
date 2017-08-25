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

#include <sal/main.h>
#include <osl/signal.h>

#include <cstdio>

#define OSL_SIGNAL_USER_TEST1   (OSL_SIGNAL_USER_RESERVED - 64)

typedef struct
{
    sal_uInt32 nData;
} SignalData;

oslSignalAction SignalHandlerFunc1(void *pData, oslSignalInfo* pInfo);
oslSignalAction SignalHandlerFunc2(void *pData, oslSignalInfo* pInfo);

SAL_IMPLEMENT_MAIN()
{
    fprintf(stdout, "Signal handling example.\n");
    fprintf(stdout, "    Before the signal nData == 0, after the signal nData == 1\n");

    oslSignalHandler hHandler1, hHandler2;
    SignalData aSigData;
    aSigData.nData = 0;

    fprintf(stdout, "        Adding signal handler 1\n");
    hHandler1 = osl_addSignalHandler(SignalHandlerFunc1, &aSigData);

    fprintf(stdout, "        Signal data set to %d.\n", aSigData.nData);

    SignalData aSetSigData;
    aSetSigData.nData = 1;
    osl_raiseSignal(OSL_SIGNAL_USER_TEST1, &aSetSigData);

    fprintf(stdout, "    Before the signal nData == 0, after the signal nData == 2\n");
    fprintf(stdout, "        Signal data set to %d.\n", aSigData.nData);

    // add a second signal handler that increments the counter...
    fprintf(stdout, "        Adding signal handler 2\n");
    hHandler2 = osl_addSignalHandler(SignalHandlerFunc2, &aSigData);

    aSigData.nData = 0;
    fprintf(stdout, "        Signal data set to %d.\n", aSigData.nData);

    osl_raiseSignal(OSL_SIGNAL_USER_TEST1, &aSetSigData);

    fprintf(stdout, "        Signal data set to %d.\n", aSigData.nData);

    fprintf(stdout, "    Remove signal handlers.\n");
    osl_removeSignalHandler(hHandler1);
    osl_removeSignalHandler(hHandler2);

    return 0;
}

oslSignalAction SignalHandlerFunc1(void *pData, oslSignalInfo* pInfo)
{
    SignalData *pSignalData = reinterpret_cast< SignalData* >(pData);
    SignalData *pPassedData = reinterpret_cast< SignalData* >(pInfo->UserData);

    if (pInfo->Signal == osl_Signal_User)
    {
       switch (pInfo->UserSignal)
       {
           case OSL_SIGNAL_USER_TEST1:
               fprintf(stdout, "        Signal handler 1 called...\n");
               pSignalData->nData = pPassedData->nData;
               break;
       }
    }

    return osl_Signal_ActCallNextHdl;
}

oslSignalAction SignalHandlerFunc2(void* /* pData */, oslSignalInfo* pInfo)
{
    SignalData *pSignalData = reinterpret_cast< SignalData* >(pInfo->UserData);

    if (pInfo->Signal == osl_Signal_User)
    {
       switch (pInfo->UserSignal)
       {
           case OSL_SIGNAL_USER_TEST1:
               fprintf(stdout, "        Signal handler 2 called...\n");
               pSignalData->nData++;
               break;
       }
    }

    return osl_Signal_ActCallNextHdl;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
