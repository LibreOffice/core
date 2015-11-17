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

class EDcr_Impl;
class ErrHdl_Impl;

class TOOLS_DLLPUBLIC ErrorInfo
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

class TOOLS_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class EDcr_Impl;

private:
    std::unique_ptr<EDcr_Impl>   pImpl;

public:

                            DynamicErrorInfo(sal_uIntPtr lUserId, sal_uInt16 nMask);
    virtual                 ~DynamicErrorInfo();

    operator                sal_uIntPtr() const;
    sal_uInt16              GetDialogMask() const;
};

class TOOLS_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
private:
    OUString                aString;

public:

                            StringErrorInfo( sal_uIntPtr lUserId,
                                            const OUString& aStringP,
                                            sal_uInt16 nFlags = 0);
    const OUString&         GetErrorString() const { return aString; }
};

class TOOLS_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
private:
    OUString aArg1;
    OUString aArg2;

public:

    TwoStringErrorInfo(sal_uIntPtr nUserID, const OUString & rTheArg1,
                       const OUString & rTheArg2, sal_uInt16 nFlags = 0):
        DynamicErrorInfo(nUserID, nFlags), aArg1(rTheArg1), aArg2(rTheArg2) {}

    virtual ~TwoStringErrorInfo() {}

    const OUString& GetArg1() const { return aArg1; }
    const OUString& GetArg2() const { return aArg2; }
};

class TOOLS_DLLPUBLIC MessageInfo : public DynamicErrorInfo
{
public:

    MessageInfo(sal_uIntPtr UserId, sal_uInt16 nFlags = 0) :
        DynamicErrorInfo(UserId, nFlags) {}
    MessageInfo(sal_uIntPtr UserId, const OUString &rArg, sal_uInt16 nFlags = 0 ) :
        DynamicErrorInfo(UserId, nFlags), aArg(rArg) {}

    const OUString&         GetMessageArg() const { return aArg; }

private:
    OUString           aArg;
};

struct ErrorContextImpl;
class TOOLS_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

private:
    std::unique_ptr<ErrorContextImpl> pImpl;

public:
                            ErrorContext(vcl::Window *pWin=nullptr);
    virtual                 ~ErrorContext();

    virtual bool            GetString( sal_uIntPtr nErrId, OUString& rCtxStr ) = 0;
    vcl::Window*            GetParent();

    static ErrorContext*    GetContext();
};

typedef sal_uInt16 WindowDisplayErrorFunc(
    vcl::Window *, sal_uInt16 nMask, const OUString &rErr, const OUString &rAction);

typedef void BasicDisplayErrorFunc(
    const OUString &rErr, const OUString &rAction);

class TOOLS_DLLPUBLIC ErrorHandler
{
    friend class ErrHdl_Impl;

private:
    std::unique_ptr<ErrHdl_Impl>  pImpl;

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
