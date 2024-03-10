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

#include <sal/config.h>
#include <xmlsec-wrapper.h>

#include <xmlsec/errorcallback.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <xmlsec/xmlsec.h>
#include <xmlsec/errors.h>

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#include <comphelper/windowserrorstring.hxx>
#endif

extern "C" {

static void errorCallback(const char* file,
                   int line,
                   const char* func,
                   const char* errorObject,
                   const char* errorSubject,
                   int reason,
                   const char* msg)
{
    const char* pErrorObject = errorObject ? errorObject : "";
    const char* pErrorSubject = errorSubject ? errorSubject : "";
    const char* pMsg = msg ? msg : "";
    OUString systemErrorString;

#ifdef _WIN32
    systemErrorString = " " + WindowsErrorString(GetLastError());
#endif

    SAL_WARN("xmlsecurity.xmlsec", file << ":" << line << ": " << func << "() '" << pErrorObject << "' '" << pErrorSubject << "' " << reason << " '" << pMsg << "'" << systemErrorString);
}

}

void setErrorRecorder()
{
    xmlSecErrorsSetCallback(errorCallback);
}

void clearErrorRecorder()
{
    xmlSecErrorsSetCallback(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
