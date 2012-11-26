/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __RSC

#ifndef _EINF_HXX
#define _EINF_HXX

#include <limits.h>
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
