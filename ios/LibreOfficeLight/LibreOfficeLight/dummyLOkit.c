//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
#include "lokit-Bridging-Header.h"



// replaces LibreOfficeKit and thereby reducing turn around time, which is
// favorized while working on pure UI issues
// It can be turned on/off by adding/removing it from settings/build phases/compile sources

int BridgeLOkit_Init(const char *path)
{
    (void)path;
    return 0;
}



int BridgeLOkit_open(const char *path)
{
    (void)path;
    return 0;
}



int BridgeLOkit_ClientCommand(const char *input)
{
    (void)input;
    return 0;
}



int BridgeLOkit_Hipernate()
{
    return 0;
}



int BridgeLOkit_LeaveHipernate()
{
    return 0;
}



int BridgeLOkit_Sizing(const int countXtiles, const int countYtiles,
                       const int pixelsXtile, const int pixelsYtile)
{
    (void)countXtiles;
    (void)countYtiles;
    (void)pixelsXtile;
    (void)pixelsYtile;
    return 0;
}


