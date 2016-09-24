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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_GETFILENAMEWRAPPER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_GETFILENAMEWRAPPER_HXX

#include <sal/types.h>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <commdlg.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/*
    A simple wrapper around the GetOpenFileName/GetSaveFileName API.
    Because currently the Win32 API functions GetOpenFileName/GetSaveFileName
    work only properly in an Single Threaded Apartment.
*/

class CGetFileNameWrapper
{
public:
    CGetFileNameWrapper();

    bool getOpenFileName(LPOPENFILENAME lpofn);
    bool getSaveFileName(LPOPENFILENAME lpofn);
    int  commDlgExtendedError();

private:
    int m_ExtendedDialogError;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
