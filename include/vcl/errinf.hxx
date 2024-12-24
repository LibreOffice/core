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

#ifndef INCLUDED_VCL_ERRINF_HXX
#define INCLUDED_VCL_ERRINF_HXX

#include <rtl/ustring.hxx>
#include <comphelper/errcode.hxx>
#include <vcl/dllapi.h>

#include <vector>
#include <memory>

namespace weld { class Window; }

class ErrorHandler;
class ErrorContext;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) ErrorStringFactory
{
public:
    static bool CreateString(const ErrCodeMsg&, OUString&);
};

typedef void (* DisplayFnPtr)();

typedef DialogMask WindowDisplayErrorFunc(
    weld::Window*, DialogMask eMask, const OUString &rErr, const OUString &rAction);

typedef void BasicDisplayErrorFunc(
    const OUString &rErr, const OUString &rAction);

class VCL_DLLPUBLIC ErrorRegistry
{
    friend class ErrorHandler;
    friend class ErrorContext;
    friend class ErrorStringFactory;

public:
                                ErrorRegistry();

    static void                 RegisterDisplay(BasicDisplayErrorFunc*);
    static void                 RegisterDisplay(WindowDisplayErrorFunc*);

    static void SetLock(bool bLock);
    static bool GetLock();

    static void                 Reset();

private:
    DisplayFnPtr                pDsp;
    bool                        bIsWindowDsp;

    bool m_bLock;

    std::vector<ErrorHandler*>  errorHandlers;
    std::vector<ErrorContext*>  contexts;
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorHandler
{
    friend class ErrorStringFactory;

public:
                            ErrorHandler();
    virtual                 ~ErrorHandler();

    /** Handles an error.

        If nFlags is not set, the DynamicErrorInfo flags or the
        resource flags will be used. Thus the order is:

        1. nFlags,
        2. Resource Flags
        3. Dynamic Flags
        4. Default ButtonsOk, MessageError

        @param nErrCodeId        error id
        @param pParent           parent window the error dialog will be modal for. nullptr for unrecommended "pick default"
        @param nFlags            error flags.

        @return what sort of dialog to use, with what buttons
    */
    static DialogMask       HandleError(const ErrCodeMsg& nId, weld::Window* pParent = nullptr, DialogMask nMask = DialogMask::MAX);
    static bool             GetErrorString(const ErrCodeMsg& nId, OUString& rStr);

protected:
    virtual bool            CreateString(const ErrCodeMsg&, OUString &) const = 0;

};

struct ImplErrorContext;

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

public:
                            ErrorContext(weld::Window *pWin);
    virtual                 ~ErrorContext();

    virtual bool            GetString(const ErrCodeMsg& nErrId, OUString& rCtxStr) = 0;
    weld::Window*           GetParent();

    static ErrorContext*    GetContext();

private:
    std::unique_ptr<ImplErrorContext> pImpl;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
