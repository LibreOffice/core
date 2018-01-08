//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

// Bridge functions between LibreOfficeKit library and swift application.
// Only functions mentioned here can be used from the application
// The swift compiler uses this header to generate a needed interface
// The functions (LibreOfficeKit.mm) calls functions directly in LibreOffice

int BridgeLOkit_Init(const char *path);
int BridgeLOkit_Sizing(const int countXtiles, const int countYtiles,
                       const int pixelsXtile, const int pixelsYtile);
int BridgeLOkit_open(const char *path);
int BridgeLOkit_ClientCommand(const char *input);
int BridgeLOkit_Hipernate(void);
int BridgeLOkit_LeaveHipernate(void);


// Use LOK_USE_UNSTABLE_API to get access to the low level
// LibreOfficeKit.
// REMARK: There are no guarantee these functions will work !!
// REMARK: BridgeLOkit_Init MUST be called, NOT the original init()

#ifdef LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

LibreOfficeKit* BridgeLOkit_getLOK(void);
#endif

