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

#ifndef _EINF_HXX
#define _EINF_HXX

#include <limits.h>
#include <tools/rtti.hxx>
#include <tools/errcode.hxx>
#include <tools/string.hxx>
#include "tools/toolsdllapi.h"

class EDcr_Impl;
class ErrHdl_Impl;
class Window;

class ErrorInfo
{
private:
    sal_uIntPtr             lUserId;

public:
                            TYPEINFO();

                            ErrorInfo( sal_uIntPtr lArgUserId ) :
                                lUserId( lArgUserId ){}
    virtual                 ~ErrorInfo(){}

    sal_uIntPtr             GetErrorCode() const { return lUserId; }

    static ErrorInfo*       GetErrorInfo(sal_uIntPtr);
};

class TOOLS_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class EDcr_Impl;

private:
    EDcr_Impl*              pImpl;

public:
                            TYPEINFO();

                            DynamicErrorInfo(sal_uIntPtr lUserId, sal_uInt16 nMask);
    virtual                 ~DynamicErrorInfo();

    operator                sal_uIntPtr() const;
    sal_uInt16              GetDialogMask() const;
};

class TOOLS_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
private:
    String                  aString;

public:
                            TYPEINFO();

                            StringErrorInfo( sal_uIntPtr lUserId,
                                            const String& aStringP,
                                            sal_uInt16 nFlags = 0);
    const String&           GetErrorString() const { return aString; }
};

class TOOLS_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
private:
    String aArg1;
    String aArg2;

public:
    TYPEINFO();

    TwoStringErrorInfo(sal_uIntPtr nUserID, const String & rTheArg1,
                       const String & rTheArg2, sal_uInt16 nFlags = 0):
     DynamicErrorInfo(nUserID, nFlags), aArg1(rTheArg1), aArg2(rTheArg2) {}
    virtual ~TwoStringErrorInfo() {}

    String GetArg1() const { return aArg1; }
    String GetArg2() const { return aArg2; }
};

class TOOLS_DLLPUBLIC MessageInfo : public DynamicErrorInfo
{
public:
                            TYPEINFO();
                            MessageInfo(sal_uIntPtr UserId, sal_uInt16 nFlags = 0) :
                                DynamicErrorInfo(UserId, nFlags){}
                            MessageInfo(sal_uIntPtr UserId, const String &rArg,
                                        sal_uInt16 nFlags = 0 ) :
                                DynamicErrorInfo(UserId, nFlags), aArg(rArg) {}
    const String&           GetMessageArg() const { return aArg; }

private:
    String           aArg;
};

class TOOLS_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

private:
    ErrorContext*           pNext;
    Window*                 pWin;

public:
                            ErrorContext(Window *pWin=0);
    virtual                 ~ErrorContext();

    virtual sal_Bool        GetString( sal_uIntPtr nErrId, String& rCtxStr ) = 0;
    Window*                 GetParent() { return pWin; }

    static ErrorContext*    GetContext();
};

typedef sal_uInt16 WindowDisplayErrorFunc(
    Window *, sal_uInt16 nMask, const String &rErr, const String &rAction);

typedef void BasicDisplayErrorFunc(
    const String &rErr, const String &rAction);

class TOOLS_DLLPUBLIC ErrorHandler
{
    friend class ErrHdl_Impl;

private:
    ErrHdl_Impl*        pImpl;

    static sal_uInt16       HandleError_Impl( sal_uIntPtr lId,
                      sal_uInt16 nFlags,
                      sal_Bool bJustCreateString,
                      String & rError);
protected:
    virtual sal_Bool        CreateString( const ErrorInfo *,
                      String &, sal_uInt16& nMask ) const = 0;

public:
                        ErrorHandler();
    virtual             ~ErrorHandler();

    static sal_uInt16   HandleError ( sal_uIntPtr lId, sal_uInt16 nMask = USHRT_MAX );
    static sal_Bool     GetErrorString( sal_uIntPtr lId, String& rStr );

    static void         RegisterDisplay( BasicDisplayErrorFunc* );
    static void         RegisterDisplay( WindowDisplayErrorFunc* );
};

class TOOLS_DLLPUBLIC SimpleErrorHandler : private ErrorHandler
{
protected:
    virtual sal_Bool    CreateString( const ErrorInfo*, String &,
                                      sal_uInt16 &nMask ) const;

public:
                        SimpleErrorHandler();
};

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
