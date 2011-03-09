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
    sal_uIntPtr                   lUserId;

public:
                            TYPEINFO();

                            ErrorInfo( sal_uIntPtr lArgUserId ) :
                                lUserId( lArgUserId ){}
    virtual                 ~ErrorInfo(){}

    sal_uIntPtr                   GetErrorCode() const { return lUserId; }

    static ErrorInfo*       GetErrorInfo(sal_uIntPtr);
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

                            DynamicErrorInfo(sal_uIntPtr lUserId, sal_uInt16 nMask);
    virtual                 ~DynamicErrorInfo();

    operator                sal_uIntPtr() const;
    sal_uInt16                  GetDialogMask() const;
};


// ---------------------
// - StandardErrorInfo -
// ---------------------

class StandardErrorInfo : public DynamicErrorInfo
{
private:
    sal_uIntPtr                   lExtId;

public:
                            TYPEINFO();

                            StandardErrorInfo( sal_uIntPtr lUserId, sal_uIntPtr lExtId,
                                              sal_uInt16 nFlags = 0);
    sal_uIntPtr                   GetExtendedErrorCode() const { return lExtId; }

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

                            StringErrorInfo( sal_uIntPtr lUserId,
                                            const String& aStringP,
                                            sal_uInt16 nFlags = 0);
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

    TwoStringErrorInfo(sal_uIntPtr nUserID, const String & rTheArg1,
                       const String & rTheArg2, sal_uInt16 nFlags = 0):
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
                            MessageInfo(sal_uIntPtr UserId, sal_uInt16 nFlags = 0) :
                                DynamicErrorInfo(UserId, nFlags){}
                            MessageInfo(sal_uIntPtr UserId, const String &rArg,
                                        sal_uInt16 nFlags = 0 ) :
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

    virtual sal_Bool            GetString( sal_uIntPtr nErrId, String& rCtxStr ) = 0;
    Window*                 GetParent() { return pWin; }

    static ErrorContext*    GetContext();
};


// ----------------
// - ErrorHandler -
// ----------------

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
            sal_Bool        ForwCreateString( const ErrorInfo*,
                      String&, sal_uInt16& nMask ) const;

public:
                        ErrorHandler();
    virtual             ~ErrorHandler();

    static sal_uInt16       HandleError ( sal_uIntPtr lId, sal_uInt16 nMask = USHRT_MAX );
    static sal_Bool         GetErrorString( sal_uIntPtr lId, String& rStr );

    static void         RegisterDisplay( BasicDisplayErrorFunc* );
    static void         RegisterDisplay( WindowDisplayErrorFunc* );
};


// ----------------------
// - SimpleErrorHandler -
// ----------------------

class TOOLS_DLLPUBLIC SimpleErrorHandler : private ErrorHandler
{
protected:
    virtual sal_Bool        CreateString( const ErrorInfo*, String &,
                                      sal_uInt16 &nMask ) const;

public:
                        SimpleErrorHandler();

};

#endif
#endif
