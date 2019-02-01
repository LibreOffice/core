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
#include <vcl/errcode.hxx>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

#include <vector>
#include <memory>

#include <limits.h>

namespace weld { class Window; }

class ErrorHandler;
class ErrorContext;
class ErrorInfo;
class DynamicErrorInfo;
class ImplDynamicErrorInfo;
enum class DialogMask;

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
    friend class ImplDynamicErrorInfo;

public:
                                ErrorRegistry();

    static void                 RegisterDisplay(BasicDisplayErrorFunc*);
    static void                 RegisterDisplay(WindowDisplayErrorFunc*);

private:
    DisplayFnPtr                pDsp;
    bool                        bIsWindowDsp;
    sal_uInt16                  nNextError;

    std::vector<ErrorHandler*>  errorHandlers;
    std::vector<ErrorContext*>  contexts;

    DynamicErrorInfo*           ppDynErrInfo[ERRCODE_DYNAMIC_COUNT];
};

enum class DialogMask
{
    NONE                    = 0x0000,
    ButtonsOk               = 0x0001,
    ButtonsCancel           = 0x0002,
    ButtonsRetry            = 0x0004,
    ButtonsNo               = 0x0008,
    ButtonsYes              = 0x0010,
    ButtonsYesNo            = 0x0018,

    ButtonDefaultsOk        = 0x0100,
    ButtonDefaultsCancel    = 0x0200,
    ButtonDefaultsYes       = 0x0300,
    ButtonDefaultsNo        = 0x0400,

    MessageError            = 0x1000,
    MessageWarning          = 0x2000,
    MessageInfo             = 0x3000,

    MAX                     = USHRT_MAX,
};
namespace o3tl
{
    template<> struct typed_flags<DialogMask> : is_typed_flags<DialogMask, 0xffff> {};
}

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
    static DialogMask       HandleError(ErrCode nId, weld::Window* pParent = nullptr, DialogMask nMask = DialogMask::MAX);
    static bool             GetErrorString(ErrCode nId, OUString& rStr);

protected:
    virtual bool            CreateString(const ErrorInfo*, OUString &) const = 0;

};

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorInfo
{
public:
                            ErrorInfo(ErrCode nArgUserId) :
                                nUserId(nArgUserId) {}
    virtual                 ~ErrorInfo();

    ErrCode const &         GetErrorCode() const { return nUserId; }

    static std::unique_ptr<ErrorInfo> GetErrorInfo(ErrCode);

private:
    ErrCode const           nUserId;
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class ImplDynamicErrorInfo;

public:
                            DynamicErrorInfo(ErrCode nUserId, DialogMask nMask);
    virtual                 ~DynamicErrorInfo() override;

    operator                ErrCode() const;
    DialogMask              GetDialogMask() const;

private:
    std::unique_ptr<ImplDynamicErrorInfo> pImpl;

};

class SAL_WARN_UNUSED VCL_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
public:
                            StringErrorInfo(ErrCode nUserId,
                                            const OUString& aStringP,
                                            DialogMask nMask = DialogMask::NONE);

    const OUString&         GetErrorString() const { return aString; }

private:
    OUString const          aString;

};

class SAL_WARN_UNUSED VCL_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
public:
    TwoStringErrorInfo(ErrCode nUserID, const OUString & rTheArg1,
                       const OUString & rTheArg2, DialogMask nMask):
        DynamicErrorInfo(nUserID, nMask), aArg1(rTheArg1), aArg2(rTheArg2) {}

    const OUString& GetArg1() const { return aArg1; }
    const OUString& GetArg2() const { return aArg2; }

private:
    OUString const aArg1;
    OUString const aArg2;

};

struct ImplErrorContext;

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

public:
                            ErrorContext(weld::Window *pWin);
    virtual                 ~ErrorContext();

    virtual bool            GetString(ErrCode nErrId, OUString& rCtxStr) = 0;
    weld::Window*           GetParent();

    static ErrorContext*    GetContext();

private:
    std::unique_ptr<ImplErrorContext> pImpl;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
