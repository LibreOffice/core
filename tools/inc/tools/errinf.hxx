/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// -------------
// - ErrorInfo -
// -------------

class ErrorInfo
{
private:
    ULONG                   lUserId;

public:
                            TYPEINFO();

                            ErrorInfo( ULONG lArgUserId ) :
                                lUserId( lArgUserId ){}
    virtual                 ~ErrorInfo(){}

    ULONG                   GetErrorCode() const { return lUserId; }

    static ErrorInfo*       GetErrorInfo(ULONG);
};


// --------------------
// - DynamicErrorInfo -
// --------------------

class TOOLS_DLLPUBLIC DynamicErrorInfo : public ErrorInfo
{
    friend class EDcr_Impl;

private:
    EDcr_Impl*              pImpl;

public:
                            TYPEINFO();

                            DynamicErrorInfo(ULONG lUserId, USHORT nMask);
    virtual                 ~DynamicErrorInfo();

    operator                ULONG() const;
    USHORT                  GetDialogMask() const;
};


// ---------------------
// - StandardErrorInfo -
// ---------------------

class StandardErrorInfo : public DynamicErrorInfo
{
private:
    ULONG                   lExtId;

public:
                            TYPEINFO();

                            StandardErrorInfo( ULONG lUserId, ULONG lExtId,
                                              USHORT nFlags = 0);
    ULONG                   GetExtendedErrorCode() const { return lExtId; }

};


// -------------------
// - StringErrorInfo -
// -------------------

class TOOLS_DLLPUBLIC StringErrorInfo : public DynamicErrorInfo
{
private:
    String                  aString;

public:
                            TYPEINFO();

                            StringErrorInfo( ULONG lUserId,
                                            const String& aStringP,
                                            USHORT nFlags = 0);
    const String&           GetErrorString() const { return aString; }
};

//=============================================================================
class TOOLS_DLLPUBLIC TwoStringErrorInfo: public DynamicErrorInfo
{
private:
    String aArg1;
    String aArg2;

public:
    TYPEINFO();

    TwoStringErrorInfo(ULONG nUserID, const String & rTheArg1,
                       const String & rTheArg2, USHORT nFlags = 0):
     DynamicErrorInfo(nUserID, nFlags), aArg1(rTheArg1), aArg2(rTheArg2) {}
    virtual ~TwoStringErrorInfo() {}

    String GetArg1() const { return aArg1; }
    String GetArg2() const { return aArg2; }
};

// -------------------
// - MessageInfo -
// -------------------

class TOOLS_DLLPUBLIC MessageInfo : public DynamicErrorInfo
{
  public:

                            TYPEINFO();
                            MessageInfo(ULONG UserId, USHORT nFlags = 0) :
                                DynamicErrorInfo(UserId, nFlags){}
                            MessageInfo(ULONG UserId, const String &rArg,
                                        USHORT nFlags = 0 ) :
                                DynamicErrorInfo(UserId, nFlags), aArg(rArg) {}
    const String&           GetMessageArg() const { return aArg; }

  private:

    String           aArg;
};


// ----------------
// - ErrorContext -
// ----------------

class TOOLS_DLLPUBLIC ErrorContext
{
    friend class ErrorHandler;

private:
    ErrorContext*           pNext;
    Window*                 pWin;

public:
                            ErrorContext(Window *pWin=0);
    virtual                 ~ErrorContext();

    virtual BOOL            GetString( ULONG nErrId, String& rCtxStr ) = 0;
    Window*                 GetParent() { return pWin; }

    static ErrorContext*    GetContext();
};


// ----------------
// - ErrorHandler -
// ----------------

typedef USHORT WindowDisplayErrorFunc(
    Window *, USHORT nMask, const String &rErr, const String &rAction);

typedef void BasicDisplayErrorFunc(
    const String &rErr, const String &rAction);

class TOOLS_DLLPUBLIC ErrorHandler
{
    friend class ErrHdl_Impl;

private:
    ErrHdl_Impl*        pImpl;

    static USHORT       HandleError_Impl( ULONG lId,
                      USHORT nFlags,
                      BOOL bJustCreateString,
                      String & rError);
protected:
    virtual BOOL        CreateString( const ErrorInfo *,
                      String &, USHORT& nMask ) const = 0;
            BOOL        ForwCreateString( const ErrorInfo*,
                      String&, USHORT& nMask ) const;

public:
                        ErrorHandler();
    virtual             ~ErrorHandler();

    static USHORT       HandleError ( ULONG lId, USHORT nMask = USHRT_MAX );
    static BOOL         GetErrorString( ULONG lId, String& rStr );

    static void         RegisterDisplay( BasicDisplayErrorFunc* );
    static void         RegisterDisplay( WindowDisplayErrorFunc* );
};


// ----------------------
// - SimpleErrorHandler -
// ----------------------

class TOOLS_DLLPUBLIC SimpleErrorHandler : private ErrorHandler
{
protected:
    virtual BOOL        CreateString( const ErrorInfo*, String &,
                                      USHORT &nMask ) const;

public:
                        SimpleErrorHandler();

};

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
