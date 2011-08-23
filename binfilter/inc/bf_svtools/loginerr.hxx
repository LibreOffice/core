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

#ifndef _LOGINERR_HXX
#define _LOGINERR_HXX

#include <tools/string.hxx>
#include <tools/errcode.hxx>

#include <bf_svtools/hint.hxx>

namespace binfilter
{

//=========================================================================

#define LOGINERROR_FLAG_SET_SAVE_PASSWORD	1
#define LOGINERROR_FLAG_MODIFY_ACCOUNT		2
#define LOGINERROR_FLAG_MODIFY_USER_NAME	4
#define LOGINERROR_FLAG_PERSISTENT_PASSWORD	8

class LoginErrorInfo
{
private:
    String _aTitle;
    String _aServer;
    String _aAccount;
    String _aUserName;
    String _aPassword;
    String _aPath;
    String _aErrorText;
    BYTE   _nFlags;
    USHORT _nRet;

public:
                    LoginErrorInfo()
                    : _nFlags( LOGINERROR_FLAG_MODIFY_USER_NAME ),
                      _nRet( ERRCODE_BUTTON_CANCEL )
                    {
                    }

    const String& 	GetTitle() const 	 	{ return _aTitle; }
    const String& 	GetServer() const  		{ return _aServer; }
    const String& 	GetAccount() const  	{ return _aAccount; }
    const String& 	GetUserName() const  	{ return _aUserName; }
    const String& 	GetPassword() const  	{ return _aPassword; }
    const String& 	GetPath() const 	 	{ return _aPath; }
    const String& 	GetErrorText() const	{ return _aErrorText; }
    BOOL		 	GetIsPersistentPassword() const
                    { return ( _nFlags & LOGINERROR_FLAG_PERSISTENT_PASSWORD ); }
    BOOL  			GetIsSavePassword() const
                    { return ( _nFlags & LOGINERROR_FLAG_SET_SAVE_PASSWORD ); }
    BYTE			GetFlags() const		{ return _nFlags; }
    USHORT 			GetResult() const    	{ return _nRet; }

    void 	        SetTitle( const String& aTitle )
                    { _aTitle = aTitle; }
    void 	        SetServer( const String& aServer )
                    { _aServer = aServer; }
    void 	        SetAccount( const String& aAccount )
                    { _aAccount = aAccount; }
    void 	        SetUserName( const String& aUserName )
                    { _aUserName = aUserName; }
    void 	        SetPassword( const String& aPassword )
                    { _aPassword = aPassword; }
    void 	        SetPath( const String& aPath )
                    { _aPath = aPath; }
    void 	        SetErrorText( const String& aErrorText )
                    { _aErrorText = aErrorText; }
    void            SetFlags( BYTE nFlags )
                    { _nFlags = nFlags; }
    inline void		SetSavePassword( BOOL bSet );
    inline void		SetPersistentPassword( BOOL bSet );
    inline void		SetModifyAccount( BOOL bSet );
    inline void		SetModifyUserName( BOOL bSet );
    void            SetResult( USHORT nRet )
                    { _nRet = nRet; }
};

inline void	LoginErrorInfo::SetSavePassword( BOOL bSet )
{
    if ( bSet )
        _nFlags |= LOGINERROR_FLAG_SET_SAVE_PASSWORD;
    else
        _nFlags &= ~LOGINERROR_FLAG_SET_SAVE_PASSWORD;
}

inline void	LoginErrorInfo::SetPersistentPassword( BOOL bSet )
{
    if ( bSet )
        _nFlags |= LOGINERROR_FLAG_PERSISTENT_PASSWORD;
    else
        _nFlags &= ~LOGINERROR_FLAG_PERSISTENT_PASSWORD;
}

inline void LoginErrorInfo::SetModifyAccount( BOOL bSet )
{
    if ( bSet )
        _nFlags |= LOGINERROR_FLAG_MODIFY_ACCOUNT;
    else
        _nFlags &= ~LOGINERROR_FLAG_MODIFY_ACCOUNT;
}

inline void LoginErrorInfo::SetModifyUserName( BOOL bSet )
{
    if ( bSet )
        _nFlags |= LOGINERROR_FLAG_MODIFY_USER_NAME;
    else
        _nFlags &= ~LOGINERROR_FLAG_MODIFY_USER_NAME;
}

//=========================================================================

class CntLoginErrorHint : public SfxHint
{
private:
    LoginErrorInfo* 		_pInfo;

                            CntLoginErrorHint( const CntLoginErrorHint& ); // n.i.
    CntLoginErrorHint&		operator=( const CntLoginErrorHint& ); // n.i.

public:
                            TYPEINFO();

                            CntLoginErrorHint( LoginErrorInfo* pInfo )
                            :	_pInfo( pInfo )
                            {
                            }

    LoginErrorInfo&			GetErrorInfo() const { return *_pInfo; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
