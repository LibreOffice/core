/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: errinf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:11:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __RSC

#ifndef _EINF_HXX
#define _EINF_HXX

#include <limits.h>
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

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
