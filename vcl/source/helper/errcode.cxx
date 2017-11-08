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

#include <vcl/errcode.hxx>

VCL_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const ErrCode& err)
{
    os << err.toHexString() << "(" << (err.IsWarning() ? "Warning" : "Error");
    if (err.IsDynamic())
        os << " Dynamic";
    else
    {
        os << " Area:";
        switch (err.GetArea())
        {
            case ErrCodeArea::Io:
                os << "Io";
                break;
            case ErrCodeArea::Sv:
                os << "Sv";
                break;
            case ErrCodeArea::Sfx:
                os << "Sfx";
                break;
            case ErrCodeArea::Inet:
                os << "Inet";
                break;
            case ErrCodeArea::Vcl:
                os << "Vcl";
                break;
            case ErrCodeArea::Svx:
                os << "Svx";
                break;
            case ErrCodeArea::So:
                os << "So";
                break;
            case ErrCodeArea::Sbx:
                os << "Sbx";
                break;
            case ErrCodeArea::Db:
                os << "Db";
                break;
            case ErrCodeArea::Java:
                os << "Java";
                break;
            case ErrCodeArea::Uui:
                os << "Uui";
                break;
            case ErrCodeArea::Lib2:
                os << "Lib2";
                break;
            case ErrCodeArea::Chaos:
                os << "Chaos";
                break;
            case ErrCodeArea::Sc:
                os << "Sc";
                break;
            case ErrCodeArea::Sd:
                os << "Sd";
                break;
            case ErrCodeArea::Sw:
                os << "Sw";
                break;
            default:
                os << "Unknown";
        }
        os << " Class:";
        switch (err.GetClass())
        {
            case ErrCodeClass::NONE:
                os << "NONE";
                break;
            case ErrCodeClass::Abort:
                os << "Abort";
                break;
            case ErrCodeClass::General:
                os << "General";
                break;
            case ErrCodeClass::NotExists:
                os << "NotExists";
                break;
            case ErrCodeClass::AlreadyExists:
                os << "AlreadyExists";
                break;
            case ErrCodeClass::Access:
                os << "Access";
                break;
            case ErrCodeClass::Path:
                os << "Path";
                break;
            case ErrCodeClass::Locking:
                os << "Locking";
                break;
            case ErrCodeClass::Parameter:
                os << "Parameter";
                break;
            case ErrCodeClass::Space:
                os << "Space";
                break;
            case ErrCodeClass::NotSupported:
                os << "NotSupported";
                break;
            case ErrCodeClass::Read:
                os << "Read";
                break;
            case ErrCodeClass::Write:
                os << "Write";
                break;
            case ErrCodeClass::Unknown:
                os << "Unknown";
                break;
            case ErrCodeClass::Version:
                os << "Version";
                break;
            case ErrCodeClass::Format:
                os << "Format";
                break;
            case ErrCodeClass::Create:
                os << "Create";
                break;
            case ErrCodeClass::Import:
                os << "Import";
                break;
            case ErrCodeClass::Export:
                os << "Export";
                break;
            case ErrCodeClass::So:
                os << "So";
                break;
            case ErrCodeClass::Sbx:
                os << "Sbx";
                break;
            case ErrCodeClass::Runtime:
                os << "Runtime";
                break;
            case ErrCodeClass::Compiler:
                os << "Compiler";
                break;
        }
        os << " Code:" << OUString::number(err.GetRest());
    }
    os << ")";
    return os;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
