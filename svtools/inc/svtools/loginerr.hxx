/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loginerr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:27:18 $
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

#ifndef _LOGINERR_HXX
#define _LOGINERR_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif

#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif

//=========================================================================

#define LOGINERROR_FLAG_SET_SAVE_PASSWORD   1
#define LOGINERROR_FLAG_MODIFY_ACCOUNT      2
#define LOGINERROR_FLAG_MODIFY_USER_NAME    4
#define LOGINERROR_FLAG_PERSISTENT_PASSWORD 8

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

    const String&   GetTitle() const        { return _aTitle; }
    const String&   GetServer() const       { return _aServer; }
    const String&   GetAccount() const      { return _aAccount; }
    const String&   GetUserName() const     { return _aUserName; }
    const String&   GetPassword() const     { return _aPassword; }
    const String&   GetPath() const         { return _aPath; }
    const String&   GetErrorText() const    { return _aErrorText; }
    BOOL            GetIsPersistentPassword() const
                    { return ( _nFlags & LOGINERROR_FLAG_PERSISTENT_PASSWORD ); }
    BOOL            GetIsSavePassword() const
                    { return ( _nFlags & LOGINERROR_FLAG_SET_SAVE_PASSWORD ); }
    BYTE            GetFlags() const        { return _nFlags; }
    USHORT          GetResult() const       { return _nRet; }

    void            SetTitle( const String& aTitle )
                    { _aTitle = aTitle; }
    void            SetServer( const String& aServer )
                    { _aServer = aServer; }
    void            SetAccount( const String& aAccount )
                    { _aAccount = aAccount; }
    void            SetUserName( const String& aUserName )
                    { _aUserName = aUserName; }
    void            SetPassword( const String& aPassword )
                    { _aPassword = aPassword; }
    void            SetPath( const String& aPath )
                    { _aPath = aPath; }
    void            SetErrorText( const String& aErrorText )
                    { _aErrorText = aErrorText; }
    void            SetFlags( BYTE nFlags )
                    { _nFlags = nFlags; }
    inline void     SetSavePassword( BOOL bSet );
    inline void     SetPersistentPassword( BOOL bSet );
    inline void     SetModifyAccount( BOOL bSet );
    inline void     SetModifyUserName( BOOL bSet );
    void            SetResult( USHORT nRet )
                    { _nRet = nRet; }
};

inline void LoginErrorInfo::SetSavePassword( BOOL bSet )
{
    if ( bSet )
        _nFlags |= LOGINERROR_FLAG_SET_SAVE_PASSWORD;
    else
        _nFlags &= ~LOGINERROR_FLAG_SET_SAVE_PASSWORD;
}

inline void LoginErrorInfo::SetPersistentPassword( BOOL bSet )
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
    LoginErrorInfo*         _pInfo;

                            CntLoginErrorHint( const CntLoginErrorHint& ); // n.i.
    CntLoginErrorHint&      operator=( const CntLoginErrorHint& ); // n.i.

public:
                            TYPEINFO();

                            CntLoginErrorHint( LoginErrorInfo* pInfo )
                            :   _pInfo( pInfo )
                            {
                            }

    LoginErrorInfo&         GetErrorInfo() const { return *_pInfo; }
};

#endif
