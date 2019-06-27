/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SHELL_INC_SPSUPP_REGISTRAR_H
#define INCLUDED_SHELL_INC_SPSUPP_REGISTRAR_H

#include <initializer_list>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

class Registrar {
public:
    explicit Registrar(REFIID riidCLSID);
    // First version in list becomes default
    HRESULT RegisterObject(REFIID riidTypeLib,
                           const wchar_t* sProgram,
                           const wchar_t* sComponent,
                           std::initializer_list<int> aVersions,
                           const wchar_t* Path);
    HRESULT UnRegisterObject(const wchar_t* sProgram, const wchar_t* sComponent,
                             std::initializer_list<int> aVersions);
    // First version in list becomes the default
    HRESULT RegisterProgIDs(const wchar_t* sProgram, const wchar_t* sComponent,
                            std::initializer_list<int> aVersions);
    HRESULT UnRegisterProgIDs(const wchar_t* sProgram, const wchar_t* sComponent,
                              std::initializer_list<int> aVersions);

private:
    HRESULT RegisterProgID(const wchar_t* sProgram, const wchar_t* sComponent, int nVersion,
                           bool bSetDefault);
    HRESULT UnRegisterProgID(const wchar_t* sProgram, const wchar_t* sComponent, int nVersion);

    static const size_t nGUIDlen = 40;
    wchar_t m_sCLSID[nGUIDlen];
    HRESULT m_ConstructionResult;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
