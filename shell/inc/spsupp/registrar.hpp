/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _REGISTRAR_H
#define _REGISTRAR_H

#include "windows.h"

namespace Registrar {
    HRESULT RegisterObject(REFIID riidCLSID,
                           REFIID riidTypeLib,
                           const wchar_t* sProgram,
                           const wchar_t* sComponent,
                           const wchar_t* Path);
    HRESULT UnRegisterObject(REFIID riidCLSID, const wchar_t* LibId, const wchar_t* ClassId);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
