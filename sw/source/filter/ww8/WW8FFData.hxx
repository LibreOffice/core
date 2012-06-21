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

#include <vector>
#include <rtl/ustring.hxx>

class SvStream;

namespace sw
{

class WW8FFData
{
private:
    // offset 0x4
    sal_uInt8 mnType; // :2 0x3
    sal_uInt8 mnResult; // :5 0x7c
    bool mbOwnHelp; // :1 0x80

    // offset 5
    bool mbOwnStat; // :1 0x01
    bool mbProtected; // :1 0x02
    bool mbSize; // :1 0x04
    sal_uInt8 mnTextType; // :3 0x38
    bool mbRecalc; // :1 0x4
    bool mbListBox; // :1 0x80

    // offset 6
    sal_uInt16 mnMaxLen; // :15 0x7fff maximum length of text field, 0 <=> no limit

    // offset 8
    sal_uInt16 mnCheckboxHeight;

    // offset 10 and beyond
    ::rtl::OUString msName;
    ::rtl::OUString msDefault; // only for type == 0
    sal_uInt16 mnDefault; // only for type != 0
    ::rtl::OUString msFormat;
    ::rtl::OUString msHelp;
    ::rtl::OUString msStatus;
    ::rtl::OUString msMacroEnter;
    ::rtl::OUString msMacroExit;

    ::std::vector< ::rtl::OUString > msListEntries;

protected:
    void WriteOUString(SvStream * pStream, const ::rtl::OUString & rStr, bool bAddZero);

public:
    WW8FFData();
    ~WW8FFData();

    void setType(sal_uInt8 nType) { mnType = nType; }
    sal_uInt8 getType() const { return mnType; }
    void setResult(sal_uInt8 nResult) { mnResult = nResult; }
    sal_uInt8 getResult() const { return mnResult; }
    void setProptexted(bool bProtected) { mbProtected = bProtected; }
    bool getProtected() const { return mbProtected; }
    void setSize(bool bSize) { mbSize = bSize; }
    bool getSize() const { return mbSize; }
    void setTextType(sal_uInt8 nTextType) { mnTextType = nTextType; }
    sal_uInt8 getTextType() const { return mnTextType; }
    void setRecalc(bool bRecalc) { mbRecalc = bRecalc; }
    bool getRecalc() const { return mbRecalc; }
    void setListBox(bool bListBox) { mbListBox = bListBox; }
    bool getListBox() const { return mbListBox; }
    void setMaxLen(sal_uInt16 nMaxLen) { mnMaxLen = nMaxLen; }
    sal_uInt16 getMaxLen() const { return mnMaxLen; }
    void setCheckboxHeight(sal_uInt16 nCheckboxHeight) { mnCheckboxHeight = nCheckboxHeight; }
    sal_uInt16 getCheckboxHeight() const { return mnCheckboxHeight; }
    void setName(const ::rtl::OUString & rName) { msName = rName; }
    const ::rtl::OUString & getName() const { return msName; }
    void setDefaultString(const ::rtl::OUString & rDefault) { msDefault = rDefault; }
    const ::rtl::OUString & getDefaultString() const { return msDefault; }
    void setDefaultResult(sal_uInt16 nDefault) { mnDefault = nDefault; }
    sal_uInt16 getDefaultResult() const { return mnDefault; }
    void setFormat(const ::rtl::OUString & rFormat) { msFormat = rFormat; }
    const ::rtl::OUString & getFormat() const { return msFormat; }
    void setHelp(const ::rtl::OUString & rHelp);
    const ::rtl::OUString getHelp() const { return msHelp; }
    void setStatus(const ::rtl::OUString & rStatus);
    const ::rtl::OUString & getStatus() const { return msStatus; }
    void setMacroEnter(const ::rtl::OUString & rMacroEnter) { msMacroEnter = rMacroEnter; }
    const ::rtl::OUString & getMacroEnter() const { return msMacroEnter; }
    void setMacroExit(const ::rtl::OUString & rMacroExit) { msMacroExit = rMacroExit; }
    const ::rtl::OUString & getMacroExit() const { return msMacroExit; }

    void addListboxEntry(const ::rtl::OUString & rEntry);

    void Write(SvStream * pDataStrm);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
