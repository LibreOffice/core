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

#ifndef INCLUDED_TOOLS_ERRINF_HXX
#define INCLUDED_TOOLS_ERRINF_HXX

#include <limits.h>
#include <rtl/ustring.hxx>
#include <tools/errcode.hxx>
#include <tools/toolsdllapi.h>
#include <memory>

// FIXME: horrible legacy dependency on VCL from tools.
namespace vcl { class Window; }

class DynamicErrorInfo_Impl;
class ErrorHandler_Impl;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC ErrorInfo
{
private:
    sal_uIntPtr             lUserId;

public:

                            ErrorInfo( sal_uIntPtr lArgUserId ) :
                                lUserId( lArgUserId ){}
    virtual                 ~ErrorInfo();

    sal_uIntPtr             GetErrorCode() const { return lUserId; }

    static ErrorInfo*       GetErrorInfo(sal_uIntPtr);
};

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class DynamicErrorInfo_Impl;

private:
    std::unique_ptr<DynamicErrorInfo_Impl>   pImpl;

public:

                            DynamicErrorInfo(sal_uIntPtr lUserId, sal_uInt16 nMask);
    virtual                 ~DynamicErrorInfo() override;

    sal_uIntPtr             GetErrorCode() const;
    sal_uInt16              GetDialogMask() const;
};

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
private:
    OUString                aString;

public:

                            StringErrorInfo( sal_uIntPtr lUserId,
                                            const OUString& aStringP,
                                            sal_uInt16 nMask = 0);
    const OUString&         GetErrorString() const { return aString; }
};

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
private:
    OUString aArg1;
    OUString aArg2;

public:

    TwoStringErrorInfo(sal_uIntPtr nUserID, const OUString & rTheArg1,
                       const OUString & rTheArg2, sal_uInt16 nMask):
        DynamicErrorInfo(nUserID, nMask), aArg1(rTheArg1), aArg2(rTheArg2) {}

    const OUString& GetArg1() const { return aArg1; }
    const OUString& GetArg2() const { return aArg2; }
};

struct ErrorContextImpl;
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

private:
    std::unique_ptr<ErrorContextImpl> pImpl;

public:
                            ErrorContext(vcl::Window *pWin);
    virtual                 ~ErrorContext();

    virtual bool            GetString( sal_uIntPtr nErrId, OUString& rCtxStr ) = 0;
    vcl::Window*            GetParent();

    static ErrorContext*    GetContext();
};

typedef sal_uInt16 WindowDisplayErrorFunc(
    vcl::Window *, sal_uInt16 nMask, const OUString &rErr, const OUString &rAction);

typedef void BasicDisplayErrorFunc(
    const OUString &rErr, const OUString &rAction);

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC ErrorHandler
{
    friend class ErrorHandler_Impl;

private:
    static sal_uInt16   HandleError_Impl( sal_uIntPtr lId,
                              sal_uInt16 nFlags,
                              bool bJustCreateString,
                              OUString & rError);
protected:
    virtual bool        CreateString( const ErrorInfo *,
                              OUString &, sal_uInt16& nMask ) const = 0;

public:
                        ErrorHandler();
    virtual             ~ErrorHandler();

    static sal_uInt16   HandleError ( sal_uIntPtr lId, sal_uInt16 nMask = USHRT_MAX );
    static bool         GetErrorString( sal_uIntPtr lId, OUString& rStr );

    static void         RegisterDisplay( BasicDisplayErrorFunc* );
    static void         RegisterDisplay( WindowDisplayErrorFunc* );
};

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
