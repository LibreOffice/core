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

#ifndef __RSC

#ifndef INCLUDED_VCL_ERRINF_HXX
#define INCLUDED_VCL_ERRINF_HXX

#include <rtl/ustring.hxx>
#include <tools/errcode.hxx>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

#include <vector>
#include <memory>
#include <limits.h>

namespace vcl { class Window; }

class ErrorHandler;
class ErrorContext;
class ErrorStringFactory;
class ErrorInfo;
class DynamicErrorInfo;
class DynamicErrorInfo_Impl;
enum class DialogMask;

namespace {
  typedef void (* DisplayFnPtr)();
}

typedef DialogMask WindowDisplayErrorFunc(
    vcl::Window *, DialogMask eMask, const OUString &rErr, const OUString &rAction);

typedef void BasicDisplayErrorFunc(
    const OUString &rErr, const OUString &rAction);

class VCL_DLLPUBLIC ErrorRegistry
{
    friend class ErrorHandler;
    friend class ErrorContext;
    friend class ErrorStringFactory;
    friend class DynamicErrorInfo_Impl;

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
    ButtonsOkCancel         = 0x0003,
    ButtonsNo               = 0x0008,
    ButtonsYes              = 0x0010,
    ButtonsYesNo            = 0x0018,
    ButtonsYesNoCancel      = 0x001a,

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

typedef DialogMask WindowDisplayErrorFunc(
    vcl::Window *, DialogMask nMask, const OUString &rErr, const OUString &rAction);

typedef void BasicDisplayErrorFunc(
    const OUString &rErr, const OUString &rAction);

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorHandler
{
    friend class ErrorStringFactory;

public:
                        ErrorHandler();
    virtual             ~ErrorHandler();

    static DialogMask   HandleError ( sal_uInt32 lId, DialogMask nMask = DialogMask::MAX );
    static bool         GetErrorString( sal_uInt32 lId, OUString& rStr );

protected:
    virtual bool        CreateString(const ErrorInfo*, OUString &) const = 0;

};

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorInfo
{
private:
    sal_uInt32             lUserId;

public:

                            ErrorInfo( sal_uInt32 lArgUserId ) :
                                lUserId( lArgUserId ){}
    virtual                 ~ErrorInfo();

    sal_uInt32             GetErrorCode() const { return lUserId; }

    static ErrorInfo*       GetErrorInfo(sal_uInt32);
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class DynamicErrorInfo_Impl;

private:
    std::unique_ptr<DynamicErrorInfo_Impl>   pImpl;

public:

                            DynamicErrorInfo(sal_uInt32 lUserId, DialogMask nMask);
    virtual                 ~DynamicErrorInfo() override;

    operator                sal_uInt32() const;
    DialogMask       GetDialogMask() const;
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
private:
    OUString                aString;

public:

                            StringErrorInfo( sal_uInt32 lUserId,
                                            const OUString& aStringP,
                                            DialogMask nMask = DialogMask::NONE);
    const OUString&         GetErrorString() const { return aString; }
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
private:
    OUString aArg1;
    OUString aArg2;

public:

    TwoStringErrorInfo(sal_uInt32 nUserID, const OUString & rTheArg1,
                       const OUString & rTheArg2, DialogMask nMask):
        DynamicErrorInfo(nUserID, nMask), aArg1(rTheArg1), aArg2(rTheArg2) {}

    const OUString& GetArg1() const { return aArg1; }
    const OUString& GetArg2() const { return aArg2; }
};

struct ErrorContextImpl;

class SAL_WARN_UNUSED VCL_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

private:
    std::unique_ptr<ErrorContextImpl> pImpl;

public:
                            ErrorContext(vcl::Window *pWin);
    virtual                 ~ErrorContext();

    virtual bool            GetString( sal_uInt32 nErrId, OUString& rCtxStr ) = 0;
    vcl::Window*            GetParent();

    static ErrorContext*    GetContext();
};

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
