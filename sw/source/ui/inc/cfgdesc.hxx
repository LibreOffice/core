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
#ifndef _CFGDESC_HXX
#define _CFGDESC_HXX

#include <tools/string.hxx>

class SwCfgDesc
{
protected:
    String aFuncText;
    String aLibName;
    sal_uInt16 nEvent;

    SwCfgDesc(const String &rFuncName, sal_uInt16 nId);
    SwCfgDesc(const String &rLib,const String &rMacro);
    ~SwCfgDesc();

public:
    void SetEvent(sal_uInt16 nE);               // inline
    void SetFuncText(const String &rStr);   // inline
    void SetMacro(const String &rLib, const String &rMac);
    sal_Bool IsMacro() const;
    sal_uInt16 GetEvent() const;                // inline
    String GetFuncText() const;             // inline
    String GetMacro() const;                // inline
    String GetLib() const;              // inline
};

// INLINE METHODE --------------------------------------------------------
inline void SwCfgDesc::SetEvent(sal_uInt16 nE) {
    nEvent = nE;
}

inline void SwCfgDesc::SetFuncText(const String &rStr) {
    aFuncText = rStr;
}

inline  sal_uInt16 SwCfgDesc::GetEvent() const { return nEvent; }
inline  String SwCfgDesc::GetFuncText() const { return aFuncText; }
inline  String SwCfgDesc::GetMacro() const { return aFuncText; }
inline  String SwCfgDesc::GetLib() const { return aLibName; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
