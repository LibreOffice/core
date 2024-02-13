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

#include <comphelper/errcode.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/runtimetooustring.hxx>

UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OUString ErrCode::toString() const
{
    std::u16string_view pWarningError;
    if (IsWarning())
        pWarningError = u"Warning";
    else
        pWarningError = u"Error";

    std::u16string_view pArea;
    switch (GetArea())
    {
        case ErrCodeArea::Io:
            pArea = u"Io";
            break;
        case ErrCodeArea::Sfx:
            pArea = u"Sfx";
            break;
        case ErrCodeArea::Inet:
            pArea = u"Inet";
            break;
        case ErrCodeArea::Vcl:
            pArea = u"Vcl";
            break;
        case ErrCodeArea::Svx:
            pArea = u"Svx";
            break;
        case ErrCodeArea::So:
            pArea = u"So";
            break;
        case ErrCodeArea::Sbx:
            pArea = u"Sbx";
            break;
        case ErrCodeArea::Uui:
            pArea = u"Uui";
            break;
        case ErrCodeArea::Sc:
            pArea = u"Sc";
            break;
        case ErrCodeArea::Sd:
            pArea = u"Sd";
            break;
        case ErrCodeArea::Sw:
            pArea = u"Sw";
            break;
    }

    std::u16string_view pClass;
    switch (GetClass())
    {
        case ErrCodeClass::NONE:
            pClass = u"NONE";
            break;
        case ErrCodeClass::Abort:
            pClass = u"Abort";
            break;
        case ErrCodeClass::General:
            pClass = u"General";
            break;
        case ErrCodeClass::NotExists:
            pClass = u"NotExists";
            break;
        case ErrCodeClass::AlreadyExists:
            pClass = u"AlreadyExists";
            break;
        case ErrCodeClass::Access:
            pClass = u"Access";
            break;
        case ErrCodeClass::Path:
            pClass = u"Path";
            break;
        case ErrCodeClass::Locking:
            pClass = u"Locking";
            break;
        case ErrCodeClass::Parameter:
            pClass = u"Parameter";
            break;
        case ErrCodeClass::Space:
            pClass = u"Space";
            break;
        case ErrCodeClass::NotSupported:
            pClass = u"NotSupported";
            break;
        case ErrCodeClass::Read:
            pClass = u"Read";
            break;
        case ErrCodeClass::Write:
            pClass = u"Write";
            break;
        case ErrCodeClass::Unknown:
            pClass = u"Unknown";
            break;
        case ErrCodeClass::Version:
            pClass = u"Version";
            break;
        case ErrCodeClass::Format:
            pClass = u"Format";
            break;
        case ErrCodeClass::Create:
            pClass = u"Create";
            break;
        case ErrCodeClass::Import:
            pClass = u"Import";
            break;
        case ErrCodeClass::Export:
            pClass = u"Export";
            break;
        case ErrCodeClass::So:
            pClass = u"So";
            break;
        case ErrCodeClass::Sbx:
            pClass = u"Sbx";
            break;
        case ErrCodeClass::Runtime:
            pClass = u"Runtime";
            break;
        case ErrCodeClass::Compiler:
            pClass = u"Compiler";
            break;
    }
    return toHexString() + "(" + pWarningError + " Area:" + pArea + " Class:" + pClass
           + " Code:" + OUString::number(GetCode()) + ")";
}

COMPHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const ErrCode& err)
{
    os << err.toString();
    return os;
}

UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OUString ErrCodeMsg::toString() const
{
    OUString s = mnCode.toString();
    if (!maArg1.isEmpty())
        s += " arg1=" + maArg1;
    if (!maArg2.isEmpty())
        s += " arg2=" + maArg2;
#ifdef LIBO_ERRMSG_USE_SOURCE_LOCATION
    if (!moLoc)
        s += OUString::Concat(" func=") + o3tl::runtimeToOUString(moLoc->function_name()) + " src="
             + o3tl::runtimeToOUString(moLoc->file_name()) + ":" + OUString::number(moLoc->line());
#endif
    return s;
}

COMPHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const ErrCodeMsg& err)
{
    os << err.toString();
    return os;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
