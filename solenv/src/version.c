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





#include <_version.h>


struct VersionInfo
{
    const char* pTime;
    const char* pDate;
    const char* pUpd;
    const char* pMinor;
    const char* pBuild;
    const char* pInpath;
};

static const struct VersionInfo g_aVersionInfo =
{
    __TIME__,
    __DATE__,
    _UPD,
    _LAST_MINOR,
    _BUILD,
    _INPATH
};

#if defined(WNT) || defined(OS2)
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void);
#endif

#if defined(WNT) || defined(OS2)
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void)
#else
const struct VersionInfo *GetVersionInfo(void)
#endif
{
    return &g_aVersionInfo;
}

#if 0
#include <stdio.h>

int main( int argc, char **argv )
{
    const VersionInfo *pInfo = GetVersionInfo();
    fprintf( stderr, "Date : %s\n", pInfo->pDate);
    fprintf( stderr, "Time : %s\n", pInfo->pTime);
    fprintf( stderr, "UPD : %s\n", pInfo->pUpd);
    delete pInfo;
    return 0;
}
#endif

